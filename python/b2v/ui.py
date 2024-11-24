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
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"

    @classmethod
    def poll(cls, context):
        return context.engine in cls.COMPAT_ENGINES


# class VISION_RENDER_PT_BasePanel(bpy.types.Panel, VisionWidget):

#     property_cls = properties.VisionFilterSetting
    
#     def attr_type(self):
#         return self.property_cls.attr_type
    
#     def setting_name(self):
#         return self.property_cls.setting_name

#     def draw(self, context):
#         scene = context.scene
#         layout = self.layout
#         row = layout.row()
#         layout.use_property_split = True
#         layout.use_property_decorate = False
#         vs = getattr(scene, self.setting_name())
#         row.prop(vs, self.attr_type())
#         cur_item = getattr(vs, self.attr_type())
#         for attr in dir(vs):
#             if attr.startswith(cur_item):
#                 attr_name = attr[len(cur_item) + 1 :]
#                 layout.row().prop(vs, attr, text=attr_name)


class VISION_RENDER_PT_Filter(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Filter"
    bl_label = "Filter"
    property_cls = properties.VisionFilterSetting

    def attr_type(self):
        return self.property_cls.attr_type
    
    def setting_name(self):
        return self.property_cls.setting_name

    def draw(self, context):
        scene = context.scene
        layout = self.layout
        row = layout.row()
        layout.use_property_split = True
        layout.use_property_decorate = False
        vs = getattr(scene, self.setting_name())
        row.prop(vs, self.attr_type())
        cur_item = getattr(vs, self.attr_type())
        for attr in dir(vs):
            if attr.startswith(cur_item):
                attr_name = attr[len(cur_item) + 1 :]
                layout.row().prop(vs, attr, text=attr_name)



class VISION_RENDER_PT_LightSampler(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_LightSampler"
    bl_label = "LightSampler"

    def draw(self, context):
        layout = self.layout


class VISION_RENDER_PT_Spectrum(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Spectrum"
    bl_label = "Spectrum"

    def draw(self, context):
        layout = self.layout


class VISION_RENDER_PT_Sampler(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Sampler"
    bl_label = "Sampler"

    def draw(self, context):
        layout = self.layout


class VISION_RENDER_PT_Intergrator(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Intergrator"
    bl_label = "Intergrator"
    property_cls = properties.VisionIntegratorSetting
    
    def attr_type(self):
        return self.property_cls.attr_type
    
    def setting_name(self):
        return self.property_cls.setting_name

    def draw(self, context):
        scene = context.scene
        layout = self.layout
        row = layout.row()
        layout.use_property_split = True
        layout.use_property_decorate = False
        vs = getattr(scene, self.setting_name())
        row.prop(vs, self.attr_type())
        cur_item = getattr(vs, self.attr_type())
        for attr in dir(vs):
            if attr.startswith(cur_item):
                attr_name = attr[len(cur_item) + 1 :]
                layout.row().prop(vs, attr, text=attr_name)


class VISION_RENDER_PT_Operator(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Operator"
    bl_label = "Operator"

    def draw(self, context):
        layout = self.layout
