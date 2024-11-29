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


def vision_register_class(cls):
    if not getattr(cls, "is_registered", False): 
        bpy.utils.register_class(cls)   

def vision_unregister_class(cls):
    if getattr(cls, "is_registered", False):
        bpy.utils.unregister_class(cls)
        
    
bpy.utils.vision_register_class = vision_register_class
bpy.utils.vision_unregister_class = vision_unregister_class
    
class Vision(bpy.types.RenderEngine):
    bl_idname = "VISION_RENDER_ENGINE"
    bl_label = "Vision"
    bl_use_preview = True 

    def render(self, context):
        self.report({'INFO'}, "Rendering with Vision...")



def register():
    print("Registering Vision ---")
    bpy.utils.vision_register_class(Vision)
    auto_load.register()

def unregister():
    print("Unregistering Vision ---")
    auto_load.unregister()
    bpy.utils.vision_unregister_class(Vision)


if __name__ == "__main__":
    register()
