//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "node.h"
#include "util/image_io.h"
#include "base/color/spectrum.h"
#include "base/scattering/interaction.h"

namespace vision {

class ShaderNode : public Node {
protected:
    ShaderNodeType _type{};

public:
    using Desc = ShaderNodeDesc;

public:
    [[nodiscard]] static bool is_zero(const ShaderNode *tex) noexcept {
        return tex ? tex->is_zero() : true;
    }
    [[nodiscard]] static bool nonzero(const ShaderNode *tex) noexcept {
        return !is_zero(tex);
    }

public:
    explicit ShaderNode(const ShaderNodeDesc &desc) : Node(desc), _type(desc.type) {}
    [[nodiscard]] virtual bool is_zero() const noexcept { return false; }
    [[nodiscard]] virtual Float4 eval(const AttrEvalContext &tec) const noexcept = 0;
    [[nodiscard]] virtual Float4 eval(const Float2 &uv) const noexcept {
        return eval(AttrEvalContext(uv));
    }
    [[nodiscard]] virtual ColorDecode eval_albedo_spectrum(const AttrEvalContext &tec,
                                                           const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] virtual ColorDecode eval_illumination_spectrum(const AttrEvalContext &tec,
                                                                 const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] virtual ColorDecode eval_albedo_spectrum(const Float2 &uv,
                                                           const SampledWavelengths &swl) const noexcept;
    [[nodiscard]] virtual ColorDecode eval_illumination_spectrum(const Float2 &uv,
                                                                 const SampledWavelengths &swl) const noexcept;
    virtual void for_each_pixel(const function<ImageIO::foreach_signature> &func) const noexcept {
        OC_ERROR("call error");
    }
    [[nodiscard]] virtual uint2 resolution() const noexcept { return make_uint2(0); }
};

template<uint dim = 1>
class Slot {
private:
    string _channel_mask{};
    const ShaderNode *_input{};

public:
    explicit Slot(const ShaderNode *input, string channel)
        : _input(input), _channel_mask(channel) {}
    //    Var<Vector<float, dim>>
};

}// namespace vision