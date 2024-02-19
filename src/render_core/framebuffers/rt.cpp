//
// Created by Zero on 2024/2/18.
//

#include "base/frame_buffer.h"
#include "rhi/resources/shader.h"
#include "base/mgr/scene.h"
#include "base/mgr/pipeline.h"

namespace vision {

class RayTracingFrameBuffer : public FrameBuffer {
private:
    using signature = void(uint, Buffer<PixelGeometry>, Buffer<float4>, Buffer<float4>);
    Shader<signature> _shader;

public:
    explicit RayTracingFrameBuffer(const FrameBufferDesc &desc)
        : FrameBuffer(desc) {}
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }

    void compile() noexcept override {
        Camera *camera = scene().camera().get();
        Sampler *sampler = scene().sampler();
        LightSampler *light_sampler = scene().light_sampler();
        Kernel kernel = [&](Uint frame_index, BufferVar<PixelGeometry> gbuffer,
                            BufferVar<float4> albedo_buffer, BufferVar<float4> emission_buffer) {
            RenderEnv render_env;
            render_env.initial(sampler, frame_index, spectrum());
            Uint2 pixel = dispatch_idx().xy();
            sampler->start(pixel, frame_index, 0);
            camera->load_data();
            SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
            RayState rs = camera->generate_ray(ss);
            OCHit hit = pipeline()->trace_closest(rs.ray);

            OCPixelGeometry geom;
            Float3 albedo;
            Float3 emission;
            $if(hit->is_hit()) {
                Interaction it = pipeline()->compute_surface_interaction(hit, rs.ray, true);
                geom.normal.set(it.ng);
                $if(it.has_material()) {
                    scene().materials().dispatch(it.material_id(), [&](const Material *material) {
                        MaterialEvaluator bsdf = material->create_evaluator(it, render_env.sampled_wavelengths());
                        albedo = spectrum().linear_srgb(bsdf.albedo(), render_env.sampled_wavelengths());
                    });
                };
            };
            gbuffer.write(dispatch_id(), geom);
            albedo_buffer.write(dispatch_id(), make_float4(albedo, 1.f));
        };
        _shader = device().compile(kernel, "rt_GBuffer");
    }

    [[nodiscard]] CommandList compute_GBuffer(uint frame_index, Buffer<PixelGeometry> &gbuffer,
                                              Buffer<float4> &albedo, Buffer<float4> &emission) const noexcept {
        CommandList ret;
        ret << _shader(frame_index, gbuffer, albedo, emission).dispatch(resolution());
        return ret;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RayTracingFrameBuffer)