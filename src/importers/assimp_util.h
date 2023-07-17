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
    [[nodiscard]] static const aiScene *load_scene(const fs::path &fn, Assimp::Importer &ai_importer,
                                                   bool swap_handed = false, bool smooth = true,
                                                   bool flip_uv = false);
};

}// namespace vision