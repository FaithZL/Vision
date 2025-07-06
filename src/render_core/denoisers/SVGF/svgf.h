//
// Created by Zero on 2024/2/15.
//

#include "math/basic_types.h"
#include "dsl/dsl.h"
#include "base/denoiser.h"
#include "reproject.h"
#include "filter_moments.h"
#include "atrous.h"
#include "modulator.h"
#include "utils.h"

namespace vision::svgf {
using namespace ocarina;

class SVGF : public Denoiser {
public:
    RegistrableBuffer<SVGFData> svgf_data;
    Buffer<float> history;

private:
    Reproject reproject_{this};
    FilterMoments filter_moments_{this};
    AtrousFilter atrous_{this};
    Modulator modulator_{this};

private:
    bool switch_{false};
    bool moment_filter_switch_{true};
    bool reproject_switch_{true};
    uint N;
    float alpha_{0.05f};
    float moments_alpha_{0.2f};
    uint history_limit_{32};
    int moments_filter_radius_{3};
    float sigma_rt_{10.f};
    float sigma_normal_{128.f};

public:
    SVGF() = default;
    explicit SVGF(const DenoiserDesc &desc)
        : Denoiser(desc),
          svgf_data(pipeline()->bindless_array()),
          N(desc["N"].as_uint(3)),
          alpha_(desc["alpha"].as_float(0.05f)),
          moments_alpha_(desc["moments_alpha"].as_float(0.2f)),
          history_limit_(desc["history_limit"].as_uint(32)),
          moments_filter_radius_(desc["moments_filter_radius"].as_int(3)),
          sigma_rt_(desc["sigma_rt"].as_float(10.f)),
          sigma_normal_(desc["sigma_normal"].as_float(30.f)) {}
    VS_MAKE_PLUGIN_NAME_FUNC
    OC_MAKE_MEMBER_GETTER(alpha, )
    OC_MAKE_MEMBER_GETTER(moments_alpha, )
    OC_MAKE_MEMBER_GETTER(moments_filter_radius, )
    OC_MAKE_MEMBER_GETTER(sigma_rt, )
    OC_MAKE_MEMBER_GETTER(sigma_normal, )
    OC_MAKE_MEMBER_GETTER(history_limit, )
    void prepare_buffers();
    void render_sub_UI(ocarina::Widgets *widgets) noexcept override;
    [[nodiscard]] uint svgf_data_base() const noexcept { return svgf_data.index().hv(); }
    [[nodiscard]] uint cur_svgf_index(uint frame_index) const noexcept;
    [[nodiscard]] uint prev_svgf_index(uint frame_index) const noexcept;
    [[nodiscard]] BufferView<SVGFData> cur_svgf_buffer(uint frame_index) const noexcept;
    [[nodiscard]] BufferView<SVGFData> prev_svgf_buffer(uint frame_index) const noexcept;
    void prepare() noexcept override;
    void compile() noexcept override;
    [[nodiscard]] CommandList dispatch(vision::RealTimeDenoiseInput &input) noexcept override;
    [[nodiscard]] static Float cal_weight(const Float &cur_depth, const Float &neighbor_depth, const Float &sigma_depth,
                                          const Float3 &cur_normal, const Float3 &neighbor_normal, const Float &sigma_normal,
                                          const Float &cur_illumi, const Float &neighbor_illumi, const Float &sigma_illumi) noexcept;
};

}// namespace vision::svgf
