//
// Created by Zero on 09/09/2022.
//

#include "base/light.h"
#include "core/render_pipeline.h"

namespace vision {

class AreaLight : public Light {
private:
    uint _inst_idx{InvalidUI32};
    bool _two_sided{false};
    Distribution *_distribution{nullptr};

public:
    explicit AreaLight(const LightDesc &desc)
        : Light(desc, LightType::Area),
          _two_sided{desc.two_sided}, _inst_idx(desc.inst_id) {}

    void prepare(RenderPipeline *rp) noexcept override {
        _distribution = rp->scene().load_distribution();
        Shape *shape = rp->scene().get_shape(_inst_idx);
        vector<float> weights = shape->surface_area();
        _distribution->build(std::move(weights));
        _distribution->prepare(rp);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::AreaLight)