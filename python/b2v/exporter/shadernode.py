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


func_dict = {
    "TEX_IMAGE": parse_image_node,
    "TEX_ENVIRONMENT": parse_image_node,
}


def parse_node(exporter, socket, dim, min=0, max=1):
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
                    "min": min,
                    "max": max,
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
                    "min": min,
                    "max": max,
                },
            },
        }
