bl_info = {
    "name": "Vision",
    "author": "zhuling",
    "version": (1, 0),
    "blender": (3, 0, 0),
    "location": "Render Engine Menu",
    "description": "vision renderer and exporter",
    "category": "Render",
}

import bpy
from. import ui

from . import auto_load

auto_load.init()


class Vision(bpy.types.RenderEngine):
    bl_idname = "VISION_RENDER_ENGINE"
    bl_label = "Vision"
    bl_use_preview = True 

    def render(self, context):
        self.report({'INFO'}, "Rendering with Vision...")
        result = self.begin_result(0, 0, context.region.width, context.region.height)
        layer = result.layers[0].passes["Combined"]
        layer.rect = [[(0, 1, 0, 1)] * context.region.width] * context.region.height
        self.end_result(result)



def register():
    print("Registering Vision -000000000000--")
    auto_load.register()

    bpy.utils.register_class(Vision)
    # ui.register()

def unregister():
    print("Unregistering Vision ---")
    bpy.utils.unregister_class(Vision)
    # ui.unregister()
    auto_load.unregister()


if __name__ == "__main__":
    register()
