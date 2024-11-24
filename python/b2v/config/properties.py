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


class VisionBaseSetting(bpy.types.PropertyGroup):
    
    @classmethod
    def get_dict(cls, fn):
        with open(os.path.join(dirname(__file__), fn)) as file:
            ret = json.load(file)
        return ret
    
    @classmethod
    def register_impl(cls):
        dic = cls.get_dict(cls.fn)
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


class VisionFilterSetting(VisionBaseSetting):

    setting_name = "vision_filter_setting"
    attr_type = "filter_type"
    fn = "filters.json"

    @classmethod
    def register(cls):
        cls.register_impl()


class VisionIntegratorSetting(VisionBaseSetting):

    setting_name = "vision_integrator_setting"
    attr_type = "integrator_type"
    fn = "integrators.json"

    @classmethod
    def register(cls):
        cls.register_impl()
