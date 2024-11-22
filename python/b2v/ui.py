import bpy
from bpy.props import (
    BoolProperty,
    CollectionProperty,
    EnumProperty,
    FloatProperty,
    IntProperty,
    PointerProperty,
    StringProperty
)
from . import config
    
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
        vs = scene.vision_setting
        row.prop(vs, "filter_type")
        params = vs.filter_parameter(vs.filter_type)
        # row.prop()
        # for name, val in  params.items():
            
        
        
        
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