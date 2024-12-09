from . import base
from .base import *

dic = {
    "thin_lens": {
        "label": "thin_lens",
        "description": "thin lens camera",
        "parameters": {
            "velocity": {"type": "Float", "args": {"default": 10, "max": 100, "min": 1}},
            "sensitivity": {"type": "Float", "args": {"default": 0.5, "max": 5, "min": 0.5}},
            "focal_distance": {"type": "Float", "args": {"default": 5, "max": 100, "min": 0}},
            "lens_radius": {"type": "Float", "args": {"default": 0, "max": 0.5, "min": 0}},
        },
    },
    "pinhole": {
        "label": "pinhole",
        "description": "pinhole camera",
        "parameters": {
            "velocity": {"type": "Float", "args": {"default": 10, "max": 100, "min": 1}},
            "sensitivity": {"type": "Float", "args": {"default": 0.5, "max": 5, "min": 0.5}},
        },
    }
}


class VISION_RENDER_PT_Camera(bpy.types.Panel, VISION_RENDER_PT_VisionBasePanel):
    bl_idname = "VISION_RENDER_PT_Camera"
    bl_label = "Camera"
    attr_type = "Camera"
    dic = dic
