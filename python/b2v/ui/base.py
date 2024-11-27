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

import os
from os.path import basename, dirname
import json

class VisionProperties(bpy.types.PropertyGroup):
    key = "vision_setting"
    @classmethod
    def register(cls):
        setattr(
            bpy.types.Scene,
            cls.key,
            PointerProperty(
                name=cls.key,
                description=cls.key,
                type=cls,
            ),
        )
    

class VisionBaseSetting(bpy.types.PropertyGroup):

    @classmethod
    def register_impl(cls):
        dic = cls.dic
        tab = [
            (name, name, val["description"], i)
            for i, (name, val) in enumerate(dic.items())
        ]
        setattr(cls, cls.attr_type, bpy.props.EnumProperty(name="type", items=tab))
        setattr(
            bpy.types.Scene,
            cls.setting_name,
            PointerProperty(
                name=cls.setting_name,
                description=cls.setting_name,
                type=cls,
            ),
        )
        for val in dic.values():
            label = val["label"]
            for param_name, param in val["parameters"].items():
                key = label + "_" + param_name
                property_func = getattr(bpy.props, param["type"] + "Property")
                ppt = property_func(**param["args"])
                setattr(cls, key, ppt)


class VisionWidget:
    COMPAT_ENGINES = {"VISION_RENDER_ENGINE"}
    bl_context = "render"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"

    @classmethod
    def poll(cls, context):
        return context.engine in cls.COMPAT_ENGINES


class VISION_RENDER_PT_VisionBasePanel(VisionWidget):
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
