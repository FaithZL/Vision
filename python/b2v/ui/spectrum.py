from . import base
from .base import *

dic = {
    "SRGB": {"label": "SRGB", "description": "SRGB Spectrum", "parameters": {}},
    "HeroWavelength": {
        "label": "HeroWavelength",
        "description": "HeroWavelength Spectrum",
        "parameters": {
            "dimension": {"type": "Int", "args": {"default": 3, "max": 20, "min": 1}}
        },
    },
}


class VisionSpectrumSetting(VisionBaseSetting):
    attr_type = "spectrum_type"
    dic = dic

    @classmethod
    def register(cls):
        cls.register_impl()


class VISION_RENDER_PT_Spectrum(bpy.types.Panel, VISION_RENDER_PT_VisionBasePanel):
    bl_idname = "VISION_RENDER_PT_Spectrum"
    bl_label = "Spectrum"
    property_cls = VisionSpectrumSetting
