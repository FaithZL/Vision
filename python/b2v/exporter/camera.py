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

def export_filter(exporter):
    pass

def export(exporter, object):
    camera = object.data
    res_x = exporter.context.scene.render.resolution_x
    res_y = exporter.context.scene.render.resolution_y
    print((object.matrix_world))
    print(exporter.correct_matrix())
    print(exporter.correct_matrix() * object.matrix_world)
    print(object.matrix_world * exporter.correct_matrix())

    m1 = np.array(utils.matrix_to_list(object.matrix_world))
    m2 = np.array(utils.matrix_to_list(exporter.correct_matrix()))
    
    m3 = np.matmul(m1, m2)
    print(m3)
    
    m3 = np.matmul(m2, m1)
    print(m3)

    ret = {
        "type" : "thin_lens",
        "param" : {
            "fov_y" : math.degrees(camera.angle_y),
			"name" : object.name,
			"velocity" : 15,
            "transform" : {
                "type" : "matrix4x4",
                "param" : {
                    "matrix4x4" : utils.matrix_to_list(object.matrix_world)
                }
            }
        }
    }
    return ret