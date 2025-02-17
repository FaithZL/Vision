//
// Created by Zero on 09/11/2022.
//

#include "math/box.h"
#include "core/stl.h"
#include "rhi/common.h"
#include "core/string_util.h"
#include "math/transform.h"
#include "math/geometry.h"
#include "base/scattering/microfacet.h"
#include <random>

using namespace vision;
using namespace ocarina;

[[nodiscard]] static uint _calculate_mask(const string &channels) noexcept {
    uint ret{};
    map<char, uint> dict{{'x', 0u}, {'y', 1u}, {'z', 2u}, {'w', 3u}};
    for (char channel : channels) {
        ret = (ret << 4) | dict[channel];
    }
    return ret;
}

int main(int argc, char *argv[]) {

    float3 wo = normalize(make_float3(1,0,-2));

    float3 wi;
    float eta = 1.5f;
    refract<H>(wo, make_float3(0,0,1), 1.5f , &wi);

    float3 wr = reflect(wo, make_float3(0,0,-1));

//    cout << to_str(wr) << endl;
//    cout << to_str(wo) << endl;
    cout << to_str(wi) << endl;

    refract<H>(wo, make_float3(0,0,-1), 1.5f , &wi);
    cout << to_str(wi) << endl;

    float3 wh = wo + eta * wi;
//
//    cout << to_str(wh) << endl;

    return 0;
}