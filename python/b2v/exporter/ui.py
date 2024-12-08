import os
import bpy
from pathlib import Path
from bpy.props import (
    StringProperty,
    BoolProperty,
)
from bpy_extras.io_utils import ExportHelper, orientation_helper, axis_conversion
import json
import traceback
from . import geometry
from . import material
from . import light
from . import camera
import time
import shutil


@orientation_helper(axis_forward="Z", axis_up="Y")
class VisionExporter(bpy.types.Operator, ExportHelper):
    bl_idname = "export_scene.vision"
    bl_label = "Vision Exporter"

    filename_ext = ".json"
    filter_glob: StringProperty(default="*.json", options={"HIDDEN"})

    split_files: BoolProperty(
        name="Split File",
        description="Split scene XML file in smaller fragments",
        default=False,
    )

    export_ids: BoolProperty(
        name="Export IDs",
        description="Add an 'id' field for each object (shape, emitter, camera...)",
        default=False,
    )

    ignore_background: BoolProperty(
        name="Ignore Default Background",
        description="Ignore blender's default constant gray background when exporting to Mitsuba.",
        default=True,
    )

    visible_only: BoolProperty(
        name="Visible Only",
        description="Export visible objects only",
        default=True,
    )

    mesh_dir = "meshes"
    tex_dir = "textures"

    def __init__(self):
        context = None
        self.reset()

    def reset(self):
        context = None
        pass

    def output_directory(self):
        return os.path.splitext(self.filepath)[0]

    def try_makedir(self, directory):
        if not os.path.exists(directory):
            os.makedirs(directory)

    def make_output_dir(self):
        d = self.output_directory()
        if os.path.exists(d):
            shutil.rmtree(d)
        os.makedirs(d)

    def try_make_mesh_dir(self):
        d = self.mesh_path()
        self.try_makedir(d)

    def try_make_tex_dir(self):
        d = self.texture_path()
        self.try_makedir(d)

    def mesh_path(self, fn=None):
        if fn is None:
            return os.path.join(self.output_directory(), self.mesh_dir)
        else:
            return os.path.join(self.mesh_path(), fn)

    def texture_path(self, fn=None):
        if fn is None:
            return os.path.join(self.output_directory(), self.tex_dir)
        else:
            return os.path.join(self.texture_path(), fn)

    def json_name(self):
        return os.path.basename(self.filepath)

    def json_path(self):
        return os.path.join(self.output_directory(), self.json_name())

    def export_settings(self, context):
        vp = getattr(context.scene, "vision")
        ret = {
            "filter": {},
            "sampler": {},
            "light_sampler": {},
            "spectrum": {},
            "integrator": {},
        }
        for k, v in ret.items():
            ret[k] = vp.get_params(k)
        return ret

    def save_json(self, data):
        with open(self.json_path(), "w") as outputfile:
            json.dump(data, outputfile, indent=4)

    def correct_matrix(self, mat=None):
        axis_mat = axis_conversion(
                to_forward=self.axis_forward,
                to_up=self.axis_up,
            ).to_4x4()
        if mat is None:
            return axis_mat
        else:
            return axis_mat * mat

    def convert_materials(self, mat_dict):
        ret = []
        for k, v in mat_dict.items():
            v["name"] = k
            ret.append(v)
        return ret

    def execute(self, context):
        self.make_output_dir()
        self.context = context
        data = self.export_settings(context)
        window_manager = context.window_manager

        if self.visible_only:
            objects = [obj for obj in context.scene.objects if obj.visible_get()]
        else:
            objects = [obj for obj in context.scene.objects]

        window_manager.progress_begin(0, len(objects))
        shapes = []
        cameras = []
        lights = []
        materials = {}
        data["shapes"] = shapes

        bpy.ops.object.select_all(action="DESELECT")
        viewlayer = bpy.context.view_layer
        for i, object in enumerate(objects):
            object_type = object.type
            if object_type in ("MESH", "FONT", "SURFACE", "META"):
                shape = geometry.export(self, object, materials)
                shapes.append(shape)
            elif object_type == "CAMERA":
                cam = camera.export(self, object)
                cameras.append(cam)
            elif object_type == "LIGHT":
                lit = light.export(self, object)
                lights.append(lit)
            window_manager.progress_update(i)
        window_manager.progress_end()

        viewlayer.objects.active = None

        bpy.ops.object.select_all(action="DESELECT")
        data["materials"] = self.convert_materials(materials)
        data["camera"] = cameras[0]
        self.save_json(data)
        return {"FINISHED"}


def menu_export_func(self, context):
    self.layout.operator(VisionExporter.bl_idname, text="vision (.json)")


def register():
    bpy.types.TOPBAR_MT_file_export.append(menu_export_func)


def unregister():
    bpy.types.TOPBAR_MT_file_export.remove(menu_export_func)
