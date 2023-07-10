//
// Created by Zero on 2023/6/14.
//

#pragma once

#include "base/mgr/pipeline.h"
#include "base/bake_utlis.h"
#include "dilate_filter.h"
#include "baker.h"

namespace vision {

/**
 * Process of baking
 * 1. unwrap uv and cache
 * 2. rasterize position, normal map and cache
 * 3. transform instance position , normal map to world space
 * 4. bake_old (to increase parallelism, mesh's normal, position map can be merged) and save to cache
 * 5. postprocess eg. denoise, padding ...
 * 6. update geometry data
 * 7. display
 */
class BakePipeline : public Pipeline {
private:
    BakerStats _baker_stats;
    vector<BakedShape> _baked_shapes;
    Shader<void(uint, uint)> _display_shader;
    uint _lightmap_base_index{InvalidUI32};
    PipelineDesc _desc;

public:
    explicit BakePipeline(const PipelineDesc &desc);
    static void create_cache_directory_if_necessary() {
        Context::create_directory_if_necessary(Global::instance().scene_cache_path());
    }
    void compile_shaders() noexcept override;
    void compile_displayer() noexcept;
    void init_postprocessor(const vision::SceneDesc &scene_desc) override;
    void init_scene(const vision::SceneDesc &scene_desc) override;
    void prepare() noexcept override;
    void display(double dt) noexcept override;
    void render(double dt) noexcept override;

    void bake_all() noexcept;
    void preprocess() noexcept override;
    template<typename Func>
    void for_each_need_bake(Func &&func) {
        auto &meshes = _scene.shapes();
        std::for_each(meshes.begin(), meshes.end(), [&](vision::Shape *item) {
            if (item->has_emission()) {
                return;
            }
            func(item);
        });
    }
};

}// namespace vision
