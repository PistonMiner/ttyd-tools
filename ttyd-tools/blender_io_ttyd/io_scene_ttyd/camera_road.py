# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright 2019 Linus S. (aka PistonMiner)

import math
import datetime

import bpy
import mathutils

from .util import *

class CameraRoadMarker:
	def __init__(self):
		self.name = ""
		self.polygons = []
		self.vertex_indices = []
		self.vertex_positions = []

	def link(self, linker):
		marker_blob_name = "markers:" + str(linker.get_uid())
		marker_data = bytearray(0x68)

		struct.pack_into(">64s", marker_data, 0x00, self.name.encode("shift_jis"))

		# Bounding box
		bbox = get_bbox(self.vertex_positions)
		assert(bbox != None)
		struct.pack_into(
			">ffffff",
			marker_data,
			0x40,
			bbox[0][0],
			bbox[0][1],
			bbox[0][2],
			bbox[1][0],
			bbox[1][1],
			bbox[1][2]
		)

		vertex_position_base_index = linker.get_section_blob_count("vertex_positions")
		vertex_position_blob_name_prefix = "vertex_positions:{}:".format(linker.get_uid())
		for i, vertex_position in enumerate(self.vertex_positions):
			vertex_position_blob_name = vertex_position_blob_name_prefix + str(i)
			vertex_position_data = bytearray(0xc)
			struct.pack_into(
				">fff",
				vertex_position_data,
				0x0,
				vertex_position[0],
				vertex_position[1],
				vertex_position[2]
			)
			linker.add_blob(vertex_position_blob_name, vertex_position_data)
			linker.place_blob_in_section(vertex_position_blob_name, "vertex_positions")

		struct.pack_into(">L", marker_data, 0x58, vertex_position_base_index)
		struct.pack_into(">L", marker_data, 0x5c, len(self.vertex_positions))

		vertex_index_base_index = linker.get_section_blob_count("vertex_indices")
		vertex_index_blob_name_prefix = "indices:{}:".format(linker.get_uid())
		for i, vertex_index in enumerate(self.vertex_indices):
			vertex_index_blob_name = vertex_index_blob_name_prefix + str(i)
			vertex_index_data = bytearray(0x4)

			struct.pack_into(">L", vertex_index_data, 0x0, vertex_index)

			linker.add_blob(vertex_index_blob_name, vertex_index_data)
			linker.place_blob_in_section(vertex_index_blob_name, "vertex_indices")

		polygon_base_index = linker.get_section_blob_count("polygons")
		polygon_blob_name_prefix = "polygons:{}:".format(linker.get_uid())
		for i, polygon in enumerate(self.polygons):
			polygon_blob_name = polygon_blob_name_prefix + str(i)
			polygon_data = bytearray(0x8)

			polygon_start_index = vertex_index_base_index + polygon[0]
			struct.pack_into(">L", polygon_data, 0x0, polygon_start_index)
			struct.pack_into(">L", polygon_data, 0x4, polygon[1])

			linker.add_blob(polygon_blob_name, polygon_data)
			linker.place_blob_in_section(polygon_blob_name, "polygons")

		struct.pack_into(">L", marker_data, 0x60, polygon_base_index)
		struct.pack_into(">L", marker_data, 0x64, len(self.polygons))

		linker.add_blob(marker_blob_name, marker_data)
		linker.place_blob_in_section(marker_blob_name, "markers")

		return marker_blob_name

	@staticmethod
	def from_blender_object(blender_object, global_matrix = None):
		if blender_object.type != 'MESH':
			return None

		marker = CameraRoadMarker()

		marker_vertex_positions = []
		marker_vertex_indices = []
		marker_polygons = []

		blender_depsgraph = bpy.context.evaluated_depsgraph_get()
		blender_evaluated_object = blender_object.evaluated_get(blender_depsgraph)
		blender_mesh = blender_evaluated_object.to_mesh()
		blender_index_map = {}

		def assign_index_from_blender_index(blender_index):
			if blender_index in blender_index_map:
				return blender_index_map[blender_index]

			# Not in buffer, add.
			new_index = len(marker_vertex_positions)
			vertex_position = blender_mesh.vertices[blender_index].co
			vertex_position = blender_object.matrix_world @ vertex_position
			if global_matrix != None:
				vertex_position = global_matrix @ vertex_position
			marker_vertex_positions.append(tuple(vertex_position))
			blender_index_map[blender_index] = new_index
			return new_index

		# todo-blender_io_ttyd: This shares a lot with
		# DmdModel.list_from_blender_mesh, see about extracting some common
		# code.
		for blender_polygon in blender_mesh.polygons:
			loop_indices = [i for i in blender_polygon.loop_indices]
			loop_index_polygons = []
			if len(loop_indices) == 3:
				loop_index_polygons.append(loop_indices[:])
			elif len(loop_indices) > 3:
				polyline_vertices = []
				for loop_index in loop_indices:
					loop = blender_mesh.loops[loop_index]
					loop_vertex = blender_mesh.vertices[loop.vertex_index]
					polyline_vertices.append(loop_vertex.co)
				tessellated_triangles = mathutils.geometry.tessellate_polygon([polyline_vertices])
				for tri in tessellated_triangles:
					tri_loop_indices = [loop_indices[i] for i in tri]
					loop_index_polygons.append(tri)

			for loop_index_polygon in loop_index_polygons:
				polygon_start = len(marker_vertex_indices)
				polygon_size = 0
				for loop_index in loop_index_polygon:
					loop = blender_mesh.loops[loop_index]
					blender_vertex_index = loop.vertex_index
					vertex_index = assign_index_from_blender_index(blender_vertex_index)
					marker_vertex_indices.append(vertex_index)
					polygon_size += 1

				marker_polygons.append((polygon_start, polygon_size))

		blender_evaluated_object.to_mesh_clear()

		marker.polygons = marker_polygons
		marker.vertex_indices = marker_vertex_indices
		marker.vertex_positions = marker_vertex_positions

		return marker

class CameraRoadCurve:
	def __init__(self):
		self.name = ""
		self.points = []
		self.markers = []

		# Whether the curve should be extended past the clamp start
		# (necessary if the player may stand outside of the curve area)
		self.extend = True

		# These vary a lot throughout TTYD. I've chosen values based on the
		# original maps which seem to me like the default values.
		self.should_lock_y = False
		self.lock_y_value = 0.0
		self.disabled = False

		self.should_clamp = 1
		self.clamp_distance_left = 20.0
		self.clamp_distance_right = 20.0

		self.target_distance = 55.0
		self.elevation = 16.0
		self.pitch = -5.0

		self.shift_x_rate = 20.0

	def link(self, linker):
		curve_blob_name = "curves.{}".format(self.name)
		curve_data = bytearray(0xb8)
		struct.pack_into(">32s", curve_data, 0x00, self.name.encode("shift_jis"))
		struct.pack_into(">L", curve_data, 0x20, self.should_lock_y) # Lock Y?
		struct.pack_into(">f", curve_data, 0x24, self.lock_y_value) # Value to lock Y at

		# Maximum distance the camera is allowed to travel left and right of the curve
		struct.pack_into(">f", curve_data, 0x48, self.clamp_distance_left)
		struct.pack_into(">f", curve_data, 0x4c, self.clamp_distance_right)

		# Distance from camera to target
		struct.pack_into(">f", curve_data, 0x58, self.target_distance)

		# Camera elevation/pitch (degrees)
		struct.pack_into(">f", curve_data, 0x64, self.elevation)
		struct.pack_into(">f", curve_data, 0x70, self.pitch)

		# Shift X rate (how far/fast the camera slides ahead of the player
		# when traveling on the X-axis)
		struct.pack_into(">f", curve_data, 0x7c, self.shift_x_rate)

		# Enable clamping?
		struct.pack_into(">L", curve_data, 0x84, self.should_clamp)

		# Curve data
		assert(len(self.points))
		curve_data_name_prefix = "curve_data:{}:".format(linker.get_uid())
		curve_data_base_index = linker.get_section_blob_count("curve_data")
		curve_data_count = 0
		points = self.points

		# Bounding box (excludes extended points on purpose)
		marker_bboxes = [get_bbox(m.vertex_positions) for m in self.markers]
		bbox = merge_bboxes(marker_bboxes)
		assert(bbox != None)
		struct.pack_into(
			">ffffff",
			curve_data,
			0x88,
			bbox[0][0], bbox[0][1], bbox[0][2], bbox[1][0], bbox[1][1], bbox[1][2]
		)

		# Calculate progress for points
		def point_distance(lhs, rhs):
			return math.sqrt(sum([(lhs[i] - rhs[i]) ** 2 for i in range(2)]))
		curve_length = 0.0
		point_lengths = [0.0]
		for i in range(1, len(points)):
			curve_length += point_distance(points[i - 1][0], points[i][0])
			point_lengths.append(curve_length)
		point_progresses = [length / curve_length for length in point_lengths]
		
		# Extend curve along normals at start and end
		if self.extend:
			def extend_from_point(point, length):
				tangent = point[1]
				position = tuple([point[0][i] + point[1][i] * length for i in range(2)])
				return (position, tangent)

			# Length looks constant based on original exporter data
			extend_length = 10000
			start_point = extend_from_point(points[0], -extend_length)
			end_point = extend_from_point(points[-1], extend_length)
			
			points = [start_point] + points[:] + [end_point]
			point_progresses = [-1.0] + point_progresses + [2.0]

			start_clamp_index = 1
			end_clamp_index = len(points) - 2
		else:
			start_clamp_index = 0
			end_clamp_index = len(points) - 1

		# Start/end clamp segment index
		struct.pack_into(">L", curve_data, 0x40, start_clamp_index)
		struct.pack_into(">L", curve_data, 0x44, end_clamp_index)

		struct.pack_into(">f", curve_data, 0x50, 0.0) # Start clamp progress
		struct.pack_into(">f", curve_data, 0x54, 1.0) # End clamp progress
		
		# Positions, then tangents
		for attribute_index in range(2):
			for point_index, point in enumerate(points):
				curve_data_blob_name = curve_data_name_prefix + str(curve_data_count)
				curve_data_data = bytearray(0xc) # Horrendous naming, I know, but it has to be.
	
				struct.pack_into(">f", curve_data_data, 0x0, point[attribute_index][0])

				# Add progress in Y-field for tangent
				if attribute_index == 1:
					struct.pack_into(">f", curve_data_data, 0x4, point_progresses[point_index])

				struct.pack_into(">f", curve_data_data, 0x8, point[attribute_index][1])
	
				linker.add_blob(curve_data_blob_name, curve_data_data)
				linker.place_blob_in_section(curve_data_blob_name, "curve_data")
				curve_data_count += 1

		struct.pack_into(">L", curve_data, 0xa0, curve_data_base_index)
		struct.pack_into(">L", curve_data, 0xa4, curve_data_count)
		
		# Markers
		marker_base_index = linker.get_section_blob_count("markers")
		for marker in self.markers:
			marker.link(linker)
		struct.pack_into(">L", curve_data, 0xa8, marker_base_index)
		struct.pack_into(">L", curve_data, 0xac, len(self.markers))

		linker.add_blob(curve_blob_name, curve_data)
		linker.place_blob_in_section(curve_blob_name, "curves")

		return curve_blob_name

	@staticmethod
	def from_blender_object(blender_object, camera_road_collection, global_matrix = None):
		if blender_object.type != 'CURVE':
			return None

		curve = CameraRoadCurve()
		curve.name = blender_object.name

		# Parse user properties
		# todo-blender_io_ttyd: This is not the cleanest way to do this.
		# Potentially find a better way to do this neatly.
		def read_property(property_name, type):
			# Read a property only if it exists and is convertible
			if property_name not in blender_object:
				return

			property_value = blender_object[property_name]

			try:
				cast_property_value = type(property_value)
			except ValueError:
				return

			setattr(curve, property_name, cast_property_value)
		
		read_property("extend", bool)

		read_property("should_lock_y", bool)
		read_property("lock_y_value", float)
		read_property("disabled", bool)

		read_property("should_clamp", bool)
		read_property("clamp_distance_left", float)
		read_property("clamp_distance_right", float)

		read_property("target_distance", float)
		read_property("elevation", float)
		read_property("pitch", float)
		read_property("shift_x_rate", float)

		blender_curve = blender_object.data

		assert(len(blender_curve.splines) == 1)
		spline = blender_curve.splines[0]

		# Calculate point positions and tangents
		curve.points = []
		assert(len(spline.points) > 1)
		for i in range(len(spline.points)):
			# Figure out surrounding points for tangent
			current_point = spline.points[i]
			if i > 0:
				previous_point = spline.points[i - 1]
			else:
				previous_point = None
			if i < len(spline.points) - 1:
				next_point = spline.points[i + 1]
			else:
				next_point = None

			# Calculate tangent
			if previous_point == None:
				assert(next_point != None)
				tangent = next_point.co - current_point.co
			elif next_point == None:
				assert(previous_point != None)
				tangent = current_point.co - previous_point.co
			else:
				tangent_in = current_point.co - previous_point.co
				tangent_out = next_point.co - current_point.co
				tangent = tangent_in * 0.5 + tangent_out * 0.5

			# Finalize data
			position = current_point.co
			position = blender_object.matrix_world @ position
			tangent = blender_object.matrix_world @ tangent
			if global_matrix != None:
				position = global_matrix @ position
				tangent = global_matrix @ tangent

			# Project onto XZ-plane
			position[1] = 0.0
			tangent[1] = 0.0
			tangent = tangent.normalized()

			position_2d = (position[0], position[2])
			tangent_2d = (tangent[0], tangent[2])
			curve.points.append((position_2d, tangent_2d))

		# Markers
		curve.markers = []
		marker_index = 0
		while True:
			marker_property_name = "marker{}".format(marker_index)
			if not marker_property_name in blender_object:
				break

			marker_name = blender_object[marker_property_name]
			assert(isinstance(marker_name, str))
			assert(marker_name in camera_road_collection.all_objects)
			marker_object = camera_road_collection.all_objects[marker_name]
			marker = CameraRoadMarker.from_blender_object(marker_object, global_matrix)
			curve.markers.append(marker)

			marker_index += 1

		assert(len(curve.markers) > 0)

		return curve

class CameraRoadFile:
	def __init__(self):
		self.curves = []
		pass

	@staticmethod
	def from_blender_scene(blender_scene, settings):
		file = CameraRoadFile()

		if "axis_conversion_matrix" in settings:
			global_matrix = settings["axis_conversion_matrix"]
		else:
			global_matrix = None

		file.curves = []
		for object in settings["camera_road_root"].all_objects:
			if object.type != "CURVE":
				continue
			curve = CameraRoadCurve.from_blender_object(
				object,
				settings["camera_road_root"],
				global_matrix
			)
			file.curves.append(curve)

		return file

	def serialize(self):
		linker = Linker()

		header_blob_name = "header"
		header_data = bytearray(0x10c)
		struct.pack_into(">64s", header_data, 0x004, "MarioSt_CameraRoadExport".encode())
		struct.pack_into(">64s", header_data, 0x044, "1.01".encode()) # version
		date_text = datetime.datetime.utcnow().strftime("%Y/%m/%d")
		struct.pack_into(">64s", header_data, 0x084, date_text.encode())

		camera_parameter_blob_name = "camera_parameters"
		camera_parameter_data = bytearray(0xc)
		# fov/near/far
		# These values don't change the actual camera parameters, but at least
		# the FoV is used in camera shift calculations to figure out how much
		# to shift the camera, so these should be correct. TTYD uses 25 degree
		# FoV.
		struct.pack_into(">f", camera_parameter_data, 0x0, 25.0)
		struct.pack_into(">f", camera_parameter_data, 0x4, 0.01)
		struct.pack_into(">f", camera_parameter_data, 0x8, 1000.0)
		linker.add_blob(camera_parameter_blob_name, camera_parameter_data)
		linker.place_blob_in_section(camera_parameter_blob_name, "camera_parameters")

		for curve in self.curves:
			curve_blob_name = curve.link(linker)

		# Place sections apart from header now so addresses are available
		linker.place_section_at("camera_parameters", len(header_data))
		linker.place_section("curves")
		linker.place_section("markers")
		linker.place_section("polygons")
		linker.place_section("curve_data")
		linker.place_section("vertex_positions")
		linker.place_section("vertex_indices")

		# Finish header
		header_data_section_names = [
			"camera_parameters",
			"curves",
			"markers",
			"polygons",
			"curve_data",
			"vertex_positions",
			"vertex_indices"
		]
		for i, section_name in enumerate(header_data_section_names):
			entry_count = linker.get_section_blob_count(section_name)
			struct.pack_into(">L", header_data, 0xc4 + i * 4, entry_count)
			section_address = linker.get_section_address(section_name)
			struct.pack_into(">L", header_data, 0xe8 + i * 4, section_address)

		linker.add_blob(header_blob_name, header_data)
		linker.place_blob_in_section(header_blob_name, "header")

		# Finalize data
		linker.place_section_at("header", 0x0)
		assert(linker.resolve_relocations())
		linked_data = linker.serialize()

		# Fill size field
		struct.pack_into(">L", linked_data, 0x0, len(linked_data))

		return linked_data