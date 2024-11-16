bl_info = {
    "name": "My Blender Addon",
    "author": "Your Name",
    "version": (1, 0),
    "blender": (3, 0, 0),
    "location": "View3D > Tool",
    "description": "A simple Blender addon example",
    "category": "3D View",
}

import bpy
from . import my_operator, my_panel

def register():
    my_operator.register()
    my_panel.register()

def unregister():
    my_operator.unregister()
    my_panel.unregister()

if __name__ == "__main__":
    register()