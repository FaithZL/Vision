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
    print(object.matrix_world)
    print(transform)
    print(utils.matrix_to_list(transform))
    ret = exporter.get_params("Camera")
    param = {
        "fov_y": math.degrees(camera.angle_y),
        "name": object.name,
        "filter": exporter.get_params("filter"),
        "transform": {
            "type": "matrix4x4",
            "param": {"matrix4x4": utils.matrix_to_list(transform)},
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
