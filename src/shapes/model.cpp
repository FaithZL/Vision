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

    void fill_device_data(DeviceData &data, size_t *inst_id) const noexcept override {
        for (const Mesh &mesh : _meshes) {
            mesh.fill_device_data(data, inst_id);
        }
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Model)