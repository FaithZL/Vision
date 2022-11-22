# -*- coding:utf-8 -*-

import json
from pathlib import Path
from posixpath import abspath
from sys import argv
import os
import glm

g_textures = []
g_lights = []

table = {
    "gamma" : 0,
    "filmic" : 1,
    "reinhard" : 2,
    "linear" : 3,
}

def write_scene(scene_output, filepath):
    with open(filepath, "w") as outputfile:
        json.dump(scene_output, outputfile, indent=4)
    abspath = os.path.join(os.getcwd(), filepath)
    print("lumi scene save to:", abspath)

def parse_attr(attr):
    if type(attr) == str:
        try_add_textures(attr)
    return attr

def convert_vec(value, dim=3):
    if type(value) == str:
        return parse_attr(value)
    if type(value) != list:
        ret = []
        for i in range(0, dim):
            ret.append(value)
        return ret
    assert len(value) == dim
    return value


def convert_roughness(r):
    return glm.sqrt(r)

def convert_matte(mat_input):
    ret = {
        "type" : "matte",
        "name" : mat_input["name"],
        "param" :{
            "color" : convert_vec(mat_input["albedo"], 3)
        }
    }
    return ret

def convert_glass(mat_input):
    ret = {
        "type" : "glass",
        "name" : mat_input["name"],
        "param" : {
            "ior" : mat_input["ior"],
            "roughness" :convert_vec(mat_input.get("roughness", 0), 2),
            "color" : convert_vec(mat_input.get("albedo", 1), 3)
        }
    }
    return ret

def convert_substrate(mat_input):
    ret = {
        "type" : "substrate",
        "name" : mat_input["name"],
        "param" : {
            "color" : convert_vec(mat_input.get("albedo", 1), 3),
            "ior": 1.5,
            "specular" : [0.04,0.04,0.04],
            "roughness": convert_vec(mat_input.get("roughness", 0.01), 2),
            "remapping_roughness" : False
        }
    }
    return ret

def convert_glass(mat_input):
    ret = {
        "type" : "glass",
        "name" : mat_input["name"],
        "param" : {
            "ior" : mat_input["ior"],
            "roughness" :convert_vec(mat_input.get("roughness", 0), 2),
            "color" : convert_vec(mat_input.get("albedo", 1), 3)
        }
    }
    return ret

def convert_metal(mat_input):
    ret = {
        "type" : "metal",
        "name" : mat_input["name"],
        "param" : {
            "metal_name" : mat_input.get("material", ""),
            "eta" : convert_vec(mat_input.get("eta", 0), 3),
            "k" : convert_vec(mat_input.get("k", 0), 3),
            "roughness" :convert_vec(mat_input.get("roughness", 0.01), 2),
            "remapping_roughness" : False
        }
    }
    return ret

def convert_mirror(mat_input):
    ret = {
        "type" : "mirror",
        "name" : mat_input["name"],
        "param" : {
            "color" : convert_vec(mat_input.get("albedo", 1), 3),
            "roughness" : 0.001
        }
    }
    return ret

def convert_null_material(mat_input):
    ret = {
        "type": "null",
        "name": mat_input["name"],
        "param": {}
    }
    return ret

def convert_disney(mat_input):
    ret = {
        "type" : "DisneyMaterial",
        "name" : mat_input["name"],
        "param" : {
            "color" : convert_vec(mat_input.get("albedo", 1), 3),
            "ior": 1.5,
            "roughness":0.1,
            "metallic": 0,
            "spec_tint": 0.9,
            "anisotropic": 0,
            "sheen": 0.0,
            "sheen_tint": 0.0,
            "clearcoat": 0.59,
            "clearcoat_alpha": 0.38,
            "spec_trans": 0,
            "flatness": 0,
            "scatter_distance": [0,0,0],
            "diff_trans": 0,
            "thin": False
        }
    }
    return ret

def convert_camera(scene_input):
    return {}

def convert_integrator(scene_input):
    return {}

def convert_sampler(scene_input):
    return {}

def convert_shapes(scene_input):
    return []



def convert_materials(scene_input):
    mat_inputs = scene_input["bsdfs"]
    mat_outputs = []
    for mat_input in mat_inputs:
        mat_type = mat_input["type"]
        mat_output = None
        if mat_type == "lambert" or mat_type == "oren_nayar":
            mat_output = convert_matte(mat_input)
        elif mat_type == "dielectric" or mat_type == "rough_dielectric":
            mat_output = convert_glass(mat_input)
        elif mat_type == "mirror":
            mat_output = convert_mirror(mat_input)
        elif mat_type == "conductor" or mat_type == "rough_conductor":
            mat_output = convert_metal(mat_input)
        elif mat_type == "plastic" or mat_type == "rough_plastic":
            mat_output = convert_substrate(mat_input)
        elif mat_type == "null":
            mat_output = convert_null_material(mat_input)
        elif mat_type != "null":
            mat_output = convert_disney(mat_input)

        if mat_output:
            mat_outputs.append(mat_output)
            
    return mat_outputs

def convert_lightsampler(scene_input):
    return {}

def main():
    fn = 'res\\render_scene\\cornell-box\\tungsten_scene.json'
    parent = os.path.dirname(fn)
    output_fn = os.path.join(parent, "vision_scene.json")

    with open(fn) as file:
        scene_input = json.load(file)
        
    scene_output = {
        "materials" : convert_materials(scene_input),
        "shapes" : convert_shapes(scene_input),
        "sampler" : convert_sampler(scene_input),
        "camera" : convert_camera(scene_input),
        "light_sampler" : convert_lightsampler(scene_input),
        "warper" :{
            "type" : "alias_table"
        },
        "integrator" : convert_integrator(scene_input),
    }
    write_scene(scene_output, output_fn)
    

if __name__ == "__main__":
    main()