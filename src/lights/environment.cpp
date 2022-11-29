//
// Created by Zero on 29/11/2022.
//

#include "base/light.h"
#include "base/render_pipeline.h"
#include "base/texture.h"
#include "math/warp.h"

namespace vision {

class EnvironmentLight : public Light {
private:
    Warper2D warper;
    Texture *_texture{nullptr};
    float4x4 _o2w;
    float _scale{1.f};

public:

};

}// namespace vision