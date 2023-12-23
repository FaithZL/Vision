//
// Created by Zero on 2023/6/14.
//

#include "base/integrator.h"
#include "base/mgr/pipeline.h"
#include "math/warp.h"
#include "base/color/spectrum.h"

namespace vision {

class AmbientOcclusionIntegrator : public Integrator {
private:
    Serial<float> _distance{1.f};
    Serial<uint> _cos_sample{true};
    Serial<uint> _sample_num{64u};

public:
    explicit AmbientOcclusionIntegrator(const IntegratorDesc &desc)
        : Integrator(desc),
          _distance(desc["distance"].as_float(1.f)),
          _cos_sample(desc["cos_sample"].as_bool(true)),
          _sample_num(desc["sample_num"].as_uint(32u)) {}

    OC_SERIALIZABLE_FUNC(Integrator, _distance, _cos_sample, _sample_num)
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    [[nodiscard]] Float3 Li(vision::RayState rs, Float scatter_pdf, Interaction *first_it) const noexcept override {
        Float3 L = make_float3(0.f);
        Pipeline *rp = pipeline();
        Sampler *sampler = scene().sampler();
        Geometry &geom = rp->geometry();
        $while(true) {
            Var hit = geom.trace_closest(rs.ray);
            $if(hit->is_miss()) {
                $break;
            };
            Interaction it = geom.compute_surface_interaction(hit, rs.ray);
            $if(!it.has_material()) {
                rs = it.spawn_ray_state(rs.direction());
                $continue;
            };
            Float3 wi;
            Float pdf;
            $for(i, *_sample_num) {
                if (_cos_sample.hv()) {
                    wi = square_to_cosine_hemisphere(sampler->next_2d());
                    pdf = cosine_hemisphere_PDF(geometry::abs_cos_theta(wi));
                } else {
                    wi = square_to_hemisphere(sampler->next_2d());
                    pdf = uniform_hemisphere_PDF();
                }
                it.shading.z = face_forward(it.shading.normal(), -rs.direction());
                wi = it.shading.to_world(wi);
                Bool occ = geom.trace_any(it.spawn_ray(wi, *_distance));
                $if(!occ) {
                    L += dot(wi, it.shading.normal()) / (pdf * *_sample_num);
                };
            };
            $break;
        };
        return L;
    }

    void compile() noexcept override {
        Pipeline *rp = pipeline();
        Camera *camera = scene().camera().get();
        Sampler *sampler = scene().sampler();
        Geometry &geom = rp->geometry();

        ocarina::Kernel<signature> kernel = [&](Uint frame_index) -> void {
            Uint2 pixel = dispatch_idx().xy();
            sampler->start(pixel, frame_index, 0);
            SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
            camera->load_data();
            RayState rs = camera->generate_ray(ss);

            Float3 L = make_float3(0.f);

            Pipeline *rp = pipeline();
            Geometry &geom = rp->geometry();

            $while(true) {
                Var hit = geom.trace_closest(rs.ray);
                $if(hit->is_miss()) {
                    $break;
                };
                Interaction it = geom.compute_surface_interaction(hit, rs.ray);
                $if(!it.has_material()) {
                    rs = it.spawn_ray_state(rs.direction());
                    $continue;
                };
                Float3 wi;
                Float pdf;
                $for(i, *_sample_num) {
                    if (_cos_sample.hv()) {
                        wi = square_to_cosine_hemisphere(sampler->next_2d());
                        pdf = cosine_hemisphere_PDF(geometry::abs_cos_theta(wi));
                    } else {
                        wi = square_to_hemisphere(sampler->next_2d());
                        pdf = uniform_hemisphere_PDF();
                    }
                    it.shading.z = face_forward(it.shading.normal(), -rs.direction());
                    wi = it.shading.to_world(wi);
                    Bool occ = geom.trace_any(it.spawn_ray(wi, *_distance));
                    $if(!occ) {
                        L += dot(wi, it.shading.normal()) / (pdf * *_sample_num);
                    };
                };
                $break;
            };

            camera->radiance_film()->add_sample(pixel, L, frame_index);
        };
        _shader = rp->device().compile(kernel);
    }

    void render() const noexcept override {
        const Pipeline *rp = pipeline();
        Stream &stream = rp->stream();
        stream << _shader(_frame_index++).dispatch(rp->resolution());
        stream << synchronize();
        stream << commit();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::AmbientOcclusionIntegrator)