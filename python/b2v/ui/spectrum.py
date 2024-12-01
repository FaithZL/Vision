from . import base
from .base import *

dic = {
    "srgb": {"label": "srgb", "description": "SRGB Spectrum", "parameters": {}},
    "hero": {
        "label": "hero",
        "description": "HeroWavelength Spectrum",
        "parameters": {
            "dimension": {"type": "Int", "args": {"default": 3, "max": 20, "min": 1}}
        },
    },
}

class VISION_RENDER_PT_Spectrum(bpy.types.Panel, VISION_RENDER_PT_VisionBasePanel):
    bl_idname = "VISION_RENDER_PT_Spectrum"
    bl_label = "Spectrum"
    attr_type = "spectrum"
    dic = dic
