//
// Created by Zero on 05/04/2023.
//

#include "base/color/spd.h"
#include "base/shader_graph/shader_node.h"

namespace vision {

class SPDNode : public ShaderNode {
private:
    SPD spd_{nullptr};

public:
    SPDNode() = default;
    explicit SPDNode(const ShaderNodeDesc &desc)
        : ShaderNode(desc), spd_(scene().pipeline()) {
        spd_.init(desc["value"].data());
    }
    OC_ENCODABLE_FUNC(ShaderNode, spd_)
    VS_HOTFIX_MAKE_RESTORE(ShaderNode, spd_)
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] vector<float> average() const noexcept override {
        float3 color = spd_.eval(rgb_spectrum_peak_wavelengths);
        return vector<float>{color.x, color.y, color.z};
    }
    ShaderNode &update_value(vector<float> values) noexcept override {
        spd_.init(std::move(values));
        spd_.upload();
        return *this;
    }
    void prepare() noexcept override {
        spd_.prepare();
    }
    [[nodiscard]] AttrEvaluation evaluate(const AttrEvalContext &ctx,
                                               const SampledWavelengths &swl) const noexcept override {
        return spd_.eval(swl);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, SPDNode)