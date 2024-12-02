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

def export_mesh(exporter, mesh, transform):
    exporter.try_make_mesh_dir()

def export(exporter, instance, materials):
    object = instance.object
    if object.type == 'MESH':
        b_mesh = object.data
    else:
        b_mesh = object.to_mesh()
    material.export(exporter, b_mesh.materials[0], materials)
    is_instance = instance.is_instance
    transform = object.matrix_world
    print("geom start")

    export_mesh(exporter, b_mesh, transform)
    print("geom end")
    return instance