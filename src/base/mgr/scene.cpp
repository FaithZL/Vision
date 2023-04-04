//
// Created by Zero on 06/09/2022.
//

#include "scene.h"
#include "core/context.h"
#include "rhi/dynamic_module.h"
#include "base/scattering/interaction.h"

namespace vision {

Scene::Scene(vision::Context *ctx, RenderPipeline *rp)
    : _context(ctx),
      _rp(rp) {}

Node *Scene::load_node(const NodeDesc &desc) {
    const DynamicModule *module = _context->obtain_module(desc.plugin_name());
    auto creator = reinterpret_cast<Node::Creator *>(module->function_ptr("create"));
    auto deleter = reinterpret_cast<Node::Deleter *>(module->function_ptr("destroy"));
    _all_nodes.emplace_back(creator(desc), deleter);
    return _all_nodes.back().get();
}

RenderPipeline *Scene::render_pipeline() noexcept { return _rp; }

void Scene::init(const SceneDesc &scene_desc) {
    TIMER(init_scene);
    _warper_desc = scene_desc.warper_desc;
    _render_setting = scene_desc.render_setting;
    _light_sampler = load<LightSampler>(scene_desc.light_sampler_desc);
    _camera = load<Camera>(scene_desc.sensor_desc);
    load_materials(scene_desc.material_descs);
    load_shapes(scene_desc.shape_descs);
    load_mediums(scene_desc.mediums_desc.mediums);
    _integrator = load<Integrator>(scene_desc.integrator_desc);
    _spectrum = load<Spectrum>(scene_desc.spectrum_desc);
    _sampler = load<Sampler>(scene_desc.sampler_desc);
}

Slot Scene::create_slot(const SlotDesc &desc) noexcept {
    desc.scene = this;
    const ShaderNode *shader_node = load_shader_node(desc.node);
    return Slot(shader_node, desc.channels);
}

void Scene::prepare() noexcept {
    _camera->prepare();
    _sampler->prepare();
    _camera->update_device_data();
    build_warpers();
    prepare_shadernodes();
    prepare_materials();
    _rp->spectrum().prepare();
}

void Scene::build_warpers() noexcept {
    _light_sampler->for_each([&](Light *light) noexcept {
        light->prepare();
    });
}

void Scene::load_materials(const vector<MaterialDesc> &material_descs) noexcept {
    for (const MaterialDesc &desc : material_descs) {
        _materials.push_back(load<Material>(desc));
    }
    OC_INFO_FORMAT("This scene contains {} material types with {} material instances",
                   _materials.type_num(),
                   _materials.all_instance_num());
}

void Scene::load_shapes(const vector<ShapeDesc> &descs) noexcept {

    switch (polymorphic_mode()) {
        case Instance:
            OC_INFO("polymorphic mode is instance");
            for (const auto &desc : descs) {
                Shape *shape = const_cast<Shape *>(load<Shape>(desc));
                const Material *material = _materials[shape->handle.mat_id];
                shape->update_material_id(encode_id<H>(shape->handle.mat_id,
                                                       _materials.type_index(material)));
                _aabb.extend(shape->aabb);
                _shapes.push_back(shape);
            }
            break;
        case Type:
            OC_INFO("polymorphic mode is type");
            for (const auto &desc : descs) {
                Shape *shape = const_cast<Shape *>(load<Shape>(desc));
                const Material *material = _materials[shape->handle.mat_id];
                shape->update_material_id(encode_id<H>(_materials.data_index(material),
                                                       _materials.type_index(material)));

                _aabb.extend(shape->aabb);
                _shapes.push_back(shape);
            }
            break;
        default:
            OC_ASSERT(false);
            break;
    }
}

void Scene::load_mediums(const vector<MediumDesc> &descs) noexcept {
    for (const MediumDesc &desc : descs) {
        auto medium = load<Medium>(desc);
        _mediums.push_back(medium);
    }
}

Light *Scene::load_light(const LightDesc &desc) noexcept {
    OC_ASSERT(_light_sampler != nullptr);
    auto ret = load<Light>(desc);
    _light_sampler->add_light(ret);
    return ret;
}

ShaderNode *Scene::load_shader_node(const ShaderNodeDesc &desc) noexcept {
    auto ret = load<ShaderNode>(desc);
    _shadernodes.push_back(ret);
    return ret;
}

void Scene::load_lights(const vector<LightDesc> &descs) noexcept {
    for (const LightDesc &desc : descs) {
        load_light(desc);
    }
}

void Scene::prepare_materials() noexcept {
    switch (polymorphic_mode()) {
        case Instance:{
            _materials.for_each_instance([&](const Material *material) noexcept {
                const_cast<Material *>(material)->prepare();
            });
            break;
        }
        case Type:{
            _materials.for_each_representative([&](Material *material) {
                ManagedWrapper<float> data_set{render_pipeline()->resource_array()};
                _materials.set_datas(material, move(data_set));
            });
            _materials.for_each_instance([&](const Material *material) noexcept {
                const_cast<Material *>(material)->prepare();

                ManagedWrapper<float> &data_set = _materials.datas(material);
                material->fill_data(data_set);
            });
            _materials.for_each_representative([&](Material *material) {
                ManagedWrapper<float> &datas = _materials.datas(material);
                datas.reset_device_buffer(render_pipeline()->device());
                datas.register_self();
                datas.upload_immediately();
            });
            break;
        }
    }
}

void Scene::prepare_shadernodes() noexcept {
}

void Scene::upload_data() noexcept {
    _camera->update_device_data();
}

}// namespace vision