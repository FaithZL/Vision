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

using namespace vision;

int main(int argc, char *argv[]) {
    fs::path path(argv[0]);
    Context context(path.parent_path());
    Device device = context.create_device("cuda");
    Stream stream = device.create_stream();
    context.clear_cache();
    float3 vec = make_float3(2);
    float4x4 mat = scale(2);
    Transform tsf = Transform(mat);

    auto ttt = Transform(scale(0.1f));

    auto m = rotation(make_float3(1.f), 30.f, false);

    vec = tsf.apply_point(vec);

    Kernel kernel = [&]() {
        Var f3 = make_float3(1,2,1);
        Frame frame = Frame(f3);

        f3 = frame.to_local(f3);
    };
    auto shader = device.compile(kernel);
    return 0;
}