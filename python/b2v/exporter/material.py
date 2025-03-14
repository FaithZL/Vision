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


def export_principled(exporter, bsdf):
    ret = {
        "type": "principled_bsdf",
        "param": {
            "color": shadernode.parse_node(exporter, bsdf.inputs["Base Color"], 3),
            "roughness": shadernode.parse_node(exporter, bsdf.inputs["Roughness"], 1),
            "ior": shadernode.parse_node(exporter, bsdf.inputs["IOR"], 1),
            "metallic": shadernode.parse_node(exporter, bsdf.inputs["Metallic"], 1),
            "spec_tint" : shadernode.parse_node(exporter, bsdf.inputs["Specular Tint"], 3),
            "anisotropic" : shadernode.parse_node(exporter, bsdf.inputs["Anisotropic"], 1),
            
            "sheen_weight" : shadernode.parse_node(exporter, bsdf.inputs["Sheen Weight"], 1),
            "sheen_roughness" : shadernode.parse_node(exporter, bsdf.inputs["Sheen Roughness"], 1),
            "sheen_tint" : shadernode.parse_node(exporter, bsdf.inputs["Sheen Tint"], 3),
            
            "coat_weight" : shadernode.parse_node(exporter, bsdf.inputs["Coat Weight"], 1),
            "coat_roughness" : shadernode.parse_node(exporter, bsdf.inputs["Coat Roughness"], 1),
            "coat_ior" : shadernode.parse_node(exporter, bsdf.inputs["Coat IOR"], 1),
            "coat_tint" : shadernode.parse_node(exporter, bsdf.inputs["Coat Tint"], 3),
            
            "subsurface_weight" : shadernode.parse_node(exporter, bsdf.inputs["Subsurface Weight"], 1),
            "subsurface_radius" : shadernode.parse_node(exporter, bsdf.inputs["Subsurface Radius"], 3),
            "subsurface_scale" : shadernode.parse_node(exporter, bsdf.inputs["Subsurface Scale"], 1),
            
            "transmission_weight" : shadernode.parse_node(exporter, bsdf.inputs["Transmission Weight"], 1),
        },
    }
    return ret


def export_glass(exporter, bsdf):
    ret = {
        "type": "glass",
        "param": {
            "color": shadernode.parse_node(exporter, bsdf.inputs["Color"], 3),
            "roughness": shadernode.parse_node(exporter, bsdf.inputs["Roughness"], 1),
            "ior": shadernode.parse_node(exporter, bsdf.inputs["IOR"], 1),
        },
    }
    return ret


def export_mirror(exporter, bsdf):
    roughness = shadernode.parse_node(exporter, bsdf.inputs["Roughness"], 1)
    ret = {
        "type": "mirror",
        "param": {
            "color": shadernode.parse_node(exporter, bsdf.inputs["Color"], 3),
            "roughness": roughness,
            "anisotropic": shadernode.parse_node(
                exporter, bsdf.inputs["Anisotropy"], 1, -1, 1
            ),
        },
    }
    return ret


def export_mix(exporter, bsdf):
    ret = {"type": "mix"}
    return ret


def export_emission(exporter, bsdf):
    socket = bsdf.inputs["Color"]
    ret = {
        "type": "area",
        "param": {
            "color": shadernode.parse_node(exporter, socket, 3),
            "scale": bsdf.inputs["Strength"].default_value,
        },
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
    "BSDF_PRINCIPLED": export_principled,
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
        return materials[material.name]
    export_func = func_tab[bsdf.type]
    data = export_func(exporter, bsdf)

    if bsdf.type == "ADD_SHADER":
        materials[material.name] = data["param"]["material"]
    else:
        materials[material.name] = data
    return data
