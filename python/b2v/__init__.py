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


class VisionProperties(bpy.types.PropertyGroup):
    key = "vision"
    @classmethod
    def register(cls):
        setattr(
            bpy.types.Scene,
            cls.key,
            bpy.props.PointerProperty(
                name=cls.key,
                description=cls.key,
                type=cls,
            ),
        )
            
    @classmethod
    def unregister(cls):
        delattr(bpy.types.Scene, cls.key)
    
class Vision(bpy.types.RenderEngine):
    bl_idname = "VISION_RENDER_ENGINE"
    bl_label = "Vision"
    bl_use_preview = True 

    def render(self, context):
        self.report({'INFO'}, "Rendering with Vision...")



def register():
    print("Registering Vision ---")
    bpy.utils.register_class(Vision)
    bpy.utils.register_class(VisionProperties)
    auto_load.register()

def unregister():
    print("Unregistering Vision ---")
    auto_load.unregister()
    bpy.utils.unregister_class(VisionProperties)
    bpy.utils.unregister_class(Vision)


if __name__ == "__main__":
    register()
