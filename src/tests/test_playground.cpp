//
// Created by Zero on 15/09/2022.
//

#include <math/box.h>
#include "core/stl.h"
#include "rhi/common.h"
#include "core/string_util.h"
#include "math/transform.h"
#include "math/constants.h"

using namespace vision;

int main(int argc, char *argv[]) {
    fs::path path(argv[0]);
    Context context(path.parent_path());
    Device device = context.create_device("cuda");
    Stream stream = device.create_stream();

    float3 vec = make_float3(2);
    float4x4 mat = scale(2);
    Transform tsf = Transform(mat);

    vec = tsf.apply_point(vec);

    Kernel kernel = [&]() {
        Var s = 2.f;
        Float3 vec = make_float3(s);
        Float4x4 mat = make_float4x4(s);
        vec = Transform(mat).apply_vector(vec);
    };
    auto shader = device.compile(kernel);
    return 0;
}