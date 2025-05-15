//
// Created by Zero on 10/10/2022.
//

#include "math/box.h"
#include "core/stl.h"
#include "rhi/common.h"
#include "core/string_util.h"
#include "math/transform.h"
#include "math/geometry.h"
#include "base/scattering/interaction.h"
#include "base/scattering/bxdf.h"

using namespace vision;
using namespace ocarina;

auto get_cube(float x = 1, float y = 1, float z = 1) {
    x = x / 2.f;
    y = y / 2.f;
    z = z / 2.f;
    auto vertices = vector<float3>{
        float3(-x, -y, z), float3(x, -y, z), float3(-x, y, z), float3(x, y, z),    // +z
        float3(-x, y, -z), float3(x, y, -z), float3(-x, -y, -z), float3(x, -y, -z),// -z
        float3(-x, y, z), float3(x, y, z), float3(-x, y, -z), float3(x, y, -z),    // +y
        float3(-x, -y, z), float3(x, -y, z), float3(-x, -y, -z), float3(x, -y, -z),// -y
        float3(x, -y, z), float3(x, y, z), float3(x, y, -z), float3(x, -y, -z),    // +x
        float3(-x, -y, z), float3(-x, y, z), float3(-x, y, -z), float3(-x, -y, -z),// -x
    };
    auto triangles = vector<Triangle>{
        Triangle(0, 1, 3),
        Triangle(0, 3, 2),
        Triangle(6, 5, 7),
        Triangle(4, 5, 6),
        Triangle(10, 9, 11),
        Triangle(8, 9, 10),
        Triangle(13, 14, 15),
        Triangle(13, 12, 14),
        Triangle(18, 17, 19),
        Triangle(17, 16, 19),
        Triangle(21, 22, 23),
        Triangle(20, 21, 23),
        };

    return ocarina::make_pair(vertices, triangles);
}

int main(int argc, char *argv[]) {
    fs::path path(argv[0]);
    FileManager &file_manager = FileManager::instance();
    Device device = file_manager.create_device("cuda");
    device.init_rtx();
    Stream stream = device.create_stream();
    Env::printer().init(device);
    uint count = 1;
    Kernel kernel = [&](Uint _) {
        outline("a",[&] {
            float_array ccc{123u};
//            ccc[122] = 256;
            outline("b", [&] {
                Uint i = 122;
                Float a = ccc[i];
////                $info("a = {} ---------", a);
////                outline("c", [&] {
////                    Uint i = 122;
////                    Float a = ccc[i];
////                    $info("a = {} ", a);
////                });
            });

        });
    };
    auto shader = device.compile(kernel);
    stream << shader(1u).dispatch(1) << Env::printer().retrieve()<< synchronize() << commit();
    return 0;
}