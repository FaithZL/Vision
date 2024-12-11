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


def export(exporter, object):
    camera = object.data
    res_x = exporter.context.scene.render.resolution_x
    res_y = exporter.context.scene.render.resolution_y
    transform = exporter.correct_matrix(object.matrix_world)
    ret = exporter.get_params("Camera")
    
    mat = utils.to_mat(object.matrix_world)
    pos = mat[3][0:3]
    euler = object.rotation_euler
    pitch = math.degrees(euler.x) - 90
    yaw = -math.degrees(euler.z)
    
    param = {
        "fov_y": math.degrees(camera.angle_y) * 1.5,
        "name": object.name,
        "filter": exporter.get_params("filter"),
        "transform": {
            "type": "Euler",
            "param": {
                "yaw": yaw,
                "pitch": pitch,
                "position": [pos[0], pos[2], -pos[1]]
            }
        },
        "film": {
            "type": "rgb",
            "param": {
                "resolution": [res_x, res_y],
                "tone_mapper": exporter.get_params("tone_mapper"),
            },
        },
    }
    ret["param"].update(param)
    return ret
