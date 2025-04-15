//
// Created by ling.zhu on 2025/1/31.
//

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
#include "GUI/window.h"
#include "math/basic_types.h"
#include "base/mgr/global.h"
#include "base/importer.h"
#include "rhi/stats.h"

using namespace ocarina;
using namespace vision;

struct App {
    Device device;
    SP<Pipeline> rp{};
    App() : device(FileManager::instance().create_device("cuda")) {
        Global::instance().set_device(&device);
        fs::path file_path = parent_path(__FILE__, 1) / "precompute.json";
        rp = Importer::import_scene(file_path);
        Global::instance().set_pipeline(rp.get());
        rp->upload_bindless_array();
    }

    int run() {

        vision::precompute_albedo();

        return 0;
    }
};

int main(int argc, char *argv[]) {
    App app;
    return app.run();
}