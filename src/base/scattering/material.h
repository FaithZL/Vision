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

struct BxDFSet : public ocarina::Hashable {
public:
    [[nodiscard]] virtual SampledSpectrum albedo(const Float3 &wo) const noexcept = 0;
    [[nodiscard]] virtual ScatterEval evaluate_local(Float3 wo, Float3 wi, MaterialEvalMode mode, Uint flag) const noexcept = 0;
    [[nodiscard]] virtual BSDFSample sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept = 0;
    [[nodiscard]] virtual SampledDirection sample_wi(Float3 wo, Uint flag, Sampler *sampler) const noexcept {
        OC_ASSERT(false);
        return {};
    }
    virtual BxDFSet &operator=(const BxDFSet &other) noexcept = default;
    virtual void regularize() noexcept {}
    virtual void mollify() noexcept {}
    [[nodiscard]] virtual optional<Bool> is_dispersive() const noexcept { return {}; }
    virtual ~BxDFSet() = default;
};

#define VS_MAKE_BxDFSet_ASSIGNMENT(ClassName)                            \
    ClassName &operator=(const BxDFSet &other) noexcept override {       \
        OC_ASSERT(dynamic_cast<const ClassName *>(&other));              \
        *this = dynamic_cast<ClassName &>(const_cast<BxDFSet &>(other)); \
        return *this;                                                    \
    }

class MaterialEvaluator : public PolyEvaluator<BxDFSet> {
public:
    using Super = PolyEvaluator<BxDFSet>;

protected:
    PartialDerivative<Float3> shading_frame;
    Float3 ng;
    const SampledWavelengths *swl{};

protected:
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, MaterialEvalMode mode, Uint flag) const noexcept;
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept;

public:
    explicit MaterialEvaluator(const Interaction &it, const SampledWavelengths &swl)
        : shading_frame(it.shading), ng(it.ng), swl(&swl) {}

    void regularize() noexcept;
    void mollify() noexcept;
    [[nodiscard]] SampledSpectrum albedo(const Float3 &world_wo) const noexcept;
    [[nodiscard]] optional<Bool> is_dispersive() const noexcept;
    [[nodiscard]] ScatterEval evaluate(Float3 world_wo, Float3 world_wi, MaterialEvalMode mode = All,
                                       const Uint &flag = BxDFFlag::All) const noexcept;
    [[nodiscard]] BSDFSample sample(Float3 world_wo, Sampler *sampler,
                                    const Uint &flag = BxDFFlag::All) const noexcept;
};

class Material : public Node, public Serializable<float> {
public:
    using Desc = MaterialDesc;

    using Evaluator = MaterialEvaluator;

protected:
    uint index_{InvalidUI32};
    VS_MAKE_SLOT_(bump);
    VS_MAKE_SLOT_(bump_scale);

protected:
    static constexpr uint stride = sizeof(Slot);
    struct SlotCursor {
        // The offset of the first slot in the object
        uint offset{0u};
        uint num{0u};
    };
    SlotCursor slot_cursor_;
    const Slot &get_slot(uint index) const noexcept {
        const Slot *head = reinterpret_cast<const Slot *>(reinterpret_cast<const char *>(this) + slot_cursor_.offset);
        return head[index];
    }

    Slot &get_slot(uint index) noexcept {
        const Slot *head = reinterpret_cast<const Slot *>(reinterpret_cast<const char *>(this) + slot_cursor_.offset);
        return (const_cast<Slot *>(head))[index];
    }

public:
    explicit Material(const MaterialDesc &desc);
    OC_MAKE_MEMBER_GETTER_SETTER_(index, )
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;
    void init_slot_cursor(const Slot *ptr, uint num) noexcept {
        uint offset = reinterpret_cast<const char *>(ptr) - reinterpret_cast<char *>(this);
        slot_cursor_.offset = offset;
        slot_cursor_.num = num;
    }
    void init_slot_cursor(const Slot *head, const Slot *back) noexcept {
        uint offset = reinterpret_cast<const char *>(head) - reinterpret_cast<char *>(this);
        slot_cursor_.offset = offset;
        slot_cursor_.num = (back - head) + 1;
    }

    template<typename T, typename F>
    auto reduce_slots(T &&initial, F &&func) const noexcept {
        T ret = OC_FORWARD(initial);
        if (bump_) {
            ret = OC_FORWARD(func)(ret, bump_);
        }
        if (bump_scale_) {
            ret = OC_FORWARD(func)(ret, bump_scale_);
        }
        for (int i = 0; i < slot_cursor_.num; ++i) {
            const Slot &slot = get_slot(i);
            if (slot) {
                ret = OC_FORWARD(func)(ret, slot);
            }
        }
        return ret;
    }

    template<typename T, typename F>
    auto reduce_slots(T &&initial, F &&func) noexcept {
        T ret = OC_FORWARD(initial);
        if (bump_) {
            ret = OC_FORWARD(func)(ret, bump_);
        }
        if (bump_scale_) {
            ret = OC_FORWARD(func)(ret, bump_scale_);
        }
        for (int i = 0; i < slot_cursor_.num; ++i) {
            Slot &slot = get_slot(i);
            if (slot) {
                ret = OC_FORWARD(func)(ret, slot);
            }
        }
        return ret;
    }

    template<typename F>
    void for_each_slot(F &&func) const noexcept {
        if (bump_) {
            func(bump_);
        }
        if (bump_scale_) {
            func(bump_scale_);
        }
        for (int i = 0; i < slot_cursor_.num; ++i) {
            const Slot &slot = get_slot(i);
            if (slot) {
                func(slot);
            }
        }
    }

    template<typename F>
    void for_each_slot(F &&func) noexcept {
        if (bump_) {
            func(bump_);
        }
        if (bump_scale_) {
            func(bump_scale_);
        }
        for (int i = 0; i < slot_cursor_.num; ++i) {
            Slot &slot = get_slot(i);
            if (slot) {
                func(slot);
            }
        }
    }

    [[nodiscard]] uint element_num() const noexcept override;
    [[nodiscard]] bool has_device_value() const noexcept override;
    void reset_device_value() const noexcept override;
    void encode(RegistrableManaged<float> &data) const noexcept override;
    void decode(const DataAccessor<float> *da) const noexcept override;
    void reset_status() noexcept override;
    bool has_changed() noexcept override;

protected:
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override;
    [[nodiscard]] uint64_t _compute_hash() const noexcept override;
    virtual void _apply_bump(Interaction *it, const SampledWavelengths &swl) const noexcept;

public:
    [[nodiscard]] static Uint combine_flag(Float3 wo, Float3 wi, Uint flag) noexcept;
    virtual void _build_evaluator(Evaluator &evaluator, const Interaction &it, const SampledWavelengths &swl) const noexcept = 0;
    virtual UP<BxDFSet> create_lobe_set(Interaction it, const SampledWavelengths &swl) const noexcept = 0;
    [[nodiscard]] Evaluator create_evaluator(const Interaction &it, const SampledWavelengths &swl) const noexcept;
    void build_evaluator(Evaluator &evaluator, Interaction it, const SampledWavelengths &swl) const noexcept;
};
}// namespace vision