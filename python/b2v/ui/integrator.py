from . import base
from .base import *

dic = {
    "pt": {
        "label": "pt",
        "description": "path tracing integrator",
        "parameters": {
            "max_depth": {"type": "Int", "args": {"default": 8, "max": 50, "min": 1}},
            "min_depth": {"type": "Int", "args": {"default": 3, "max": 10, "min": 0}},
            "rr_threshold": {
                "type": "Float",
                "args": {"default": 1, "max": 1, "min": 0},
            },
        },
    }
}

class VISION_RENDER_PT_Intergrator(bpy.types.Panel, VISION_RENDER_PT_VisionBasePanel):
    bl_idname = "VISION_RENDER_PT_Intergrator"
    bl_label = "Intergrator"
    attr_type = "integrator"
    dic = dic
