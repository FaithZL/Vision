import bpy
import os
import json
from bpy.props import (
    BoolProperty,
    CollectionProperty,
    EnumProperty,
    FloatProperty,
    IntProperty,
    PointerProperty,
    StringProperty,
)

def get_panels():
    exclude_panels = {
        'VIEWLAYER_PT_filter',
        'VIEWLAYER_PT_layer_passes',
        'RENDER_PT_simplify',
        'RENDER_PT_color_management',
        'RENDER_PT_freestyle'
    }

    panels = []
    for panel in bpy.types.Panel.__subclasses__():
        if hasattr(panel, 'COMPAT_ENGINES') and 'BLENDER_RENDER' in panel.COMPAT_ENGINES:
            if panel.__name__ not in exclude_panels:
                panels.append(panel)

    return panels

class VisionProperties(bpy.types.PropertyGroup):
    key = "vision"

    @classmethod
    def register(cls):
        setattr(
            bpy.types.Scene,
            cls.key,
            bpy.props.PointerProperty(
                name=cls.key,
                description=cls.key,
                type=cls,
            ),
        )

    def get_params(self, data_type):
        cur_item = getattr(self, data_type)
        params = {}
        ret = {"type": cur_item, "params": params}
        for attr_name in dir(self):
            if attr_name.startswith(cur_item):
                simple_attr_name = attr_name[len(cur_item) + 1 :]
                attr = getattr(self, attr_name)
                params[simple_attr_name] = attr
        return ret

    @classmethod
    def unregister(cls):
        delattr(bpy.types.Scene, cls.key)


def register():
    bpy.utils.vision_register_class(VisionProperties)
    for panel in get_panels():
        panel.COMPAT_ENGINES.add('VISION_RENDER_ENGINE')


def unregister():
    bpy.utils.vision_unregister_class(VisionProperties)
    for panel in get_panels():
        if 'VISION_RENDER_ENGINE' in panel.COMPAT_ENGINES:
            panel.COMPAT_ENGINES.remove('VISION_RENDER_ENGINE')


class VisionWidget:
    COMPAT_ENGINES = {"VISION_RENDER_ENGINE"}
    bl_context = "render"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"

    @classmethod
    def poll(cls, context):
        return context.engine in cls.COMPAT_ENGINES


class VISION_RENDER_PT_VisionBasePanel(VisionWidget):

    @classmethod
    def register(cls):
        dic = cls.dic
        tab = [
            (name, name, val["description"], i)
            for i, (name, val) in enumerate(dic.items())
        ]
        setattr(
            VisionProperties,
            cls.attr_type,
            bpy.props.EnumProperty(name="type", items=tab),
        )
        for val in dic.values():
            label = val["label"]
            for param_name, param in val["parameters"].items():
                key = label + "_" + param_name
                property_func = getattr(bpy.props, param["type"] + "Property")
                ppt = property_func(**param["args"])
                setattr(VisionProperties, key, ppt)

    def draw(self, context):
        scene = context.scene
        layout = self.layout
        row = layout.row()
        layout.use_property_split = True
        layout.use_property_decorate = False
        vs = getattr(scene, VisionProperties.key)
        row.prop(vs, self.attr_type)
        cur_item = getattr(vs, self.attr_type)
        for attr in dir(vs):
            if attr.startswith(cur_item):
                attr_name = attr[len(cur_item) + 1 :]
                layout.row().prop(vs, attr, text=attr_name)
