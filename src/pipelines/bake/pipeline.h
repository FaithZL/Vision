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
 * 2. rasterize triangle index and cache
 * 3. bake (to increase parallelism, mesh's normal, position map can be merged) and save to cache
 * 4. postprocess eg. denoise, padding ...
 * 5. update geometry data
 * 6. display
 */
class BakePipeline : public Pipeline {
private:
    BakerStats baker_stats_;
    vector<BakedShape> baked_shapes_;
    Shader<void(uint, uint)> display_shader_;
    uint lightmap_base_index_{InvalidUI32};
    PipelineDesc _desc;

public:
    explicit BakePipeline(const PipelineDesc &desc);
    static void create_cache_directory_if_necessary() {
        FileManager::create_directory_if_necessary(Global::instance().scene_cache_path());
    }
    VS_MAKE_PLUGIN_NAME_FUNC
    void compile() noexcept override;
    void compile_displayer() noexcept;
    void init_postprocessor(const DenoiserDesc &desc) override;
    void init_scene(const vision::SceneDesc &scene_desc) override;
    void prepare() noexcept override;
    void display(double dt) noexcept override;
    void render(double dt) noexcept override;
    // todo
    [[nodiscard]] uint frame_index() const noexcept override { return 1; }

    void bake_all() noexcept;
    void preprocess() noexcept override;
    template<typename Func>
    void for_each_need_bake(Func &&func) {
        auto &instances = scene_.instances();
        std::for_each(instances.begin(), instances.end(), [&](SP<ShapeInstance> item) {
            if (item->has_emission()) {
                return;
            }
            func(item);
        });
    }
};

}// namespace vision
