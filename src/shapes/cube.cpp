//
// Created by Zero on 21/10/2022.
//

#include "base/shape.h"

#include "base/shape.h"

namespace vision {

class Cube : public vision::Mesh {
public:
    using Super = vision::Mesh;

public:
    explicit Cube(const ShapeDesc *desc) : Super(desc) {
        init(desc);
    }
    void init(const ShapeDesc *desc) noexcept {
        float x = desc->x;
        float y = desc->y;
        float z = desc->z;
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
        triangles = vector<Triangle>{
            Triangle(0, 1, 3), Triangle(0, 3, 2),
            Triangle(6, 5, 7), Triangle(4, 5, 6),
            Triangle(10, 9, 11), Triangle(8, 9, 10),
            Triangle(13, 14, 15), Triangle(13, 12, 14),
            Triangle(18, 17, 19), Triangle(17, 16, 19),
            Triangle(21, 22, 23), Triangle(20, 21, 23),
        };
        for (int i = 0; i < P.size(); ++i) {
            vertices.emplace_back(P[i], N[i], UVs[i]);
            aabb.extend(P[i]);
        }
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Cube)