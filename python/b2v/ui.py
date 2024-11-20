import bpy
# from bpy.types import Operator
# from bpy.props import EnumProperty

# class MyDropdownOperator(Operator):
#     bl_idname = "object.my_dropdown_operator"
#     bl_label = "My Dropdown Operator"
#     bl_description = "An operator with a dropdown menu"
    
#     # 定义下拉框的属性
#     my_dropdown = EnumProperty(
#         name="My Dropdown",
#         description="Choose an option",
#         items=[
#             ('OPTION_ONE', "Option One", "Description of Option One"),
#             ('OPTION_TWO', "Option Two", "Description of Option Two"),
#             ('OPTION_THREE', "Option Three", "Description of Option Three")
#         ],
#         default='OPTION_ONE'
#     )
    
#     # # 定义操作符的执行逻辑
#     # def execute(self, context):
#     #     print(f"Selected option: {self.my_dropdown}")
#     #     return {'FINISHED'}

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