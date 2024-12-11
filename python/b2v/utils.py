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
import numpy as np

def to_mat(matrix4x4):
    items = []
    for col in matrix4x4.col:
        items.extend(col)
    mat = np.array(items).reshape(4, 4)
    return mat

def matrix_to_list(matrix):
    matrix = matrix.transposed()
    return [
        list(matrix[0]),
        list(matrix[1]),
        list(matrix[2]),
        list(matrix[3]),
    ]
