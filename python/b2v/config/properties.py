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

with open(os.path.join(dirname(__file__), "integrators.json")) as file:
    integrator_dict = json.load(file)
with open(os.path.join(dirname(__file__), "filters.json")) as file:
    filter_dict = json.load(file)
with open(os.path.join(dirname(__file__), "samplers.json")) as file:
    sampler_dict = json.load(file)
with open(os.path.join(dirname(__file__), "lightsamplers.json")) as file:
    lightsampler_dict = json.load(file)


class VisionBaseSetting(bpy.types.PropertyGroup):
    @classmethod
    def register_impl(cls):
        tab = [
            (name, name, val["description"], i)
            for i, (name, val) in enumerate(cls.dic.items())
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
        for val in cls.dic.values():
            label = val["label"]
            for param_name, param in val["parameters"].items():
                key = label + "_" + param_name
                property_func = getattr(bpy.props, param["type"] + "Property")
                ppt = property_func(**param["args"])
                setattr(cls, key, ppt)


class VisionFilterSetting(VisionBaseSetting):

    setting_name = "vision_filter_setting"
    dic = filter_dict
    attr_type = "filter_type"

    @classmethod
    def register(cls):
        cls.register_impl()


class VisionIntegratorSetting(VisionBaseSetting):
    
    setting_name = "vision_integrator_setting"
    dic = integrator_dict
    attr_type = "integrator_type"
    
    @classmethod
    def register(cls):
        cls.register_impl()

