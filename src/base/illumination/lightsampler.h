//
// Created by Zero on 09/09/2022.
//

#pragma once

#include <utility>
#include "dsl/dsl.h"
#include "base/node.h"
#include "core/stl.h"
#include "light.h"
#include "math/warp.h"
#include "base/scattering/interaction.h"
#include "UI/polymorphic.h"
#include "hotfix/hotfix.h"

namespace vision {
using namespace ocarina;

struct SampledLight {
    Uint light_index;
    Float PMF;
};

class Sampler;

class LightSampler : public Node, public Observer {
public:
    using Desc = LightSamplerDesc;

protected:
    PolymorphicGUI<TLight> lights_;
    TEnvironment env_light_{};
    bool env_separate_{false};
    float env_prob_{};

protected:
    [[nodiscard]] virtual SampledLight select_light_(const LightSampleContext &lsc, const Float &u) const noexcept = 0;
    [[nodiscard]] virtual Float PMF_(const LightSampleContext &lsc, const Uint &index) const noexcept = 0;

public:
    LightSampler() = default;
    explicit LightSampler(const LightSamplerDesc &desc);
    VS_HOTFIX_MAKE_RESTORE(Node, lights_, env_light_, env_separate_, env_prob_)
    void prepare() noexcept override;
    void update_device_data() noexcept;
    VS_MAKE_GUI_STATUS_FUNC(Node, lights_)
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    template<typename... Args>
    void set_mode(Args &&...args) noexcept { lights_.set_mode(OC_FORWARD(args)...); }
    void update_runtime_object(const vision::IObjectConstructor *constructor) noexcept override;
    [[nodiscard]] float env_prob() const noexcept {
        return (!env_light_) ? 0 : (lights_.empty() ? 1 : env_prob_);
    }
    [[nodiscard]] const Environment *env_light() const noexcept { return env_light_.get(); }
    [[nodiscard]] Environment *env_light() noexcept { return env_light_.get(); }
    [[nodiscard]] uint env_index() const noexcept { return env_light()->index(); }
    void tidy_up() noexcept;
    [[nodiscard]] const Polymorphic<TLight> &lights() const noexcept { return lights_; }
    [[nodiscard]] Polymorphic<TLight> &lights() noexcept { return lights_; }
    [[nodiscard]] uint light_num() const noexcept { return lights_.size(); }
    [[nodiscard]] uint punctual_light_num() const noexcept { return light_num() - environment_light_num(); }
    [[nodiscard]] uint environment_light_num() const noexcept { return static_cast<int>(bool(env_light_)); }
    [[nodiscard]] Uint correct_index(Uint index) const noexcept;
    void add_light(TLight light) noexcept { lights_.push_back(ocarina::move(light)); }
    [[nodiscard]] Float PMF(const LightSampleContext &lsc, const Uint &index) const noexcept;
    [[nodiscard]] SampledLight select_light(const LightSampleContext &lsc, Float u) const noexcept;
    [[nodiscard]] LightEval evaluate_hit_wi(const LightSampleContext &p_ref, const Interaction &it,
                                            const SampledWavelengths &swl, LightEvalMode mode = LightEvalMode::All) const noexcept;
    [[nodiscard]] LightEval evaluate_hit_point(const LightSampleContext &p_ref, const Interaction &it,
                                               const Float &pdf_wi,
                                               const SampledWavelengths &swl,
                                               Float *light_pdf_point = nullptr, LightEvalMode mode = LightEvalMode::All) const noexcept;
    [[nodiscard]] LightEval evaluate_miss_wi(const LightSampleContext &p_ref, Float3 wi,
                                             const SampledWavelengths &swl, LightEvalMode mode = LightEvalMode::All) const noexcept;
    [[nodiscard]] LightEval evaluate_miss_point(const LightSampleContext &p_ref, const Float3 &wi,
                                                const Float &pdf_wi,
                                                const SampledWavelengths &swl,
                                                Float *light_pdf_point = nullptr, LightEvalMode mode = LightEvalMode::All) const noexcept;
    [[nodiscard]] pair<Uint, Uint> extract_light_id(const Uint &index) const noexcept;
    [[nodiscard]] Uint combine_to_light_index(const Uint &type_id, const Uint &inst_id) const noexcept;
    [[nodiscard]] Uint extract_light_index(const Interaction &it) const noexcept;
    [[nodiscard]] LightSample sample_wi(const SampledLight &sampled_light,
                                        const LightSampleContext &lsc,
                                        const Float2 &u, const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] LightSample sample_wi(const LightSampleContext &lsc, TSampler &sampler,
                                        const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] LightSample evaluate_point(const LightSampleContext &lsc, const LightSurfacePoint &lsp,
                                             const SampledWavelengths &swl, LightEvalMode mode = LightEvalMode::All) const noexcept;
    [[nodiscard]] Float PDF_point(const LightSampleContext &lsc, const LightSurfacePoint &lsp,
                                  const Float &pdf_wi) const noexcept;
    [[nodiscard]] LightSurfacePoint sample_only(const LightSampleContext &lsc, TSampler &sampler) const noexcept;
    void dispatch_light(const Uint &id, const std::function<void(const Light *)> &func) const noexcept;
    void dispatch_light(const Uint &type_id, const Uint &inst_id, const std::function<void(const Light *)> &func) const noexcept;
    void dispatch_environment(const std::function<void(const Environment *)> &func) const noexcept;
    template<typename Func>
    void for_each(Func &&func) noexcept {
        if constexpr (std::invocable<Func, TLight>) {
            for (TLight light : lights_) {
                func(light);
            }
        } else {
            uint i = 0u;
            for (TLight light : lights_) {
                func(light, i++);
            }
        }
    }

    template<typename Func>
    void for_each(Func &&func) const noexcept {
        if constexpr (std::invocable<Func, SP<const Light>>) {
            for (const TLight &light : lights_) {
                func(light);
            }
        } else {
            uint i = 0u;
            for (const TLight &light : lights_) {
                func(light, i++);
            }
        }
    }
};

using TLightSampler = TObject<LightSampler>;

}// namespace vision