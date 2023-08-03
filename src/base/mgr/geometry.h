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
struct Geometry {
private:
    RegistrableManaged<Vertex> _vertices;
    RegistrableManaged<Triangle> _triangles;
    RegistrableManaged<InstanceHandle> _instances;
    RegistrableManaged<Mesh::Handle> _mesh_handles;

public:
    ocarina::Accel accel;
    Pipeline *rp{};

private:
    [[nodiscard]] Interaction compute_surface_interaction(const OCHit &hit, bool is_complete) const noexcept;

public:
    explicit Geometry(Pipeline *rp = nullptr);

    void accept(const vector<Vertex> &vert,
                const vector<Triangle> &tri,
                InstanceHandle handle);
    /**
     * update shape attribute
     * There is no need to update the acceleration structure
     * @param shapes
     */
    void update_instances(const vector<ShapeInstance> &instances);
    void reset_device_buffer();
    void build_accel();
    void build_TLAS(vector<RHIMesh> &meshes);
    void upload() const;
    void clear() noexcept;

    // for dsl
    [[nodiscard]] OCHit trace_closest(const OCRay &ray) const noexcept;
    [[nodiscard]] Bool trace_any(const OCRay &ray) const noexcept;
    [[nodiscard]] Bool occluded(const Interaction &it, const Float3 &pos, RayState *rs = nullptr) const noexcept;
    [[nodiscard]] SampledSpectrum Tr(Scene &scene, const SampledWavelengths &swl, const RayState &ray_state) const noexcept;
    [[nodiscard]] LightEvalContext compute_light_eval_context(const Uint &inst_id,
                                                              const Uint &prim_id,
                                                              const Float2 &bary) const noexcept;
    [[nodiscard]] array<Var<Vertex>, 3> get_vertices(const Var<Triangle> &tri, const Uint &offset) const noexcept;
    [[nodiscard]] Interaction compute_surface_interaction(const OCHit &hit, OCRay &ray) const noexcept {
        auto ret = compute_surface_interaction(hit, true);
        ret.wo = normalize(-ray->direction());
        ray.dir_max.w = length(ret.pos - ray->origin()) / length(ray->direction());
        return ret;
    }
};
}// namespace vision