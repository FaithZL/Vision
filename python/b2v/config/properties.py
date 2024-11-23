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
    integrator_data = json.load(file)
with open(os.path.join(dirname(__file__), "filters.json")) as file:
    filter_dict = json.load(file)
    
class VisionFilterSetting(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):   
        filter_tab = [
            (name, name, val["description"],i) for i, (name, val) in enumerate(filter_dict.items())
        ]
        cls.__filter_dict = filter_dict
        cls.filter_type = bpy.props.EnumProperty(
            name="type", items=filter_tab)
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
        
    @classmethod
    def filter_parameter(cls, key):
        return cls.__filter_dict[key]["parameters"]