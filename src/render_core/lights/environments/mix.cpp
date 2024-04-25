//
// Created by Zero on 2024/1/24.
//

#include "base/illumination/light.h"

namespace vision {

class MixEnvironment : public EnvironmentImpl {
private:
    Serial<float> _scale0;
    SP<EnvironmentImpl> _env0;
    Serial<float> _scale1;
    SP<EnvironmentImpl> _env1;

public:
    explicit MixEnvironment(const Desc &desc)
        : EnvironmentImpl(desc, LightType::Infinite) {}

    OC_SERIALIZABLE_FUNC(EnvironmentImpl, _scale0, *_env0, _scale1, *_env1)

    [[nodiscard]] float3 power() const noexcept override {
        return _scale0.hv() * _env0->power() + _scale1.hv() * _env1->power();
    }
};

}// namespace vision