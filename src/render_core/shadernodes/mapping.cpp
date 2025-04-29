//
// Created by Zero on 07/03/2023.
//

#include "base/shader_graph/shader_node.h"

//todo

/**
 * input
 *
 * camera data
 * fresnel
 * geometry
 * Texture coord
 * color attr
 *
 * Color
 *
 * Brightness contrast
 * Gamma
 * HSV
 * invert color
 * RBG Curve
 *
 * Converter
 *
 * clamp
 * Color ramp
 * Combine color
 * Combine XYZ
 * Float curve
 * map range
 * math
 * mix
 * separate XYZ
 * separate Color
 *
 * shader
 *
 * emission
 *
 * texture
 * image
 * noisy
 *
 * vector
 * bump
 * displacement
 * mapping
 * normal
 * normal map
 * vector displacement
 * vector rotate
 * vector transform
 *
 *
 */
namespace vision {
class UVMapping : public ShaderNode {
private:
    EncodedData<float2> offset_{};
    EncodedData<float2> scale_{};

public:
    explicit UVMapping(const ShaderNodeDesc &desc)
        : ShaderNode(desc) {}
    VS_MAKE_PLUGIN_NAME_FUNC
};
}// namespace vision