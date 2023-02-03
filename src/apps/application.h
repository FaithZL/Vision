//
// Created by Zero on 26/10/2022.
//

#pragma once

#include <iostream>
#include "core/cli_parser.h"
#include "descriptions/scene_desc.h"
#include "core/stl.h"
#include "core/context.h"
#include "util/image_io.h"
#include "core/logging.h"

namespace vision {
class App {
public:
    vision::Context context;
    Device device;
    mutable Window::Wrapper window{nullptr, nullptr};
    SceneDesc scene_desc;
    RenderPipeline rp;
    ImageIO radiance_image;
    float2 last_cursor_pos = make_float2(0);
    bool left_key_press{false};
    bool right_key_press{false};
    bool need_save{false};
    bool need_update{false};

public:
    App(int argc, char *argv[])
        : context(argc, argv),
          device(context.create_device("cuda")),
          rp(context.create_pipeline(&device)) {
        device.init_rtx();
        if (context.cli_parser().clear_cache()) {
            context.clear_cache();
        }
    }
    void prepare() noexcept;
    void update(double dt) noexcept;
    void check_and_save() noexcept;
    void register_event() noexcept;
    void update_camera_view(float d_yaw, float d_pitch) noexcept;
    void on_key_event(int key, int action) noexcept;
    void on_window_size_change(uint2 size) noexcept;
    void on_mouse_event(int button, int action, float2 pos) noexcept;
    void on_scroll_event(float2 scroll) noexcept;
    void on_cursor_move(float2 pos) noexcept;
    int run() noexcept;
};
}// namespace vision