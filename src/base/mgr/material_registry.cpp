//
// Created by Zero on 2023/8/6.
//

#include "material_registry.h"

namespace vision {

MaterialRegistry *MaterialRegistry::s_material_registry = nullptr;

SP<Material> MaterialRegistry::register_(SP<vision::Material> material) noexcept {
    uint64_t hash = material->hash();
    auto iter = std::find_if(_materials.begin(), _materials.end(), [&](SP<Material> mat) {
        return mat->hash() == hash;
    });
    if (iter == _materials.cend()) {
        _materials.push_back(material);
        return material;
    }
    return *iter;
}

SP<Material> MaterialRegistry::get_material(uint64_t hash) noexcept {
    auto iter = std::find_if(_materials.begin(), _materials.end(), [&](SP<Material> mat) {
        return mat->hash() == hash;
    });
    if (iter == _materials.cend()) {
        return nullptr;
    }
    return *iter;
}

void MaterialRegistry::push_back(SP<vision::Material> material) noexcept {
    _materials.push_back(ocarina::move(material));
}

bool MaterialRegistry::render_UI(ocarina::Widgets *widgets) noexcept {
    bool open = widgets->use_folding_header("materials", [&] {

    });
    return true;
}

void MaterialRegistry::tidy_up() noexcept {
    _materials.for_each_instance([&](SP<Material> material, uint i) {
        material->set_index(i);
    });
}

void MaterialRegistry::remove_unused_materials() noexcept {
    for (auto iter = _materials.begin(); iter != _materials.end();) {
        if (iter->use_count() == 1) {
            iter = _materials.erase(iter);
        } else {
            ++iter;
        }
    }
    tidy_up();
}

MaterialRegistry &MaterialRegistry::instance() {
    if (s_material_registry == nullptr) {
        s_material_registry = new MaterialRegistry();
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