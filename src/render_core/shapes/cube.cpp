//
// Created by Zero on 21/10/2022.
//

#include "base/shape.h"
#include "math/transform.h"

namespace vision {

class Cube : public vision::ShapeGroup {
public:
    using Super = vision::ShapeGroup;

public:
    explicit Cube(const ShapeDesc &desc) : Super(desc) {
        init(desc);
        post_init(desc);
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    void init(const ShapeDesc &desc) noexcept {
        Mesh mesh;
        float x = desc["x"].as_float(1.f);
        float y = desc["y"].as_float(1.f);
        float z = desc["z"].as_float(1.f);
        y = y == 0 ? x : y;
        z = z == 0 ? y : z;
        x = x / 2.f;
        y = y / 2.f;
        z = z / 2.f;

        auto P = vector<float3>{
            float3(-x, -y, z), float3(x, -y, z), float3(-x, y, z), float3(x, y, z), // +z
            float3(-x, y, -z), float3(x, y, -z), float3(-x, -y, -z), float3(x, -y, -z), // -z
            float3(-x, y, z), float3(x, y, z), float3(-x, y, -z), float3(x, y, -z),  // +y
            float3(-x, -y, z), float3(x, -y, z), float3(-x, -y, -z), float3(x, -y, -z), // -y
            float3(x, -y, z), float3(x, y, z), float3(x, y, -z), float3(x, -y, -z), // +x
            float3(-x, -y, z), float3(-x, y, z), float3(-x, y, -z), float3(-x, -y, -z), // -x
        };

        auto N = vector<float3>{
            float3(0, 0, 1), float3(0, 0, 1), float3(0, 0, 1), float3(0, 0, 1),
            float3(0, 0, -1), float3(0, 0, -1), float3(0, 0, -1), float3(0, 0, -1),
            float3(0, 1, 0), float3(0, 1, 0), float3(0, 1, 0), float3(0, 1, 0),
            float3(0, -1, 0), float3(0, -1, 0), float3(0, -1, 0), float3(0, -1, 0),
            float3(1, 0, 0), float3(1, 0, 0), float3(1, 0, 0), float3(1, 0, 0),
            float3(-1, 0, 0), float3(-1, 0, 0), float3(-1, 0, 0), float3(-1, 0, 0),
        };

        auto UVs = vector<float2>{
            float2(0, 0), float2(1, 0), float2(0, 1), float2(1, 1),
            float2(0, 1), float2(1, 1), float2(0, 0), float2(1, 0),
            float2(0, 1), float2(1, 1), float2(0, 0), float2(1, 0),
            float2(0, 1), float2(1, 1), float2(0, 0), float2(1, 0),
            float2(0, 1), float2(1, 1), float2(1, 0), float2(0, 0),
            float2(0, 1), float2(1, 1), float2(1, 0), float2(0, 0),
        };
        
        mesh.set_triangles(vector<Triangle>{
            Triangle(0, 1, 3), Triangle(0, 3, 2),
            Triangle(6, 5, 7), Triangle(4, 5, 6),
            Triangle(10, 9, 11), Triangle(8, 9, 10),
            Triangle(13, 14, 15), Triangle(13, 12, 14),
            Triangle(18, 17, 19), Triangle(17, 16, 19),
            Triangle(21, 22, 23), Triangle(20, 21, 23),
        });
        for (int i = 0; i < P.size(); ++i) {
            mesh.vertices().emplace_back(P[i], N[i], UVs[i]);
        }
        mesh.update_data();
        add_instance(ShapeInstance(std::move(mesh)));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Cube)