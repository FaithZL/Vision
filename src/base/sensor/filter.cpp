//
// Created by Zero on 2024/2/10.
//

#include "filter.h"

namespace vision {

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
    return ret;
}

}// namespace vision
