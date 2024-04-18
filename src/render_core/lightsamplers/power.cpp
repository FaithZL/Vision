//
// Created by Zero on 28/10/2022.
//

#include "base/illumination/lightsampler.h"
#include "base/warper.h"
#include "base/mgr/pipeline.h"
#include "base/sampler.h"

namespace vision {
class PowerLightSampler : public LightSampler {
private:
    SP<Warper> _warper{};

protected:
    [[nodiscard]] Float PMF_(const LightSampleContext &lsc, const Uint &index) const noexcept override {
        return _warper->PMF(index);
    }

public:
    explicit PowerLightSampler(const LightSamplerDesc &desc)
        : LightSampler(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] SampledLight select_light_(const LightSampleContext &lsc, const Float &u) const noexcept override {
        SampledLight ret;
        ret.light_index = _warper->sample_discrete(u, nullptr, nullptr);
        ret.PMF = PMF_(lsc, ret.light_index);
        return ret;
    }

    void prepare() noexcept override {
        LightSampler::prepare();
        _warper = scene().load_warper();
        vector<float> weights;
        if (env_separate_) {
            lights_.for_each_instance([&](SP<Light> light) {
                float weight = 0;
                if (!light->match(LightType::Infinite)) {
                    weight = luminance(light->power());
                }
                weights.push_back(weight);
            });
        } else {
            lights_.for_each_instance([&](SP<Light> light) {
                weights.push_back(luminance(light->power()));
            });
        }
        _warper->build(std::move(weights));
        _warper->prepare();
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::PowerLightSampler)