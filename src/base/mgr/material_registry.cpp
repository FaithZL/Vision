//
// Created by Zero on 2023/8/6.
//

#include "material_registry.h"

namespace vision {

MaterialRegistry *MaterialRegistry::s_material_registry = nullptr;

SP<Material> MaterialRegistry::register_(SP<vision::Material> material) noexcept {
    uint64_t hash = material->hash();
    auto iter = std::find_if(materials_.begin(), materials_.end(), [&](SP<Material> mat) {
        return mat->hash() == hash;
    });
    if (iter == materials_.cend()) {
        materials_.push_back(material);
        return material;
    }
    return *iter;
}

SP<Material> MaterialRegistry::get_material(uint64_t hash) noexcept {
    auto iter = std::find_if(materials_.begin(), materials_.end(), [&](SP<Material> mat) {
        return mat->hash() == hash;
    });
    if (iter == materials_.cend()) {
        return nullptr;
    }
    return *iter;
}

void MaterialRegistry::push_back(SP<vision::Material> material) noexcept {
    materials_.push_back(ocarina::move(material));
}

bool MaterialRegistry::has_dispersive() const noexcept {
    return std::any_of(materials_.begin(), materials_.end(),
                       [&](const SP<Material> &mat) {
                           return mat->is_dispersive();
                       });
}

void MaterialRegistry::upload_device_data() noexcept {
    if (has_changed()) {
        materials_.update();
    }
}

bool MaterialRegistry::render_UI(ocarina::Widgets *widgets) noexcept {
    bool open = widgets->use_folding_header("materials", [&] {
        materials_.render_UI(widgets);
    });
    return open;
}

void MaterialRegistry::update_runtime_object(const IObjectConstructor *constructor) noexcept {
    for (int i = 0; i < materials_.size(); ++i) {
        SP<Material> material = materials_[i];
        if (!constructor->match(material.get())) {
            continue;
        }
        SP<Material> new_material = constructor->construct_shared<Material>();
        new_material->restore(material.get());
        materials_.replace(i, new_material);
    }
}

void MaterialRegistry::precompute_albedo() noexcept {

}

void MaterialRegistry::tidy_up() noexcept {
    materials_.for_each_instance([&](SP<Material> material, uint i) {
        material->set_index(i);
    });
}

void MaterialRegistry::remove_unused_materials() noexcept {
    for (auto iter = materials_.begin(); iter != materials_.end();) {
        if (iter->use_count() == 1) {
            iter = materials_.erase(iter);
        } else {
            ++iter;
        }
    }
    tidy_up();
}

MaterialRegistry &MaterialRegistry::instance() {
    if (s_material_registry == nullptr) {
        s_material_registry = new MaterialRegistry();
        HotfixSystem::instance().register_static_var("MaterialRegistry", *s_material_registry);
    }
    return *s_material_registry;
}

void MaterialRegistry::destroy_instance() {
    if (s_material_registry) {
        delete s_material_registry;
        s_material_registry = nullptr;
    }
}

}// namespace vision