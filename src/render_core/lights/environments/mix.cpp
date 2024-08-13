//
// Created by Zero on 2024/1/24.
//

#include "base/illumination/light.h"

namespace vision {

class MixEnvironment : public Environment {
private:
    EncodedData<float> scale0_;
    SP<Environment> env0_;
    EncodedData<float> scale1_;
    SP<Environment> env1_;

public:
    explicit MixEnvironment(const Desc &desc)
        : Environment(desc, LightType::Infinite) {}

    OC_ENCODABLE_FUNC(Environment, scale0_, *env0_, scale1_, *env1_)

    [[nodiscard]] float3 power() const noexcept override {
        return scale0_.hv() * env0_->power() + scale1_.hv() * env1_->power();
    }
};

}// namespace vision