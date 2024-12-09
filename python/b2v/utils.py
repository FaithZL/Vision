import bpy
import os
import json
from bpy.props import (
    BoolProperty,
    CollectionProperty,
    EnumProperty,
    FloatProperty,
    IntProperty,
    PointerProperty,
    StringProperty,
)


def matrix_to_list(matrix):
    matrix = matrix.transposed()
    return [
        list(matrix[0]),
        list(matrix[1]),
        list(matrix[2]),
        list(matrix[3]),
    ]
