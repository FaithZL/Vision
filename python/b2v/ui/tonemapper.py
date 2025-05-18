from . import base
from .base import *

dic = {
    "linear": {
        "label": "linear",
        "description": "linear tone mapping.",
        "parameters": {},
    },
    "aces": {
        "label": "aces",
        "description": "ACES tone mapping.",
        "parameters": {},
    },
    "reinhard": {
        "label": "reinhard",
        "description": "reinhard tone mapping.",
        "parameters": {},
    },
    "exposure": {
        "label": "exposure",
        "description": "exposure tone mapping.",
        "parameters": {
            "exposure": {"type": "Float", "args": {"default": 1, "max": 5, "min": 0.1}},
        },
    },
}


class VISION_RENDER_PT_ToneMapper(bpy.types.Panel, VISION_RENDER_PT_VisionBasePanel):
    bl_idname = "VISION_RENDER_PT_ToneMapper"
    bl_label = "ToneMapper"
    attr_type = "tone_mapper"
    bl_parent_id = "VISION_RENDER_PT_Camera"
    dic = dic
