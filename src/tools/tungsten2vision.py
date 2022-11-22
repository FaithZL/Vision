# -*- coding:utf-8 -*-

import json
from pathlib import Path
from posixpath import abspath
from sys import argv
import os
import glm

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
        ret = {
            "fn" : attr,
            "color_space" : "srgb"
        }
        return ret
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
        "type": "matte",
        "name": mat_input["name"],
        "param": {
            "color" : [0,0,0]
        }
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
    camera_input = scene_input["camera"]
    transform = camera_input["transform"]
    ret = {
        "type" : "thin_lens",
        "param" : {
            "fov_y" : camera_input["fov"],
            "name": "view1",
            "velocity" : 5,
            "transform" : {
                "type" : "look_at",
                "param": {
                    "position" : transform["position"],
                    "up" : transform["up"],
                    "target_pos" : transform["look_at"]
                }
            },
            "film" : {
                "type": "rgb",
                "param" : {
                    "resolution" : convert_vec(camera_input.get("resolution", [768, 768]), 2),
                    "fb_state": 0
                }
            },
            "filter": {
                "type": "box",
                "name": "boxFilter",
                "param": {
                    "radius": [
                        0.5,
                        0.5
                    ]
                }
            }
        },
    }
    return ret

def convert_integrator(scene_input):
    integrator = scene_input["integrator"]
    ret = {
        "type" : "pt",
        "param" : {
			"min_depth" : integrator["min_bounces"],
			"max_depth" : integrator["max_bounces"],
			"rr_threshold" : 1
		}
    }
    return ret

def convert_sampler(scene_input):
    ret = {
        "type": "independent",
        "name": "sampler0",
        "param": {
            "spp": 1
        }
    }
    return ret


def rotateZXY(R):
    return glm.rotate(R.y, (0, 1, 0)) * glm.rotate(R.x, (1, 0, 0)) * glm.rotate(R.z, (0, 0, 1)) 

def convert_srt(S, R, T):
    return glm.translate(T) * rotateZXY(R) * glm.scale(S)

def convert_shape_transform(transform, scale=1):
    T = glm.vec3(transform.get("position", [0,0,0]))
    R = glm.radians(glm.vec3(transform.get("rotation", [0,0,0])))
    S = glm.vec3(transform.get("scale", [1,1,1]))
    M = convert_srt(S, R, T)
    M = glm.scale(glm.vec3([-1,1,1])) * M
    matrix4x4 = []
    for i in M:
        arr = []
        matrix4x4.append(arr)
        for j in i:
            arr.append(j)
            
    ret = {
        "type" : "matrix4x4",
        "param" : {
            "matrix4x4" : matrix4x4
        }
    }
    return ret

def convert_quad(shape_input, index):
    ret = {
        "type" : "quad",
        "name" : "shape_" + str(index),
        "param" : {
            "width": 1.0,
            "height": 1.0,
            "material" : shape_input["bsdf"],
            # "swap_handed" : True,
            "transform" : convert_shape_transform(shape_input["transform"], -1)
        }
    }
    return ret


def get_emission(shape):
    if "emission" in shape:
        return convert_vec(shape["emission"], 3)
    else:
        power_scale = 100 * glm.pi()
        power = convert_vec(shape["power"], 3)
        emission = glm.vec3(power) / power_scale;
        return [emission[0], emission[1], emission[2]]

def convert_area_light(shape_input, shape_output):
    shape_output["param"]["emission"] = {
        "type" : "area",
        "param" : {
            "radiance" : get_emission(shape_input),
            "two_sided" : False,
            "scale" : 1
        }
    }

def convert_envmap(shape_input, shape_output):
    assert False

def convert_light(shape_input, shape_output):
    type = shape_input["type"]
    if type == "infinite_sphere":
        convert_envmap(shape_input, shape_output)
    else:
        convert_area_light(shape_input, shape_output)
        
def convert_cube(shape_input, index):
    ret = {
        "type" : "cube",
        "name" : "shape_" + str(index),
        "param" : {
            "x" : 1,
            "y" : 1,
            "z" : 1,
            # "swap_handed" : True,
            "material" : shape_input["bsdf"],
            "transform" : convert_shape_transform(shape_input["transform"], -1)
        }
    }
    return ret


def convert_shapes(scene_input):
    shape_outputs = []
    shape_inputs = scene_input["primitives"]
    for i, shape_input in enumerate(shape_inputs):
        shape_output = None
        if shape_input["type"] == "quad":
            shape_output = convert_quad(shape_input, i)
        elif shape_input["type"] == "cube":    
            shape_output = convert_cube(shape_input, i)

        if "emission" in shape_input or "power" in shape_input:
            convert_light(shape_input, shape_output)
        if shape_output:
            shape_outputs.append(shape_output)
    return shape_outputs

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
    ret = {
        "type" : "uniform",
        "param" : {
            "lights": g_lights
        }
    }
    return ret

def main():
    fn = 'res\\render_scene\\cornell-box\\tungsten_scene.json'
    # fn = 'res\\render_scene\\staircase\\tungsten_scene.json'
    parent = os.path.dirname(fn)
    output_fn = os.path.join(parent, "vision_scene.json")

    with open(fn) as file:
        scene_input = json.load(file)
        
    scene_output = {
        "materials" : convert_materials(scene_input),
        "shapes" : convert_shapes(scene_input),
        "sampler" : convert_sampler(scene_input),
        "camera" : convert_camera(scene_input),
        "warper" :{
            "type" : "alias_table"
        },
        "integrator" : convert_integrator(scene_input),
        "light_sampler" : convert_lightsampler(scene_input),
    }
    write_scene(scene_output, output_fn)
    

if __name__ == "__main__":
    main()