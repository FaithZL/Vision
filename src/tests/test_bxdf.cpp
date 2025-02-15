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

    float3 wo = normalize(make_float3(argc, argc * 2, argc * 3));
    std::uniform_real_distribution<> distrib(0.0, 1.0);
    float ax = 0.5f * argc;
    float ay = 0.16f * argc;

    uint num = 1000000 * argc;

    float ret = 0;

    Clock clk;

    float3 wh = make_float3(0);

    std::mt19937 gen(12345);
    for (int i = 1; i < num; ++i) {
        float2 u = make_float2(distrib(gen));
        float3 wo = make_float3(distrib(gen));
        wh += sample_wh<H>(wo, u, 0.5f, 0.7f, true, GGX);
    }

    cout << clk.elapse_ms() << endl;
    cout << to_str(wh) << endl;


    ret = 0;
    wh = make_float3(0);
    clk.start();
    std::mt19937 gen2(12345);
    for (int i = 1; i < num; ++i) {
        float2 u = make_float2(distrib(gen));
        float3 wo = make_float3(distrib(gen));
        wh += sample_wh<H>(wo, u, 0.5f, 0.7f, false, GGX);
    }

    cout << clk.elapse_ms() << endl;
    cout << to_str(wh) << endl;

    ret = 0;
    wh = make_float3(0);
    clk.start();
    std::mt19937 gen3(12345);
    for (int i = 1; i < num; ++i) {
        float2 u = make_float2(distrib(gen3));
        float3 wo = make_float3(distrib(gen3));
        wh += sample_wh_visible_area<H>(wo, u, 0.5f, 0.7f, GGX);
    }

    cout << clk.elapse_ms() << endl;
    cout << to_str(wh) << endl;
    return 0;

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