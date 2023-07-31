//
// Created by Zero on 2023/7/17.
//

#pragma once

#include "base/shape.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/Subdivision.h>
#include <assimp/scene.h>

namespace vision {

class AssimpParser {
private:
    Assimp::Importer _ai_importer;
    const aiScene *_ai_scene{};
    fs::path _directory;

public:
    const aiScene *load_scene(const fs::path &fn,
                              bool swap_handed = false, bool smooth = true,
                              bool flip_uv = false);
    [[nodiscard]] vector<SP<Mesh>> parse_meshes(bool parse_material,
                                                    uint32_t subdiv_level = 0u);
    [[nodiscard]] vision::Light *parse_light(aiLight *ai_light) noexcept;
    [[nodiscard]] vector<vision::Light *> parse_lights() noexcept;
    [[nodiscard]] static std::pair<string, float4> parse_texture(const aiMaterial *mat, aiTextureType type);
    [[nodiscard]] vector<vision::MaterialDesc> parse_materials() noexcept;
    [[nodiscard]] vision::MaterialDesc parse_material(aiMaterial *ai_material) noexcept;
};

}// namespace vision