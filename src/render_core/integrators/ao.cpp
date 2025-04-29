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
    EncodedData<float> distance_{1.f};
    EncodedData<uint> cos_sample_{true};
    EncodedData<uint> sample_num_{64u};

public:
    explicit AmbientOcclusionIntegrator(const IntegratorDesc &desc)
        : Integrator(desc),
          distance_(desc["distance"].as_float(1.f)),
          cos_sample_(desc["cos_sample"].as_bool(true)),
          sample_num_(desc["sample_num"].as_uint(32u)) {}

    OC_ENCODABLE_FUNC(Integrator, distance_, cos_sample_, sample_num_)
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] Float3 Li(vision::RayState rs, Float scatter_pdf, SampledSpectrum throughput,
                            const HitContext &hc, const RenderEnv &render_env) const noexcept override {
        Float3 L = make_float3(0.f);
        Pipeline *rp = pipeline();
        TSampler &sampler = scene().sampler();
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
            $for(i, *sample_num_) {
                if (cos_sample_.hv()) {
                    wi = square_to_cosine_hemisphere(sampler->next_2d());
                    pdf = cosine_hemisphere_PDF(geometry::abs_cos_theta(wi));
                } else {
                    wi = square_to_hemisphere(sampler->next_2d());
                    pdf = uniform_hemisphere_PDF();
                }
                it.shading.z = face_forward(it.shading.normal(), -rs.direction());
                wi = it.shading.to_world(wi);
                Bool occ = geom.trace_occlusion(it.spawn_ray(wi, *distance_));
                $if(!occ) {
                    L += dot(wi, it.shading.normal()) / (pdf * *sample_num_);
                };
            };
            $break;
        };
        return L;
    }

    void compile() noexcept override {
        Pipeline *rp = pipeline();
        Sensor *camera = scene().camera().get();
        TSampler &sampler = scene().sampler();
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
                $for(i, *sample_num_) {
                    if (cos_sample_.hv()) {
                        wi = square_to_cosine_hemisphere(sampler->next_2d());
                        pdf = cosine_hemisphere_PDF(geometry::abs_cos_theta(wi));
                    } else {
                        wi = square_to_hemisphere(sampler->next_2d());
                        pdf = uniform_hemisphere_PDF();
                    }
                    it.shading.z = face_forward(it.shading.normal(), -rs.direction());
                    wi = it.shading.to_world(wi);
                    Bool occ = geom.trace_occlusion(it.spawn_ray(wi, *distance_));
                    $if(!occ) {
                        L += dot(wi, it.shading.normal()) / (pdf * *sample_num_);
                    };
                };
                $break;
            };

            camera->film()->add_sample(pixel, L, frame_index);
        };
        shader_ = rp->device().compile(kernel);
    }

    void render() const noexcept override {
        const Pipeline *rp = pipeline();
        Stream &stream = rp->stream();
        stream << shader_(frame_index_++).dispatch(rp->resolution());
        stream << synchronize();
        stream << commit();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::AmbientOcclusionIntegrator)