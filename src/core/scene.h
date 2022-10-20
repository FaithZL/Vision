//
// Created by Zero on 06/09/2022.
//

#pragma once

#include "base/sensor.h"
#include "base/lightsampler.h"
#include "descriptions/scene_desc.h"

namespace vision {

using namespace ocarina;

class Context;

class Scene {
private:
    vision::Context *_context{nullptr};
    vector<Node::Handle> _all_nodes;
    Camera *_camera{nullptr};
    LightSampler *_light_sampler{nullptr};

public:
    explicit Scene(vision::Context *ctx);
    void init(const SceneDesc& scene_desc);
    void prepare(RenderPipeline *rp) noexcept;
    [[nodiscard]] Node *load_node(const NodeDesc *desc);
    template<typename T, typename desc_ty>
    [[nodiscard]] T *load(const desc_ty *desc) noexcept {
        auto ret = dynamic_cast<T *>(load_node(desc));
        OC_ERROR_IF(ret == nullptr, "error node load ", desc->name);
        return ret;
    }
    [[nodiscard]] Camera *load_camera(const SensorDesc *desc) { return load<Camera>(desc); }
    [[nodiscard]] Filter *load_filter(const FilterDesc *desc) { return load<Filter>(desc); }
    [[nodiscard]] Film *load_film(const FilmDesc *desc) { return load<Film>(desc); }
    [[nodiscard]] LightSampler *load_light_sampler(const LightSamplerDesc *desc) { return load<LightSampler>(desc); }
};

}// namespace vision