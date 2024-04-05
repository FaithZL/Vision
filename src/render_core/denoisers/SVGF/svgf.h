//
// Created by Zero on 2024/2/15.
//

#include "core/basic_types.h"
#include "dsl/dsl.h"
#include "base/denoiser.h"
#include "reproject.h"
#include "filter_moments.h"
#include "atrous.h"
#include "modulate.h"
#include "utils.h"

namespace vision::svgf {
using namespace ocarina;

class SVGF : public Denoiser {
public:
    RegistrableBuffer<SVGFData> svgf_data;
    Buffer<float> history;

private:
    Reproject _reproject{this};
    FilterMoments _filter_moments{this};
    AtrousFilter _atrous{this};
    Modulate _modulate{this};

private:
    bool _switch{false};
    bool _moment_filter_switch{true};
    uint N;
    float _alpha{0.05f};
    float _moments_alpha{0.2f};
    uint _history_limit{32};
    int _moments_filter_radius{3};
    float _sigma_rt{10.f};
    float _sigma_normal{128.f};

public:
    explicit SVGF(const DenoiserDesc &desc)
        : Denoiser(desc),
          svgf_data(pipeline()->bindless_array()),
          N(desc["N"].as_uint(3)),
          _alpha(desc["alpha"].as_float(0.05f)),
          _moments_alpha(desc["moments_alpha"].as_float(0.2f)),
          _history_limit(desc["history_limit"].as_uint(32)),
          _moments_filter_radius(desc["moments_filter_radius"].as_int(3)),
          _sigma_rt(desc["sigma_rt"].as_float(10.f)),
          _sigma_normal(desc["sigma_normal"].as_float(30.f)) {}
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
