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

public:
    [[nodiscard]] const aiScene *load_scene(const fs::path &fn,
                                                   bool swap_handed = false, bool smooth = true,
                                                   bool flip_uv = false);
    [[nodiscard]] static vector<vision::Mesh> process_mesh(const aiScene *ai_scene, bool parse_material,
                                                           uint32_t subdiv_level = 0u);
};

}// namespace vision