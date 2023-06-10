//
// Created by Zero on 26/10/2022.
//

#pragma once

#include <iostream>
#include "core/cli_parser.h"
#include "descriptions/scene_desc.h"
#include "core/stl.h"
#include "base/mgr/render_pipeline.h"
#include "rhi/context.h"
#include "util/image_io.h"
#include "core/logging.h"
#include "base/denoiser.h"

namespace vision {

struct LaunchParams {
    fs::path working_dir;
    fs::path scene_file;
    fs::path scene_path;
    fs::path output_dir;
    bool clear_cache{false};
    string backend{"cuda"};

    LaunchParams() = default;

    void init(const CLIParser *cli_parser) noexcept {
        if (cli_parser == nullptr) {
            return ;
        }
        working_dir = cli_parser->working_dir();
        scene_path = cli_parser->scene_path();
        scene_file = cli_parser->scene_file();
        output_dir = cli_parser->output_dir();
        clear_cache = cli_parser->clear_cache();
        backend = cli_parser->backend();
    }
};

VS_EXPORT_API int execute(char *working_dir, char *scene_fn);

class App {
public:
    UP<CLIParser> cli_parser{};
    ocarina::Context context;
    Device device;
    mutable Window::Wrapper window{nullptr, nullptr};
    SceneDesc scene_desc;
    RenderPipeline rp;
    float2 last_cursor_pos = make_float2(0);
    bool left_key_press{false};
    bool right_key_press{false};
    bool need_save{false};
    bool need_update{false};
    LaunchParams params;

public:
    App(int argc, char *argv[])
        : cli_parser(make_unique<CLIParser>(argc, argv)),
          context(fs::path(argv[0]).parent_path()),
          device(context.create_device(cli_parser->backend())),
          rp(create_pipeline()) {
        init(argc);
    }
    App(const fs::path &working_dir, const fs::path &scene_fn)
        : context(working_dir),
          device(context.create_device("cuda")),
          rp(create_pipeline()) {
        params.working_dir = working_dir;
        params.scene_file = scene_fn;
        params.scene_path = scene_fn.parent_path();
        init();
    }
    [[nodiscard]] RenderPipeline create_pipeline() { return {&device, &context}; }
    void init(int argc = 0);
    void prepare();
    void update(double dt) noexcept;
    void check_and_save() noexcept;
    void save_result() noexcept;
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