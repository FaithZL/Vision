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
    # print(utils.matrix_to_list(object.matrix_world))
    # print(utils.matrix_to_list(exporter.correct_matrix(object.matrix_world)))
    
    mat = utils.to_mat(object.matrix_world)
    # pos = mat[3][0:3]
    # euler = object.rotation_euler
    # pitch = math.degrees(euler.x) + 180
    # roll = math.degrees(euler.y)
    # yaw = -math.degrees(euler.z)
    
    
    # mat = utils.matrix_to_list(exporter.correct_matrix(object.matrix_world))
    

    mat = np.matmul(mat, utils.to_luminous())
    
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
                        'matrix4x4': mat.tolist()

                    }
            }
        },
    }
    return ret


def export_point(exporter, object):
    pass


def export_spot(exporter, object):
    pass


func_tab = {
    "AREA": export_area,
    "POINT": export_point,
    "SPOT": export_spot,
}


def export(exporter, object):
    ret = func_tab[object.data.type](exporter, object)
    return ret
