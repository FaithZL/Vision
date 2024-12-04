import os
import bpy
from pathlib import Path
from bpy.props import (
    StringProperty,
    BoolProperty,
)
from bpy_extras.io_utils import ImportHelper, orientation_helper, axis_conversion
import json
import traceback
import time
import shutil


@orientation_helper(axis_forward="Z", axis_up="Y")
class VisionImporter(bpy.types.Operator, ImportHelper):
    bl_idname = "import_scene.vision"
    bl_label = "Vision Importer"

    filename_ext = ".json"
    filter_glob: StringProperty(default="*.json", options={"HIDDEN"})

    override_scene: BoolProperty(
        name="Override Current Scene",
        description="Override the current scene with the imported Mitsuba scene. "
        "Otherwise, creates a new scene for Mitsuba objects.",
        default=True,
    )

    def __init__(self):
        self.reset()

    def reset(self):
        pass

    def execute(self, context):
        # Conversion matrix to shift the "Up" Vector. This can be useful when exporting single objects to an existing mitsuba scene.
        axis_mat = axis_conversion(
            to_forward=self.axis_forward,
            to_up=self.axis_up,
        ).to_4x4()
        self.report({"INFO"}, "Scene imported successfully.")

        return {"FINISHED"}


def menu_import_func(self, context):
    self.layout.operator(VisionImporter.bl_idname, text="vision (.json)")


def register():
    bpy.types.TOPBAR_MT_file_import.append(menu_import_func)


def unregister():
    bpy.types.TOPBAR_MT_file_import.remove(menu_import_func)
