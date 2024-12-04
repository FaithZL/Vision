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

def export_mesh(exporter, object, transform):
    exporter.try_make_mesh_dir()
    bpy.ops.export_scene.gltf(filepath=exporter.output_directory()+'/meshes/'+object.name+'.gltf',
                              export_texture_dir='../textures',
                              export_format='GLTF_SEPARATE',
                              export_materials='PLACEHOLDER',
                              use_selection=True)

def export(exporter, object, materials):
    bpy.context.view_layer.objects.active = object
    
    if object.type == 'MESH':
        b_mesh = object.data
    else:
        b_mesh = object.to_mesh()
    material.export(exporter, b_mesh.materials[0], materials)
    transform = object.matrix_world
    object.select_set(True)
    export_mesh(exporter, object, transform)
    object.select_set(False)
    return object