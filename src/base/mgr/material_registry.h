//
// Created by Zero on 2023/8/6.
//

#pragma once

#include "base/scattering/material.h"
#include "UI/polymorphic.h"
#include "hotfix/hotfix.h"

namespace vision {
using namespace ocarina;

template<typename T>
class TRegistry : public GUI, public Observer {
public:
    using element_ty = T;
protected:
    PolymorphicGUI<SP<T>> elements_;

public:
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void update_runtime_object(const vision::IObjectConstructor *constructor) noexcept override;
    void push_back(SP<T> element) noexcept;
    void upload_device_data() noexcept;
    void prepare() noexcept;
    void remedy() noexcept;
    void remove_unused_materials() noexcept;
    void tidy_up() noexcept;
    OC_MAKE_MEMBER_GETTER(elements, &)
};

class MaterialRegistry : public GUI, public Observer {
private:
    OC_MAKE_INSTANCE_CONSTRUCTOR(MaterialRegistry, s_material_registry)

private:
    PolymorphicGUI<SP<Material>> materials_;

public:
    OC_MAKE_INSTANCE_FUNC_DECL(MaterialRegistry)
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void update_runtime_object(const vision::IObjectConstructor *constructor) noexcept override;
    VS_MAKE_GUI_STATUS_FUNC(GUI, materials_)
    [[nodiscard]] SP<Material> register_(SP<Material> material) noexcept;
    [[nodiscard]] SP<Material> get_material(uint64_t hash) noexcept;
    [[nodiscard]] bool has_dispersive() const noexcept;
    void push_back(SP<Material> material) noexcept;
    void upload_device_data() noexcept;
    void prepare() noexcept;
    void remedy() noexcept;
    void remove_unused_materials() noexcept;
    void tidy_up() noexcept;
    OC_MAKE_MEMBER_GETTER(materials, &)
};

void precompute_albedo() noexcept;

}// namespace vision