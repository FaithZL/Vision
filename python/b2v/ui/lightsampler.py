from . import base
from .base import *
from bpy_extras.node_utils import find_node_input


def panel_node_draw(layout, id_data, output_type, input_name):
    if not id_data.use_nodes:
        layout.operator("cycles.use_shading_nodes", icon="NODETREE")
        return False

    ntree = id_data.node_tree

    node = ntree.get_output_node("CYCLES")
    if node:
        input = find_node_input(node, input_name)
        if input:
            layout.template_node_view(ntree, node, input)
        else:
            layout.label(text="Incompatible output node")
    else:
        layout.label(text="No output node")

    return True


# class VISION_LIGHT_PT_preview(VisionWidget, bpy.types.Panel):
#     bl_label = "Preview"
#     bl_context = "data"
#     bl_options = {'DEFAULT_CLOSED'}

#     # @classmethod
#     # def poll(cls, context):
#     #     return (
#     #         context.light and
#     #         not (
#     #             context.light.type == 'AREA' and
#     #             context.light.cycles.is_portal
#     #         ) and
#     #         VisionWidget.poll(context)
#     #     )

#     def draw(self, context):
#         self.layout.template_preview(context.light)


class VISION_LIGHT_PT_nodes(VisionWidget, bpy.types.Panel):
    bl_label = "Nodes"
    bl_context = "data"

    @classmethod
    def poll(cls, context):
        return (
            context.light
            and not (context.light.type == "AREA" and context.light.cycles.is_portal)
            and VisionWidget.poll(context)
        )

    def draw(self, context):
        layout = self.layout

        layout.use_property_split = True

        light = context.light
        panel_node_draw(layout, light, "OUTPUT_LIGHT", "Surface")


class VISION_LIGHT_PT_light(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_LIGHT_PT_light"
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

        if self.bl_space_type == "PROPERTIES":
            layout.row().prop(light, "type", expand=True)
            layout.use_property_split = True
        else:
            layout.use_property_split = True
            layout.row().prop(light, "type")

        col = layout.column()

        col.prop(light, "color")
        col.prop(light, "energy")
        col.separator()

        if light.type in {"POINT", "SPOT"}:
            col.prop(light, "use_soft_falloff")
            col.prop(light, "shadow_soft_size", text="Radius")
        elif light.type == "SUN":
            col.prop(light, "angle")
        elif light.type == "AREA":
            col.prop(light, "shape", text="Shape")
            sub = col.column(align=True)

            if light.shape in {"SQUARE", "DISK"}:
                sub.prop(light, "size")
            elif light.shape in {"RECTANGLE", "ELLIPSE"}:
                sub.prop(light, "size", text="Size X")
                sub.prop(light, "size_y", text="Y")

        if not (light.type == "AREA" and clamp.is_portal):
            col.separator()
            sub = col.column()
            sub.prop(clamp, "max_bounces")

        sub = col.column(align=True)
        sub.active = not (light.type == "AREA" and clamp.is_portal)
        sub.prop(light, "use_shadow", text="Cast Shadow")
        sub.prop(clamp, "use_multiple_importance_sampling", text="Multiple Importance")

        if light.type == "AREA":
            col.prop(clamp, "is_portal", text="Portal")


class VISION_LIGHT_PT_beam_shape(VisionWidget, bpy.types.Panel):
    bl_label = "Beam Shape"
    bl_parent_id = "VISION_LIGHT_PT_light"
    bl_context = "data"

    @classmethod
    def poll(cls, context):
        if context.light.type in {"SPOT", "AREA"}:
            return context.light and VisionWidget.poll(context)

    def draw(self, context):
        layout = self.layout
        light = context.light
        layout.use_property_split = True

        col = layout.column()
        if light.type == "SPOT":
            col.prop(light, "spot_size", text="Spot Size")
            col.prop(light, "spot_blend", text="Blend", slider=True)
            col.prop(light, "show_cone")
        elif light.type == "AREA":
            col.prop(light, "spread", text="Spread")


dic = {
    "uniform": {
        "label": "uniform",
        "description": "uniform light sampler",
        "parameters": {
            "env_prob": {
                "type": "Float",
                "args": {
                    "default": 0.5,
                    "max": 0.99,
                    "min": 0.01,
                },
            },
            "env_separate": {
                "type": "Bool",
                "args": {
                    "default": False,
                },
            },
        },
    },
    "power": {
        "label": "power",
        "description": "power light sampler",
        "parameters": {
            "env_prob": {
                "type": "Float",
                "args": {
                    "default": 0.5,
                    "max": 0.99,
                    "min": 0.01,
                },
            },
            "env_separate": {
                "type": "Bool",
                "args": {
                    "default": False,
                },
            },
        },
    },
    "bvh": {
        "label": "bvh",
        "description": "bvh light sampler",
        "parameters": {
            "env_prob": {
                "type": "Float",
                "args": {
                    "default": 0.5,
                    "max": 0.99,
                    "min": 0.01,
                },
            },
            "env_separate": {
                "type": "Bool",
                "args": {
                    "default": False,
                },
            },
        },
    },
}


class VISION_RENDER_PT_LightSampler(bpy.types.Panel, VISION_RENDER_PT_VisionBasePanel):
    bl_idname = "VISION_RENDER_PT_LightSampler"
    bl_label = "LightSampler"
    attr_type = "light_sampler"
    dic = dic
