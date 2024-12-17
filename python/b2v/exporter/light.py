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
from ..import utils
from mathutils import Matrix, Vector
import numpy as np

def export_area(exporter, object):
    light = object.data
    width = 1
    height = 1
    if light.shape == "SQUARE":
        width = light.size * object.scale.x
        height = light.size * object.scale.y
    elif light.shape == "RECTANGLE":
        width = light.size * object.scale.x
        height = light.size_y * object.scale.y

    
    rotation_matrix_x = Matrix.Rotation(math.radians(-90), 4, 'X')
    mat = exporter.correct_matrix(object.matrix_world)
    mat_list = utils.matrix_to_list(mat @ rotation_matrix_x)
    
    ret = {
        "type": "area",
        "param": {
            "color": {"channels": "xyz", "node": list(light.color)},
            "scale": light.energy,
            "width": width,
            "height": height,
            "o2w" : {
                'type': 'matrix4x4',
                'param': {
                    'matrix4x4': mat_list
                }
            }
        },
    }
    return ret


def export_point(exporter, object):
    light = object.data
    pos = object.location
    p = exporter.correct_matrix(pos)
    ret = {
        "type": "point",
        "param" :{
            "color": {"channels": "xyz", "node": list(light.color)},
            "scale": light.energy / (4 * np.pi),
            "position" : list(p)
        }
    }
    return ret


def export_spot(exporter, object):
    light = object.data
    pos = object.location
    p = exporter.correct_matrix(pos)
    rotation_euler = object.rotation_euler
    direction_vector = Vector((0.0, 0.0, -1.0))
    direction_vector.rotate(rotation_euler)
    ret = {
        "type": "spot",
        "param" :{
            "color": {"channels": "xyz", "node": list(light.color)},
            "scale": light.energy / (4 * np.pi),
            "position" : list(p)
        }
    }
    return ret


func_tab = {
    "AREA": export_area,
    "POINT": export_point,
    "SPOT": export_spot,
}


def export(exporter, object):
    print("------------------------", object.data.type)
    ret = func_tab[object.data.type](exporter, object)
    return ret
