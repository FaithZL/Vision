from . import base
from .base import *


class VISION_RENDER_PT_Reload(bpy.types.Operator):
    bl_idname = "my.custom_button_operator"
    bl_label = "My Custom Button Operator"
    
    def execute(self, context):
        print("Button was clicked! Executing callback function.")
        print(context)
        return {'FINISHED'}  


class VISION_RENDER_PT_Hotfix(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Hotfix"
    bl_label = "Hotfix"
    def draw(self, context):
        layout = self.layout
        row = layout.row()
        row.operator(VISION_RENDER_PT_Reload.bl_idname, text="reload")
        