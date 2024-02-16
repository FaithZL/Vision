//
// Created by Zero on 2024/2/15.
//

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "base/denoiser.h"
#include "compute_gbuffer.h"
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

template<typename Func>
void for_each_neighbor(const Int2 &radius, Func func) {
    Int2 cur_pixel = make_int2(dispatch_idx().xy());
    Int2 res = make_int2(dispatch_dim().xy());
    Int x_start = cur_pixel.x - radius.x;
    x_start = max(0, x_start);
    Int x_end = cur_pixel.x + radius.x;
    x_end = min(x_end, res.x - 1);
    Int y_start = cur_pixel.y - radius.y;
    y_start = max(0, y_start);
    Int y_end = cur_pixel.y + radius.y;
    y_end = min(y_end, res.y - 1);
    $for(x, x_start, x_end + 1) {
        $for(y, y_start, y_end + 1) {
            func(make_int2(x, y));
        };
    };
}

using SVGFDataVar = Var<SVGFData>;

class SVGF : public Denoiser {
public:
    Buffer<float4> prev_normal_depth;
    RegistrableBuffer<SVGFData> svgf_data;
    Buffer<float> history;

private:
    ComputeGBuffer _compute_gbuffer{this};
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
    [[nodiscard]] uint cur_svgf_index(uint frame_index) const noexcept;
    [[nodiscard]] uint prev_svgf_index(uint frame_index) const noexcept;
    void prepare() noexcept override;
    void compile() noexcept override;
    [[nodiscard]] CommandList dispatch(vision::DenoiseInput &input) noexcept override;
};

}// namespace vision
