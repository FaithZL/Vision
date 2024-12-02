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
from . import material

def export(context, instance, materials):
    object = instance.object
    material.export(context, object.data.materials[0], materials)
    
    return instance