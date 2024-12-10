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
from . import shadernode


def export_matte(exporter, bsdf):
    socket = bsdf.inputs["Color"]
    ret = {
        "type": "matte",
        "param": {"color": shadernode.parse_node(exporter, socket, 3)},
    }
    return ret


def export_disney(exporter, bsdf):
    ret = {"type": "disney"}
    return ret


def export_glass(exporter, bsdf):
    ret = {"type": "glass"}
    return ret


def export_mirror(exporter, bsdf):
    ret = {"type": "mirror"}
    return ret


def export_mix(exporter, bsdf):
    ret = {"type": "mix"}
    return ret


def export_emission(exporter, bsdf):
    socket = bsdf.inputs["Color"]
    ret = {
        "type": "area",
        "param": {"color": shadernode.parse_node(exporter, socket, 3)},
    }
    return ret


def export_add(exporter, bsdf):
    node0 = bsdf.inputs[0].links[0].from_node
    node1 = bsdf.inputs[1].links[0].from_node

    emission_node = node0 if node0.type == "EMISSION" else node1
    material_node = node1 if node0.type == "EMISSION" else node0
    ret = {
        "type": "add",
        "param": {
            "material": func_tab[material_node.type](exporter, material_node),
            "emission": func_tab[emission_node.type](exporter, emission_node),
        },
    }
    return ret


func_tab = {
    "BSDF_DIFFUSE": export_matte,
    "BSDF_PRINCIPLED": export_disney,
    "BSDF_GLASS": export_glass,
    "BSDF_GLOSSY": export_mirror,
    "MIX_SHADER": export_mix,
    "ADD_SHADER": export_add,
    "EMISSION": export_emission,
}


def export(exporter, material, materials):
    output_node_id = "Material Output"
    output = material.node_tree.nodes[output_node_id]
    bsdf = output.inputs["Surface"].links[0].from_node
    print("material export start")

    if material.name in materials:
        return
    export_func = func_tab[bsdf.type]
    data = export_func(exporter, bsdf)
    
    if bsdf.type == "ADD_SHADER":
        materials[material.name] = data["param"]["material"]
    else:
        materials[material.name] = data
    return data
