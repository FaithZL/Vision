//
// Created by Zero on 29/11/2022.
//

#include "base/scattering/medium.h"
#include "base/sampler.h"
#include "base/mgr/pipeline.h"

namespace vision {

class HomogeneousMedium : public Medium {
private:
    EncodedData<float3> sigma_a_;
    EncodedData<float3> sigma_s_;
    EncodedData<float> g_;

public:
    HomogeneousMedium() = default;
    explicit HomogeneousMedium(const MediumDesc &desc)
        : Medium(desc),
          sigma_a_(desc.sigma_a["value"].as_float3()),
          sigma_s_(desc.sigma_s["value"].as_float3()),
          g_(desc.g["value"].as_float()) {}

    VS_MAKE_PLUGIN_NAME_FUNC
    OC_ENCODABLE_FUNC(Medium, sigma_a_, sigma_s_, g_)
    VS_HOTFIX_MAKE_RESTORE(Medium, sigma_a_, sigma_s_, g_)

    [[nodiscard]] Float3 sigma_t() const noexcept { return sigma_s() + sigma_a(); }
    [[nodiscard]] Float3 sigma_s() const noexcept { return *sigma_s_ * *scale_; }
    [[nodiscard]] Float3 sigma_a() const noexcept { return *sigma_a_ * *scale_; }

    [[nodiscard]] SampledSpectrum Tr(const Float &t, const SampledWavelengths &swl) const noexcept {
        SampledSpectrum sigma_t_sp = spectrum()->decode_to_unbound_spectrum(sigma_t(), swl).sample;
        return exp(-sigma_t_sp * min(RayTMax, t));
    }

    void render_sub_UI(ocarina::Widgets *widgets) noexcept override {
        Medium::render_sub_UI(widgets);
        changed_ |= widgets->color_edit("sigma_a", addressof(sigma_a_.hv()));
        changed_ |= widgets->color_edit("sigma_s", addressof(sigma_s_.hv()));
        changed_ |= widgets->drag_float("g", addressof(g_.hv()), 0.01, -1, 1);
    }

    [[nodiscard]] SampledSpectrum Tr(const RayVar &ray, const SampledWavelengths &swl,
                                     TSampler &sampler) const noexcept override {
        return Tr(length(ray->direction()) * ray->t_max(), swl);
    }

    [[nodiscard]] SampledSpectrum sample(const RayState &rs, Interaction &it, TSampler &sampler,
                                         const SampledWavelengths &swl) const noexcept override {
        RayVar ray = rs.ray;
        Uint medium_id = rs.medium;
        SampledSpectrum sigma_t_sp = spectrum()->decode_to_unbound_spectrum(sigma_t(), swl).sample;
        SampledSpectrum sigma_s_sp = spectrum()->decode_to_unbound_spectrum(sigma_s(), swl).sample;
        Uint channel = min(cast<uint>(sampler->next_1d() * swl.dimension()), swl.dimension() - 1);
        Float dist = -log(1 - sampler->next_1d()) / sigma_t_sp[channel];
        Float t = min(dist / length(ray->direction()), ray->t_max());
        Bool sampled_medium = t < ray->t_max();
        $if(sampled_medium) {
            it = Interaction(ray->at(t), -ray->direction(), true);
            it.init_phase(*g_, swl);
            it.set_medium(medium_id, medium_id);
        };
        SampledSpectrum tr = Tr(t, swl);
        SampledSpectrum density = select(sampled_medium, sigma_t_sp * tr, tr);
        Float pdf = density.average();
        SampledSpectrum ret = select(sampled_medium, tr * sigma_s_sp / pdf, tr / pdf);
        return ret;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, HomogeneousMedium)
VS_REGISTER_CURRENT_PATH(0, "vision-medium-homogeneous.dll")