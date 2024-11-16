import bpy

class MySimpleOperator(bpy.types.Operator):
    bl_idname = "object.my_simple_operator"
    bl_label = "Simple Operator"
    bl_description = "This is a simple operator example"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        self.report({'INFO'}, "Simple Operator executed")
        return {'FINISHED'}

def register():
    bpy.utils.register_class(MySimpleOperator)

def unregister():
    bpy.utils.unregister_class(MySimpleOperator)
