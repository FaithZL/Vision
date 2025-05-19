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
    using gbuffer_signature = void(uint, Buffer<PixelGeometry>, Buffer<float2>, Buffer<float4>, Buffer<float4>);
    Shader<gbuffer_signature> compute_geom_;

    using grad_signature = void(uint, Buffer<PixelGeometry>);
    Shader<grad_signature> compute_grad_;

    Shader<void(Buffer<TriangleHit>, uint)> compute_hit_;

public:
    RayTracingFrameBuffer() = default;
    explicit RayTracingFrameBuffer(const FrameBufferDesc &desc)
        : FrameBuffer(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
    VS_HOTFIX_MAKE_RESTORE(FrameBuffer, compute_geom_, compute_grad_, compute_hit_)

    void compile_compute_geom() noexcept {
        TSensor &camera = scene().sensor();
        TSampler &sampler = scene().sampler();
        TLightSampler &light_sampler = scene().light_sampler();
        Kernel kernel = [&](Uint frame_index, BufferVar<PixelGeometry> gbuffer, BufferVar<float2> motion_vectors,
                            BufferVar<float4> albedo_buffer, BufferVar<float4> emission_buffer) {
            RenderEnv render_env;
            render_env.initial(sampler, frame_index, spectrum());
            Uint2 pixel = dispatch_idx().xy();
            sampler->start(pixel, frame_index, 0);
            camera->load_data();

            const SampledWavelengths &swl = render_env.sampled_wavelengths();

            SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
            RayState rs = camera->generate_ray(ss);
            TriangleHitVar hit = pipeline()->trace_closest(rs.ray);

            Float2 motion_vec = make_float2(0.f);

            PixelGeometryVar geom;
            geom.p_film = ss.p_film;
            Float3 albedo;
            Float3 emission;
            $if(hit->is_hit()) {
                Interaction it = pipeline()->compute_surface_interaction(hit, rs.ray, true);
                geom->set_normal(it.shading.normal());
                geom.linear_depth = camera->linear_depth(it.pos);
                $if(it.has_material()) {
                    scene().materials().dispatch(it.material_id(), [&](const Material *material) {
                        MaterialEvaluator bsdf = material->create_evaluator(it, swl);
                        albedo = spectrum()->linear_srgb(bsdf.albedo(it.wo), swl);
                    });
                };
                $if(it.has_emission()) {
                    LightSampleContext p_ref;
                    p_ref.pos = rs.origin();
                    p_ref.ng = rs.direction();
                    LightEval eval = light_sampler->evaluate_hit_wi(p_ref, it, swl);
                    emission = spectrum()->linear_srgb(eval.L, swl);
                };
                motion_vec = compute_motion_vec(camera, ss.p_film, it.pos, true);
            };
            gbuffer.write(dispatch_id(), geom);
            motion_vectors.write(dispatch_id(), motion_vec);
            albedo_buffer.write(dispatch_id(), make_float4(albedo, 1.f));
            emission_buffer.write(dispatch_id(), make_float4(emission, 1.f));
        };
        compute_geom_ = device().compile(kernel, "rt_geom");
    }

    void compute_gradient(PixelGeometryVar &center_data,
                          const BufferVar<PixelGeometry> &gbuffer) const noexcept {
        Uint x_sample_num = 0u;
        Uint y_sample_num = 0u;
        Float3 normal_dx = make_float3(0.f);
        Float3 normal_dy = make_float3(0.f);

        Float depth_dx = 0.f;
        Float depth_dy = 0.f;

        Uint2 center = dispatch_idx().xy();
        foreach_neighbor(dispatch_idx().xy(), [&](const Int2 &pixel) {
            Uint index = dispatch_id(pixel);
            PixelGeometryVar neighbor_data = gbuffer.read(index);
            $if(center.x > pixel.x) {
                x_sample_num += 1;
                normal_dx += center_data.normal_fwidth.xyz() - neighbor_data.normal_fwidth.xyz();
                depth_dx += center_data.linear_depth - neighbor_data.linear_depth;
            }
            $elif(pixel.x > center.x) {
                x_sample_num += 1;
                normal_dx += neighbor_data.normal_fwidth.xyz() - center_data.normal_fwidth.xyz();
                depth_dx += neighbor_data.linear_depth - center_data.linear_depth;
            };

            $if(center.y > pixel.y) {
                y_sample_num += 1;
                normal_dy += center_data.normal_fwidth.xyz() - neighbor_data.normal_fwidth.xyz();
                depth_dy += center_data.linear_depth - neighbor_data.linear_depth;
            }
            $elif(pixel.y > center.y) {
                y_sample_num += 1;
                normal_dy += neighbor_data.normal_fwidth.xyz() - center_data.normal_fwidth.xyz();
                depth_dy += neighbor_data.linear_depth - center_data.linear_depth;
            };
        });
        normal_dx /= cast<float>(x_sample_num);
        normal_dy /= cast<float>(y_sample_num);
        Float3 normal_fwidth = abs(normal_dx) + abs(normal_dy);
        center_data->set_normal_fwidth(length(normal_fwidth));

        depth_dx /= x_sample_num;
        depth_dy /= y_sample_num;
        center_data.depth_gradient = abs(depth_dx) + abs(depth_dy);
    }

    void compile_compute_grad() noexcept {
        Kernel kernel = [&](Uint frame_index, BufferVar<PixelGeometry> gbuffer) {
            PixelGeometryVar center_data = gbuffer.read(dispatch_id());
            compute_gradient(center_data, gbuffer);
            gbuffer.write(dispatch_id(), center_data);
        };
        compute_grad_ = device().compile(kernel, "rt_gradient");
    }

    void compile_compute_hit() noexcept {
        TSensor &camera = scene().sensor();
        TSampler &sampler = scene().sampler();
        Kernel kernel = [&](BufferVar<TriangleHit> hit_buffer, Uint frame_index) {
            Uint2 pixel = dispatch_idx().xy();
            sampler->start(pixel, frame_index, 0);
            camera->load_data();

            SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
            RayState rs = camera->generate_ray(ss);
            TriangleHitVar hit = pipeline()->trace_closest(rs.ray);
            hit_buffer.write(dispatch_id(), hit);
        };
        compute_hit_ = device().compile(kernel, "rt_compute_pixel");
    }

    void compile() noexcept override {
        FrameBuffer::compile();
        compile_compute_geom();
        compile_compute_grad();
        compile_compute_hit();
    }

    [[nodiscard]] CommandList compute_hit(uint frame_index) const noexcept override {
        CommandList ret;
        ret << compute_hit_(hit_buffer_, frame_index).dispatch(resolution());
        return ret;
    }

    [[nodiscard]] CommandList compute_geom(uint frame_index, BufferView<PixelGeometry> gbuffer, BufferView<float2> motion_vectors,
                                           BufferView<float4> albedo, BufferView<float4> emission) const noexcept override {
        CommandList ret;
        ret << compute_geom_(frame_index, gbuffer, motion_vectors, albedo, emission).dispatch(resolution());
        return ret;
    }

    CommandList compute_grad(uint frame_index, BufferView<vision::PixelGeometry> gbuffer) const noexcept override {
        CommandList ret;
        ret << compute_grad_(frame_index, gbuffer).dispatch(resolution());
        return ret;
    }

    [[nodiscard]] CommandList compute_GBuffer(uint frame_index, BufferView<PixelGeometry> gbuffer,
                                              BufferView<float2> motion_vectors,
                                              BufferView<float4> albedo,
                                              BufferView<float4> emission) const noexcept override {
        CommandList ret;
        ret << compute_geom_(frame_index, gbuffer, motion_vectors, albedo, emission).dispatch(resolution());
        ret << compute_grad_(frame_index, gbuffer).dispatch(resolution());
        return ret;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, RayTracingFrameBuffer)
//VS_REGISTER_CURRENT_PATH(0, "vision-framebuffer-rt.dll")