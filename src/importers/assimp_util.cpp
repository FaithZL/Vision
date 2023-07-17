//
// Created by Zero on 2023/7/17.
//

#include "assimp_util.h"

namespace vision {

const aiScene *AssimpUtil::load_scene(const fs::path &fn, Assimp::Importer &ai_importer, bool swap_handed, bool smooth, bool flip_uv)  {
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

}