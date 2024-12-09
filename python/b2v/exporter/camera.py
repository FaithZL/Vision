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

def export_filter(exporter):
    pass

def export(exporter, object):
    camera = object.data
    res_x = exporter.context.scene.render.resolution_x
    res_y = exporter.context.scene.render.resolution_y
    transform = exporter.correct_matrix(object.matrix_world)
    print(transform)
    ret = {
        "type" : "thin_lens",
        "param" : {
            "fov_y" : math.degrees(camera.angle_y),
			"name" : object.name,
			"velocity" : 15,
            "transform" : {
                "type" : "matrix4x4",
                "param" : {
                    "matrix4x4" : utils.matrix_to_list(transform)
                }
            }
        }
    }
    return ret