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

class VISION_LIGHT_PT_light(bpy.types.Panel, VisionWidget):
    bl_label = "Light"
    bl_context = "data"

    @classmethod
    def poll(cls, context):
        engine = context.engine
        return context.light and (engine in cls.COMPAT_ENGINES)

    def draw(self, context):
        layout = self.layout

        light = context.light
        clamp = light.cycles

        if self.bl_space_type == 'PROPERTIES':
            layout.row().prop(light, "type", expand=True)
            layout.use_property_split = True
        else:
            layout.use_property_split = True
            layout.row().prop(light, "type")

        col = layout.column()

        col.prop(light, "color")
        col.prop(light, "energy")
        col.separator()

        if light.type in {'POINT', 'SPOT'}:
            col.prop(light, "use_soft_falloff")
            col.prop(light, "shadow_soft_size", text="Radius")
        elif light.type == 'SUN':
            col.prop(light, "angle")
        elif light.type == 'AREA':
            col.prop(light, "shape", text="Shape")
            sub = col.column(align=True)

            if light.shape in {'SQUARE', 'DISK'}:
                sub.prop(light, "size")
            elif light.shape in {'RECTANGLE', 'ELLIPSE'}:
                sub.prop(light, "size", text="Size X")
                sub.prop(light, "size_y", text="Y")

        if not (light.type == 'AREA' and clamp.is_portal):
            col.separator()
            sub = col.column()
            sub.prop(clamp, "max_bounces")

        sub = col.column(align=True)
        sub.active = not (light.type == 'AREA' and clamp.is_portal)
        sub.prop(light, "use_shadow", text="Cast Shadow")
        sub.prop(clamp, "use_multiple_importance_sampling", text="Multiple Importance")

        if light.type == 'AREA':
            col.prop(clamp, "is_portal", text="Portal")


class VISION_RENDER_PT_LightSampler(bpy.types.Panel, VISION_RENDER_PT_VisionBasePanel):
    bl_idname = "VISION_RENDER_PT_LightSampler"
    bl_label = "LightSampler"
    attr_type = "light_sampler"
    dic = dic
