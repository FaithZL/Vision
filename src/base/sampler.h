//
// Created by Zero on 09/09/2022.
//

#pragma once

#include "dsl/dsl.h"
#include "node.h"
#include "base/sensor/photosensory.h"
#include "base/sensor/filter.h"

namespace vision {
using namespace ocarina;

enum Dimension {
    PathTracing,
    ReSTIR_RIS,
    ReSTIR_combine,
    ReSTIRGI_init,
    ReSTIRGI_temporal,
    ReSTIRGI_spatial
};

class Sampler : public Node {
public:
    using Desc = SamplerDesc;

protected:
    uint spp_{1u};

public:
    Sampler() = default;
    explicit Sampler(const SamplerDesc &desc)
        : Node(desc), spp_(desc["spp"].as_uint(1u)) {}
    VS_HOTFIX_MAKE_RESTORE(Node, spp_)
    virtual void load_data() noexcept = 0;
    [[nodiscard]] virtual Float next_1d() noexcept = 0;
    [[nodiscard]] virtual bool is_valid() const noexcept = 0;
    bool render_UI(ocarina::Widgets *widgets) noexcept override {
        return widgets->use_folding_header(ocarina::format("{} sampler", impl_type().data()), [&] {
            render_sub_UI(widgets);
        });
    }
    void try_load_data() noexcept {
        if (is_valid()) {
            return;
        }
        load_data();
    }
    /**
     * performed without changing the state quantity
     * @param func
     */
    virtual void temporary(const ocarina::function<void(Sampler *)> &func) noexcept = 0;
    [[nodiscard]] virtual uint sample_per_pixel() const noexcept { return spp_; }
    virtual void start(const Uint2 &pixel, const Uint &sample_index, const Uint &dim) noexcept = 0;
    void start(const Uint2 &pixel, const Uint &sample_index, Dimension dim) noexcept {
        start(pixel, sample_index, to_underlying(dim));
    }
    [[nodiscard]] virtual Float2 next_2d() noexcept {
        Float x = next_1d();
        Float y = next_1d();
        return make_float2(x, y);
    }
    [[nodiscard]] SensorSample sensor_sample(const Uint2 &pixel, const TFilter &filter) {
        SensorSample ss;
        FilterSample fs = filter->sample(next_2d());
        ss.p_film = make_float2(pixel) + make_float2(0.5f) + fs.p;
        ss.p_lens = next_2d();
        ss.time = next_1d();
        ss.filter_weight = fs.weight;
        return ss;
    }
};

using TSampler = TObject<Sampler>;

}// namespace vision