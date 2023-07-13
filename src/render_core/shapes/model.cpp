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
    uint _mat_id{InvalidUI32};
    uchar _inside_medium{InvalidUI8};
    uchar _outside_medium{InvalidUI8};
    float4x4 _o2w;

protected:
    [[nodiscard]] uint64_t _compute_hash() const noexcept override {
        uint64_t ret = Hash64::default_seed;
        for (const Mesh &mesh : _meshes) {
            ret = hash64(mesh.hash(), ret);
        }
        return ret;
    }

public:
    explicit Model(const ShapeDesc &desc)
        : Shape(desc),
          _mat_id(desc.material.id),
          _o2w(desc.o2w.mat),
          _inside_medium(desc.inside_medium.id),
          _outside_medium(desc.outside_medium.id) {
        load(desc);
    }

    void set_lightmap_id(ocarina::uint id) noexcept override {
        for (vision::Mesh &mesh : _meshes) {
            mesh.set_lightmap_id(id);
        }
    }

    [[nodiscard]] const aiScene *load_scene(const fs::path &fn, Assimp::Importer &ai_importer,
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

    [[nodiscard]] vector<Mesh> process_mesh(const aiScene *ai_scene, uint32_t subdiv_level) {
        std::vector<Mesh> meshes;
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
                tex_coord = has_invalid(normal) ? make_float2(0.f) : tex_coord;
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
            mesh.handle().mat_id = _mat_id;
            mesh.handle().outside_medium = _outside_medium;
            mesh.handle().inside_medium = _inside_medium;
            mesh.handle().o2w = _o2w;
            mesh.handle().light_id = handle().light_id;
            mesh.aabb = aabb;
            this->aabb.extend(aabb);
            meshes.push_back(mesh);
        }
        return meshes;
    }

    [[nodiscard]] float4x4 o2w() const noexcept override { return _o2w; }

    void load(const ShapeDesc &desc) noexcept {
        auto fn = scene_path() / desc["fn"].as_string();
        Assimp::Importer ai_importer;
        const aiScene *ai_scene = load_scene(fn, ai_importer, desc["swap_handed"].as_bool(false),
                                             desc["smooth"].as_bool(false),
                                             desc["flip_uv"].as_bool(true));
        _meshes = process_mesh(ai_scene, desc["subdiv_level"].as_uint(0u));
    }

    void fill_geometry(Geometry &data) const noexcept override {
        for (const Mesh &mesh : _meshes) {
            mesh.fill_geometry(data);
        }
    }

    [[nodiscard]] Mesh &mesh_at(ocarina::uint i) noexcept override {
        return _meshes[i];
    }

    [[nodiscard]] const Mesh &mesh_at(ocarina::uint i) const noexcept override {
        return _meshes[i];
    }

    void for_each_mesh(const std::function<void(vision::Mesh &, uint)> &func) noexcept override {
        for (uint i = 0; i < _meshes.size(); ++i) {
            func(_meshes[i], i);
        }
    }

    void for_each_mesh(const std::function<void(const vision::Mesh &, uint)> &func) const noexcept override {
        for (uint i = 0; i < _meshes.size(); ++i) {
            func(_meshes[i], i);
        }
    }

    void update_material_id(uint id) noexcept override {
        for (Mesh &mesh : _meshes) {
            mesh.update_material_id(id);
        }
        _mat_id = id;
    }

    [[nodiscard]] vector<float> surface_areas() const noexcept override {
        vector<float> ret;
        for (const Mesh &mesh : _meshes) {
            auto v = mesh.surface_areas();
            append(ret, v);
        }
        return ret;
    }

    [[nodiscard]] vector<float> ref_surface_areas() const noexcept override {
        vector<float> ret;
        for (const Mesh &mesh : _meshes) {
            auto v = mesh.ref_surface_areas();
            append(ret, v);
        }
        return ret;
    }
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::Model)