import bpy
import os
import json
import shutil
from bpy.props import (
    BoolProperty,
    CollectionProperty,
    EnumProperty,
    FloatProperty,
    IntProperty,
    PointerProperty,
    StringProperty,
)


def parse_image_node(exporter, from_node, dim):
    exporter.try_make_tex_dir()
    src_path = bpy.path.abspath(from_node.image.filepath)
    dst_path = exporter.texture_path(from_node.image.name)
    shutil.copyfile(src_path, dst_path)
    r_path = exporter.tex_dir + "/" + from_node.image.name
    _, extension = os.path.splitext(r_path)
    color_space = "linear" if extension in [".hdr", ".exr"] else "srgb"
    channels = "w" if dim == 1 else "xyz"
    ret = {
        "channels": channels,
        "node": {
            "type": "image",
            "param": {
                "fn": r_path,
                "color_space": color_space,
            },
        },
    }
    return ret


def parse_mix(exporter, from_node, dim):
    pass


def parse_add(exporter, from_node, dim):
    pass


func_dict = {
    "TEX_IMAGE": parse_image_node,
    "TEX_ENVIRONMENT": parse_image_node,
    "MIX_SHADER": parse_mix,
    "ADD_SHADER": parse_add,
}


def parse_node(exporter, socket, dim, node_tab):
    if socket.is_linked:
        from_node = socket.links[0].from_node
        func = func_dict[from_node.type]
        return func(exporter, from_node, dim)
    if dim == 1:
        value = socket.default_value
        return {
            "channels": "x",
            "node": {
                "type": "number",
                "param": {
                    "value": value,
                },
            },
        }
    else:
        value = list(socket.default_value)[0:dim]
        return {
            "channels": "xyz",
            "node": {
                "type": "number",
                "param": {
                    "value": value,
                },
            },
        }
