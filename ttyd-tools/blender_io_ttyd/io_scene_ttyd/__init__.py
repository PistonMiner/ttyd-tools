# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright 2019 Linus S. (aka PistonMiner)

import bpy
from bpy.props import (
	StringProperty,
	EnumProperty
)
from bpy_extras.io_utils import (
	ExportHelper,
	axis_conversion
)

import os.path

from . import export_ttyd

def find_collection_by_name(scene, name):
	collections_left = [scene.collection]
	while len(collections_left) > 0:
		c = collections_left.pop(0)
		if c.name == name:
			return c
		collections_left.extend(c.children)
	return None


def enum_collections(self, context):
	if context is None:
		return []

	# todo-blender_io_ttyd: Rework this
	collections = []
	collections_left = [context.scene.collection]
	while len(collections_left):
		collection = collections_left.pop(0)
		child_collections = [x for x in collection.children]
		collections_left.extend(child_collections)
		collections.extend(child_collections)

	items = []
	for c in collections:
		items.append((c.name, c.name, ""))
	return items

def enum_root_objects(self, context):
	if context is None:
		return []

	items = []
	for obj in context.scene.objects:
		if obj.parent == None:
			items.append((obj.name, obj.name, ""))
	return items

def find_object_by_name(scene, name):
	for object in scene.objects:
		if object.name == name:
			return object
	return None

bl_info = {
	"name": "Paper Mario TTYD Map Data File",
	"author": "PistonMiner (Linus S.)",
	"version": (1, 0, 0),
	"blender": (2, 80, 0),
	"location": "File > Import-Export",
	"description": "Import-Export as Paper Mario TTYD Map Data File",
	"tracker_url": "https://github.com/PistonMiner/ttyd-tools/issues/",
	"support": "COMMUNITY",
	"category": "Import-Export",
}

class ExportTTYDMap(bpy.types.Operator, ExportHelper):
	bl_idname = "export_scene.ttyd_map"
	bl_label = "Export TTYD map"
	bl_options = {'UNDO', 'PRESET'}

	filename_ext = ""
	# todo-blender_io_ttyd: The GLTF2 addon still uses = instead of : here; it
	#                       gave a warning for me. Double-check that this
	#                       syntax is right.
	filter_glob: StringProperty(default="d;c;t", options={'HIDDEN'})

	map_root: EnumProperty(
			name="Map Collection",
			items=enum_collections
		)
	hit_root: EnumProperty(
			name="Hit Collection",
			items=enum_collections
		)
	camera_road_root: EnumProperty(
			name="Camera Road Collection",
			items=enum_collections
		)

	"""def draw(self, context):
		layout = self.layout

		row = layout.row(align=True)
		row.prop(self, "map_collection")"""

	def execute(self, context):
		settings = {}
		settings['root_path'] = os.path.dirname(self.filepath)
		map_root = find_collection_by_name(context.scene, self.map_root)
		hit_root = find_collection_by_name(context.scene, self.hit_root)
		camera_road_root = find_collection_by_name(context.scene, self.camera_road_root)

		collection_list = [
			map_root,
			hit_root,
			camera_road_root
		]
		if len(collection_list) != len(set(collection_list)):
			self.report({'ERROR'}, "Map/Hit/Camera Road collections must not be the same!")
			return {'CANCELLED'}

		settings["map_root"] = map_root
		settings["hit_root"] = hit_root
		settings["camera_road_root"] = camera_road_root

		settings["axis_conversion_matrix"] = axis_conversion(
			to_forward='-Z',
			to_up='Y'
		).to_4x4()
		return export_ttyd.export(context, settings)


def menu_func_export_map(self, context):
	self.layout.operator(ExportTTYDMap.bl_idname, text="MarioSt Map (d/c/t)")

classes = (
	ExportTTYDMap,
)

def register():
	for cls in classes:
		bpy.utils.register_class(cls)
	
	bpy.types.TOPBAR_MT_file_export.append(menu_func_export_map)

def unregister():
	for cls in reversed(classes):
		bpy.utils.unregister_class(cls)

	bpy.types.TOPBAR_MT_file_export.remove(menu_func_export_map)