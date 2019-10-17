# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright 2019 Linus S. (aka PistonMiner)

import os

import bpy

from . import dmd
from . import tpl
from . import camera_road

def export(context, settings):
	# Generate main DMD
	dmd_file = dmd.DmdFile.from_blender_scene(
		context.scene,
		settings
	)
	dmd_data = dmd_file.serialize()
	dmd_path = os.path.join(settings["root_path"], "d")
	with open(dmd_path, "wb") as f:
		f.write(dmd_data)
	#print("Wrote DMD to {}".format(dmd_path))

	# Generate TPL
	tpl_file = tpl.TplFile()
	for dmd_texture in dmd_file.textures:
		blender_image = bpy.data.images[dmd_texture.name]
		tpl_texture = tpl.TplTexture.from_blender_image(blender_image)
		tpl_file.textures.append(tpl_texture)
	tpl_data = tpl_file.serialize()
	tpl_path = os.path.join(settings["root_path"], "t")
	with open(tpl_path, "wb") as f:
		f.write(tpl_data)

	# Generate camera road
	camera_road_file = camera_road.CameraRoadFile.from_blender_scene(
		context.scene,
		settings
	)
	camera_road_data = camera_road_file.serialize()
	camera_road_path = os.path.join(settings["root_path"], "c")
	with open(camera_road_path, "wb") as f:
		f.write(camera_road_data)

	return {'FINISHED'}