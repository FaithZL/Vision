//
// Created by Zero on 2024/2/15.
//

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "base/denoiser.h"
#include "reproject.h"
#include "filter_moment.h"
#include "atrous.h"

namespace vision {
using namespace ocarina;
struct SVGFData {
    array<float, 3> illumination{};
    float variance{};
    float history{};
    float2 moments{};
};
}// namespace vision
// clang-format off
OC_STRUCT(vision::SVGFData, illumination, variance, history, moments) {

};
// clang-format on

namespace vision {
using namespace ocarina;

using SVGFDataVar = Var<SVGFData>;

class SVGF : public Denoiser {
public:
    Buffer<float4> prev_normal_depth;
    RegistrableBuffer<SVGFData> svgf_data;
    Buffer<float> history;

private:
    Reproject _reproject{this};
    FilterMoment _filter_moment{this};
    AtrousFilter _atrous;
    uint N;

public:
    explicit SVGF(const DenoiserDesc &desc)
        : Denoiser(desc),
          svgf_data(pipeline()->bindless_array()),
          _atrous(desc.filter_desc, this),
          N(desc["N"].as_uint(3)) {}
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    void prepare_buffers();
    [[nodiscard]] uint svgf_data_base() const noexcept { return svgf_data.index().hv(); }
    void prepare() noexcept override;
    void compile() noexcept override;
    [[nodiscard]] CommandList dispatch(vision::DenoiseInput &input) noexcept override;
};

}// namespace vision
