//
// Created by Zero on 25/10/2022.
//

#pragma once

#include "rhi/common.h"
#include "base/shape.h"
#include "base/color/spectrum.h"
#include "base/scattering/interaction.h"

namespace vision {
using namespace ocarina;
class Scene;
class Pipeline;
class ShapeInstance;
class Geometry {
private:
    RegistrableManaged<Vertex> vertices_;
    RegistrableManaged<Triangle> triangles_;
    RegistrableManaged<float4x4> transforms_;
    RegistrableManaged<InstanceHandle> instances_;
    RegistrableManaged<MeshHandle> mesh_handles_;
    ocarina::Accel accel_;

public:
    Pipeline *rp{};

public:
    explicit Geometry(Pipeline *rp = nullptr);

    OC_MAKE_MEMBER_GETTER(accel, &)

    /**
     * update shape attribute
     * There is no need to update the acceleration structure
     * @param shapes
     */
    void update_instances(const vector<SP<ShapeInstance>> &instances);
    void reset_device_buffer();
    void build_accel();
    void upload() const;
    void clear() noexcept;

    // for dsl
    [[nodiscard]] HitVar trace_closest(const RayVar &ray) const noexcept;
    [[nodiscard]] Bool trace_any(const RayVar &ray) const noexcept;
    [[nodiscard]] Bool occluded(const Interaction &it, const Float3 &pos, RayState *rs = nullptr) const noexcept;
    template<typename ...Args>
    [[nodiscard]] auto visibility(Args &&...args) const noexcept {
        Bool occ = occluded(OC_FORWARD(args)...);
        return cast<int>(!occ);
    }
    [[nodiscard]] SampledSpectrum Tr(Scene &scene, const SampledWavelengths &swl, const RayState &ray_state) const noexcept;
    [[nodiscard]] LightEvalContext compute_light_eval_context(const Uint &inst_id,
                                                              const Uint &prim_id,
                                                              const Float2 &bary) const noexcept;
    [[nodiscard]] array<Var<Vertex>, 3> get_vertices(const Var<Triangle> &tri, const Uint &offset) const noexcept;
    [[nodiscard]] Interaction compute_surface_interaction(const HitVar &hit, bool is_complete) const noexcept;
    [[nodiscard]] Interaction compute_surface_interaction(const HitVar &hit, const Float3 &view_pos) const noexcept {
        auto ret = compute_surface_interaction(hit, true);
        ret.update_wo(view_pos);
        return ret;
    }
    [[nodiscard]] Interaction compute_surface_interaction(const HitVar &hit, RayVar &ray, bool is_complete = true) const noexcept {
        auto ret = compute_surface_interaction(hit, is_complete);
        ret.wo = normalize(-ray->direction());
        ray.dir_max.w = length(ret.pos - ray->origin()) / length(ray->direction());
        return ret;
    }
};
}// namespace vision