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
import numpy as np

def rotate_x(theta):
    theta = math.radians(theta)
    sinTheta = math.sin(theta)
    cosTheta = math.cos(theta)
    mat = [
        [1, 0,        0,         0],
        [0, cosTheta, sinTheta, 0],
        [0, -sinTheta, cosTheta,  0],
        [0, 0,        0,         1]
    ]
    return np.array(mat)

def scale(s):
    mat = [
        [s[0], 0, 0, 0],
        [0, s[1], 0, 0],
        [0, 0, s[2], 0],
        [0, 0, 0,     1]
    ]
    return np.array(mat)

def to_mat(matrix4x4):
    items = []
    for col in matrix4x4.col:
        items.extend(col)
    mat = np.array(items).reshape(4, 4)
    return mat

def to_luminous():
    r = rotate_x(0)
    s = scale([1, 1, -1])
    t = np.matmul(s, r)
    return t

def matrix_to_list(matrix):
    matrix = matrix.transposed()
    return [
        list(matrix[0]),
        list(matrix[1]),
        list(matrix[2]),
        list(matrix[3]),
    ]
