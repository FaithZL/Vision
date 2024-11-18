import bpy

class FilterPanel(bpy.types.Panel):
    bl_idname = "VISION_FILTER"
    bl_label = "filter"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "render"

    def draw(self, context):
        layout = self.layout