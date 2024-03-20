//
// Created by Zero on 05/04/2023.
//

#include "base/color/spd.h"
#include "base/shader_graph/shader_node.h"

namespace vision {

class SPDNode : public ShaderNode {
private:
    SPD _spd{nullptr};

public:
    explicit SPDNode(const ShaderNodeDesc &desc)
        : ShaderNode(desc), _spd(scene().pipeline()) {
        _spd.init(desc["value"].data());
    }
    OC_SERIALIZABLE_FUNC(ShaderNode, _spd)
    VS_MAKE_PLUGIN_NAME_FUNC
    [[nodiscard]] vector<float> average() const noexcept override {
        float3 color = _spd.eval(rgb_spectrum_peak_wavelengths);
        return vector<float>{color.x, color.y, color.z};
    }
    void prepare() noexcept override {
        _spd.prepare();
    }
    [[nodiscard]] DynamicArray<float> evaluate(const AttrEvalContext &ctx,
                                        const SampledWavelengths &swl) const noexcept override {
        return _spd.eval(swl);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SPDNode)