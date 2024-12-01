from . import base
from .base import *

dic = {
    "independent": {
        "label": "independent",
        "description": "independent sampler",
        "parameters": {
            "spp": {"type": "Int", "args": {"default": 128, "max": 1000000, "min": 1}}
        },
    }
}

class VISION_RENDER_PT_Sampler(bpy.types.Panel, VISION_RENDER_PT_VisionBasePanel):
    bl_idname = "VISION_RENDER_PT_Sampler"
    bl_label = "Sampler"
    attr_type = "sampler"
    dic = dic
