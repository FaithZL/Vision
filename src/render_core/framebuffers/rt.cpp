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
    using gbuffer_signature = void(uint, Buffer<PixelGeometry>, Buffer<float4>, Buffer<float4>);
    Shader<gbuffer_signature> _compute_geom;

    using grad_signature = void(uint, Buffer<PixelGeometry>);
    Shader<grad_signature> _compute_grad;

public:
    explicit RayTracingFrameBuffer(const FrameBufferDesc &desc)
        : FrameBuffer(desc) {}
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }

    void compile_compute_geom() noexcept {
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

            const SampledWavelengths &swl = render_env.sampled_wavelengths();

            SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
            RayState rs = camera->generate_ray(ss);
            OCHit hit = pipeline()->trace_closest(rs.ray);

            OCPixelGeometry geom;
            Float3 albedo;
            Float3 emission;
            $if(hit->is_hit()) {
                Interaction it = pipeline()->compute_surface_interaction(hit, rs.ray, true);
                geom.normal.set(it.ng);
                Float4x4 w2c = inverse(camera->device_c2w());
                Float3 c_pos = transform_point(w2c, it.pos);
                geom.linear_depth = c_pos.z;
                $if(it.has_material()) {
                    scene().materials().dispatch(it.material_id(), [&](const Material *material) {
                        MaterialEvaluator bsdf = material->create_evaluator(it, swl);
                        albedo = spectrum().linear_srgb(bsdf.albedo(), swl);
                    });
                };
                $if(it.has_emission()) {
                    LightSampleContext p_ref;
                    p_ref.pos = rs.origin();
                    p_ref.ng = rs.direction();
                    LightEval eval = light_sampler->evaluate_hit_wi(p_ref, it, swl);
                    emission = spectrum().linear_srgb(eval.L, swl);
                };
            };
            gbuffer.write(dispatch_id(), geom);
            albedo_buffer.write(dispatch_id(), make_float4(albedo, 1.f));
            emission_buffer.write(dispatch_id(), make_float4(emission, 1.f));
        };
        _compute_geom = device().compile(kernel, "rt_geom");
    }

    void compile_compute_grad() noexcept {
        Kernel kernel = [&](Uint frame_index, BufferVar<PixelGeometry> gbuffer) {
            Int2 radius = make_int2(1);
            Uint x_sample_num = 0u;
            Uint y_sample_num = 0u;
            Float3 normal_dx = make_float3(0.f);
            Float3 normal_dy = make_float3(0.f);

            Float depth_dx = 0.f;
            Float depth_dy = 0.f;

            Uint2 center = dispatch_idx().xy();
            OCPixelGeometry center_data = gbuffer.read(dispatch_id());
            for_each_neighbor(radius, [&](const Int2 &pixel) {
                Uint index = dispatch_id(pixel);
                OCPixelGeometry neighbor_data = gbuffer.read(index);
                $if(center.x > pixel.x) {
                    x_sample_num += 1;
                    normal_dx += center_data.normal.as_vec() - neighbor_data.normal.as_vec();
                    depth_dx += center_data.linear_depth - neighbor_data.linear_depth;
                }
                $elif(pixel.x > center.x) {
                    x_sample_num += 1;
                    normal_dx += neighbor_data.normal.as_vec() - center_data.normal.as_vec();
                    depth_dx += neighbor_data.linear_depth - center_data.linear_depth;
                };

                $if(center.y > pixel.y) {
                    y_sample_num += 1;
                    normal_dy += center_data.normal.as_vec() - neighbor_data.normal.as_vec();
                    depth_dy += center_data.linear_depth - neighbor_data.linear_depth;
                }
                $elif(pixel.y > center.y) {
                    y_sample_num += 1;
                    normal_dy += neighbor_data.normal.as_vec() - center_data.normal.as_vec();
                    depth_dy += neighbor_data.linear_depth - center_data.linear_depth;
                };
            });
            normal_dx /= cast<float>(x_sample_num);
            normal_dy /= cast<float>(y_sample_num);
            Float3 normal_fwidth = abs(normal_dx) + abs(normal_dy);
            center_data.normal_fwidth = length(normal_fwidth);

            depth_dx /= x_sample_num;
            depth_dy /= y_sample_num;
            center_data.depth_gradient = abs(depth_dx) + abs(depth_dy);
            gbuffer.write(dispatch_id(), center_data);
        };
        _compute_grad = device().compile(kernel, "rt_gradient");
    }

    void compile() noexcept override {
        compile_compute_geom();
        compile_compute_grad();
    }

    [[nodiscard]] CommandList compute_GBuffer(uint frame_index, Buffer<PixelGeometry> &gbuffer,
                                              Buffer<float4> &albedo, Buffer<float4> &emission) const noexcept {
        CommandList ret;
        ret << _compute_geom(frame_index, gbuffer, albedo, emission).dispatch(resolution());
        ret << _compute_grad(frame_index, gbuffer).dispatch(resolution());
        return ret;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::RayTracingFrameBuffer)