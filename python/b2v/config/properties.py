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

import os
from os.path import basename, dirname
import json

with open(os.path.join(dirname(__file__), "integrators.json")) as file:
    integrator_dict = json.load(file)
with open(os.path.join(dirname(__file__), "filters.json")) as file:
    filter_dict = json.load(file)
    
class VisionFilterSetting(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):   
        tab = [
            (name, name, val["description"],i) for i, (name, val) in enumerate(filter_dict.items())
        ]
        cls.filter_type = bpy.props.EnumProperty(
            name="type", items=tab)
        bpy.types.Scene.vision_filter_setting = PointerProperty(
            name="vision_filter_setting",
            description="vision_filter_setting",   
            type=cls,
        )
        for val in filter_dict.values():
            filter_name = val["label"]
            for param_name, param in val["parameters"].items():
                key = filter_name + "_" + param_name
                property_func = getattr(bpy.props, param["type"] + "Property")
                ppt = property_func(**param["args"])
                setattr(cls, key, ppt)
                
                
class VisionIntegratorSetting(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):   
        tab = [
            (name, name, val["description"],i) for i, (name, val) in enumerate(integrator_dict.items())
        ]
        cls.integrator_type = bpy.props.EnumProperty(
            name="type", items=tab)
        bpy.types.Scene.vision_integrator_setting = PointerProperty(
            name="vision_integrator_setting",
            description="vision_integrator_setting",   
            type=cls,
        )
        for val in integrator_dict.values():
            integrator_name = val["label"]
            for param_name, param in val["parameters"].items():
                key = integrator_name + "_" + param_name
                property_func = getattr(bpy.props, param["type"] + "Property")
                ppt = property_func(**param["args"])
                setattr(cls, key, ppt)