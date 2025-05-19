//
// Created by Zero on 2023/4/18.
//

#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"

namespace vision {

//{
//    "type" : "multiply",
//    "param" : {
//        "lhs" : {
//            "channels" : "xyz",
//            "node" : {
//                "type" : "image",
//                "param" : {
//                    "fn" : "checker.jpg",
//                    "color_space" : "srgb"
//                }
//            }
//        },
//        "rhs" : {
//            "channels" : "xyz",
//            "node" : {
//                "type": "number",
//                "param": {
//                    "value": [0.9,1,0.9]
//                }
//            }
//        }
//    }
//}
class BinaryOpNode : public ShaderNode {
private:
    VS_MAKE_SLOT(lhs)
    VS_MAKE_SLOT(rhs)
    std::string op_;

public:
    BinaryOpNode() = default;
    explicit BinaryOpNode(const ShaderNodeDesc &desc)
        : ShaderNode(desc) {
        lhs_.set(ShaderNodeSlot::create_slot(*desc.slot("lhs")));
        rhs_.set(ShaderNodeSlot::create_slot(*desc.slot("rhs")));
    }

    OC_ENCODABLE_FUNC(ShaderNode, lhs_, rhs_)
    VS_MAKE_PLUGIN_NAME_FUNC
    VS_MAKE_GUI_STATUS_FUNC(ShaderNode, lhs_, rhs_)
    VS_HOTFIX_MAKE_RESTORE(ShaderNode, lhs_, rhs_)

    bool render_UI(ocarina::Widgets *widgets) noexcept override {
        bool ret = widgets->use_tree(name_, [&] {
            widgets->text("type:%s", impl_type().data());
            lhs_.render_UI(widgets);
            rhs_.render_UI(widgets);
        });
        return ret;
    }

    [[nodiscard]] bool is_uniform() const noexcept override {
        return lhs_->is_uniform() && rhs_->is_uniform();
    }
    [[nodiscard]] bool is_constant() const noexcept override {
        return lhs_->is_constant() && rhs_->is_constant();
    }
    [[nodiscard]] uint64_t compute_hash() const noexcept override {
        return hash64(lhs_.hash(), rhs_.hash(), op_);
    }
    [[nodiscard]] uint64_t compute_topology_hash() const noexcept override {
        return hash64(lhs_.topology_hash(), rhs_.topology_hash(), op_);
    }
    [[nodiscard]] ocarina::vector<float> average() const noexcept override {
        return lhs_.average() * rhs_.average();
    }
    [[nodiscard]] DynamicArray<float> evaluate(const AttrEvalContext &ctx,
                                               const SampledWavelengths &swl) const noexcept override {
        if (op_ == "Add") {
            return lhs_.evaluate(ctx, swl) + rhs_.evaluate(ctx, swl);
        } else if (op_ == "Subtract") {
            return lhs_.evaluate(ctx, swl) - rhs_.evaluate(ctx, swl);
        } else if (op_ == "Multiply") {
            return lhs_.evaluate(ctx, swl) * rhs_.evaluate(ctx, swl);
        } else if (op_ == "Divide") {
            return lhs_.evaluate(ctx, swl) / rhs_.evaluate(ctx, swl);
        }
        OC_WARNING("fallback to mul");
        return lhs_.evaluate(ctx, swl) * rhs_.evaluate(ctx, swl);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR_HOTFIX(vision, BinaryOpNode)
