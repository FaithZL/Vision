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

class AssimpUtil {
private:
    Assimp::Importer _ai_importer;
    const aiScene *_ai_scene{};

public:
    const aiScene *load_scene(const fs::path &fn,
                              bool swap_handed = false, bool smooth = true,
                              bool flip_uv = false);
    [[nodiscard]] vector<vision::Mesh> parse_meshes(bool parse_material,
                                                    uint32_t subdiv_level = 0u);
    [[nodiscard]] vector<vision::Light *> parse_lights() noexcept;
    [[nodiscard]] vector<vision::Material *> parse_materials() noexcept;
};

}// namespace vision