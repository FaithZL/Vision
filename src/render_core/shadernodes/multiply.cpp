//
// Created by Zero on 2023/4/18.
//

#include "base/shader_graph/shader_node.h"
#include "base/mgr/scene.h"

namespace vision {

//{
//    "type" : "multiply",
//        "param" : {
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
class Multiply : public ShaderNode {
private:
    VS_MAKE_SLOT(lhs)
    VS_MAKE_SLOT(rhs)

public:
    explicit Multiply(const ShaderNodeDesc &desc)
        : ShaderNode(desc) {
        _lhs.set(Global::node_mgr().create_slot(*desc.slot("lhs")));
        _rhs.set(Global::node_mgr().create_slot(*desc.slot("rhs")));
    }

    OC_SERIALIZABLE_FUNC(ShaderNode, _lhs, _rhs)
    VS_MAKE_PLUGIN_NAME_FUNC
    VS_MAKE_GUI_STATUS_FUNC(ShaderNode, _lhs, _rhs)

    bool render_UI(ocarina::Widgets *widgets) noexcept override {
        bool ret = widgets->use_tree(_name, [&] {
            widgets->text("type:%s", impl_type().data());
            _lhs.render_UI(widgets);
            _rhs.render_UI(widgets);
        });
        return ret;
    }

    [[nodiscard]] bool is_uniform() const noexcept override {
        return _lhs->is_uniform() && _rhs->is_uniform();
    }
    [[nodiscard]] bool is_constant() const noexcept override {
        return _lhs->is_constant() && _rhs->is_constant();
    }
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        return hash64(_lhs.hash(), _rhs.hash());
    }
    [[nodiscard]] uint64_t _compute_type_hash() const noexcept override {
        return hash64(_lhs.type_hash(), _rhs.type_hash());
    }
    [[nodiscard]] ocarina::vector<float> average() const noexcept override {
        return _lhs.average() * _rhs.average();
    }
    [[nodiscard]] DynamicArray<float> evaluate(const AttrEvalContext &ctx,
                                               const SampledWavelengths &swl) const noexcept override {
        return _lhs.evaluate(ctx, swl) * _rhs.evaluate(ctx, swl);
    }
};
}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Multiply)