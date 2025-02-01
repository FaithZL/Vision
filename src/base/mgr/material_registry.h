//
// Created by Zero on 2023/8/6.
//

#pragma once

#include "base/scattering/material.h"
#include "UI/polymorphic.h"
#include "hotfix/hotfix.h"

namespace vision {
using namespace ocarina;
class MaterialRegistry : public GUI, public Observer {
private:
    static MaterialRegistry *s_material_registry;
    MaterialRegistry() = default;
    MaterialRegistry(const MaterialRegistry &) = default;
    MaterialRegistry(MaterialRegistry &&) noexcept = default;
    MaterialRegistry operator=(const MaterialRegistry &) = delete;
    MaterialRegistry operator=(MaterialRegistry &&) = delete;

private:
    PolymorphicGUI<SP<Material>> materials_;

public:
    static MaterialRegistry &instance();
    static void destroy_instance();
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void update_runtime_object(const vision::IObjectConstructor *constructor) noexcept override;
    VS_MAKE_GUI_STATUS_FUNC(GUI, materials_)
    [[nodiscard]] SP<Material> register_(SP<Material> material) noexcept;
    [[nodiscard]] SP<Material> get_material(uint64_t hash) noexcept;
    [[nodiscard]] bool has_dispersive() const noexcept;
    void push_back(SP<Material> material) noexcept;
    void precompute_albedo() noexcept;
    void upload_device_data() noexcept;
    void remove_unused_materials() noexcept;
    void tidy_up() noexcept;
    OC_MAKE_MEMBER_GETTER(materials, &)
};

}// namespace vision