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
    filter_data = json.load(file)
    
class SceneSettingItem(bpy.types.PropertyGroup):
    @classmethod
    def register(cls):   
        filter_tab = [
            (name, name, val["description"],i) for i, (name, val) in enumerate(filter_data.items())
        ]
        cls.filterType = bpy.props.EnumProperty(
            name="type", items=filter_tab)
        bpy.types.Scene.vision_setting = PointerProperty(
            name="vision_setting Render Settings",
            description="vision_setting render settings",   
            type=cls,
        )