//
// Created by Zero on 06/09/2022.
//

#include "scene.h"
#include "ocarina/src/core/dynamic_module.h"
#include "pipeline.h"
#include "base/scattering/interaction.h"
#include "mesh_registry.h"
#include "GUI/window.h"

namespace vision {

Pipeline *Scene::pipeline() noexcept { return Global::instance().pipeline(); }

void Scene::init(const SceneDesc &scene_desc) {
    TIMER(init_scene);
    _warper_desc = scene_desc.warper_desc;
    _render_setting = scene_desc.render_setting;
    materials().set_mode(_render_setting.polymorphic_mode);
    OC_INFO_FORMAT("polymorphic mode is {}", materials().mode());
    _light_sampler = load<LightSampler>(scene_desc.light_sampler_desc);
    _light_sampler->set_mode(_render_setting.polymorphic_mode);
    _spectrum = load<Spectrum>(scene_desc.spectrum_desc);
    load_materials(scene_desc.material_descs);
    load_mediums(scene_desc.mediums_desc);
    _camera = load<Camera>(scene_desc.sensor_desc);
    load_shapes(scene_desc.shape_descs);
    _integrator = load<Integrator>(scene_desc.integrator_desc);
    _sampler = load<Sampler>(scene_desc.sampler_desc);
    _min_radius = scene_desc.render_setting.min_world_radius;
    Interaction::set_ray_offset_factor(scene_desc.render_setting.ray_offset_factor);
}

void Scene::tidy_up() noexcept {
    _light_sampler->tidy_up();
    material_registry().tidy_up();
    MeshRegistry::instance().tidy_up();
    tidy_up_mediums();
    OC_INFO_FORMAT("This scene contains {} material types with {} material instances",
                   materials().type_num(),
                   materials().all_instance_num());
}

void Scene::tidy_up_materials() noexcept {
    materials().for_each_instance([&](SP<Material> material, uint i) {
        material->set_index(i);
    });
}

void Scene::tidy_up_mediums() noexcept {
    _mediums.for_each_instance([&](SP<Medium> medium, uint i) {
        medium->set_index(i);
    });
}

Slot Scene::create_slot(const SlotDesc &desc) {
    return Global::node_mgr().create_slot(desc);
}

SP<Material> Scene::obtain_black_body() noexcept {
    if (!_black_body) {
        MaterialDesc md;
        md.sub_type = "black_body";
        _black_body = load<Material>(md);
        materials().push_back(_black_body);
    }
    return _black_body;
}

void Scene::prepare() noexcept {
    material_registry().remove_unused_materials();
    tidy_up();
    fill_instances();
    _camera->prepare();
    _sampler->prepare();
    _integrator->prepare();
    _camera->update_device_data();
    prepare_lights();
    prepare_materials();
    pipeline()->spectrum().prepare();
}

void Scene::prepare_lights() noexcept {
    _light_sampler->prepare();
    auto &light = _light_sampler->lights();
    OC_INFO_FORMAT("This scene contains {} light types with {} light instances",
                   light.type_num(),
                   light.all_instance_num());
}

void Scene::add_material(SP<vision::Material> material) noexcept {
    materials().push_back(ocarina::move(material));
}

void Scene::add_light(SP<vision::Light> light) noexcept {
    _light_sampler->add_light(ocarina::move(light));
}

void Scene::load_materials(const vector<MaterialDesc> &material_descs) {
    for (const MaterialDesc &desc : material_descs) {
        add_material(ocarina::move(load<Material>(desc)));
    }
}

void Scene::add_shape(const SP<vision::ShapeGroup> &group, ShapeDesc desc) {
    _groups.push_back(group);
    _aabb.extend(group->aabb);
    group->for_each([&](ShapeInstance &instance, uint i) {
        auto iter = materials().find_if([&](SP<Material> &material) {
            return material->name() == instance.material_name();
        });

        if (iter != materials().end() && !instance.has_material()) {
            instance.set_material(*iter);
        }

        if (desc.emission.valid()) {
            desc.emission.set_value("inst_id", _instances.size());
            SP<IAreaLight> light = load_light<IAreaLight>(desc.emission);
            instance.set_emission(light);
            light->set_instance(&instance);
        }
        if (has_medium()) {
            auto inside = _mediums.find_if([&](SP<Medium> &medium) {
                return medium->name() == instance.inside_name();
            });
            if (inside != _mediums.end()) {
                instance.set_inside(*inside);
            }
            auto outside = _mediums.find_if([&](SP<Medium> &medium) {
                return medium->name() == instance.outside_name();
            });
            if (outside != _mediums.end()) {
                instance.set_outside(*outside);
            }
        }
        _instances.push_back(instance);
    });
}

void Scene::clear_shapes() noexcept {
    _instances.clear();
    _groups.clear();
}

void Scene::load_shapes(const vector<ShapeDesc> &descs) {
    for (const auto &desc : descs) {
        SP<ShapeGroup> group = load<ShapeGroup>(desc);
        add_shape(group, desc);
    }
}

void Scene::fill_instances() {
    for (ShapeInstance &instance : _instances) {
        if (instance.has_material()) {
            const Material *material = instance.material().get();
            instance.update_material_id(materials().encode_id(material->index(), material));
        }
        if (instance.has_emission()) {
            const Light *emission = instance.emission().get();
            instance.update_light_id(_light_sampler->lights().encode_id(emission->index(), emission));
        }
        instance.fill_mesh_id();
        if (has_medium()) {
            if (instance.has_inside()) {
                instance.update_inside_medium_id(instance.inside()->index());
            }
            if (instance.has_outside()) {
                instance.update_outside_medium_id(instance.outside()->index());
            }
        }
    }
}

void Scene::load_mediums(const MediumsDesc &md) {
    _global_medium.name = md.global;
    for (uint i = 0; i < md.mediums.size(); ++i) {
        const MediumDesc &desc = md.mediums[i];
        auto medium = load<Medium>(desc);
        medium->set_index(i);
        _mediums.push_back(medium);
    }
    uint index = _mediums.get_index([&](const SP<Medium> &medium) {
        return medium->name() == _global_medium.name;
    });
    if (index != InvalidUI32) {
        _global_medium.object = _mediums[index];
    }
}

void Scene::prepare_materials() {
    materials().for_each_instance([&](const SP<Material> &material) noexcept {
        material->prepare();
    });
    auto rp = pipeline();
    materials().prepare(rp->bindless_array(), rp->device());
}

void Scene::upload_data() noexcept {
    _camera->update_device_data();
    _integrator->update_device_data();
    _light_sampler->update_device_data();
    _material_registry->upload_device_data();
}

}// namespace vision