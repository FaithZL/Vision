//
// Created by Zero on 15/09/2022.
//

#include <math/box.h>
#include "core/stl.h"
#include "rhi/common.h"
#include "core/string_util.h"
#include "math/transform.h"
#include "math/geometry.h"
#include "math/constants.h"
#include "plugins/spectra/srgb2spec.h"
#include "base/color/spd.h"

using namespace vision;
using namespace ocarina;

int main(int argc, char *argv[]) {

    float3 lambdas = make_float3(520, 586, 652);
    float3 eta = make_float3(1.5201596882463, 1.516865556749, 1.5144566604975);

    CauchyDispersion cauchy_dispersion(eta, lambdas);

    for (int i = 360; i < 830; i += 10) {
        cout << i << "   " << cauchy_dispersion.eta(i) << endl;
    }

    return 0;
}