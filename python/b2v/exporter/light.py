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
from mathutils import Matrix
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
    
    ret = {
        "type": "point",
        "param" :{
            "color": {"channels": "xyz", "node": list(light.color)},
        }
    }
    return ret


def export_spot(exporter, object):
    light = object.data
    
    ret = {
        "type": "spot",
        "param" :{
            "color": {"channels": "xyz", "node": list(light.color)},
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
