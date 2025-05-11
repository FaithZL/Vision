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


def parse_image_node(exporter, link, dim, node_tab):
    from_node = link.from_node
    exporter.try_make_tex_dir()
    src_path = bpy.path.abspath(from_node.image.filepath)
    dst_path = exporter.texture_path(from_node.image.name)
    shutil.copyfile(src_path, dst_path)
    r_path = exporter.tex_dir + "/" + from_node.image.name
    _, extension = os.path.splitext(r_path)
    color_space = "linear" if extension in [".hdr", ".exr"] else "srgb"
    fs = link.from_socket
    if fs.name == "Color":
        channels = "xyz" if dim == 3 else "x"
    elif fs.name == "Alpha":
        channels = "w" if dim == 1 else "www"
        
    node_name = str(link.from_node)
    
    val = {
            "type": "image",
            "param": {
                "fn": r_path,
                "color_space": color_space,
            },
        }
        
    ret = {
        "channels": channels,
        "output_key" : fs.name, 
        "node": node_name,
    }
    
    if not (node_name in node_tab):
        node_tab[node_name] = val
    return ret


def parse_mix(exporter, link, dim, node_tab):
    pass


def parse_add(exporter, link, dim, node_tab):
    pass


func_dict = {
    "TEX_IMAGE": parse_image_node,
    "TEX_ENVIRONMENT": parse_image_node,
    "MIX_SHADER": parse_mix,
    "ADD_SHADER": parse_add,
}


def parse_node(exporter, socket, dim, node_tab):
    if socket.is_linked:
        link = socket.links[0]
        func = func_dict[link.from_node.type]
        return func(exporter, link, dim, node_tab)
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
