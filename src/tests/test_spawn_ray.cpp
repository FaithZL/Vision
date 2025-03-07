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
//    file_manager.clear_cache();
    float3 vec = make_float3(2);
    float4x4 mat = scale(2);
    Transform tsf = Transform(mat);

    auto [vertices, triangle] = get_cube();

    Buffer v_buffer = device.create_buffer<float3>(vertices.size());
    Buffer t_buffer = device.create_buffer<Triangle>(triangle.size());

    ocarina::RHIMesh cube = device.create_mesh(v_buffer.view(), t_buffer.view());

    stream << v_buffer.upload_sync(vertices.data());
    stream << t_buffer.upload_sync(triangle.data());

    Accel accel = device.create_accel();
//    accel.add_mesh(cube, make_float4x4(1.f));
    stream << accel.build_bvh();
    stream << synchronize() << commit();


    Kernel kernel = [&]() {
        DynamicArray<float> arr = DynamicArray<float>::create(10.f, 9.f);
//        arr[0] = 9.6f;
        arr = arr.map([&](Float a) {
            return a * 2;
        });
        prints("----- {}", arr[0]);
    };
    auto shader = device.compile(kernel);
//    Stream stream = device.create_stream();
    stream << shader().dispatch(1) << synchronize() << commit();
    return 0;
}