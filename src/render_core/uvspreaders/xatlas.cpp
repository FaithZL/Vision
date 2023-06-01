//
// Created by Zero on 2023/6/2.
//

#include "base/uv_spreader.h"

namespace vision {
using namespace ocarina;

class XAtlas : public UVSpreader {
private:
public:
    explicit XAtlas(const UVSpreaderDesc &desc) : UVSpreader(desc) {}
};

}// namespace vision

VS_MAKE_CLASS_CREATOR(vision::XAtlas)