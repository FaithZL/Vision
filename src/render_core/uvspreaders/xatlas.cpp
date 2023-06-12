//
// Created by Zero on 2023/6/2.
//

#include "base/uv_spreader.h"
#include "ext/xatlas/xatlas.h"

namespace vision {
using namespace ocarina;

class XAtlas : public UVSpreader {
public:
    explicit XAtlas(const UVSpreaderDesc &desc)
        : UVSpreader(desc) {}

    [[nodiscard]] static xatlas::MeshDecl mesh_decl(vision::Mesh *mesh) {
        xatlas::MeshDecl ret;
        return ret;
    }

    void apply(vision::Mesh *mesh) override {
        mesh->allocate_lightmap_uv();
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::XAtlas)