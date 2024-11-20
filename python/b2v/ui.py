import bpy
# from bpy.types import Operator
# from bpy.props import EnumProperty


import bpy
from bpy.props import EnumProperty

# 定义操作符的类
class MyEnumOperator(bpy.types.Operator):
    """示例操作符，展示下拉框"""
    bl_idname = "object.my_enum_operator"
    bl_label = "My Enum Operator"
    bl_description = "这是一个包含下拉框的示例操作符"
    bl_options = {'REGISTER', 'UNDO'}

    # 定义一个枚举属性（下拉框）
    my_enum: EnumProperty(
        name="My Enum",
        description="选择一个选项",
        items=[
            ('OPTION_ONE', "Option One", "描述一"),
            ('OPTION_TWO', "Option Two", "描述二"),
            ('OPTION_THREE', "Option Three", "描述三")
        ],
        default='OPTION_ONE'
    )

    def execute(self, context):
        print(f"选中的选项是: {self.my_enum}")
        return {'FINISHED'}
    
    
class VisionWidget:
    COMPAT_ENGINES = {'VISION_RENDER_ENGINE'}
    bl_context = "render"
    
    @classmethod
    def poll(cls, context):
        return context.engine in cls.COMPAT_ENGINES


class VISION_RENDER_PT_Filter(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Filter"
    bl_label = "Filter"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'

    def draw(self, context):
        layout = self.layout
        op = layout.operator(MyEnumOperator.bl_idname, text="My Enum Operator")
        # op.my_enum = context.object.my_enum_property
        
class VISION_RENDER_PT_LightSampler(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_LightSampler"
    bl_label = "LightSampler"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    
    def draw(self, context):
        layout = self.layout

class VISION_RENDER_PT_Spectrum(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Spectrum"
    bl_label = "Spectrum"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    
    def draw(self, context):
        layout = self.layout

class VISION_RENDER_PT_Sampler(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Sampler"
    bl_label = "Sampler"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    
    def draw(self, context):
        layout = self.layout

class VISION_RENDER_PT_Intergrator(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Intergrator"
    bl_label = "Intergrator"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    
    def draw(self, context):
        layout = self.layout
        
class VISION_RENDER_PT_Operator(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Operator"
    bl_label = "Operator"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    
    def draw(self, context):
        layout = self.layout       