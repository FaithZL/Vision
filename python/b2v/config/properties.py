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
    rfilter_data = json.load(file)
    
class SceneSettingItem(bpy.types.PropertyGroup):
    filterTab = [("GaussianFilter", "GaussianFilter", "", 1),
                   ("BoxFilter", "BoxFilter", "", 2),
                   ("TriangleFilter", "TriangleFilter", "", 3),
                   ("LanczosSincFilter", "LanczosSincFilter", "", 4),
                   ("MitchellFilter", "MitchellFilter", "", 5)]
    
    filterType : bpy.props.EnumProperty(
        name="type", items=filterTab, default="GaussianFilter")
    
    @classmethod
    def register(cls):
        bpy.types.Scene.vision_setting = PointerProperty(
            name="vision_setting Render Settings",
            description="vision_setting render settings",
            type=cls,
        )