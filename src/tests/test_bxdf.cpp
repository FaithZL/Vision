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

    float2 u = make_float2(0.28, 0.28);

    float ax = 0.01;
    float ay = 0.01;


    string s = "SDaf";
//    cout << 'x' - 'a';

        cout << std::hex << _calculate_mask("wyzw");

    //    Microfacet<H> mf(ax, ay);
    //    float3 wo = normalize(make_float3(1,1,0.6));
    //    auto wh = mf.sample_wh(wo, u);
    //    wh = make_float3(0,0,1);
    //    auto pdf = mf.PDF_wh(wo, wh);
    //    cout << pdf << endl;
    //    auto wi = reflect(wo, wh);
    //    cout << format("{} {} {}", wh.x, wh.y, wh.z);
    //    print("{} {} {}", wh.x, wh.y, wh.z);
    //    auto brdf = mf.BRDF(wo, wi, make_float3(1.f), wo.z, wi.z);
    return 0;
}