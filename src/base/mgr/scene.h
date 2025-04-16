//
// Created by Zero on 06/09/2022.
//

#pragma once

#include "core/scene_desc.h"
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
#include "material_registry.h"
#include "mesh_registry.h"
#include "UI/GUI.h"
#include "hotfix/hotfix.h"

namespace vision {

using namespace ocarina;

#define MAKE_GETTER(member)                                                \
    [[nodiscard]] auto member() const noexcept { return member##_.get(); } \
    [[nodiscard]] auto member() noexcept { return member##_.get(); }

class Scene : public GUI, public hotfix::Observer {
private:
    Box3f aabb_;
    TCamera camera_{};
    TSampler sampler_{};
    TIntegrator integrator_{};
    TLightSampler light_sampler_{};
    vector<SP<ShapeGroup>> groups_;
    vector<SP<ShapeInstance>> instances_;
    Polymorphic<SP<Medium>> mediums_;
    WarperDesc warper_desc_;
    RenderSettingDesc render_setting_{};
    MaterialRegistry *material_registry_{&MaterialRegistry::instance()};
    MeshRegistry *mesh_registry_{&MeshRegistry::instance()};
    TSpectrum spectrum_{};
    TObject<Medium> global_medium_{};
    SP<Material> black_body_{};
    float min_radius_{};
    friend class Pipeline;

public:
    Scene() = default;
    void init(const SceneDesc &scene_desc);
    void prepare() noexcept;
    void update_runtime_object(const vision::IObjectConstructor *constructor) noexcept override;
    [[nodiscard]] PolymorphicMode polymorphic_mode() const noexcept { return render_setting_.polymorphic_mode; }
    void update_resolution(uint2 res) noexcept;
    [[nodiscard]] Pipeline *pipeline() noexcept;
    VS_MAKE_GUI_ALL_FUNC(GUI, camera_, integrator_, light_sampler_,
                         material_registry_, spectrum_, sampler_)
    OC_MAKE_MEMBER_GETTER_SETTER(sampler, &)
    OC_MAKE_MEMBER_GETTER_SETTER(light_sampler, &)
    OC_MAKE_MEMBER_GETTER_SETTER(integrator, &)
    OC_MAKE_MEMBER_GETTER_SETTER(camera, &)
    OC_MAKE_MEMBER_GETTER_SETTER(spectrum, &)
    OC_MAKE_MEMBER_GETTER(global_medium, )
    OC_MAKE_MEMBER_GETTER(groups, &)
    OC_MAKE_MEMBER_GETTER(instances, &)
    [[nodiscard]] const auto &material_registry() const noexcept { return *material_registry_; }
    [[nodiscard]] auto &material_registry() noexcept { return *material_registry_; }
    [[nodiscard]] auto film() noexcept { return camera()->film(); }
    [[nodiscard]] auto film() const noexcept { return camera()->film(); }
    [[nodiscard]] const auto &materials() const noexcept { return material_registry().elements(); }
    [[nodiscard]] auto &materials() noexcept { return material_registry().elements(); }
    [[nodiscard]] const auto &mediums() const noexcept { return mediums_; }
    void tidy_up() noexcept;
    void tidy_up_materials() noexcept;
    void tidy_up_mediums() noexcept;
    void mark_selected(TriangleHit hit) noexcept;
    [[nodiscard]] SP<Material> obtain_black_body() noexcept;

    [[nodiscard]] uint light_num() const noexcept { return light_sampler_->light_num(); }
    void prepare_lights() noexcept;
    [[nodiscard]] SP<Warper> load_warper() noexcept { return Node::create_shared<Warper>(warper_desc_); }
    [[nodiscard]] SP<Warper2D> load_warper2d() noexcept {
        WarperDesc warper_desc = warper_desc_;
        warper_desc.sub_type += "2d";
        return Node::create_shared<Warper2D>(warper_desc);
    }
    [[nodiscard]] bool has_medium() const noexcept { return !mediums_.empty(); }
    void load_shapes(const vector<ShapeDesc> &descs);
    void add_shape(const SP<ShapeGroup> &group, ShapeDesc desc = {});
    void clear_shapes() noexcept;
    void load_mediums(const MediumsDesc &desc);
    void add_material(SP<Material> material) noexcept;
    void load_materials(const vector<MaterialDesc> &material_descs);
    void fill_instances();
    void add_light(TLight light) noexcept;
    template<typename T = Light>
    TObject<T, LightDesc> load_light(const LightDesc &desc) {
        OC_ASSERT(light_sampler_);
        auto ret = TObject<T, LightDesc>(desc);
        light_sampler_->add_light(ret);
        return ret;
    }
    void prepare_materials();
    [[nodiscard]] float3 world_center() const noexcept { return aabb_.center(); }
    [[nodiscard]] float world_radius() const noexcept { return ocarina::max(aabb_.radius(), min_radius_); }
    [[nodiscard]] float world_diameter() const noexcept { return world_radius() * 2; }
    void upload_data() noexcept;
    [[nodiscard]] ShapeInstance *get_instance(uint id) noexcept { return instances_[id].get(); }
    [[nodiscard]] const ShapeInstance *get_instance(uint id) const noexcept { return instances_[id].get(); }
};

#undef MAKE_GETTER

}// namespace vision