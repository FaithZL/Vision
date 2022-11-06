//
// Created by Zero on 21/10/2022.
//

#include "base/shape.h"

namespace vision {

class Model : public Shape {
private:
    vector<Mesh> _meshes;

public:
    explicit Model(const ShapeDesc &desc) : Shape(desc) {}

    void fill_geometry(Geometry &data) const noexcept override {
        for (const Mesh &mesh : _meshes) {
            mesh.fill_geometry(data);
        }
    }

    [[nodiscard]] vector<float> surface_area() const noexcept override {
        vector<float> ret;
        for (const Mesh &mesh : _meshes) {
            auto v = mesh.surface_area();
            ret.insert(ret.cend(), v.cbegin(), v.cend());
        }
        return ret;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Model)