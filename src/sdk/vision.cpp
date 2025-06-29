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
    void add_instance(const Instance &instance, const char *mat) override;
    void build_accel() override;
    void update_camera(vision::sdk::Sensor camera) override;
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
    _pipeline->upload_bindless_array();
    auto &geometry = _pipeline->geometry();
    auto camera = _pipeline->scene().sensor();
    auto rad_collector = camera->rad_collector();
    auto sampler = _pipeline->scene().sampler();
    Buffer<float4> &buffer = rad_collector->rt_buffer().device_buffer();
    Kernel kernel = [&](Uint frame_index) {
        Uint2 pixel = dispatch_idx().xy();
        sampler->start(pixel, 0, 0);
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
            Env::printer().info_with_location("inst {}, prim {}, pos {} {} {}", hit.inst_id, hit.prim_id, it.pos);
        };

        buffer.write(dispatch_id(), make_float4(L, 1.f));
    };
    _shader = _device->compile(kernel);
}

void VisionRendererImpl::render() {
    Stream &stream = _pipeline->stream();
    auto camera = _pipeline->scene().sensor();
    auto rad_collector = camera->rad_collector();
    uint2 res = rad_collector->resolution();
    if (!_shader.has_function()) {
        return;
    }

    float3 up = camera->up();
    float3 right = camera->right();
    float3 forward = camera->forward();

    _pipeline->upload_data();
    stream << _shader(_frame_index).dispatch(res) << synchronize();
    stream << commit();
    Env::printer().retrieve_immediately([&](int ,const char *str) {
        int i = 0;
    });
    ++_frame_index;
}

void VisionRendererImpl::invalidation() {
    _frame_index = 0;
}

void VisionRendererImpl::init_pipeline(const char *rpath) {
    PipelineDesc desc;
    FileManager::instance().init(rpath);
    _device = make_unique<Device>(FileManager::instance().create_device("cuda"));
    _device->init_rtx();
    Global::instance().set_device(_device.get());
    desc.device = _device.get();
    desc.sub_type = "fixed";
    _pipeline = Node::create_shared<Pipeline>(desc);
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
    auto camera = _pipeline->scene().sensor();
    auto &buffer = camera->rad_collector()->rt_buffer();
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

void VisionRendererImpl::add_instance(const vision::sdk::Instance &instance, const char *mat) {
    SP<ShapeGroup> group = std::make_shared<ShapeGroup>(ShapeDesc{});
    MaterialDesc md;
    md.init(Value::parse(mat));
    ShapeInstance inst = from_sdk_instance(instance);
    inst.set_mesh(MeshRegistry::instance().register_(inst.mesh()));
//    auto material = NodeMgr::instance().load<Material>(md);
//    inst.set_material(material);
//    pipeline_->scene().add_material(material);
    group->add_instance(inst);
    _pipeline->scene().add_shape(group);
}

void VisionRendererImpl::build_accel() {
    _pipeline->scene().tidy_up();
    _pipeline->scene().fill_instances();
    Geometry &geom = _pipeline->geometry();
    Accel &accel = geom.accel();
    auto impl = accel.impl();
    _pipeline->prepare_geometry();
    _pipeline->upload_bindless_array();
    _prepared = true;
    OC_INFO("build accel_");
}

void VisionRendererImpl::update_camera(vision::sdk::Sensor c) {
    float4x4 o2w = from_array(c.c2w.m);
    auto camera = _pipeline->scene().sensor();
    camera->set_mat(o2w);
    camera->set_fov_y(45);
    OC_INFO("update_camera");
}

void VisionRendererImpl::update_resolution(uint32_t width, uint32_t height) {
    auto camera = _pipeline->scene().sensor();
    auto rad_collector = camera->rad_collector();
    camera->set_resolution(make_uint2(width, height));
    _pipeline->scene().prepare();
    _pipeline->upload_bindless_array();
}

}// namespace vision::sdk

VS_EXPORT_API vision::sdk::VisionRenderer *create() {
    return ocarina::new_with_allocator<vision::sdk::VisionRendererImpl>();
}
OC_EXPORT_API void destroy(vision::sdk::VisionRenderer *obj) {
    ocarina::delete_with_allocator(obj);
}
