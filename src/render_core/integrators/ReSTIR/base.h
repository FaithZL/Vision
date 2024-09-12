//
// Created by Zero on 2024/9/12.
//

#pragma once

#include "util.h"
#include "base/encoded_object.h"
#include "base/mgr/global.h"
#include "base/mgr/pipeline.h"
#include "core/thread_pool.h"
#include "hotfix/hotfix.h"

namespace vision {

class ReSTIR : public EncodedObject, public Context, public RenderEnv, public GUI, public RuntimeObject {
public:
    [[nodiscard]] auto prev_surfaces() const noexcept {
        return pipeline()->buffer_var<SurfaceData>(frame_buffer().prev_surfaces_index(frame_index()));
    }
    [[nodiscard]] auto cur_surfaces() const noexcept {
        return pipeline()->buffer_var<SurfaceData>(frame_buffer().cur_surfaces_index(frame_index()));
    }
    [[nodiscard]] auto prev_surface_extends() const noexcept {
        return pipeline()->buffer_var<SurfaceExtend>(frame_buffer().prev_surface_extends_index(frame_index()));
    }
    [[nodiscard]] auto cur_surface_extends() const noexcept {
        return pipeline()->buffer_var<SurfaceExtend>(frame_buffer().cur_surface_extends_index(frame_index()));
    }
};

}// namespace vision
