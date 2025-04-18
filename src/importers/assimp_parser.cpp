//
// Created by Zero on 2023/7/17.
//

#include "assimp_parser.h"
#include "base/mgr/global.h"
#include "base/mgr/pipeline.h"

namespace vision {

float4x4 AssimpParser::parse_camera(aiCamera *ai_camera) noexcept {
    SensorDesc desc;
    DataWrap param = DataWrap::object();
    float3 pos = assimp::from_vec3(ai_camera->mPosition);
    float3 up = assimp::from_vec3(ai_camera->mUp);
    float3 target_pos = assimp::from_vec3(ai_camera->mLookAt);
    return look_at<H>(pos, target_pos, up);
}

vector<float4x4> AssimpParser::parse_cameras() noexcept {
    vector<float4x4> ret;
    ret.reserve(ai_scene_->mNumCameras);
    vector<aiCamera *> ai_cameras(ai_scene_->mNumCameras);
    std::copy(ai_scene_->mCameras, ai_scene_->mCameras + ai_scene_->mNumCameras, ai_cameras.begin());
    for (auto ai_camera : ai_cameras) {
        ret.push_back(parse_camera(ai_camera));
    }
    return ret;
}

TLight AssimpParser::point_light(aiLight *ai_light) noexcept {
    LightDesc desc;
    DataWrap param = DataWrap::object();

    auto color = assimp::from_color3(ai_light->mColorDiffuse);
    param["color"] = to_json(color);

    auto pos = assimp::from_vec3(ai_light->mPosition);
    param["position"] = to_json(pos);
    
    DataWrap ps = DataWrap::object();
    ps["param"] = param;
    ps["type"] = "point";
    desc.init(ps);
    auto ret = TLight(desc);
    return ret;
}

TLight AssimpParser::area_light(aiLight *ai_light) noexcept {
    return TLight(nullptr);
}

TLight AssimpParser::spot_light(aiLight *ai_light) noexcept {
    LightDesc desc;
    DataWrap param = DataWrap::object();
    auto color = assimp::from_color3(ai_light->mColorDiffuse);
    param["color"] = to_json(color);

    float angle = degrees(ai_light->mAngleOuterCone);
    param["angle"] = angle;

    float falloff = angle - degrees(ai_light->mAngleInnerCone);
    param["falloff"] = falloff;

    auto pos = assimp::from_vec3(ai_light->mPosition);
    param["position"] = to_json(pos);

    auto direction = assimp::from_vec3(ai_light->mDirection);
    param["direction"] = to_json(direction);

    DataWrap ps = DataWrap::object();
    ps["param"] = param;
    ps["type"] = "spot";
    desc.init(ps);
    auto ret = TLight(desc);
    return ret;
}

TLight AssimpParser::environment(aiLight *ai_light) noexcept {
    return TLight(nullptr);
}

TLight AssimpParser::directional_light(aiLight *ai_light) noexcept {
    LightDesc desc;
    DataWrap param = DataWrap::object();

    auto color = assimp::from_color3(ai_light->mColorDiffuse);
    param["color"] = to_json(color);

    auto dir = assimp::from_vec3(ai_light->mDirection);
    param["direction"] = to_json(dir);

    DataWrap ps = DataWrap::object();
    ps["param"] = param;
    ps["type"] = "directional";
    desc.init(ps);
    auto ret = TLight(desc);
    return ret;
}

TLight AssimpParser::parse_light(aiLight *ai_light) noexcept {
    switch (ai_light->mType) {
        case aiLightSource_POINT:
            return point_light(ai_light);
        case aiLightSource_DIRECTIONAL:
            return directional_light(ai_light);
        case aiLightSource_SPOT:
            return spot_light(ai_light);
        default:
            break;
    }
    return TLight(nullptr);
}

vector<TLight> AssimpParser::parse_lights() noexcept {
    vector<TLight> ret;
    ret.reserve(ai_scene_->mNumLights);
    vector<aiLight *> ai_lights(ai_scene_->mNumLights);
    std::copy(ai_scene_->mLights, ai_scene_->mLights + ai_scene_->mNumLights, ai_lights.begin());
    for (auto ai_light : ai_lights) {
        auto light = parse_light(ai_light);
        if (light) {
            ret.push_back(light);
        }
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
    auto valid = [](std::pair<string, float4> val) {
        return !val.first.empty() || nonzero(val.second);
    };

    auto color = parse_texture(ai_material, aiTextureType_DIFFUSE);
    color = valid(color) ? color : parse_texture(ai_material, aiTextureType_BASE_COLOR);
    auto bump = parse_texture(ai_material, aiTextureType_HEIGHT);
    auto sheen = parse_texture(ai_material, aiTextureType_SHEEN);
    auto metallic = parse_texture(ai_material, aiTextureType_METALNESS);
    metallic = valid(metallic) ? metallic : parse_texture(ai_material, aiTextureType_SPECULAR);
    auto rough = parse_texture(ai_material, aiTextureType_DIFFUSE_ROUGHNESS);
    auto refl = parse_texture(ai_material, aiTextureType_REFLECTION);
    refl = valid(refl) ? refl : parse_texture(ai_material, aiTextureType_AMBIENT);
    auto cc = parse_texture(ai_material, aiTextureType_CLEARCOAT);
    auto trans = parse_texture(ai_material, aiTextureType_TRANSMISSION);
    MaterialDesc desc;
    desc.name = ai_material->GetName().C_Str();
    ParameterSet data = ParameterSet(DataWrap::object());
    data.set_value("type", "principled_bsdf");
    DataWrap param = DataWrap::object();

    auto get_vec3 = [this](std::pair<string, float4> val) -> DataWrap {
        if (!val.first.empty()) {
            DataWrap ret = DataWrap::object();
            fs::path fn = directory_ / val.first;
            ret["channels"] = "xyz";
            ret["node"] = DataWrap::object();
            ret["node"]["fn"] = fn.string();
            ret["node"]["color_space"] = "linear";
            return ret;
        }
        DataWrap ret = DataWrap::array();
        ret.push_back(val.second.x);
        ret.push_back(val.second.y);
        ret.push_back(val.second.z);
        return ret;
    };

    auto get_scalar = [this](std::pair<string, float4> val) -> DataWrap {
        if (!val.first.empty()) {
            DataWrap ret = DataWrap::object();
            fs::path fn = directory_ / val.first;
            ret["channels"] = "x";
            ret["node"] = DataWrap::object();
            ret["node"]["fn"] = fn.string();
            ret["node"]["color_space"] = "linear";
            return ret;
        }
        return val.second.x;
    };

    param["color"] = get_vec3(color);
    param["metallic"] = get_scalar(metallic);

    data.set_value("param", param);
    desc.init(data);

    return desc;
}

vector<vision::MaterialDesc> AssimpParser::parse_materials() noexcept {
    vector<vision::MaterialDesc> ret;
    ret.reserve(ai_scene_->mNumMaterials);
    vector<aiMaterial *> ai_materials(ai_scene_->mNumMaterials);
    std::copy(ai_scene_->mMaterials, ai_scene_->mMaterials + ai_scene_->mNumMaterials, ai_materials.begin());
    for (auto ai_material : ai_materials) {
        ret.push_back(parse_material(ai_material));
    }
    return ret;
}

vector<ShapeInstance> AssimpParser::parse_meshes(bool parse_material,
                                                 uint32_t subdiv_level) {
    std::vector<ShapeInstance> instances;
    const aiScene *ai_scene = ai_scene_;
    vector<aiMesh *> ai_meshes(ai_scene->mNumMeshes);
    if (subdiv_level != 0u) {
        auto subdiv = Assimp::Subdivider::Create(Assimp::Subdivider::CATMULL_CLARKE);
        subdiv->Subdivide(ai_scene->mMeshes, ai_scene->mNumMeshes, ai_meshes.data(), subdiv_level);
    } else {
        std::copy(ai_scene->mMeshes, ai_scene->mMeshes + ai_scene->mNumMeshes, ai_meshes.begin());
    }

    vector<vision::MaterialDesc> materials;
    if (parse_material) {
        materials = parse_materials();
    }

    instances.reserve(ai_meshes.size());

    Scene &scene = Global::instance().pipeline()->scene();

    for (const aiMesh *ai_mesh : ai_meshes) {
        Box3f aabb;
        vector<Vertex> vertices;
        vertices.reserve(ai_mesh->mNumVertices);
        SP<Material> material;
        if (ai_mesh->mMaterialIndex >= 0 && parse_material) {
            const MaterialDesc &desc = materials[ai_mesh->mMaterialIndex];
            material = Node::create_shared<Material>(desc);
            scene.materials().push_back(material);
        }
        for (int i = 0; i < ai_mesh->mNumVertices; ++i) {
            auto ai_position = ai_mesh->mVertices[i];
            auto ai_normal = ai_mesh->mNormals[i];
            float3 position = assimp::from_vec3(ai_position);
            aabb.extend(position);
            float3 normal = assimp::from_vec3(ai_normal);
            float2 tex_coord;
            if (ai_mesh->mTextureCoords[0] != nullptr) {
                auto ai_tex_coord = ai_mesh->mTextureCoords[0][i];
                tex_coord = assimp::from_vec3(ai_tex_coord).xy();
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
        ShapeInstance instance(move(mesh));
        instance.set_material(material);
        instances.push_back(instance);
    }
    return instances;
}

const aiScene *AssimpParser::load_scene(const fs::path &fn, bool swap_handed, bool smooth, bool flip_uv) {
    directory_ = fn.parent_path();
    ai_importer_.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS,
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
    auto ai_scene = ai_importer_.ReadFile(fn.string().c_str(),
                                          post_process_steps);
    ai_scene_ = ai_scene;
    return ai_scene;
}

}// namespace vision