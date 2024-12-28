//
// Created by Zero on 21/10/2022.
//

#include "base/shape.h"
#include "math/transform.h"
#include "base/mgr/mesh_registry.h"

namespace vision {

class Quad : public vision::ShapeGroup {
public:
    using Super = vision::ShapeGroup;

public:
    explicit Quad(const ShapeDesc &desc) : Super(desc) {
        init(desc);
        post_init(desc);
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    void init(const ShapeDesc &desc) noexcept {
        Mesh mesh;
        float width = desc["width"].as_float(1.f) / 2;
        float height = desc["height"].as_float(1.f) / 2;
        vector<float3> P{make_float3(width, 0, height),
                         make_float3(width, 0, -height),
                         make_float3(-width, 0, height),
                         make_float3(-width, 0, -height)};

        vector<float3> N(4, make_float3(0, 0, 0));
        vector<float2> UV{make_float2(1, 1),
                          make_float2(1, 0),
                          make_float2(0, 1),
                          make_float2(0, 0)};
        for (int i = 0; i < P.size(); ++i) {
            mesh.vertices().emplace_back(P[i], N[i], UV[i]);
        }
        float3 p0 = mesh.vertices()[0].position();
        float3 p1 = mesh.vertices()[1].position();
        float3 p2 = mesh.vertices()[2].position();
        float3 dp02 = p0 - p2;
        float3 dp12 = p1 - p2;
        float3 ng_un = cross(dp02, dp12);
        mesh.set_triangles({Triangle{0, 1, 2}, Triangle{2, 1, 3}});
        for (Vertex &vertex : mesh.vertices()) {
            vertex.set_normal(ng_un);
        }
        mesh.update_data();
        add_instance(ShapeInstance(std::move(mesh)));
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Quad)