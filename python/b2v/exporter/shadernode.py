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


def parse_image_node(exporter, socket, dim):
    ret = {}
    print("-----------------------", socket.type)
    return ret


def parse_node(exporter, socket, dim):
    if socket.is_linked:
        return parse_image_node(exporter, socket, dim)
    value = list(socket.default_value)[0:dim]
    return {"channels": "xyz", "node": value}
