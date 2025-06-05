import bpy
import os
import json
import math
from bpy.props import (
    BoolProperty,
    CollectionProperty,
    EnumProperty,
    FloatProperty,
    IntProperty,
    PointerProperty,
    StringProperty,
)
from .. import utils
from mathutils import Matrix, Vector
import numpy as np
from . import shadernode


def export_area(exporter, object, node_tab):
    light = object.data
    width = 1
    height = 1
    if light.shape == "SQUARE":
        width = light.size * object.scale.x
        height = light.size * object.scale.y
    elif light.shape == "RECTANGLE":
        width = light.size * object.scale.x
        height = light.size_y * object.scale.y

    rotation_matrix_x = Matrix.Rotation(math.radians(-90), 4, "X")
    mat = exporter.correct_matrix(object.matrix_world)
    mat_list = utils.matrix_to_list(mat @ rotation_matrix_x)

    ret = {
        "type": "area",
        "param": {
            "color": {"channels": "xyz", "node": list(light.color)},
            "scale": light.energy,
            "width": width,
            "height": height,
            "o2w": {"type": "matrix4x4", "param": {"matrix4x4": mat_list}},
        },
    }
    return ret


def export_point(exporter, object, node_tab):
    light = object.data
    pos = object.location
    p = exporter.correct_matrix(pos)
    value = light.energy / (4 * np.pi)
    ret = {
        "type": "point",
        "param": {
            "color": {"channels": "xyz", "node": list(light.color)},
            "scale": value,
            "strength" :{
                "channels":"x",
                "node" : {
                    "type" : "number",
                    "param" : {
                        "min" : 0,
                        "max" : 1000,
                        "value" : value,
                    }
                }
            },
            "position": list(p),
        },
    }
    return ret


def export_spot(exporter, object, node_tab):
    light = object.data
    pos = object.location
    p = exporter.correct_matrix(pos)
    rotation_euler = object.rotation_euler
    direction_vector = Vector((0.0, 0.0, -1.0))
    direction_vector.rotate(rotation_euler)
    angle = math.degrees(light.spot_size / 2.0)
    direction = exporter.correct_matrix(direction_vector)
    b = light.spot_blend
    falloff = angle * b
    
    value = light.energy / (4 * np.pi)
    ret = {
        "type": "spot",
        "param": {
            "color": {"channels": "xyz", "node": list(light.color)},
            "scale": value,
            "position": list(p),
            "angle": angle,
            "falloff": falloff,
            "direction": list(direction),
            "strength" :{
                "channels":"x",
                "node" : {
                    "type" : "number",
                    "param" : {
                        "min" : 0,
                        "max" : 1000,
                        "value" : value,
                    }
                }
            },
        },
    }
    return ret


func_tab = {
    "AREA": export_area,
    "POINT": export_point,
    "SPOT": export_spot,
}


def export(exporter, object, node_tab=None):
    node_tab = {} if node_tab is None else node_tab
    ret = func_tab[object.data.type](exporter, object, node_tab)
    return ret

def export_environment(exporter):
    scene = bpy.context.scene
    if scene.world and scene.world.use_nodes:
        node_tab = {}
        world_nodes = scene.world.node_tree.nodes
        output = world_nodes["World Output"]
        env_surface = output.inputs["Surface"].links[0].from_node
        color = env_surface.inputs["Color"]
        scale = env_surface.inputs["Strength"].default_value
        if scale == 0:
            return None
        ret = {
            "type" : "spherical",
            "param" : {
                "color" : shadernode.parse_node(exporter, color, 3, node_tab),
                "scale" : scale,
                "o2w" : {
                    "type":"Euler",
                    "param": {
                        "yaw" :180
                    }
                },
            }
        }
        return ret