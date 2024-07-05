//
// Created by Zero on 2024/1/24.
//

#include "base/illumination/light.h"

namespace vision {

class MixEnvironment : public EnvironmentImpl {
private:
    EncodedData<float> scale0_;
    SP<EnvironmentImpl> env0_;
    EncodedData<float> scale1_;
    SP<EnvironmentImpl> env1_;

public:
    explicit MixEnvironment(const Desc &desc)
        : EnvironmentImpl(desc, LightType::Infinite) {}

    OC_ENCODABLE_FUNC(EnvironmentImpl, scale0_, *env0_, scale1_, *env1_)

    [[nodiscard]] float3 power() const noexcept override {
        return scale0_.hv() * env0_->power() + scale1_.hv() * env1_->power();
    }
};

}// namespace vision