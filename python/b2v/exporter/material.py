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
    socket = bsdf.inputs['Color']
    print(list(socket.default_value))
    print(dir(socket))
    ret = {
        "type" : "matte",
        "param": {
            # "color" : socket.default_value
        }
    }
    return ret

def export_disney(exporter, bsdf):
    ret = {
        "type" : "disney"
    }
    return ret

def export_glass(exporter, bsdf):
    ret = {
        "type" : "glass"
    }
    return ret

def export_mirror(exporter, bsdf):
    ret = {
        "type" : "mirror"
    }
    return ret

def export_mix(exporter, bsdf):
    ret = {
        "type" : "mix"
    }
    return ret


func_tab = {
    "BSDF_DIFFUSE" : export_matte,
    "BSDF_PRINCIPLED" : export_disney,
    "BSDF_GLASS" : export_glass,
    "BSDF_GLOSSY" : export_mirror,
    "MIX_SHADER" : export_mix,
    "ADD_SHADER" : export_mix,
}

def export(exporter, material, materials):
    output_node_id = 'Material Output'
    output = material.node_tree.nodes[output_node_id]
    bsdf = output.inputs['Surface'].links[0].from_node
    # socket = bsdf.inputs['Base Color']
    print("material export start")
    
    if material.name in materials:
        return
    export_func = func_tab[bsdf.type]
    print(bsdf.type)
    data = export_func(exporter, bsdf)
    materials[material.name] = data
    # materials[material.name] = 
    # if socket.is_linked:
    #     print(socket.links[0].from_node)
    # else:
    #     print(socket.default_value)
    print("material export end")