//
// Created by Zero on 2023/8/6.
//

#pragma once

#include "base/scattering/material.h"
#include "base/scattering/medium.h"
#include "base/sensor/sensor.h"
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
    VS_MAKE_GUI_STATUS_FUNC(GUI, elements_)
    void update_runtime_object(const vision::IObjectConstructor *constructor) noexcept override;
    void push_back(SP<T> element) noexcept;
    [[nodiscard]] virtual string_view UI_title() const noexcept { return "elements"; }
    void upload_device_data() noexcept;
    void prepare() noexcept;
    void remedy() noexcept;
    [[nodiscard]] SP<element_ty> register_(SP<element_ty> elm) noexcept;
    [[nodiscard]] SP<element_ty> get_element(uint64_t hash) noexcept;
    void remove_unused_elements() noexcept;
    void tidy_up() noexcept;
    OC_MAKE_MEMBER_GETTER(elements, &)
};

class MaterialRegistry : public TRegistry<Material> {
private:
    OC_MAKE_INSTANCE_CONSTRUCTOR(MaterialRegistry, s_material_registry)
    bool flatten_lobes_{true};
    bool individual_ns_{true};

public:
    OC_MAKE_INSTANCE_FUNC_DECL(MaterialRegistry)
    OC_MAKE_MEMBER_GETTER(flatten_lobes, &)
    OC_MAKE_MEMBER_GETTER(individual_ns, &)
    [[nodiscard]] string_view UI_title() const noexcept override { return "materials"; }
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;
    void precompute_albedo() noexcept;
    [[nodiscard]] bool has_dispersive() const noexcept;
};

struct MediumsDesc;
class MediumRegistry : public TRegistry<Medium> {
private:
    OC_MAKE_INSTANCE_CONSTRUCTOR(MediumRegistry, s_medium_registry)
    bool process_mediums_{false};
    TObject<Medium> global_medium_{};

public:
    OC_MAKE_INSTANCE_FUNC_DECL(MediumRegistry)
    [[nodiscard]] string_view UI_title() const noexcept override { return "mediums"; }
    [[nodiscard]] bool process_mediums() const noexcept;
    void set_process_mediums(bool b) noexcept { process_mediums_ = b; }
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;
    OC_MAKE_MEMBER_GETTER(global_medium, &)
    void load_mediums(const MediumsDesc &md) noexcept;
};

class SensorRegistry : public TRegistry<Sensor> {
public:
    [[nodiscard]] string_view UI_title() const noexcept override { return "sensors"; }
};

}// namespace vision