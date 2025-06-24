//
// Created by Zero on 09/09/2022.
//

#pragma once

#include <utility>
#include "base/node.h"
#include "core/stl.h"
#include "base/scattering/lobe.h"
#include "base/shader_graph/shader_graph.h"

namespace vision {

class MaterialLut {
private:
    std::map<string, RegistrableTexture> lut_map_;
    OC_MAKE_INSTANCE_CONSTRUCTOR(MaterialLut, s_material_lut)

public:
    OC_MAKE_INSTANCE_FUNC_DECL(MaterialLut)
    void load_lut(const string &name, uint2 res, PixelStorage storage, const void *data) noexcept {
        load_lut(name, make_uint3(res, 1u), storage, data);
    }
    void load_lut(const string &name, uint3 res, PixelStorage storage, const void *data) noexcept;
    void unload_lut(const string &name) noexcept;
    template<typename... Args>
    [[nodiscard]] float_array sample(const string &name, Args &&...args) const noexcept {
        return get_lut(name).sample(OC_FORWARD(args)...);
    }
    [[nodiscard]] const RegistrableTexture &get_lut(const string &name) const noexcept;
    [[nodiscard]] const EncodedData<uint> &get_index(const string &name) const noexcept;
};

class MaterialEvaluator : public PolyEvaluator<Lobe> {
public:
    using Super = PolyEvaluator<Lobe>;

protected:
    PartialDerivative<Float3> shading_frame_;
    Float3 ng_;
    const SampledWavelengths *swl_{};

protected:
    [[nodiscard]] ScatterEval evaluate_local(const Float3 &wo, const Float3 &wi,
                                             MaterialEvalMode mode,
                                             const Uint &flag,
                                             TransportMode tm) const noexcept;
    [[nodiscard]] BSDFSample sample_local(const Float3 &wo, const Uint &flag,
                                          TSampler &sampler,
                                          TransportMode tm) const noexcept;
    [[nodiscard]] BSDFSample sample_delta_local(const Float3 &wo,
                                                TSampler &sampler) const noexcept;

public:
    explicit MaterialEvaluator(const Interaction &it, const SampledWavelengths &swl)
        : shading_frame_(it.shading), ng_(it.ng), swl_(&swl) {}
    OC_MAKE_MEMBER_GETTER_SETTER(shading_frame, &)
    void regularize() noexcept;
    void mollify() noexcept;
    void update_frame(ShadingFrame shading_frame) noexcept;
    [[nodiscard]] SampledSpectrum albedo(const Float3 &world_wo) const noexcept;
    [[nodiscard]] Bool splittable() const noexcept;
    [[nodiscard]] optional<Bool> is_dispersive() const noexcept;
    [[nodiscard]] ScatterEval evaluate(const Float3 &world_wo, const Float3 &world_wi,
                                       MaterialEvalMode mode = All,
                                       const Uint &flag = BxDFFlag::All,
                                       TransportMode tm = Radiance) const noexcept;
    [[nodiscard]] BSDFSample sample_delta(const Float3 &world_wo, TSampler &sampler) const noexcept;
    [[nodiscard]] BSDFSample sample(const Float3 &world_wo, TSampler &sampler,
                                    const Uint &flag = BxDFFlag::All,
                                    TransportMode tm = Radiance) const noexcept;
    [[nodiscard]] Uint flag() const noexcept;
};

class ShapeInstance;
class ShapeGroup;

struct PrecomputedLobeTable {
    string name;
    const Type *type{nullptr};
    vector<float> data;
    double elapsed_time{};
    uint3 res{};
    [[nodiscard]] string to_string() const noexcept;
};

#define VS_MAKE_MATERIAL_EVALUATOR(Lobe)                                                  \
    void _build_evaluator(Material::Evaluator &evaluator, const Interaction &it,          \
                          const SampledWavelengths &swl) const noexcept override {        \
        auto lobe = ocarina::dynamic_unique_pointer_cast<Lobe>(create_lobe_set(it, swl)); \
        evaluator.set_shading_frame(it.shading);                                          \
        evaluator.link(std::move(lobe));                                                  \
    }

class Material : public Node, public Encodable, public ShaderGraph, public ShaderNodeSlotSet {
public:
    using Evaluator = MaterialEvaluator;
    virtual void _build_evaluator(Evaluator &evaluator, const Interaction &it,
                                  const SampledWavelengths &swl) const noexcept = 0;
    [[nodiscard]] virtual UP<Lobe> create_lobe_set(const Interaction &it,
                                                   const SampledWavelengths &swl) const noexcept = 0;

public:
    using Desc = MaterialDesc;
    static constexpr auto precompute_sample_num = 2 << 20;

protected:
    uint index_{InvalidUI32};
    VS_MAKE_SLOT(normal);
    /// use for integral albedo
    static uint exp_of_two_;
    vector<weak_ptr<ShapeInstance>> shape_instances;

protected:
    static constexpr uint stride = sizeof(ShaderNodeSlot);

    static TSampler &get_sampler() noexcept;

    template<typename TLobe, size_t N = 1>
    [[nodiscard]] PrecomputedLobeTable precompute_lobe(uint3 res) const noexcept {
        Device &device = Global::instance().device();
        Stream stream = device.create_stream();
        TSampler &sampler = get_sampler();

        using data_type = basic_t<float, N>;

        Buffer<data_type> buffer = device.create_buffer<data_type>(res.x * res.y * res.z);

        Kernel kernel = [&](Uint sample_num) {
            sampler->load_data();
            sampler->start(dispatch_idx().xy(), 0, 0);
            SampledWavelengths swl = spectrum()->sample_wavelength(sampler);
            Float3 ratio = make_float3(dispatch_idx()) / make_float3(dispatch_dim() - 1);
            UP<TLobe> lobe = TLobe::create_for_precompute(swl);
            SampledSpectrum result = lobe->precompute_with_radio(ratio, sampler, sample_num);
            if constexpr (N == 1) {
                buffer.write(dispatch_id(), result[0]);
            } else {
                buffer.write(dispatch_id(), result.values().as_vec<N>());
            }
        };

        PrecomputedLobeTable ret;
        ret.name = TLobe::name;
        ret.type = Type::of<data_type>();
        ret.data.resize(buffer.size() * ret.type->dimension());
        ret.res = res;
        Clock clk;
        clk.start();
        OC_INFO_FORMAT("start precompute albedo of {}", ret.name);
        auto shader = device.compile(kernel);
        stream << shader(precompute_sample_num).dispatch(res)
               << buffer.download(ret.data.data())
               << Env::printer().retrieve()
               << synchronize() << commit();
        clk.end();
        ret.elapsed_time = clk.elapse_s();
        OC_INFO_FORMAT("precompute albedo of {} took {:.3f} s", ret.name, ret.elapsed_time);

        return ret;
    }

public:
    Material() = default;
    explicit Material(const MaterialDesc &desc);
    void add_material_reference(SP<ShapeInstance> shape_instance) noexcept;
    OC_MAKE_MEMBER_GETTER_SETTER(index, )
    [[nodiscard]] virtual bool is_dispersive() const noexcept { return false; }
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;

    void initialize_(const vision::NodeDesc &node_desc) noexcept override;
    virtual void initialize_slots(const Desc &desc) noexcept;
    ///region general for each
    template<bool check = true, typename T, typename F>
    auto reduce_slots(T &&initial, F &&func) const noexcept {
        T ret = OC_FORWARD(initial);
        if constexpr (check) {
            if (normal_) {
                ret = func(ret, normal_);
            }
        } else {
            ret = func(ret, normal_);
        }
        ret = ShaderNodeSlotSet::reduce_slots<check>(ret, OC_FORWARD(func));
        return ret;
    }

    template<bool check = true, typename T, typename F>
    auto reduce_slots(T &&initial, F &&func) noexcept {
        T ret = OC_FORWARD(initial);
        if constexpr (check) {
            if (normal_) {
                ret = func(ret, normal_);
            }
        } else {
            ret = func(ret, normal_);
        }
        ret = ShaderNodeSlotSet::reduce_slots<check>(ret, OC_FORWARD(func));
        return ret;
    }

    template<bool check = true, typename F>
    void for_each_slot(F &&func) const noexcept {
        if constexpr (check) {
            if (normal_) {
                func(normal_);
            }
        } else {
            func(normal_);
        }
        ShaderNodeSlotSet::for_each_slot<check>(OC_FORWARD(func));
    }

    template<bool check = true, typename F>
    void for_each_slot(F &&func) noexcept {
        if constexpr (check) {
            if (normal_) {
                func(normal_);
            }
        } else {
            func(normal_);
        }
        ShaderNodeSlotSet::for_each_slot<check>(OC_FORWARD(func));
    }
    ///endregion

    void restore(vision::RuntimeObject *old_obj) noexcept override;

    ///#region encodable
    [[nodiscard]] uint compacted_size() const noexcept override;
    [[nodiscard]] bool has_device_value() const noexcept override;
    void after_decode() const noexcept override;
    void invalidate() const noexcept override;
    void encode(RegistrableManaged<buffer_ty> &data) const noexcept override;
    void decode(const DataAccessor *da) const noexcept override;
    void decode(const DynamicArray<ocarina::buffer_ty> &array) const noexcept override;
    [[nodiscard]] uint alignment() const noexcept override;
    [[nodiscard]] uint cal_offset(ocarina::uint prev_size) const noexcept override;
    ///#endregion

    ///#region GUI
    void reset_status() noexcept override;
    bool has_changed() noexcept override;
    ///#endregion

protected:
    [[nodiscard]] uint64_t compute_topology_hash() const noexcept override;
    [[nodiscard]] uint64_t compute_hash() const noexcept override;
    virtual ShadingFrame compute_shading_frame(const Interaction &it,
                                               const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] SampledSpectrum integral_albedo(const Float3 &wo, const Lobe *lobe_set) const noexcept;

public:
    [[nodiscard]] static Uint combine_flag(const Float3 &wo, const Float3 &wi,
                                           Uint flag) noexcept;
    [[nodiscard]] Evaluator create_evaluator(const Interaction &it,
                                             const SampledWavelengths &swl) const noexcept;
    void build_evaluator(Evaluator &evaluator, const Interaction &it,
                         const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] virtual vector<PrecomputedLobeTable> precompute() const noexcept;
    [[nodiscard]] virtual bool enable_delta() const noexcept { return true; }
};
}// namespace vision