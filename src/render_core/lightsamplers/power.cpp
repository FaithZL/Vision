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
    SP<Warper> warper_{};

protected:
    [[nodiscard]] Float PMF_(const LightSampleContext &lsc, const Uint &index) const noexcept override {
        return warper_->PMF(index);
    }

public:
    PowerLightSampler() =default;
    explicit PowerLightSampler(const LightSamplerDesc &desc)
        : LightSampler(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
    VS_HOTFIX_MAKE_RESTORE(LightSampler, warper_)
    [[nodiscard]] SampledLight select_light_(const LightSampleContext &lsc, const Float &u) const noexcept override {
        SampledLight ret;
        ret.light_index = warper_->sample_discrete(u, nullptr, nullptr);
        ret.PMF = PMF_(lsc, ret.light_index);
        return ret;
    }

    void prepare() noexcept override {
        LightSampler::prepare();
        warper_ = scene().load_warper();
        vector<float> weights;
        if (env_separate_) {
            lights_.for_each_instance([&](TLight light) {
                float weight = 0;
                if (!light->match(LightType::Infinite)) {
                    weight = luminance(light->power());
                }
                weights.push_back(weight);
            });
        } else {
            lights_.for_each_instance([&](TLight light) {
                weights.push_back(luminance(light->power()));
            });
        }
        warper_->allocate(weights.size());
        warper_->build(std::move(weights));
        warper_->upload_immediately();
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, PowerLightSampler)