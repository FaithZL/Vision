//
// Created by Zero on 2023/7/17.
//

#include "vision.h"
#include "base/node.h"
#include "core/stl.h"

namespace vision {

class VisionRendererImpl : public VisionRenderer {

};

}

VS_EXPORT_API vision::VisionRenderer *create() {
    return ocarina::new_with_allocator<vision::VisionRendererImpl>();
}
OC_EXPORT_API void destroy(vision::VisionRendererImpl *obj) { ocarina::delete_with_allocator(obj); }
