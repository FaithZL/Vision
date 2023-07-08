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

struct BxDFSet {
public:
    [[nodiscard]] virtual SampledSpectrum albedo() const noexcept = 0;
    [[nodiscard]] virtual ScatterEval evaluate_local(Float3 wo, Float3 wi, Uint flag) const noexcept = 0;
    [[nodiscard]] virtual BSDFSample sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept = 0;
    [[nodiscard]] virtual SampledDirection sample_wi(Float3 wo, Uint flag, Sampler *sampler) const noexcept {
        OC_ASSERT(false);
        return {};
    }
    virtual void regularize() noexcept {}
    virtual void mollify() noexcept {}
    [[nodiscard]] virtual optional<Bool> is_dispersive() const noexcept { return {}; }
    virtual ~BxDFSet() = default;
};

struct BSDF final {
public:
    UVN<Float3> shading_frame;
    Float3 ng;
    UP<BxDFSet> bxdf_set{};

protected:
    [[nodiscard]] ScatterEval evaluate_local(Float3 wo, Float3 wi, Uint flag) const noexcept {
        return bxdf_set->evaluate_local(wo, wi, flag);
    }
    [[nodiscard]] BSDFSample sample_local(Float3 wo, Uint flag, Sampler *sampler) const noexcept {
        return bxdf_set->sample_local(wo, flag, sampler);
    }

public:
    BSDF() = delete;
    explicit BSDF(const Interaction &it)
        : shading_frame(it.s_uvn), ng(it.g_uvn.normal()) {}

    explicit BSDF(const Interaction &it, UP<BxDFSet> &&bxdf_set)
        : shading_frame(it.s_uvn), ng(it.g_uvn.normal()), bxdf_set(ocarina::move(bxdf_set)) {}

    void regularize() noexcept { bxdf_set->regularize(); }
    void mollify() noexcept { bxdf_set->mollify(); }
    [[nodiscard]] SampledSpectrum albedo() const noexcept {
        return bxdf_set->albedo();
    }
    [[nodiscard]] optional<Bool> is_dispersive() const noexcept {
        return bxdf_set != nullptr ? bxdf_set->is_dispersive() : optional<Bool>{};
    }
    [[nodiscard]] static Uint combine_flag(Float3 wo, Float3 wi, Uint flag) noexcept;
    [[nodiscard]] ScatterEval evaluate(Float3 world_wo, Float3 world_wi) const noexcept;
    [[nodiscard]] BSDFSample sample(Float3 world_wo, Sampler *sampler) const noexcept;
};

class Material : public Node, public Serializable<float> {
public:
    using Desc = MaterialDesc;

protected:
    Slot _bump{};

protected:
    static constexpr uint stride = sizeof(Slot);
    struct SlotCursor {
        // The offset of the first slot in the object
        uint offset{0u};
        uint num{0u};
    };
    SlotCursor _slot_cursor;
    UP<BSDF> _bsdf{};

    const Slot &get_slot(uint index) const noexcept {
        const Slot *head = reinterpret_cast<const Slot *>(reinterpret_cast<const char *>(this) + _slot_cursor.offset);
        return head[index];
    }

    Slot &get_slot(uint index) noexcept {
        const Slot *head = reinterpret_cast<const Slot *>(reinterpret_cast<const char *>(this) + _slot_cursor.offset);
        return (const_cast<Slot *>(head))[index];
    }

public:
    explicit Material(const MaterialDesc &desc);
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
        if (_bump.node()) {
            ret = OC_FORWARD(func)(ret, _bump);
        }
        for (int i = 0; i < _slot_cursor.num; ++i) {
            const Slot &slot = get_slot(i);
            ret = OC_FORWARD(func)(ret, slot);
        }
        return ret;
    }

    template<typename F>
    void for_each_slot(F &&func) const noexcept {
        if (_bump.node()) {
            OC_FORWARD(func)(_bump);
        }
        for (int i = 0; i < _slot_cursor.num; ++i) {
            const Slot &slot = get_slot(i);
            OC_FORWARD(func)(slot);
        }
    }

    template<typename F>
    void for_each_slot(F &&func) noexcept {
        if (_bump.node()) {
            OC_FORWARD(func)(_bump);
        }
        for (int i = 0; i < _slot_cursor.num; ++i) {
            Slot &slot = get_slot(i);
            OC_FORWARD(func)(slot);
        }
    }

    [[nodiscard]] uint element_num() const noexcept override;
    [[nodiscard]] bool has_device_value() const noexcept override;
    void reset_device_value() const noexcept override;
    void encode(RegistrableManaged<float> &data) const noexcept override;
    void decode(const DataAccessor<float> *da) const noexcept override;

    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override;
    [[nodiscard]] uint64_t _compute_hash() const noexcept override;

public:
    [[nodiscard]] virtual BSDF compute_BSDF(const Interaction &it,
                                            const SampledWavelengths &swl) const noexcept = 0;
};
}// namespace vision