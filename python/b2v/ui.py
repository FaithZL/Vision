import bpy
from bpy.props import (
    BoolProperty,
    CollectionProperty,
    EnumProperty,
    FloatProperty,
    IntProperty,
    PointerProperty,
    StringProperty,
)
from . import config
from .config import properties

class VisionWidget:
    COMPAT_ENGINES = {"VISION_RENDER_ENGINE"}
    bl_context = "render"

    @classmethod
    def poll(cls, context):
        return context.engine in cls.COMPAT_ENGINES

# class VISION_RENDER_PT_Panel(bpy.types.Panel, VisionWidget):
    
#     def draw(self, context):
#         scene = context.scene
#         layout = self.layout
#         row = layout.row()
#         layout.use_property_split = True
#         layout.use_property_decorate = False
#         vs = scene.vision_filter_setting
#         row.prop(vs, "filter_type")
#         cur_filter = vs.filter_type
#         for attr in dir(vs):
#             if attr.startswith(cur_filter):
#                 attr_name = attr[len(cur_filter) + 1 :]
#                 layout.row().prop(vs, attr, text=attr_name)

class VISION_RENDER_PT_Filter(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Filter"
    bl_label = "Filter"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    
    attr_type = "filter_type"
    scene_key = "vision_filter_setting"

    def draw(self, context):
        scene = context.scene
        layout = self.layout
        row = layout.row()
        layout.use_property_split = True
        layout.use_property_decorate = False
        vs = getattr(scene, self.scene_key)
        row.prop(vs, self.attr_type)
        cur_item = getattr(vs, self.attr_type)
        for attr in dir(vs):
            if attr.startswith(cur_item):
                attr_name = attr[len(cur_item) + 1 :]
                layout.row().prop(vs, attr, text=attr_name)


class VISION_RENDER_PT_LightSampler(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_LightSampler"
    bl_label = "LightSampler"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"

    def draw(self, context):
        layout = self.layout


class VISION_RENDER_PT_Spectrum(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Spectrum"
    bl_label = "Spectrum"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"

    def draw(self, context):
        layout = self.layout


class VISION_RENDER_PT_Sampler(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Sampler"
    bl_label = "Sampler"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"

    def draw(self, context):
        layout = self.layout


class VISION_RENDER_PT_Intergrator(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Intergrator"
    bl_label = "Intergrator"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"

    def draw(self, context):
        scene = context.scene
        layout = self.layout
        row = layout.row()
        vs = scene.vision_integrator_setting
        row.prop(vs, "integrator_type")
        layout.use_property_split = True
        cur_integrator = vs.integrator_type
        for attr in dir(vs):
            if attr.startswith(cur_integrator):
                attr_name = attr[len(cur_integrator) + 1 :]
                layout.row().prop(vs, attr, text=attr_name)


class VISION_RENDER_PT_Operator(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Operator"
    bl_label = "Operator"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"

    def draw(self, context):
        layout = self.layout
