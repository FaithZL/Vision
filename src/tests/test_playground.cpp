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

float eta(float lambda) {
    float f = 1.03961212 * sqr(lambda) / (sqr(lambda) - 0.00600069867) +
        0.231792344 * sqr(lambda) / (sqr(lambda) - 0.0200179144) +
        1.01046945 * sqr(lambda) / (sqr(lambda) - 103.560653);
    return sqrt(f + 1);
}

int main(int argc, char *argv[]) {

    float e = eta(830);

    return 0;
}