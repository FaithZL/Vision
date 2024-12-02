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


def export(context, material, materials):
    output_node_id = 'Material Output'
    output = material.node_tree.nodes[output_node_id]
    bsdf = output.inputs['Surface'].links[0].from_node
    socket = bsdf.inputs['Base Color']
    if socket.is_linked:
        print(socket.links[0].from_node)
    else:
        print(socket.default_value)