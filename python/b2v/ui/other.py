from . import base
from .base import *


# 定义操作符类，用于按钮点击时的操作
class MyCustomButtonOperator(bpy.types.Operator):
    bl_idname = "my.custom_button_operator"
    bl_label = "My Custom Button Operator"
    
    # 当按钮被点击时执行的操作
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
        print(bpy.types.TOPBAR_MT_file_export)
        