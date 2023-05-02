//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "base/node.h"
#include "interaction.h"
#include "core/stl.h"
#include "base/scattering/bxdf.h"
#include "base/shader_graph/shader_node.h"

namespace vision {

struct BSDF {
public:
    UVN<Float3> shading_frame;
    Float3 ng;
    const SampledWavelengths &swl;

protected:
    [[nodiscard]] virtual ScatterEval evaluate_local(Float3 wo, Float3 wi, Uint flag) const noexcept {
        ScatterEval ret{swl.dimension()};
        ret.f = {swl.dimension(), 0.f};
        ret.pdf = 1.f;
        return ret;
    }

    [[nodiscard]] virtual BSDFSample sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept {
        BSDFSample ret{swl.dimension()};
        return ret;
    }

public:
    BSDF() = delete;
    explicit BSDF(const Interaction &it, const SampledWavelengths &swl)
        : shading_frame(it.s_uvn), ng(it.g_uvn.normal()), swl(swl) {}

    [[nodiscard]] virtual SampledSpectrum albedo() const noexcept {
        // todo
        return {swl.dimension(), 0.f};
    }
    [[nodiscard]] virtual optional<Bool> is_dispersive() const noexcept {
        return {};
    }
    [[nodiscard]] static Uint combine_flag(Float3 wo, Float3 wi, Uint flag) noexcept;
    [[nodiscard]] ScatterEval evaluate(Float3 world_wo, Float3 world_wi) const noexcept;
    [[nodiscard]] BSDFSample sample(Float3 world_wo, Sampler *sampler) const noexcept;
};

class DielectricBSDF : public BSDF {
private:
    SP<const Fresnel> _fresnel;
    MicrofacetReflection _refl;
    MicrofacetTransmission _trans;
    Bool _dispersive{};

public:
    DielectricBSDF(const Interaction &it,
                   const SP<Fresnel> &fresnel,
                   MicrofacetReflection refl,
                   MicrofacetTransmission trans,
                   const Bool &dispersive)
        : BSDF(it, refl.swl()), _fresnel(fresnel),
          _refl(ocarina::move(refl)), _trans(ocarina::move(trans)),
          _dispersive(dispersive) {}
    [[nodiscard]] SampledSpectrum albedo() const noexcept override { return _refl.albedo(); }
    [[nodiscard]] optional<Bool> is_dispersive() const noexcept override {
        return _dispersive;
    }
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, Uint flag) const noexcept override;
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept override;
};

class Material : public Node, public Serializable<float> {
public:
    using Desc = MaterialDesc;

protected:
    static constexpr uint stride = sizeof(Slot);
    struct SlotCursor {
        // The offset of the first slot in the object
        uint offset{0u};
        uint num{0u};
    };
    SlotCursor _slot_cursor;

    const Slot &get_slot(uint index) const noexcept {
        const Slot *head = reinterpret_cast<const Slot *>(reinterpret_cast<const char *>(this) + _slot_cursor.offset);
        return head[index];
    }

    Slot &get_slot(uint index) noexcept {
        const Slot *head = reinterpret_cast<const Slot *>(reinterpret_cast<const char *>(this) + _slot_cursor.offset);
        return (const_cast<Slot *>(head))[index];
    }

public:
    explicit Material(const MaterialDesc &desc) : Node(desc) {}
    void init_slot_cursor(const Slot *ptr, uint num) noexcept {
        uint offset = reinterpret_cast<const char *>(ptr) - reinterpret_cast<char *>(this);
        _slot_cursor.offset = offset;
        _slot_cursor.num = num;
    }

    void init_slot_cursor(const Slot *head, const Slot *back) noexcept {
        uint offset = reinterpret_cast<const char *>(head) - reinterpret_cast<char *>(this);
        _slot_cursor.offset = offset;
        _slot_cursor.num = (back - head) + 1;
    }

    template<typename T, typename F>
    auto reduce_slots(T &&initial, F &&func) const noexcept {
        T ret = OC_FORWARD(initial);
        for (int i = 0; i < _slot_cursor.num; ++i) {
            const Slot &slot = get_slot(i);
            ret = func(ret, slot);
        }
        return ret;
    }

    template<typename F>
    void for_each_slot(F &&func) const noexcept {
        for (int i = 0; i < _slot_cursor.num; ++i) {
            const Slot &slot = get_slot(i);
            func(slot);
        }
    }

    template<typename F>
    void for_each_slot(F &&func) noexcept {
        for (int i = 0; i < _slot_cursor.num; ++i) {
            Slot &slot = get_slot(i);
            func(slot);
        }
    }

    [[nodiscard]] uint element_num() const noexcept override;
    [[nodiscard]] bool has_device_value() const noexcept override;
    void reset_device_value() const noexcept override;
    void encode(ManagedWrapper<float> &data) const noexcept override;
    void decode(const DataAccessor<float> *da) const noexcept override;

    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override;
    [[nodiscard]] uint64_t _compute_hash() const noexcept override;

protected:
    struct Guard {
        const Material *material{};
        Guard(const Material *mat, const DataAccessor<float> *da)
            : material(mat) {
            material->decode(da);
        }
        ~Guard() {
            material->reset_device_value();
        }
    };

    [[nodiscard]] virtual UP<BSDF> _compute_BSDF(const Interaction &it, const SampledWavelengths &swl) const noexcept {
        OC_ASSERT(false);
        return make_unique<BSDF>(it, swl);
    }

public:
    [[nodiscard]] UP<BSDF> compute_BSDF(const Interaction &it,
                                        const SampledWavelengths &swl) const noexcept;
};
}// namespace vision