from . import base
from .base import *

dic = {
    "box": {
        "label": "box",
        "description": "box reconstruction filter.",
        "parameters": {
            "radius": {"type": "Float", "args": {"default": 0.5, "max": 5, "min": 0.1}}
        },
    },
    "triangle": {
        "label": "triangle",
        "description": "Simple triangular reconstruction filter.",
        "parameters": {
            "radius": {"type": "Float", "args": {"default": 1, "max": 5, "min": 0.1}}
        },
    },
    "gaussian": {
        "label": "gaussian",
        "description": "Windowed gaussian reconstruction filter.",
        "parameters": {
            "radius": {"type": "Float", "args": {"default": 2, "max": 5, "min": 0.1}},
            "sigma": {"type": "Float", "args": {"default": 1, "max": 5, "min": 0.1}},
        },
    },
    "sinc": {
        "label": "sinc",
        "description": "LanczosSinc reconstruction filter.",
        "parameters": {
            "radius": {"type": "Float", "args": {"default": 1, "max": 5, "min": 0.1}},
            "tau": {"type": "Float", "args": {"default": 3, "max": 5, "min": 0.1}},
        },
    },
    "mitchell": {
        "label": "mitchell",
        "description": "mitchell reconstruction filter.",
        "parameters": {
            "radius": {"type": "Float", "args": {"default": 1, "max": 5, "min": 0.1}},
            "b": {"type": "Float", "args": {"default": 0.333, "max": 5, "min": 0.1}},
            "c": {"type": "Float", "args": {"default": 0.333, "max": 5, "min": 0.1}},
        },
    },
}


class VisionFilterSetting(VisionBaseSetting):
    attr_type = "filter_type"
    dic = dic

    @classmethod
    def register(cls):
        cls.register_impl()


class VISION_RENDER_PT_Filter(bpy.types.Panel, VISION_RENDER_PT_VisionBasePanel):
    dic = dic
    bl_idname = "VISION_RENDER_PT_Filter"
    bl_label = "Filter"
    property_cls = VisionFilterSetting
