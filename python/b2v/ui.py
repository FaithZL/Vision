import bpy
from bpy.props import EnumProperty

class SearchEnumOperator(bpy.types.Operator):
    bl_idname = "object.search_enum_operator"
    bl_label = "Search Enum Operator"
    bl_property = "my_search"

    my_search: bpy.props.EnumProperty(
        name="My Search",
        items=(
            ('FOO', "Foo", ""),
            ('BAR', "Bar", ""),
            ('BAZ', "Baz", ""),
        ),
    )

    def execute(self, context):
        self.report({'INFO'}, "Selected:" + self.my_search)
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.invoke_popup(self)
        return {'RUNNING_MODAL'}
    

class CustomDrawOperator(bpy.types.Operator):
    bl_idname = "object.custom_draw"
    bl_label = "Simple Modal Operator"

    filepath: bpy.props.StringProperty(subtype="FILE_PATH")

    my_float: bpy.props.FloatProperty(name="Float")
    my_bool: bpy.props.BoolProperty(name="Toggle Option")
    my_string: bpy.props.StringProperty(name="String Value")

    def execute(self, context):
        print("Test", self)
        return {'FINISHED'}

    def invoke(self, context, event):
        wm = context.window_manager
        return wm.invoke_props_dialog(self)

    def draw(self, context):
        layout = self.layout
        col = layout.column()
        col.label(text="Custom Interface!")

        row = col.row()
        row.prop(self, "my_float")
        row.prop(self, "my_bool")



        col.prop(self, "my_string")

class MyPropertyGroup(bpy.types.PropertyGroup):
    custom_1: bpy.props.FloatProperty(name="My Float")
    custom_2: bpy.props.IntProperty(name="My Int")
    
class SceneSettingItem(bpy.types.PropertyGroup):
    filterTypes = [("GaussianFilter", "GaussianFilter", "", 1),
                   ("BoxFilter", "BoxFilter", "", 2),
                   ("TriangleFilter", "TriangleFilter", "", 3),
                   ("LanczosSincFilter", "LanczosSincFilter", "", 4),
                   ("MitchellFilter", "MitchellFilter", "", 5)]

    bpy.types.Scene.filterType = bpy.props.EnumProperty(
        name="Filter", items=filterTypes, default="GaussianFilter")
    
    
class VisionWidget:
    COMPAT_ENGINES = {'VISION_RENDER_ENGINE'}
    bl_context = "render"
    
    @classmethod
    def poll(cls, context):
        return context.engine in cls.COMPAT_ENGINES

class VISION_RENDER_PT_Filter(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Filter"
    bl_label = "Filter"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'


    def draw(self, context):
        scene = context.scene
        layout = self.layout
        row = layout.row()
        row.label(text="Hello world!", icon='WORLD_DATA')
        self.layout.operator(SearchEnumOperator.bl_idname, text="Search Enum Operator")
        self.layout.operator(CustomDrawOperator.bl_idname, text="Custom Draw Operator")
        row = layout.row()
        row.prop(scene, "filterType")
        # bpy.types.Object.my_prop_grp = bpy.props.PointerProperty(type=MyPropertyGroup)
        # op = layout.operator(MyEnumOperator.bl_idname, text="My Enum Operator")
        # op.my_enum = context.object.my_enum_property
        
class VISION_RENDER_PT_LightSampler(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_LightSampler"
    bl_label = "LightSampler"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    
    def draw(self, context):
        layout = self.layout

class VISION_RENDER_PT_Spectrum(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Spectrum"
    bl_label = "Spectrum"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    
    def draw(self, context):
        layout = self.layout

class VISION_RENDER_PT_Sampler(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Sampler"
    bl_label = "Sampler"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    
    def draw(self, context):
        layout = self.layout

class VISION_RENDER_PT_Intergrator(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Intergrator"
    bl_label = "Intergrator"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    
    def draw(self, context):
        layout = self.layout
        
class VISION_RENDER_PT_Operator(bpy.types.Panel, VisionWidget):
    bl_idname = "VISION_RENDER_PT_Operator"
    bl_label = "Operator"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    
    def draw(self, context):
        layout = self.layout       