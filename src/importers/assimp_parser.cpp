//
// Created by Zero on 2023/7/17.
//

#include "assimp_parser.h"
#include "base/mgr/global.h"

namespace vision {

vision::Light *AssimpParser::parse_light(aiLight *ai_light) noexcept {
    return nullptr;
}

vector<vision::Light *> AssimpParser::parse_lights() noexcept {
    vector<vision::Light *> ret;
    ret.reserve(_ai_scene->mNumLights);
    vector<aiLight *> ai_lights(_ai_scene->mNumLights);
    std::copy(_ai_scene->mLights, _ai_scene->mLights + _ai_scene->mNumLights, ai_lights.begin());
    for (auto ai_light : ai_lights) {
        ret.push_back(parse_light(ai_light));
    }
    return ret;
}

std::pair<string, float4> AssimpParser::parse_texture(const aiMaterial *mat, aiTextureType type) {
    string fn;
    for (size_t i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString str;
        mat->GetTexture(type, i, &str);
        fn = str.C_Str();
        break;
    }

    aiColor3D ai_color;
    switch (type) {
        case aiTextureType_DIFFUSE:
            mat->Get(AI_MATKEY_COLOR_DIFFUSE, ai_color);
            break;
        case aiTextureType_SPECULAR:
            mat->Get(AI_MATKEY_COLOR_SPECULAR, ai_color);
            break;
        case aiTextureType_TRANSMISSION:
            mat->Get(AI_MATKEY_COLOR_TRANSPARENT, ai_color);
            break;
        case aiTextureType_HEIGHT:
            mat->Get(AI_MATKEY_BUMPSCALING, ai_color);
            break;
        default:
            break;
    }
    float4 color = make_float4(ai_color.r, ai_color.g, ai_color.b, 1);
    return std::make_pair(fn, color);
}

vision::MaterialDesc AssimpParser::parse_material(aiMaterial *ai_material) noexcept {
    // process diffuse
    auto diff = parse_texture(ai_material, aiTextureType_DIFFUSE);
    auto spec = parse_texture(ai_material, aiTextureType_SPECULAR);
    auto bump = parse_texture(ai_material, aiTextureType_HEIGHT);
    auto sheen = parse_texture(ai_material, aiTextureType_SHEEN);
    auto metal = parse_texture(ai_material, aiTextureType_METALNESS);
    auto rough = parse_texture(ai_material, aiTextureType_DIFFUSE_ROUGHNESS);
    auto refl = parse_texture(ai_material, aiTextureType_REFLECTION);
    auto cc = parse_texture(ai_material, aiTextureType_CLEARCOAT);
    auto trans = parse_texture(ai_material, aiTextureType_TRANSMISSION);
    MaterialDesc desc;
    desc.name = ai_material->GetName().C_Str();
    ParameterSet data = ParameterSet(DataWrap::object());
    data.set_value("type", "disney");
    DataWrap param = DataWrap::object();

    data.set_value("param", param);
    desc.init(data);
    return desc;
}

vector<vision::MaterialDesc> AssimpParser::parse_materials() noexcept {
    vector<vision::MaterialDesc> ret;
    ret.reserve(_ai_scene->mNumMaterials);
    vector<aiMaterial *> ai_materials(_ai_scene->mNumMaterials);
    std::copy(_ai_scene->mMaterials, _ai_scene->mMaterials + _ai_scene->mNumMaterials, ai_materials.begin());
    for (auto ai_material : ai_materials) {
        ret.push_back(parse_material(ai_material));
    }
    return ret;
}

vector<vision::Mesh> AssimpParser::parse_meshes(bool parse_material,
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

    vector<vision::MaterialDesc> materials;
    //    if (parse_material) {
    materials = parse_materials();
    //    }

    meshes.reserve(ai_meshes.size());
    for (const aiMesh *ai_mesh : ai_meshes) {
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

const aiScene *AssimpParser::load_scene(const fs::path &fn, bool swap_handed, bool smooth, bool flip_uv) {
    _directory = fn.parent_path();
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