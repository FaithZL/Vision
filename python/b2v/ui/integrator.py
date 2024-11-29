from . import base
from .base import *

dic = {
    "path tracing": {
        "label": "path tracing",
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


class VisionIntegratorSetting(VisionBaseSetting):
    attr_type = "integrator_type"
    dic = dic

    @classmethod
    def register(cls):
        cls.register_impl()


class VISION_RENDER_PT_Intergrator(bpy.types.Panel, VISION_RENDER_PT_VisionBasePanel):
    dic = dic
    bl_idname = "VISION_RENDER_PT_Intergrator"
    bl_label = "Intergrator"
    property_cls = VisionIntegratorSetting
