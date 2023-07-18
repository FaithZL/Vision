//
// Created by Zero on 2023/7/17.
//

#include "assimp_util.h"

namespace vision {

vector<vision::Light *> AssimpUtil::parse_lights() noexcept {
    vector<vision::Light *> ret;
    return ret;
}

vector<vision::Material *> AssimpUtil::parse_materials() noexcept {
    vector<vision::Material *> ret;
    return ret;
}

vector<vision::Mesh> AssimpUtil::parse_meshes(bool parse_material,
                                              uint32_t subdiv_level) {
    std::vector<Mesh> meshes;
    const aiScene *ai_scene = _ai_scene;
    vector<aiMesh *> ai_meshes(ai_scene->mNumMeshes);
    if (subdiv_level != 0u) {
        auto subdiv = Assimp::Subdivider::Create(Assimp::Subdivider::CATMULL_CLARKE);
        subdiv->Subdivide(ai_scene->mMeshes, ai_scene->mNumMeshes, ai_meshes.data(), subdiv_level);
    } else {
        std::copy(ai_scene->mMeshes, ai_scene->mMeshes + ai_scene->mNumMeshes, ai_meshes.begin());
    }
    meshes.reserve(ai_meshes.size());
    for (const auto &ai_mesh : ai_meshes) {
        Box3f aabb;
        vector<Vertex> vertices;
        vertices.reserve(ai_mesh->mNumVertices);
        for (int i = 0; i < ai_mesh->mNumVertices; ++i) {
            auto ai_position = ai_mesh->mVertices[i];
            auto ai_normal = ai_mesh->mNormals[i];
            float3 position = make_float3(ai_position.x, ai_position.y, ai_position.z);
            aabb.extend(position);
            float3 normal = make_float3(ai_normal.x, ai_normal.y, ai_normal.z);
            float2 tex_coord;
            if (ai_mesh->mTextureCoords[0] != nullptr) {
                auto ai_tex_coord = ai_mesh->mTextureCoords[0][i];
                tex_coord = make_float2(ai_tex_coord.x, ai_tex_coord.y);
            } else {
                tex_coord = make_float2(0.f);
            }
            normal = has_invalid(normal) ? make_float3(0.f) : normal;
            tex_coord = has_invalid(tex_coord) ? make_float2(0.f) : tex_coord;
            vertices.emplace_back(position, normal, tex_coord);
        }

        vector<Triangle> triangle;
        triangle.reserve(ai_mesh->mNumFaces);
        for (int i = 0; i < ai_mesh->mNumFaces; ++i) {
            auto ai_face = ai_mesh->mFaces[i];
            if (ai_face.mNumIndices == 3) {
                triangle.emplace_back(ai_face.mIndices[0],
                                      ai_face.mIndices[1],
                                      ai_face.mIndices[2]);
            } else if (ai_face.mNumIndices == 4) {
                triangle.emplace_back(ai_face.mIndices[0],
                                      ai_face.mIndices[1],
                                      ai_face.mIndices[2]);
                triangle.emplace_back(ai_face.mIndices[0],
                                      ai_face.mIndices[2],
                                      ai_face.mIndices[3]);
            } else {
                //                    OC_WARNING("Only triangles and quads supported: ", ai_mesh->mName.data, " num is ",
                //                               ai_face.mNumIndices);
                continue;
            }
        }
        Mesh mesh(std::move(vertices), std::move(triangle));
        mesh.aabb = aabb;
        meshes.push_back(mesh);
    }
    return meshes;
}

const aiScene *AssimpUtil::load_scene(const fs::path &fn, bool swap_handed, bool smooth, bool flip_uv) {
    _ai_importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS,
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
    auto ai_scene = _ai_importer.ReadFile(fn.string().c_str(),
                                          post_process_steps);
    _ai_scene = ai_scene;
    return ai_scene;
}

}// namespace vision