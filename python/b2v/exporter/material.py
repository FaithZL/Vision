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


def export_diffuse(exporter, bsdf, node_tab):
    ret = {
        "type": "diffuse",
        "param": {
            "color": shadernode.parse_node(exporter, bsdf.inputs["Color"], 3, node_tab),
            "roughness": shadernode.parse_node(exporter, bsdf.inputs["Roughness"], 1, node_tab),
        },
    }
    return ret


def export_principled(exporter, bsdf, node_tab):
    
    normal = None
    if bsdf.inputs["Normal"].is_linked:
        normal = shadernode.parse_node(exporter, bsdf.inputs["Normal"], 3, node_tab)
    
    ret = {
        "type": "principled_bsdf",
        "param": {
            "color": shadernode.parse_node(exporter, bsdf.inputs["Base Color"], 3, node_tab),
            "roughness": shadernode.parse_node(exporter, bsdf.inputs["Roughness"], 1, node_tab),
            "ior": shadernode.parse_node(exporter, bsdf.inputs["IOR"], 1, node_tab),
            "metallic": shadernode.parse_node(exporter, bsdf.inputs["Metallic"], 1, node_tab),
            "spec_tint" : shadernode.parse_node(exporter, bsdf.inputs["Specular Tint"], 3, node_tab),
            "anisotropic" : shadernode.parse_node(exporter, bsdf.inputs["Anisotropic"], 1, node_tab),
            
            "normal" : normal,
            
            "sheen_weight" : shadernode.parse_node(exporter, bsdf.inputs["Sheen Weight"], 1, node_tab),
            "sheen_roughness" : shadernode.parse_node(exporter, bsdf.inputs["Sheen Roughness"], 1, node_tab),
            "sheen_tint" : shadernode.parse_node(exporter, bsdf.inputs["Sheen Tint"], 3, node_tab),
            
            "coat_weight" : shadernode.parse_node(exporter, bsdf.inputs["Coat Weight"], 1, node_tab),
            "coat_roughness" : shadernode.parse_node(exporter, bsdf.inputs["Coat Roughness"], 1, node_tab),
            "coat_ior" : shadernode.parse_node(exporter, bsdf.inputs["Coat IOR"], 1, node_tab),
            "coat_tint" : shadernode.parse_node(exporter, bsdf.inputs["Coat Tint"], 3, node_tab),
            
            "subsurface_weight" : shadernode.parse_node(exporter, bsdf.inputs["Subsurface Weight"], 1, node_tab),
            "subsurface_radius" : shadernode.parse_node(exporter, bsdf.inputs["Subsurface Radius"], 3, node_tab),
            "subsurface_scale" : shadernode.parse_node(exporter, bsdf.inputs["Subsurface Scale"], 1, node_tab),
            
            "transmission_weight" : shadernode.parse_node(exporter, bsdf.inputs["Transmission Weight"], 1, node_tab),
        },
    }
    return ret


def export_glass(exporter, bsdf, node_tab):
    ret = {
        "type": "glass",
        "param": {
            "color": shadernode.parse_node(exporter, bsdf.inputs["Color"], 3, node_tab),
            "roughness": shadernode.parse_node(exporter, bsdf.inputs["Roughness"], 1, node_tab),
            "ior": shadernode.parse_node(exporter, bsdf.inputs["IOR"], 1, node_tab),
        },
    }
    return ret


def export_mirror(exporter, bsdf, node_tab):
    ret = {
        "type": "mirror",
        "param": {
            "color": shadernode.parse_node(exporter, bsdf.inputs["Color"], 3, node_tab),
            "roughness": shadernode.parse_node(exporter, bsdf.inputs["Roughness"], 1, node_tab),
            "anisotropic": shadernode.parse_node(exporter, bsdf.inputs["Anisotropy"], 1, node_tab),
        },
    }
    return ret


def export_mix(exporter, bsdf, node_tab):
    node0 = bsdf.inputs[0]
    node1 = bsdf.inputs[1].links[0].from_node
    node2 = bsdf.inputs[2].links[0].from_node
    ret = {
        "type": "mix",
        "param": {
            "frac": shadernode.parse_node(exporter, node0, 1, node_tab),
            "mat0": func_tab[node1.type](exporter, node1, node_tab),
            "mat1": func_tab[node2.type](exporter, node2, node_tab),
        },
    }
    return ret


def export_emission(exporter, bsdf, node_tab):
    socket = bsdf.inputs["Color"]
    ret = {
        "type": "area",
        "param": {
            "color": shadernode.parse_node(exporter, socket, 3, node_tab),
            "scale": bsdf.inputs["Strength"].default_value,
        },
    }
    return ret


def export_add(exporter, bsdf, node_tab):
    node0 = bsdf.inputs[0].links[0].from_node
    node1 = bsdf.inputs[1].links[0].from_node

    emission_node = node0 if node0.type == "EMISSION" else node1
    material_node = node1 if node0.type == "EMISSION" else node0
    ret = {
        "type": "add",
        "param": {
            "material": func_tab[material_node.type](exporter, material_node, node_tab),
            "emission": func_tab[emission_node.type](exporter, emission_node, node_tab),
        },
    }
    return ret


func_tab = {
    "BSDF_DIFFUSE": export_diffuse,
    "BSDF_PRINCIPLED": export_principled,
    "BSDF_GLASS": export_glass,
    "BSDF_GLOSSY": export_mirror,
    "MIX_SHADER": export_mix,
    "ADD_SHADER": export_add,
    "EMISSION": export_emission,
}


def export(exporter, material, materials, node_tab=None):
    output_node_id = "Material Output"
    output = material.node_tree.nodes[output_node_id]
    bsdf = output.inputs["Surface"].links[0].from_node

    if material.name in materials:
        return materials[material.name]
    export_func = func_tab[bsdf.type]
    node_tab = {} if node_tab is None else node_tab

    data = export_func(exporter, bsdf, node_tab)
    data["node_tab"] = node_tab
    if bsdf.type == "ADD_SHADER":
        materials[material.name] = data["param"]["material"]
    else:
        materials[material.name] = data
    return data

