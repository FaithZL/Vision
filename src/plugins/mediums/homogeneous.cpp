//
// Created by Zero on 29/11/2022.
//

#include "base/scattering/medium.h"
#include "base/sampler.h"

namespace vision {

class HomogeneousMedium : public Medium {
private:
    float3 _sigma_a;
    float3 _sigma_s;
    float3 _sigma_t;
    float _g;

public:
    explicit HomogeneousMedium(const MediumDesc &desc)
        : Medium(desc),
          _sigma_a(desc.sigma_a),
          _sigma_s(desc.sigma_s),
          _sigma_t(_sigma_a + _sigma_s),
          _g(desc.g) {}

    [[nodiscard]] SampledSpectrum Tr(Float t, const SampledWavelengths &swl) const noexcept {
        // todo
        return SampledSpectrum{swl.dimension(), 0.f};
//        return exp(-_sigma_t * min(RayTMax, t));
    }

    [[nodiscard]] SampledSpectrum Tr(const OCRay &ray, const SampledWavelengths &swl,
                             Sampler *sampler) const noexcept override {
        return Tr(length(ray->direction()) * ray->t_max(), swl);
    }

    [[nodiscard]] SampledSpectrum sample(const OCRay &ray, Interaction &it,
                                 const SampledWavelengths &swl,
                                 Sampler *sampler) const noexcept override {
        // todo
        return {swl.dimension(), 0.f};
//        Uint channel = min(cast<uint>(sampler->next_1d() * 3), 2u);
//        Float3 sigma_t = _sigma_t;
//        Float dist = -log(1 - sampler->next_1d()) / sigma_t[channel];
//        Float t = min(dist / length(ray->direction()), ray->t_max());
//        Bool sampled_medium = t < ray->t_max();
//        $if(sampled_medium) {
//            it = Interaction(ray->at(t), -ray->direction());
//            it.init_phase(_g, swl);
//            it.set_medium(_index, _index);
//        };
//        SampledSpectrum tr = Tr(t, swl);
//        Float3 density = select(sampled_medium, _sigma_t * tr, tr);
//        Float pdf = (density.x + density.y + density.z) / 3.f;
//        Float3 ret = select(sampled_medium, tr * _sigma_s / pdf, tr / pdf);
//        return ret;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::HomogeneousMedium)