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
    fn = exporter.mesh_path(object.name) + ".glb"
    ret = {
        "type" : "model",
        "names" : object.name,
        "param" :{
            "fn" : os.path.join(exporter.mesh_dir, object.name + ".glb"),
            "smooth": True,
        }
    }
    bpy.ops.export_scene.gltf(filepath=exporter.mesh_path(object.name),
                            #   export_format='GLTF_SEPARATE',
                                export_materials='PLACEHOLDER',
                                use_selection=True)
    return ret

def export(exporter, object, materials):
    bpy.context.view_layer.objects.active = object
    
    if object.type == 'MESH':
        b_mesh = object.data
    else:
        b_mesh = object.to_mesh()
    mat = b_mesh.materials[0]
    material.export(exporter, mat, materials)
    transform = object.matrix_world
    object.select_set(True)
    ret = export_mesh(exporter, object, transform)
    object.select_set(False)
    return ret