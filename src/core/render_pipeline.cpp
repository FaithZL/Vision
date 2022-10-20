//
// Created by Zero on 04/09/2022.
//

#include "render_pipeline.h"

namespace vision {

RenderPipeline::RenderPipeline(Device *device, vision::Context *context)
    : _device(device),
      _context(context),
      _scene(context) {}

}// namespace vision