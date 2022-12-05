//
// Created by Zero on 29/11/2022.
//

#include "base/medium.h"
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

    [[nodiscard]] Float3 Tr(Float t) const noexcept {
        return exp(-_sigma_t * min(RayTMax, t));
    }

    [[nodiscard]] Float3 Tr(const OCRay &ray, Sampler *sampler) const noexcept override {
        return Tr(length(ray->direction()) * ray->t_max());
    }

    [[nodiscard]] pair<Float3, Interaction> sample(const OCRay &ray, Sampler *sampler) const noexcept override {
        Uint channel = min(cast<uint>(sampler->next_1d() * 3), 2u);
        Float3 sigma_t = _sigma_t;
        Float dist = -log(1 - sampler->next_1d()) / sigma_t[channel];
        Float t = min(dist / length(ray->direction()), ray->t_max());
        Bool sampled_medium = t < ray->t_max();
        Interaction it;
        $if(sampled_medium) {
            it = Interaction(ray->at(t), -ray->direction());
            it.init_phase(_g);
        };
        Float3 tr = Tr(t);
        Float3 density = select(sampled_medium, _sigma_t * tr, tr);
        Float pdf = (density.x + density.y + density.z) / 3.f;
        Float3 ret = select(sampled_medium, tr * _sigma_s / pdf, tr / pdf);
        return {ret, it};
    }

};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::HomogeneousMedium)