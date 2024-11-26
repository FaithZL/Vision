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

from . import auto_load

auto_load.init()

class VisionScene(bpy.types.bpy_struct):
    pass

class Vision(bpy.types.RenderEngine):
    bl_idname = "VISION_RENDER_ENGINE"
    bl_label = "Vision"
    bl_use_preview = True 

    def render(self, context):
        self.report({'INFO'}, "Rendering with Vision...")



def register():
    print("Registering Vision ---")
    setattr(bpy.types.Scene, "VisionScene", VisionScene)
    auto_load.register()
    bpy.utils.register_class(Vision)

def unregister():
    print("Unregistering Vision ---")
    bpy.utils.unregister_class(Vision)
    auto_load.unregister()
    delattr(bpy.types.Scene, "VisionScene")


if __name__ == "__main__":
    register()
