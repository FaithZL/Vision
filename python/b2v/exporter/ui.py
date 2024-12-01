import bpy
from bpy.props import (
        StringProperty,
        BoolProperty,
    )
from bpy_extras.io_utils import (
        ExportHelper,
        orientation_helper,
        axis_conversion
    )



@orientation_helper(axis_forward='Z', axis_up='Y')
class ExportVision(bpy.types.Operator, ExportHelper):
    bl_idname = "export_scene.vision"
    bl_label = "Vision Export"

    filename_ext = ".json"
    filter_glob: StringProperty(default="*.json", options={'HIDDEN'})

    use_selection: BoolProperty(
	        name = "Selection Only",
	        description="Export selected objects only",
	        default = False,
	    )

    split_files: BoolProperty(
            name = "Split File",
            description = "Split scene XML file in smaller fragments",
            default = False
    )

    export_ids: BoolProperty(
            name = "Export IDs",
            description = "Add an 'id' field for each object (shape, emitter, camera...)",
            default = False
    )

    ignore_background: BoolProperty(
            name = "Ignore Default Background",
            description = "Ignore blender's default constant gray background when exporting to Mitsuba.",
            default = True
    )

    def __init__(self):
        self.reset()

    def reset(self):
        pass

    def execute(self, context):
        axis_mat = axis_conversion(
	            to_forward=self.axis_forward,
	            to_up=self.axis_up,
	        ).to_4x4()
        
        print(axis_mat)
        deps_graph = context.evaluated_depsgraph_get()
        for object_instance in deps_graph.object_instances:
            print(object_instance)
            evaluated_obj = object_instance.object
            object_type = evaluated_obj.type
            print(evaluated_obj)
            print(object_type)
            
        return {'FINISHED'}


def menu_export_func(self, context):
    self.layout.operator(ExportVision.bl_idname, text="vision (.json)")


def register():
    bpy.types.TOPBAR_MT_file_export.append(menu_export_func)

def unregister():
    bpy.types.TOPBAR_MT_file_export.remove(menu_export_func)