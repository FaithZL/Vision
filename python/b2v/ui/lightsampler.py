from . import base
from .base import *

dic = {
    "uniform": {
        "label": "uniform",
        "description": "uniform light sampler",
        "parameters": {},
    },
    "power": {"label": "power", "description": "power light sampler", "parameters": {}},
    "bvh": {"label": "bvh", "description": "bvh light sampler", "parameters": {}},
}

class VISION_RENDER_PT_LightSampler(bpy.types.Panel, VISION_RENDER_PT_VisionBasePanel):
    bl_idname = "VISION_RENDER_PT_LightSampler"
    bl_label = "LightSampler"
    attr_type = "light_sampler"
    dic = dic
