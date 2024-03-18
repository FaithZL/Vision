//
// Created by Zero on 2024/2/10.
//

#include "filter.h"
#include "GUI/widgets.h"
#include "base/mgr/pipeline.h"

namespace vision {

bool Filter::render_UI(ocarina::Widgets *widgets) noexcept {
    bool dirty = false;
    bool ret = widgets->use_tree("filter", [&] {
        widgets->text("type: %s", impl_type().data());
        float2 &r = _radius.hv();
        dirty |= widgets->slider_float2("radius", &r, 0.01, 5);
    });
    pipeline()->invalidate(dirty);
    return ret;
}

vector<float> Filter::discretize(ocarina::uint width) const noexcept {
    auto mapping = [&](int i) {
        return -_radius.hv().x + (2 * _radius.hv().x) * (i + 0.5f) / width;
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

}// namespace vision
