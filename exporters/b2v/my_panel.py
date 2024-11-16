import bpy

class MySimplePanel(bpy.types.Panel):
    bl_label = "My Panel"
    bl_idname = "VIEW3D_PT_my_panel"
    bl_space_type = 'VIEW_3D'
    bl_region_type = 'UI'
    bl_category = "My Addon"

    def draw(self, context):
        layout = self.layout
        layout.operator("object.my_simple_operator")

def register():
    bpy.utils.register_class(MySimplePanel)

def unregister():
    bpy.utils.unregister_class(MySimplePanel)
