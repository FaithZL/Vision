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

def export_matte(exporter, bsdf):
    pass

def export_disney(exporter, bsdf):
    pass

def export_glass(exporter, bsdf):
    pass

def export_glossy(exporter, bsdf):
    pass

def export_mix(exporter, bsdf):
    pass

def export_add(exporter, bsdf):
    pass


func_tab = {
    "Diffuse BSDF" : export_matte,
    "Principled BSDF" : export_disney,
    "Glass BSDF" : export_glass,
    "Glossy BSDF" : export_glossy,
    "Mix Shader" : export_mix,
    "Add BSDF" : export_add,
}

def export(exporter, material, materials):
    output_node_id = 'Material Output'
    output = material.node_tree.nodes[output_node_id]
    bsdf = output.inputs['Surface'].links[0].from_node
    # socket = bsdf.inputs['Base Color']
    print("material export start")
    print(material.name)
    print(bsdf.name)
    materials[material.name] = {
        "type" : "matte"
    }
    # if socket.is_linked:
    #     print(socket.links[0].from_node)
    # else:
    #     print(socket.default_value)
    print("material export end")