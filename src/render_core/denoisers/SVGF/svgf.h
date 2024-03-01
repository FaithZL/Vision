//
// Created by Zero on 2024/2/15.
//

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "base/denoiser.h"
#include "reproject.h"
#include "filter_moment.h"
#include "atrous.h"
#include "data.h"

namespace vision {
using namespace ocarina;

using SVGFDataVar = Var<SVGFData>;

class SVGF : public Denoiser {
public:
    RegistrableBuffer<SVGFData> svgf_data;
    Buffer<float> history;

private:
    Reproject _reproject{this};
    FilterMoment _filter_moment{this};
    AtrousFilter _atrous;
    uint N;
    float _alpha{0.05f};
    float _moment_alpha{0.2f};

public:
    explicit SVGF(const DenoiserDesc &desc)
        : Denoiser(desc),
          svgf_data(pipeline()->bindless_array()),
          _atrous(desc.filter_desc, this),
          N(desc["N"].as_uint(3)),
          _alpha(desc["alpha"].as_float(0.05f)),
          _moment_alpha(desc["moment_alpha"].as_float(0.2f)) {}
    [[nodiscard]] string_view impl_type() const noexcept override { return VISION_PLUGIN_NAME; }
    OC_MAKE_MEMBER_GETTER(alpha, )
    OC_MAKE_MEMBER_GETTER(moment_alpha, )
    void prepare_buffers();
    [[nodiscard]] uint svgf_data_base() const noexcept { return svgf_data.index().hv(); }
    [[nodiscard]] uint cur_svgf_index(uint frame_index) const noexcept;
    [[nodiscard]] uint prev_svgf_index(uint frame_index) const noexcept;
    [[nodiscard]] BufferView<SVGFData> cur_svgf_buffer(uint frame_index) const noexcept;
    [[nodiscard]] BufferView<SVGFData> prev_svgf_buffer(uint frame_index) const noexcept;
    void prepare() noexcept override;
    void compile() noexcept override;
    [[nodiscard]] CommandList dispatch(vision::RealTimeDenoiseInput &input) noexcept override;
};

}// namespace vision
