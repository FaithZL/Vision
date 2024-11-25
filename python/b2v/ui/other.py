from . import base
from .base import *


class MyCustomButtonOperator(bpy.types.Operator):
    bl_idname = "my.custom_button_operator"
    bl_label = "My Custom Button Operator"
    
    def execute(self, context):
        print("Button was clicked! Executing callback function.")
        
        return {'FINISHED'}  


class VISION_RENDER_PT_Other(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Other"
    bl_label = "Other Setting"
    def draw(self, context):
        layout = self.layout
        row = layout.row()
        row.operator(MyCustomButtonOperator.bl_idname, text="My Button")
        