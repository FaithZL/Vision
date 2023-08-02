//
// Created by Zero on 06/09/2022.
//

#pragma once

#include "descriptions/scene_desc.h"
#include "global.h"
#include "base/sensor/camera.h"
#include "base/sampler.h"
#include "base/shape.h"
#include "base/integrator.h"
#include "base/illumination/lightsampler.h"
#include "base/scattering/material.h"
#include "base/scattering/medium.h"
#include "base/warper.h"
#include "base/color/spectrum.h"

namespace vision {

using namespace ocarina;

#define MAKE_GETTER(member)                                                \
    [[nodiscard]] auto member() const noexcept { return _##member.get(); } \
    [[nodiscard]] auto member() noexcept { return _##member.get(); }

class Scene {
private:
    Box3f _aabb;
    SP<Camera> _camera{nullptr};
    SP<Sampler> _sampler{nullptr};
    SP<Integrator> _integrator{nullptr};
    SP<LightSampler> _light_sampler{nullptr};
    vector<SP<ShapeGroup>> _groups;
    vector<ShapeInstance> _instances;
    Polymorphic<SP<Material>> _materials;
    Polymorphic<SP<Medium>> _mediums;
    WarperDesc _warper_desc;
    RenderSettingDesc _render_setting{};
    SP<Spectrum> _spectrum{nullptr};
    Wrap<Medium> _global_medium{};
    SP<Material> _black_body{};
    friend class Pipeline;

public:
    Scene() = default;
    void init(const SceneDesc &scene_desc);
    void prepare() noexcept;
    [[nodiscard]] PolymorphicMode polymorphic_mode() const noexcept { return _render_setting.polymorphic_mode; }
    [[nodiscard]] Pipeline *pipeline() noexcept;
    MAKE_GETTER(integrator)
    MAKE_GETTER(camera)
    MAKE_GETTER(spectrum)
    MAKE_GETTER(sampler)
    MAKE_GETTER(light_sampler)
    OC_MAKE_MEMBER_GETTER(global_medium, )
    OC_MAKE_MEMBER_GETTER(groups, &)
    OC_MAKE_MEMBER_GETTER(instances, &)
    [[nodiscard]] auto radiance_film() noexcept { return camera()->radiance_film(); }
    [[nodiscard]] auto radiance_film() const noexcept { return camera()->radiance_film(); }
    [[nodiscard]] const auto &materials() const noexcept { return _materials; }
    [[nodiscard]] auto &materials() noexcept { return _materials; }
    [[nodiscard]] const auto &mediums() const noexcept { return _mediums; }
    void tidy_up() noexcept;
    void tidy_up_materials() noexcept;
    void tidy_up_mediums() noexcept;
    [[nodiscard]] Slot create_slot(const SlotDesc &desc);
    [[nodiscard]] SP<Material> obtain_black_body() noexcept;
    template<typename T, typename desc_ty>
    [[nodiscard]] SP<T> load(const desc_ty &desc) {
        return Global::node_mgr().load<T>(desc);
    }
    [[nodiscard]] uint light_num() const noexcept { return _light_sampler->light_num(); }
    void prepare_lights() noexcept;
    [[nodiscard]] SP<Warper> load_warper() noexcept { return load<Warper>(_warper_desc); }
    [[nodiscard]] SP<Warper2D> load_warper2d() noexcept {
        WarperDesc warper_desc = _warper_desc;
        warper_desc.sub_type += "2d";
        return load<Warper2D>(warper_desc);
    }
    [[nodiscard]] bool has_medium() const noexcept { return !_mediums.empty(); }
    void load_shapes(const vector<ShapeDesc> &descs);
    void load_mediums(const MediumsDesc &desc);
    void load_materials(const vector<MaterialDesc> &material_descs);
    void fill_instances();
    void remove_unused_materials();
    template<typename T = Light>
    SP<T> load_light(const LightDesc &desc) {
        OC_ASSERT(_light_sampler != nullptr);
        auto ret = load<T>(desc);
        _light_sampler->add_light(ret);
        return ret;
    }
    void prepare_materials();
    [[nodiscard]] float world_diameter() const noexcept { return _aabb.radius() * 2; }
    void upload_data() noexcept;
    [[nodiscard]] ShapeInstance *get_instance(uint id) noexcept { return &_instances[id]; }
    [[nodiscard]] const ShapeInstance *get_instance(uint id) const noexcept { return &_instances[id]; }
};

#undef MAKE_GETTER

}// namespace vision