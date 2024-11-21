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

    bpy.types.Scene.filterType = bpy.props.EnumProperty(
        name="type", items=filterTab, default="GaussianFilter")
    
# def register():
    
#     print("wocaonima ----------------------")


# def unregister():
#     print("wocaonima00000000000000 ----------------------")