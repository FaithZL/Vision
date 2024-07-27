//
// Created by Zero on 2024/8/2.
//

#include "core/stl.h"
#include "file_inspector.h"

namespace vision::inline hotfix {
using namespace ocarina;

enum OptimizationLevel {
    Default = 0,
    Debug = 1,
    Release = 2
};

struct CompileOptions {
    vector<fs::path> include_paths;
    vector<fs::path> library_paths;
    fs::path intermediate_path;
    CompileOptions() {
        init();
    }
    void init() {
        fs::path src_dir = FileInspector::project_path() / "src";
        include_paths.push_back(src_dir / "ocarina" / "src");
        include_paths.push_back(src_dir);
    }
};

class Compiler {
};

class BuildTool {
private:
    Compiler compiler_;

public:
    BuildTool() = default;
    void clear() noexcept;
    void build(vector<fs::path> &&paths) noexcept;
};

}// namespace vision::inline hotfix