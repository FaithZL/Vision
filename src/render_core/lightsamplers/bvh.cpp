//
// Created by Zero on 28/10/2022.
//

#include "base/illumination/lightsampler.h"
#include "base/mgr/pipeline.h"
#include "base/sampler.h"

namespace vision {

struct LightBVHNode {

};

class BVHLightSampler : public LightSampler {
public:
    explicit BVHLightSampler(const LightSamplerDesc &desc)
        : LightSampler(desc) {}

    void prepare() noexcept override {
        LightSampler::prepare();
        build_bvh();
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    void build_bvh() noexcept {

    }
};
}// namespace vision