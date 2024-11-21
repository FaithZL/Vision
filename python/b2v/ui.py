import bpy
from bpy.props import EnumProperty
from . import config
    
# class SceneSettingItem(bpy.types.PropertyGroup):
#     filterTypes = [("GaussianFilter", "GaussianFilter", "", 1),
#                    ("BoxFilter", "BoxFilter", "", 2),
#                    ("TriangleFilter", "TriangleFilter", "", 3),
#                    ("LanczosSincFilter", "LanczosSincFilter", "", 4),
#                    ("MitchellFilter", "MitchellFilter", "", 5)]

#     bpy.types.Scene.filterType = bpy.props.EnumProperty(
#         name="type", items=filterTypes, default="GaussianFilter")
    
    
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
        scene = context.scene
        layout = self.layout
        row = layout.row()
        row.prop(scene, "filterType")
        # bpy.types.Object.my_prop_grp = bpy.props.PointerProperty(type=MyPropertyGroup)
        # op = layout.operator(MyEnumOperator.bl_idname, text="My Enum Operator")
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