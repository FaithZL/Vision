//
// Created by Zero on 21/10/2022.
//

#include "base/shape.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/Subdivision.h>
#include <assimp/scene.h>

namespace vision {

class Model : public Shape {
private:
    vector<Mesh> _meshes;

public:
    explicit Model(const ShapeDesc &desc) : Shape(desc) {
        load(desc);
    }

    const aiScene *load_scene(const fs::path &fn, Assimp::Importer &ai_importer,
                              bool swap_handed, bool smooth, bool flip_uv) {
        ai_importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS,
                                       aiComponent_COLORS |
                                           aiComponent_BONEWEIGHTS |
                                           aiComponent_ANIMATIONS |
                                           aiComponent_LIGHTS |
                                           aiComponent_CAMERAS |
                                           aiComponent_TEXTURES |
                                           aiComponent_MATERIALS);
        OC_INFO("Loading triangle mesh: ", fn);
        aiPostProcessSteps normal_flag = smooth ? aiProcess_GenSmoothNormals : aiProcess_GenNormals;
        aiPostProcessSteps flip_uv_flag = flip_uv ? aiProcess_FlipUVs : aiPostProcessSteps(0);
        auto post_process_steps = aiProcess_JoinIdenticalVertices |
                                  normal_flag |
                                  aiProcess_PreTransformVertices |
                                  aiProcess_ImproveCacheLocality |
                                  aiProcess_FixInfacingNormals |
                                  aiProcess_FindInvalidData |
                                  aiProcess_FindDegenerates |
                                  aiProcess_GenUVCoords |
                                  aiProcess_TransformUVCoords |
                                  aiProcess_OptimizeMeshes |
                                  aiProcess_Triangulate |
                                  flip_uv_flag;
        post_process_steps = swap_handed ?
                                 post_process_steps | aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder :
                                 post_process_steps;
        auto ai_scene = ai_importer.ReadFile(fn.string().c_str(),
                                             post_process_steps);

        return ai_scene;
    }

    void load(const ShapeDesc &desc) noexcept {
        fs::path directory = desc.fn.parent_path();
        Assimp::Importer ai_importer;
        const aiScene *ai_scene = load_scene(desc.fn, ai_importer, desc.swap_handed,
                                             desc.smooth, desc.flip_uv);
    }

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