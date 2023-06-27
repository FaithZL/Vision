//
// Created by Zero on 2023/6/27.
//

#include <math/box.h>
#include "core/stl.h"
#include "rhi/common.h"
#include "render_graph/graph.h"

using namespace vision;
using namespace ocarina;

namespace vision {

class RTPass : public RenderPass {
private:
    static constexpr auto kOutput = "radiance";

public:

};

class AccumulatePass : public RenderPass {
private:
    static constexpr auto kInput = "input";
    static constexpr auto kOutput = "output";

public:


};

}// namespace vision

int main(int argc, char *argv[]) {

    return 0;
}
