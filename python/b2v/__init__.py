bl_info = {
    "name": "Vision",
    "author": "zhuling",
    "version": (1, 0),
    "blender": (3, 0, 0),
    "location": "Render Engine Menu",
    "description": "vision renderer and exporter",
    "category": "Render",
}

import bpy

from . import auto_load

auto_load.init()


def vision_register_class(cls):
    if not getattr(cls, "is_registered", False):
        bpy.utils.register_class(cls)


def vision_unregister_class(cls):
    if getattr(cls, "is_registered", False):
        bpy.utils.unregister_class(cls)


bpy.utils.vision_register_class = vision_register_class
bpy.utils.vision_unregister_class = vision_unregister_class


class Vision(bpy.types.RenderEngine):
    bl_idname = "VISION_RENDER_ENGINE"
    bl_label = "Vision"
    bl_use_preview = True

    bl_use_eevee_viewport = True
    bl_use_exclude_layers = True
    bl_use_spherical_stereo = True
    bl_use_custom_freestyle = True
    bl_use_alembic_procedural = True
    bl_use_shading_nodes_custom = False

    def render(self, context):
        self.report({"INFO"}, "Rendering with Vision...")


def get_panels():
    exclude_panels = {
        "VIEWLAYER_PT_filter",
        "RENDER_PT_gpencil",
        'DATA_PT_light',
        "VIEWLAYER_PT_layer_passes",
        "RENDER_PT_simplify",
        "NODE_DATA_PT_light",
        "RENDER_PT_color_management",
        "RENDER_PT_freestyle",
    }

    panels = []
    for panel in bpy.types.Panel.__subclasses__():
        if (
            hasattr(panel, "COMPAT_ENGINES")
            and "BLENDER_RENDER" in panel.COMPAT_ENGINES
        ):
            if panel.__name__ not in exclude_panels:
                panels.append(panel)

    return panels


def register():
    print("Registering Vision ---")
    bpy.utils.vision_register_class(Vision)
    auto_load.register()
    for panel in get_panels():
        panel.COMPAT_ENGINES.add("VISION_RENDER_ENGINE")


def unregister():
    print("Unregistering Vision ---")
    for panel in get_panels():
        if "VISION_RENDER_ENGINE" in panel.COMPAT_ENGINES:
            panel.COMPAT_ENGINES.remove("VISION_RENDER_ENGINE")
    auto_load.unregister()
    bpy.utils.vision_unregister_class(Vision)


if __name__ == "__main__":
    register()
