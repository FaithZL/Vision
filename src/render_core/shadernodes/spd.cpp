//
// Created by Zero on 05/04/2023.
//

#include "base/color/spd.h"

namespace vision {

class SPDNode : public ShaderNode {
private:
//    SPD _spd;
public:
    explicit SPDNode(const ShaderNodeDesc &desc)
        : ShaderNode(desc) {}

    [[nodiscard]] uint data_size() const noexcept override {
        return sizeof(uint);
    }
    void fill_data(ManagedWrapper<float> &datas) const noexcept override {

    }

//    [[nodiscard]] Array<float> evaluate(const AttrEvalContext &ctx,
//                                        const SampledWavelengths &swl,
//                                        const DataAccessor *da) const noexcept override {
//    }
//    [[nodiscard]] Array<float> evaluate(const AttrEvalContext &ctx,
//                                        const SampledWavelengths &swl) const noexcept override {
//    }
};

}// namespace vision

//VS_MAKE_CLASS_CREATOR(vision::SPDNode)