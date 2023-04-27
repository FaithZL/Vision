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
        : ShaderNode(desc), _spd(desc.scene->render_pipeline()) {
        _spd.init(desc["value"].data());
    }
    OC_SERIALIZABLE_FUNC(float, _spd)

    void prepare() noexcept override {
        _spd.prepare();
    }

    [[nodiscard]] Array<float> evaluate(const AttrEvalContext &ctx,
                                        const SampledWavelengths &swl,
                                        const DataAccessor<float> *da) const noexcept override {
        Uint index = da->byte_read<uint>();
        return _spd.eval(index, swl);
    }
    [[nodiscard]] Array<float> evaluate(const AttrEvalContext &ctx,
                                        const SampledWavelengths &swl) const noexcept override {
        return _spd.eval(_spd.buffer_index().dv(), swl);
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::SPDNode)