from . import base
from .base import *

dic = {
    "spherical": {
        "label": "spherical",
        "description": "spherical integrator",
        "parameters": {
            "scale": {"type": "Float", "args": {"default": 1, "max": 50, "min": 0}},
            "color_space": {
                "type": "Enum",
                "args": {
                    "name": "color_space",
                    "items": [
                        ("linear", "linear", "linear", 0),
                        ("SRGB", "SRGB", "SRGB", 1),
                    ],
                },
            },
            "fn": {"type": "String", "args": {"subtype": "FILE_PATH"}},
        },
    }
}


class VisionEnvironmentSetting(VisionBaseSetting):
    setting_name = "vision_environment_setting"
    attr_type = "environment_type"
    dic = dic

    @classmethod
    def register(cls):
        cls.register_impl()


class VISION_RENDER_PT_Environment(bpy.types.Panel, VISION_RENDER_PT_VisionBasePanel):
    bl_idname = "VISION_RENDER_PT_Environment"
    bl_label = "Environment"
    property_cls = VisionEnvironmentSetting
