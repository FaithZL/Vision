bl_info = {
    "name": "VisionRenderer",
    "author": "Your Name",
    "version": (1, 0),
    "blender": (3, 0, 0),
    "location": "Render Engine Menu",
    "description": "vision renderer and exporter",
    "category": "Render",
}

import bpy
from. import ui

class VisionRender(bpy.types.RenderEngine):
    bl_idname = "VISION_RENDER_ENGINE"
    bl_label = "Vision"
    bl_use_preview = True  # 支持材质预览

    def render(self, context):
        """定义渲染逻辑"""
        self.report({'INFO'}, "Rendering with VisionRender...")
        # 模拟渲染结果（显示纯黑色图像）
        result = self.begin_result(0, 0, context.region.width, context.region.height)
        layer = result.layers[0].passes["Combined"]
        layer.rect = [[(0, 1, 0, 1)] * context.region.width] * context.region.height
        self.end_result(result)



def register():
    print("Registering VisionRender...")
    bpy.utils.register_class(VisionRender)
    bpy.utils.register_class(ui.FilterPanel)

    # 确保我们的渲染引擎在注册时被添加到可用引擎列表中
    # if not bpy.context.scene.render.engine in bpy.types.RenderEngine.bl_rna.properties['bl_idname'].enum_items.keys():
    #     bpy.types.RenderEngine.bl_rna.properties['bl_idname'].enum_items.add(['VISION_RENDER_ENGINE', 'Vision', 'Vision render engine'])

def unregister():
    print("Unregistering VisionRender...")
    bpy.utils.unregister_class(VisionRender)
    bpy.utils.unregister_class(ui.FilterPanel)

if __name__ == "__main__":
    register()
