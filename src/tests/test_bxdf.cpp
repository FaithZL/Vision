//
// Created by Zero on 09/11/2022.
//

#include <math/box.h>
#include "core/stl.h"
#include "rhi/common.h"
#include "core/string_util.h"
#include "math/transform.h"
#include "math/geometry.h"
#include "math/constants.h"
#include "base/microfacet.h"

using namespace vision;
using namespace ocarina;
int main(int argc, char *argv[]) {

    float2 u = make_float2(0.28, 0.28);

    float ax = 0.01;
    float ay = 0.01;

    Microfacet<H> mf(ax, ay);
    float3 wo = normalize(make_float3(1,1,0.6));
    auto wh = mf.sample_wh(wo, u);
//    wh = make_float3(0,0,1);
    auto pdf = mf.PDF_wh(wo, wh);
    cout << pdf << endl;
    auto wi = reflect(wo, wh);
    cout << format("{} {} {}", wh.x, wh.y, wh.z);
//    print("{} {} {}", wh.x, wh.y, wh.z);
    auto brdf = mf.BRDF(wo, wi, make_float3(1.f), wo.z, wi.z);
    return 0;
}