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
    print("wocao register")
    bpy.utils.register_class(VisionRender)
    # 将自定义渲染器注册到渲染器选择菜单
    print(dir(bpy.types.Scene))
    # bpy.types.Scene.render.engine = "MY_CUSTOM_RENDER"


def unregister():
    # pass
    bpy.utils.unregister_class(VisionRender)


if __name__ == "__main__":
    register()
