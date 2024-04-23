//
// Created by Zero on 2024/2/10.
//

#include "filter.h"
#include "GUI/widgets.h"
#include "base/mgr/pipeline.h"

namespace vision {

bool OldFilter::render_UI(ocarina::Widgets *widgets) noexcept {
    bool ret = widgets->use_folding_header(
        ocarina::format("{} filter", impl_type().data()),
        [&] {
            float2 &r = radius_.hv();
            changed_ |= widgets->drag_float2("radius", addressof(r), 0.1,0.01,10);
            render_sub_UI(widgets);
        });
    return ret;
}

vector<float> OldFilter::discretize(ocarina::uint width) const noexcept {
    auto mapping = [&](int i) {
        return -radius_.hv().x + (2 * radius_.hv().x) * (i + 0.5f) / width;
    };
    vector<float> ret;
    uint size = width * width;
    ret.resize(size);
    for (int i = 0; i < size; ++i) {
        uint x = i % width;
        uint y = i / width;
        float fx = mapping(x);
        float fy = mapping(y);
        ret[i] = evaluate(make_float2(fx, fy));
    }
    float sum = std::accumulate(ret.begin(), ret.end(), 0.f);
    std::transform(ret.begin(), ret.end(), ret.begin(), [&](float x) {
        return x / sum;
    });
    return ret;
}

vector<float> Filter::Impl::discretize(ocarina::uint width) const noexcept {
    auto mapping = [&](int i) {
        return -radius_.hv().x + (2 * radius_.hv().x) * (i + 0.5f) / width;
    };
    vector<float> ret;
    uint size = width * width;
    ret.resize(size);
    for (int i = 0; i < size; ++i) {
        uint x = i % width;
        uint y = i / width;
        float fx = mapping(x);
        float fy = mapping(y);
        ret[i] = evaluate(make_float2(fx, fy));
    }
    float sum = std::accumulate(ret.begin(), ret.end(), 0.f);
    std::transform(ret.begin(), ret.end(), ret.begin(), [&](float x) {
        return x / sum;
    });
    return ret;
}

bool Filter::Impl::render_UI(ocarina::Widgets *widgets) noexcept {
    bool ret = widgets->use_folding_header(
        ocarina::format("{} filter", impl_type().data()),
        [&] {
            float2 &r = radius_.hv();
            changed_ |= widgets->drag_float2("radius", addressof(r), 0.1,0.01,10);
            render_sub_UI(widgets);
        });
    return ret;
}

}// namespace vision
