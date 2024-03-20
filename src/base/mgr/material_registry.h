//
// Created by Zero on 2023/8/6.
//

#pragma once

#include "base/scattering/material.h"
#include "UI/polymorphic.h"

namespace vision {
using namespace ocarina;
class MaterialRegistry : public GUI {
private:
    static MaterialRegistry *s_material_registry;
    MaterialRegistry() = default;
    MaterialRegistry(const MaterialRegistry &) = default;
    MaterialRegistry(MaterialRegistry &&) noexcept = default;
    MaterialRegistry operator=(const MaterialRegistry &) = delete;
    MaterialRegistry operator=(MaterialRegistry &&) = delete;

private:
    PolymorphicGUI<SP<Material>> _materials;

public:
    static MaterialRegistry &instance();
    static void destroy_instance();
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    VS_MAKE_GUI_STATUS_FUNC(GUI, _materials)
    [[nodiscard]] SP<Material> register_(SP<Material> material) noexcept;
    [[nodiscard]] SP<Material> get_material(uint64_t hash) noexcept;
    void push_back(SP<Material> material) noexcept;
    void remove_unused_materials() noexcept;
    void tidy_up() noexcept;
    OC_MAKE_MEMBER_GETTER(materials, &)
};

}// namespace vision