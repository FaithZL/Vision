//
// Created by Zero on 26/10/2022.
//

#pragma once

#include <iostream>
#include "core/cli_parser.h"
#include "core/scene_desc.h"
#include "core/stl.h"
#include "base/mgr/pipeline.h"
#include "util/file_manager.h"
#include "util/image.h"
#include "core/logging.h"
#include "base/denoiser.h"
#include "base/mgr/global.h"

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
            return;
        }
        working_dir = cli_parser->working_dir();
        scene_path = cli_parser->scene_path();
        Global::instance().set_scene_path(scene_path);
        scene_file = cli_parser->scene_file();
        output_dir = cli_parser->output_dir();
        clear_cache = cli_parser->clear_cache();
        backend = cli_parser->backend();
    }
};
using namespace ocarina;
class App : public GUI {
public:
    UP<CLIParser> cli_parser{};
    Device device;
    mutable WindowWrapper window{nullptr, nullptr};
    SP<Pipeline> rp{};
    vector<float4> view_buffer;
    float2 last_cursor_pos = make_float2(0);
    bool left_key_press{false};
    bool right_key_press{false};
    bool invalidation{false};

    bool key_r_press{false};
    bool key_f_press{false};

    // debugger switching
    bool key_g_press{false};

    LaunchParams params;

public:
    App(int argc, char *argv[])
        : cli_parser(make_unique<CLIParser>(argc, argv)),
          device(FileManager::instance().create_device(cli_parser->backend())) {
        init(argc);
    }
    void init(int argc = 0);
    void prepare();
    void update(double dt) noexcept;
    bool render_UI(ocarina::Widgets *widgets) noexcept override;
    void init_pipeline();
    [[nodiscard]] Pipeline &pipeline() const { return *rp; }
    void register_event() noexcept;
    void update_camera_view(float d_yaw, float d_pitch) const noexcept;
    void on_key_event(int key, int action) noexcept;
    void on_window_size_change(uint2 size) noexcept;
    void on_mouse_event(int button, int action, float2 pos) noexcept;
    void on_scroll_event(float2 scroll) noexcept;
    void on_cursor_move(float2 pos) noexcept;
    int run() noexcept;
};
}// namespace vision