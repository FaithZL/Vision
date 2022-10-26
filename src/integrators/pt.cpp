//
// Created by Zero on 12/09/2022.
//

#include "base/integrator.h"
#include "core/render_pipeline.h"

namespace vision {
using namespace ocarina;
class PathTracingIntegrator : public Integrator {
public:
    explicit PathTracingIntegrator(const IntegratorDesc &desc) : Integrator(desc) {}

    void compile_shader(RenderPipeline *rp) noexcept override {
        Camera *camera = rp->scene().camera();
        Sampler *sampler = rp->scene().sampler();
        DeviceData &data = rp->device_data();
        Accel &accel = rp->device_data().accel;
        _kernel = [&]() -> void {
            Uint2 pixel = dispatch_idx().xy();
            SensorSample ss = sampler->sensor_sample(dispatch_idx().xy());
            auto [ray, weight] = camera->generate_ray(ss);
            Var hit = accel.trace_closest(ray);
            Var p = ray->direction();
            $if(hit->is_miss()) {
                camera->film()->add_sample(pixel, make_float4(0,0,0,1), 0);
                $return();
            };
            Var inst = data.instances.read(hit.inst_id);
            Var mesh = data.mesh_handles.read(inst.mesh_id);
            Var tri = data.triangles.read(hit.prim_id + mesh.triangle_offset);
            Var v0 = data.vertices.read(tri.i + mesh.vertex_offset);
            Var v1 = data.vertices.read(tri.j + mesh.vertex_offset);
            Var v2 = data.vertices.read(tri.k + mesh.vertex_offset);
            Var pos = hit->triangle_lerp(v0.position, v1.position, v2.position);
            Var normal = triangle_lerp(hit.bary, v0.normal, v1.normal, v2.normal);
            normal = normalize(transform_normal(inst.o2w, normal));
            Var tex_coord = triangle_lerp(hit.bary, v0.tex_coord, v1.tex_coord, v2.tex_coord);
            normal = (normal + 1.f) / 2.f;
            camera->film()->add_sample(pixel,make_float4(normal, Var(1.f)) , 0);
        };
        _shader = rp->device().compile(_kernel);
        int i = 0;

    }

    void render(RenderPipeline *rp) const noexcept override {
        Stream &stream = rp->stream();
        stream << _shader().dispatch(rp->resolution());
        stream << synchronize();
        stream << commit();
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PathTracingIntegrator)