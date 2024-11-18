import bpy
from bpy.types import Operator
from bpy.props import EnumProperty

class MyDropdownOperator(Operator):
    bl_idname = "object.my_dropdown_operator"
    bl_label = "My Dropdown Operator"
    bl_description = "An operator with a dropdown menu"
    
    # 定义下拉框的属性
    my_dropdown = EnumProperty(
        name="My Dropdown",
        description="Choose an option",
        items=[
            ('OPTION_ONE', "Option One", "Description of Option One"),
            ('OPTION_TWO', "Option Two", "Description of Option Two"),
            ('OPTION_THREE', "Option Three", "Description of Option Three")
        ],
        default='OPTION_ONE'
    )
    
    # # 定义操作符的执行逻辑
    # def execute(self, context):
    #     print(f"Selected option: {self.my_dropdown}")
    #     return {'FINISHED'}


class FilterPanel(bpy.types.Panel):
    bl_idname = "VISION_FILTER"
    bl_label = "filter"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "render"

    def draw(self, context):
        layout = self.layout
        
        # # 创建一个下拉框
        # row = layout.row()
        # row.prop(context.window_manager, "my_dropdown_operator.my_dropdown", text="My Dropdown")
        
        # # 创建一个按钮来执行操作符
        # row = layout.row()
        # row.operator("object.my_dropdown_operator", text="Execute")
        

def register():
    bpy.utils.register_class(FilterPanel)
    bpy.utils.register_class(MyDropdownOperator)
    
def unregister():
    bpy.utils.unregister_class(FilterPanel)
    bpy.utils.unregister_class(MyDropdownOperator)