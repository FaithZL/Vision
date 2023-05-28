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

struct LaunchParams {
    fs::path working_dir;
    fs::path scene_file;
    fs::path scene_path;
    fs::path output_dir;
    string backend{"cuda"};

    void init(CLIParser &cli_parser) noexcept;
};

class App {
public:
    CLIParser cli_parser;
    ocarina::Context context;
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
    LaunchParams params;

public:
    App(int argc, char *argv[])
        : cli_parser(argc, argv),
          context(fs::path(argv[0]).parent_path()),
          device(context.create_device(cli_parser.backend())),
          rp(create_pipeline()) {
        init(argc);
    }
    [[nodiscard]] RenderPipeline create_pipeline() { return {&device, &context}; }
    void init(int argc) noexcept;
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