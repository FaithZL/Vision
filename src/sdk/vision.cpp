//
// Created by Zero on 2023/7/17.
//

#include "vision.h"
#include "base/node.h"
#include "core/stl.h"
#include "base/shape.h"
#include "base/mgr/global.h"
#include "base/mgr/pipeline.h"

namespace vision::sdk {

class VisionRendererImpl : public VisionRenderer {
private:
    UP<Device> _device;
    SP<Pipeline> _pipeline{};
    ocarina::Shader<void(uint)> _shader;
    uint _frame_index{0u};
    bool _prepared{false};

public:
    void init_pipeline(const char *rpath) override;
    void init_scene() override;
    void compile() override;
    void render() override;
    void invalidation() override;
    void clear_geometries() override;
    void add_instance(const Instance &instance) override;
    void build_accel() override;
    void update_camera(vision::sdk::Camera camera) override;
    void update_resolution(uint32_t width, uint32_t height) override;
    void download_radiance(void *data) override;
};

void VisionRendererImpl::compile() {
    if (_shader.has_function()) {
        return;
    }
    if (!_prepared) {
        return;
    }
    _pipeline->scene().prepare();
    _pipeline->upload_resource_array();
    auto &geometry = _pipeline->geometry();
    auto camera = _pipeline->scene().camera();
    auto film = camera->radiance_film();
    auto sampler = _pipeline->scene().sampler();
    Buffer<float4> &buffer = film->original_buffer().device_buffer();
    Kernel kernel = [&](Uint frame_index) {
        Uint2 pixel = dispatch_idx().xy();
        sampler->start_pixel_sample(pixel, 0, 0);
        SensorSample ss = sampler->sensor_sample(pixel, camera->filter());
        camera->load_data();

        RayState rs = camera->generate_ray(ss);
        Uint2 res = dispatch_dim().xy();
        Uint2 center = res / 2u;


        Float3 L = make_float3(0.f);

        Var hit = geometry.trace_closest(rs.ray);
        $if(hit->is_miss()) {
            buffer.write(dispatch_id(), make_float4(0, 0, 0, 1));
            $return();
        };

        Interaction it = geometry.compute_surface_interaction(hit, rs.ray);

        L = (it.ng + 1.f) / 2.f;

        $if(all(0u == pixel)) {
            Printer::instance().info_with_location("inst {}, prim {}, pos {} {} {}", hit.inst_id, hit.prim_id, it.pos);
        };

        buffer.write(dispatch_id(), make_float4(L, 1.f));
    };
    _shader = _device->compile(kernel);
}

void VisionRendererImpl::render() {
    Stream &stream = _pipeline->stream();
    auto camera = _pipeline->scene().camera();
    auto film = camera->radiance_film();
    uint2 res = film->resolution();
    if (!_shader.has_function()) {
        return;
    }

    float3 up = camera->up();
    float3 right = camera->right();
    float3 forward = camera->forward();

    _pipeline->upload_data();
    stream << _shader(_frame_index).dispatch(res) << synchronize();
    stream << commit();
    Printer::instance().retrieve_immediately([&](int ,const char *str) {
        int i = 0;
    });
    ++_frame_index;
}

void VisionRendererImpl::invalidation() {
    _frame_index = 0;
}

void VisionRendererImpl::init_pipeline(const char *rpath) {
    PipelineDesc desc;
    Context::instance().init(rpath);
    _device = make_unique<Device>(Context::instance().create_device("cuda"));
    _device->init_rtx();
    desc.device = _device.get();
    desc.sub_type = "offline";
    _pipeline = Global::node_mgr().load<Pipeline>(desc);
    Global::instance().set_pipeline(_pipeline.get());
}

void VisionRendererImpl::clear_geometries() {
    std::cout << "clear_geometries" << std::endl;
    _pipeline->clear_geometry();
}

void VisionRendererImpl::init_scene() {
    SceneDesc scene_desc;
    scene_desc.init(DataWrap::object());
    Scene &scene = _pipeline->scene();
    scene.init(scene_desc);
}

void VisionRendererImpl::download_radiance(void *data) {
    Stream &stream = _pipeline->stream();
    auto camera = _pipeline->scene().camera();
    auto &buffer = camera->radiance_film()->original_buffer();
    buffer.device_buffer().download_immediately(data);
}

vision::Vertex from_sdk_vertex(const sdk::Vertex &vert) {
    vision::Vertex vertex;
    vertex.pos = vert.pos;
    vertex.n = vert.n;
    vertex.uv = vert.uv;
    return vertex;
}

Triangle from_triple(const sdk::Triple &triple) {
    return bit_cast<Triangle>(triple);
}

float4x4 from_array(std::array<float, 16> arr) {
    float4x4 ret;
    for (int i = 0; i < 16; ++i) {
        int x = i / 4;
        int y = i % 4;
        ret[x][y] = arr[i];
    }
    return ret;
}

ShapeInstance from_sdk_instance(const sdk::Instance &inst) {

    SP<Mesh> mesh = std::make_shared<Mesh>();
    for (int i = 0; i < inst.vert_num; ++i) {
        mesh->vertices().push_back(from_sdk_vertex(inst.vertices.get()[i]));
    }

    for (int i = 0; i < inst.tri_num; ++i) {
        mesh->triangles().push_back(from_triple(inst.triangles.get()[i]));
    }

    ShapeInstance ret{mesh};

    ret.set_o2w(from_array(inst.mat4.m));

    return ret;
}

void VisionRendererImpl::add_instance(const vision::sdk::Instance &instance) {
    SP<ShapeGroup> group = std::make_shared<ShapeGroup>(ShapeDesc{});
    ShapeInstance inst = from_sdk_instance(instance);
    group->add_instance(inst);
    _pipeline->scene().add_shape(group);
}

void VisionRendererImpl::build_accel() {
    Geometry &geom = _pipeline->geometry();
    Accel &accel = geom.accel;
    auto impl = accel.impl();
    _pipeline->prepare_geometry();
    _pipeline->upload_resource_array();
    _prepared = true;
    OC_INFO("build accel");
}

void VisionRendererImpl::update_camera(vision::sdk::Camera c) {
    float4x4 o2w = from_array(c.c2w.m);
    auto camera = _pipeline->scene().camera();
    camera->set_mat(o2w);
    camera->set_fov_y(c.fov_y);
    OC_INFO("update_camera");
}

void VisionRendererImpl::update_resolution(uint32_t width, uint32_t height) {
    auto camera = _pipeline->scene().camera();
    auto film = camera->radiance_film();
    film->set_resolution(make_uint2(width, height));
    _pipeline->scene().prepare();
    _pipeline->upload_resource_array();
}

}// namespace vision::sdk

VS_EXPORT_API vision::sdk::VisionRenderer *create() {
    return ocarina::new_with_allocator<vision::sdk::VisionRendererImpl>();
}
OC_EXPORT_API void destroy(vision::sdk::VisionRenderer *obj) {
    ocarina::delete_with_allocator(obj);
}
