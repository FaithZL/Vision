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

    [[nodiscard]] Float3 tr(const OCRay &ray, Sampler &sampler) const noexcept override {
        return make_float3(0.f);
    }

    [[nodiscard]] pair<Float3, Interaction> sample(const OCRay &ray, Sampler &sampler) const noexcept override {
        return {};
    }

};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::HomogeneousMedium)