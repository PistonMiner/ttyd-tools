# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright 2019 Linus S. (aka PistonMiner)

from collections import defaultdict
import struct
import math
import datetime

import bpy
import mathutils

from .util import *

def linear_to_srgb(linear):
	# Linear to sRGB conversion excluding alpha channel if present
	out_components = []
	for linear_component in linear[:3]:
		if linear_component <= 0.0031308:
			component = 12.92 * linear_component
		else:
			component = 1.055 * linear_component ** (1/2.4) - 0.055
		out_components.append(component)
	out_components += linear[3:]
	return tuple(out_components)

class DmdLinker(Linker):
	def add_string(self, source_name, source_offset, text, section_name = "strings"):
		# todo-blender_io_ttyd: Investigate effects of string deduplication
		blob_name = "{}:{}".format(section_name, self.get_uid())
		encoded_string = text.encode("shift_jis") + b"\x00"
		self.add_blob(blob_name, encoded_string)
		self.place_blob_in_section(blob_name, section_name)
		self.add_relocation(source_name, source_offset, blob_name)

# DMD vertex attribute IDs
VERTEX_ATTRIBUTE_POSITION_ID = "position"
VERTEX_ATTRIBUTE_NORMAL_ID = "normal"
VERTEX_ATTRIBUTE_TEXCOORD_ID_PREFIX = "texcoord"
VERTEX_ATTRIBUTE_COLOR_ID_PREFIX = "color"

VERTEX_ATTRIBUTE_TEXCOORD_MAX_COUNT = 8
VERTEX_ATTRIBUTE_COLOR_MAX_COUNT = 2 # Technically two but color buffer 1 support is broken

# Order in which indices are packed into the individual vertices
VERTEX_ATTRIBUTE_INDEX_ORDER = []
VERTEX_ATTRIBUTE_INDEX_ORDER.append(VERTEX_ATTRIBUTE_POSITION_ID)
VERTEX_ATTRIBUTE_INDEX_ORDER.append(VERTEX_ATTRIBUTE_NORMAL_ID)
for i in range(VERTEX_ATTRIBUTE_COLOR_MAX_COUNT):
	VERTEX_ATTRIBUTE_INDEX_ORDER.append(VERTEX_ATTRIBUTE_COLOR_ID_PREFIX + str(i))
for i in range(VERTEX_ATTRIBUTE_TEXCOORD_MAX_COUNT):
	VERTEX_ATTRIBUTE_INDEX_ORDER.append(VERTEX_ATTRIBUTE_TEXCOORD_ID_PREFIX + str(i))

# Order in which the data referenced by the indices is packed into the data sections
VERTEX_ATTRIBUTE_DATA_ORDER = []
VERTEX_ATTRIBUTE_DATA_ORDER.append(VERTEX_ATTRIBUTE_POSITION_ID)
VERTEX_ATTRIBUTE_DATA_ORDER.append(VERTEX_ATTRIBUTE_NORMAL_ID)
for i in range(VERTEX_ATTRIBUTE_TEXCOORD_MAX_COUNT):
	VERTEX_ATTRIBUTE_DATA_ORDER.append(VERTEX_ATTRIBUTE_TEXCOORD_ID_PREFIX + str(i))
for i in range(VERTEX_ATTRIBUTE_COLOR_MAX_COUNT):
	VERTEX_ATTRIBUTE_DATA_ORDER.append(VERTEX_ATTRIBUTE_COLOR_ID_PREFIX + str(i))

class DmdTexture:
	"""DMD Texture reference. Does not store pixel data."""
	def __init__(self):
		self.name = ""
		self.size = (0, 0)

	def link(self, linker):
		texture_blob_name = "textures:" + self.name
		texture_data = bytearray(0x10)
		linker.add_string(texture_blob_name, 0x0, self.name)

		# Everything past here is actually irrelevant because the game fills it
		# in with data from the TPL in mapBuildTexture, but the original exporter
		# puts accurate data here that gets overwritten later, so we will as well.
		struct.pack_into(">L", texture_data, 0x0, 0) # image format
		struct.pack_into(">HH", texture_data, 0x8, self.size[0], self.size[1]) # width/height

		linker.add_blob(texture_blob_name, texture_data)
		linker.place_blob_in_section(texture_blob_name, "texture_data")

	@staticmethod
	def from_blender_image(blender_image):
		texture = DmdTexture()
		texture.name = blender_image.name
		texture.size = tuple(blender_image.size)
		return texture

class DmdMaterial:
	def __init__(self):
		self.name = ""

		self.use_fixed_color = True
		self.fixed_color = (0.8, 0.8, 0.8, 1.0)
		# Vertex color layers: Builtin TEV modes only use one vertex color and
		# in fact support vertex color 1 is broken in retail TTYD, however mods
		# may want to make use of more than one vertex color, so we support
		# them in principle.
		self.color_layers = []

		self.samplers = []

		self.blend_mode = 0
		self.tev_mode = 0

	def get_referenced_vertex_attributes(self):
		attributes = [
			VERTEX_ATTRIBUTE_POSITION_ID,
			VERTEX_ATTRIBUTE_NORMAL_ID
		]
		for i in range(len(self.color_layers)):
			attributes.append(VERTEX_ATTRIBUTE_COLOR_ID_PREFIX + str(i))
		for i in range(len(self.samplers)):
			attributes.append(VERTEX_ATTRIBUTE_TEXCOORD_ID_PREFIX + str(i))
		return attributes

	def get_uv_layer_names(self):
		layers = []
		for sampler in self.samplers:
			layers.append(sampler["uv_layer"])
		return layers

	def get_color_layer_names(self):
		return self.color_layers

	def link(self, linker):
		material_blob_name = "materials:" + self.name
		material_data = bytearray(0x114)
		linker.add_string(material_blob_name, 0x000, self.name)
		struct.pack_into(
			">BBBB",
			material_data,
			0x004,
			int(self.fixed_color[0] * 255),
			int(self.fixed_color[1] * 255),
			int(self.fixed_color[2] * 255),
			int(self.fixed_color[3] * 255)
		)
		struct.pack_into(">B", material_data, 0x008, 0 if self.use_fixed_color else 1)
		struct.pack_into(">B", material_data, 0x00a, self.blend_mode) # blend mode
		struct.pack_into(">B", material_data, 0x00b, len(self.samplers)) # texture count

		# Serialize samplers
		for i, sampler in enumerate(self.samplers):
			# External sampler data
			sampler_blob_name = "samplers:" + str(linker.get_uid())
			sampler_data = bytearray(0xc)

			texture_blob_name = "textures:" + sampler["texture_name"]
			linker.add_relocation(sampler_blob_name, 0x0, texture_blob_name)

			struct.pack_into(">BB", sampler_data, 0x8, sampler["wrapS"], sampler["wrapT"])

			linker.add_blob(sampler_blob_name, sampler_data)
			linker.place_blob_in_section(sampler_blob_name, "sampler_data")

			linker.add_relocation(material_blob_name, 0x00c + i * 4, sampler_blob_name)

			# Material-internal transform data
			struct.pack_into(
				">ff",
				material_data,
				0x02c + i * 0x1c + 0x00,
				sampler["translation"][0],
				sampler["translation"][1]
			)
			struct.pack_into(
				">ff",
				material_data,
				0x02c + i * 0x1c + 0x08,
				sampler["scale"][0],
				sampler["scale"][1]
			)
			struct.pack_into(
				">f",
				material_data,
				0x02c + i * 0x1c + 0x10,
				sampler["rotation"]
			)

		# Fill in default texture coordinate transforms
		for i in range(len(self.samplers), 8):
			struct.pack_into(">f", material_data, 0x02c + i * 0x1c + 0x08, 1.0) # scale X
			struct.pack_into(">f", material_data, 0x02c + i * 0x1c + 0x0c, 1.0) # scale Y

		# blend alpha modulation in red channel, not sure how to integrate into model
		struct.pack_into(">L", material_data, 0x10c, 0xffffffff)
		# todo-blender_io_ttyd: Investigate additional fields of TEV configuration structure
		tev_config_blob_name = "tev_configs:" + self.name
		tev_config_data = bytearray(0xc)
		struct.pack_into(">L", tev_config_data, 0x00, self.tev_mode)
		linker.add_blob(tev_config_blob_name, tev_config_data)
		linker.place_blob_in_section(tev_config_blob_name, "tev_configs")

		linker.add_relocation(material_blob_name, 0x110, tev_config_blob_name)

		linker.add_blob(material_blob_name, material_data)
		linker.place_blob_in_section(material_blob_name, "materials")
		return material_blob_name

	@staticmethod
	def from_blender_material(blender_material):
		material = DmdMaterial()
		material.name = blender_material.name

		if not blender_material.use_nodes:
			print("io_scene_ttyd: Material {} has no node tree!".format(blender_material.name))
			return material

		# Find active output node
		nodes = blender_material.node_tree.nodes
		output_node = None
		for n in nodes:
			if not isinstance(n, bpy.types.ShaderNodeOutputMaterial):
				continue
			if not n.is_active_output:
				continue
			output_node = n
			break

		if not output_node:
			print("io_scene_ttyd: Material {} has no output node!".format(blender_material.name))
			return material

		def get_node_input_source(node, input_id):
			if isinstance(input_id, str):
				if not input_id in node.inputs:
					return None
			else:
				if input_id >= len(node.inputs):
					return None

			links = node.inputs[input_id].links
			if not len(links):
				return None

			return links[0].from_socket

		shader_source = get_node_input_source(output_node, "Surface")
		assert(shader_source != None)
		shader_node = shader_source.node

		# Handle transparent mix case
		# todo-blender_io_ttyd: This is a very crude way to handle this case.
		#                       Rethink and potentially redo this.
		allow_transparency = False
		if isinstance(shader_node, bpy.types.ShaderNodeMixShader):
			# todo-blender_io_ttyd: Verify that the order of ShaderNode.inputs
			#                       is guaranteed to be like this.
			alpha_socket = get_node_input_source(shader_node, 0)
			transparency_shader_socket = get_node_input_source(shader_node, 1)
			color_shader_socket = get_node_input_source(shader_node, 2)
			if (alpha_socket != None
				and color_shader_socket != None
				and isinstance(color_shader_socket.node, bpy.types.ShaderNodeBsdfDiffuse)
				and transparency_shader_socket != None
				and isinstance(transparency_shader_socket.node, bpy.types.ShaderNodeBsdfTransparent)):
				color_shader_node = color_shader_socket.node
				alpha_node = alpha_socket.node
				allow_transparency = True
		if not allow_transparency:
			color_shader_node = shader_node
			alpha_socket = None

		if isinstance(color_shader_node, bpy.types.ShaderNodeBsdfDiffuse):
			color_socket_name = "Color"
		else:
			# No known shader
			return material

		# Blend mode
		if not allow_transparency:
			material.blend_mode = 0
		elif blender_material.blend_method == 'OPAQUE':
			material.blend_mode = 0
		elif blender_material.blend_method == 'CLIP':
			# No blend, fixed alpha cutoff at 128
			material.blend_mode = 1
		elif blender_material.blend_method == 'BLEND':
			material.blend_mode = 2

		color_socket = color_shader_node.inputs[color_socket_name]
		if not color_socket.is_linked:
			# Fixed color
			assert(isinstance(color_socket, bpy.types.NodeSocketColor))
			material.use_fixed_color = True
			material.fixed_color = linear_to_srgb(color_socket.default_value)
			return material

		def get_sampler_from_tex_node(node):
			assert(isinstance(node, bpy.types.ShaderNodeTexImage))

			sampler = {}
			if not node.image:
				return None

			sampler["texture_name"] = node.image.name
			assert(node.extension != 'CLIP')
			blender_extension_to_wrap_mode = {
				'REPEAT': 1,
				'EXTEND': 0,
			}
			dmd_wrap_mode = blender_extension_to_wrap_mode[node.extension]
			sampler["wrapS"] = dmd_wrap_mode
			sampler["wrapT"] = dmd_wrap_mode

			# Texture coordinate transform
			sampler["translation"] = (0.0, 0.0)
			sampler["rotation"] = 0.0
			sampler["scale"] = (1.0, 1.0)

			vector_source = get_node_input_source(node, "Vector")
			if vector_source:
				vector_node = vector_source.node
				if isinstance(vector_node, bpy.types.ShaderNodeMapping):
					assert(vector_node.vector_type == 'POINT')
					sampler["translation"] = tuple(vector_node.translation[0:2])
					sampler["rotation"] = vector_node.rotation[2]
					sampler["scale"] = tuple(vector_node.scale[0:2])
					# Keep track of the mapping node in order to map UV animations
					# back to the right sampler
					sampler["mapping_node"] = vector_node.name
					uv_node = get_node_input_source(vector_node, "Vector").node
				elif isinstance(vector_node, bpy.types.ShaderNodeUVMap):
					uv_node = vector_node
				else:
					uv_node = None

				if uv_node:
					sampler["uv_layer"] = uv_node.uv_map
			else:
				# Default UV parameters
				sampler["translation"] = (0.0, 0.0)
				sampler["rotation"] = 0.0
				sampler["scale"] = (1.0, 1.0)

			if "uv_layer" not in sampler:
				sampler["uv_layer"] = "UVMap"
			return sampler

		color_source = get_node_input_source(color_shader_node, "Color")
		if color_source == None:
			print("io_scene_ttyd: Material {} has no color node!".format(blender_material.name))
			return material
		color_node = color_source.node

		# Parse vertex color if it exists, either as direct input or as multiply

		# todo-blender_io_ttyd: Implement vertex color alpha support
		# todo-blender_io_ttyd: Implement vertex color multi-layer blend support
		
		# Note that we do not support vertex color alpha or more complex vertex
		# color setups at this point. First-class support for alpha channels in
		# vertex color layers is apparently a planned feature for Blender (you
		# already can paint them, just not access in a material graph), so I
		# will put off proper implementation of vertex colors until that time
		# (expected possibly in Blender 2.81).

		if isinstance(color_node, bpy.types.ShaderNodeAttribute):
			# Direct vertex color
			material.color_layers = [color_node.attribute_name]
			material.use_fixed_color = False
			return material
		elif isinstance(color_node, bpy.types.ShaderNodeMixRGB):
			# Potentially: Multiplied vertex color over TEV setup
			if (color_node.blend_type == "MULTIPLY"
				and not color_node.inputs["Fac"].is_linked
				and color_node.inputs["Fac"].default_value == 1.0):

				color1_source = get_node_input_source(color_node, "Color1")
				color2_source = get_node_input_source(color_node, "Color2")
				if (color1_source != None
					and isinstance(color1_source.node, bpy.types.ShaderNodeAttribute)):
					material.color_layers = [color1_source.node.attribute_name]
					color_node = color2_source.node
				elif (color2_source != None
					and isinstance(color2_source.node, bpy.types.ShaderNodeAttribute)):
					material.color_layers = [color2_source.node.attribute_name]
					color_node = color1_source.node
				if len(material.color_layers):
					material.use_fixed_color = False

		if isinstance(color_node, bpy.types.ShaderNodeTexImage):
			sampler = get_sampler_from_tex_node(color_node)
			material.samplers.append(sampler)
			material.fixed_color = (1.0, 1.0, 1.0, 1.0)
			return material
			
		# todo-blender_io_ttyd: Implement additional TEV modes

		return material

class DmdVcdTable:
	def __init__(self):
		self.attribute_data = defaultdict(list)

	def store_attribute_data(self, attribute, data):
		# Try to find an existing instance of the data
		stored_data = self.attribute_data[attribute]
		for i in range(len(stored_data)):
			if stored_data[i] == data:
				return i

		# Did not find existing instance, add.
		stored_data.append(data)
		out_index = len(stored_data) - 1
		assert(out_index < 65536) # Max encodable index
		return out_index

	def link(self, linker):
		# Figure out quantizations
		quantizations = {}
		for attribute_name in self.attribute_data:
			# Color is unquantized
			if attribute_name.startswith(VERTEX_ATTRIBUTE_COLOR_ID_PREFIX):
				continue
			# Normals are quantized at fixed scale
			if attribute_name == VERTEX_ATTRIBUTE_NORMAL_ID:
				continue
			most_extreme_value = 0.0
			for entry in self.attribute_data[attribute_name]:
				for value in entry:
					most_extreme_value = max(abs(value), most_extreme_value)

			# Due to the fact that the positive maximum of a quantized value is
			# one less than the optimal power of two, we have to add this bias
			# in order to choose the next lower quantization in this case.
			most_extreme_value += 1.0

			if most_extreme_value == 0.0:
				# Corner case which would make math.log throw an exception
				max_magnitude = 0
			else:
				max_magnitude = math.ceil(math.log2(most_extreme_value))
			best_quantization = -(max_magnitude - 15)
			quantizations[attribute_name] = best_quantization

		# Serialize data in correct order
		for attribute_name in VERTEX_ATTRIBUTE_DATA_ORDER:
			if attribute_name not in self.attribute_data:
				continue

			attribute_blob_name = "vertex_attribute_data:" + attribute_name

			unsigned = False
			if attribute_name.startswith(VERTEX_ATTRIBUTE_COLOR_ID_PREFIX):
				element_width = 1
				element_count = 4
				quantization = 0
				unsigned = True
			elif attribute_name == VERTEX_ATTRIBUTE_NORMAL_ID:
				element_width = 1
				element_count = 3
				quantization = 6
			elif attribute_name == VERTEX_ATTRIBUTE_POSITION_ID:
				element_width = 2
				element_count = 3
				quantization = quantizations[attribute_name]
			elif attribute_name.startswith(VERTEX_ATTRIBUTE_TEXCOORD_ID_PREFIX):
				element_width = 2
				element_count = 2
				quantization = quantizations[attribute_name]
			else:
				assert(False)

			# Pack data into buffer
			unquantized_data = self.attribute_data[attribute_name]

			quantized_stride = element_width * element_count
			attribute_buffer_size = 4 + quantized_stride * len(unquantized_data)
			attribute_buffer_size = align_up(attribute_buffer_size, 32)
			attribute_buffer = bytearray(attribute_buffer_size)
			struct.pack_into(">L", attribute_buffer, 0x0, len(unquantized_data))

			for data_index, data in enumerate(unquantized_data):
				data_offset = 4 + quantized_stride * data_index
				for element_index, element in enumerate(data):
					element_offset = data_offset + element_width * element_index

					# Avoid unnecessary handling of unquantized data
					if quantization != 0:
						quantized_element = int(round(element * 2.0**quantization))
						#print("DmdVcdTable: Quantizing {}: {} -> {} (factor {})".format(attribute_name, element, quantized_element, 2.0**quantization))
					else:
						quantized_element = element

					# Check for over/underflow
					if quantization != 0:
						max_quantized_magnitude = 2**(8 * element_width - 1)
						if (quantized_element >= max_quantized_magnitude
							or quantized_element < -max_quantized_magnitude):
							print("DmdVcdTable: Unable to quantize {} value {}".format(attribute_name, element))

					# Get right format string
					if element_width == 1:
						format_string = ">B" if unsigned else ">b"
					elif element_width == 2:
						assert(not unsigned)
						format_string = ">h"
					else:
						assert(False)

					# Write the actual data
					struct.pack_into(
						format_string,
						attribute_buffer,
						element_offset,
						quantized_element
					)
			linker.add_blob(attribute_blob_name, attribute_buffer)
			linker.place_blob_in_section(attribute_blob_name, "vertex_attribute_data")

		# Finally create VCD table
		vcd_table_blob_name = "vcd_table"
		vcd_table_data = bytearray(0x68)

		# todo-blender_io_ttyd: Clean up the variable attribute count tracking here.
		color_count = 0
		tc_count = 0
		for attribute_name in self.attribute_data:
			attribute_blob_name = "vertex_attribute_data:" + attribute_name

			if attribute_name == VERTEX_ATTRIBUTE_POSITION_ID:
				struct.pack_into(">l", vcd_table_data, 0x44, quantizations[attribute_name])
				linker.add_relocation(vcd_table_blob_name, 0x00, attribute_blob_name)
			elif attribute_name == VERTEX_ATTRIBUTE_NORMAL_ID:
				linker.add_relocation(vcd_table_blob_name, 0x04, attribute_blob_name)
			elif attribute_name.startswith(VERTEX_ATTRIBUTE_COLOR_ID_PREFIX):
				color_index = int(attribute_name[len(VERTEX_ATTRIBUTE_COLOR_ID_PREFIX):])
				linker.add_relocation(
					vcd_table_blob_name,
					0x0c + color_index * 4,
					attribute_blob_name
				)
				color_count += 1
			elif attribute_name.startswith(VERTEX_ATTRIBUTE_TEXCOORD_ID_PREFIX):
				tc_index = int(attribute_name[len(VERTEX_ATTRIBUTE_TEXCOORD_ID_PREFIX):])
				tc_offset = tc_index * 4
				linker.add_relocation(
					vcd_table_blob_name,
					0x18 + tc_offset,
					attribute_blob_name
				)
				struct.pack_into(
					">l",
					vcd_table_data,
					0x48 + tc_offset,
					quantizations[attribute_name]
				)
				tc_count += 1

		# Store color and texture coordinate counts
		struct.pack_into(">L", vcd_table_data, 0x08, color_count)
		struct.pack_into(">L", vcd_table_data, 0x14, tc_count)

		linker.add_blob(vcd_table_blob_name, vcd_table_data)
		linker.place_blob_in_section(vcd_table_blob_name, "vcd_table")
		return vcd_table_blob_name

class DmdModel:
	"""DMD File Model with one material consisting of triangle strips"""

	def __init__(self):
		self.material_name = ""
		self.attributes = []
		self.polygons = []

	def get_bbox(self):
		# todo-blender_io_ttyd: Use util.get_bbox() instead
		first_vertex = True
		for p in self.polygons:
			for v in p:
				position = v[VERTEX_ATTRIBUTE_POSITION_ID]
				if first_vertex:
					first_vertex = False
					bbox_min = list(position)
					bbox_max = list(position)
					continue
				for i in range(3):
					bbox_min[i] = min(bbox_min[i], position[i])
					bbox_max[i] = max(bbox_max[i], position[i])
		if first_vertex:
			return None
		return (tuple(bbox_min), tuple(bbox_max))

	def link(self, linker, vcd_table):
		# Write vertex data into VCD table and get final attribute indices
		out_polygons = []
		for p in self.polygons:
			out_vertices = []
			for v in p:
				out_attribute_indices = []
				for attribute_name in VERTEX_ATTRIBUTE_INDEX_ORDER:
					if attribute_name not in v:
						continue
					index = vcd_table.store_attribute_data(attribute_name, v[attribute_name])
					out_attribute_indices.append(index)
				out_vertices.append(out_attribute_indices)
			out_polygons.append(out_vertices)

		mesh_blob_name = "meshs:" + str(linker.get_uid())

		# Align mesh size upwards to 32 bytes to maintain alignment
		# with Polygons (containing display lists)
		mesh_data_size = align_up(0x10 + 8 * len(out_polygons), 32)
		mesh_data = bytearray(mesh_data_size)
		struct.pack_into(">B", mesh_data, 0x0, 1) # Unknown
		struct.pack_into(">B", mesh_data, 0x3, 1) # bPolygonsAreDisplayLists, always 1 in v1.02
		struct.pack_into(">L", mesh_data, 0x4, len(out_polygons)) # Polygon count

		# Build element mask
		element_mask = 0
		for i, attribute in enumerate(VERTEX_ATTRIBUTE_INDEX_ORDER):
			if attribute in self.attributes:
				element_mask |= (1 << i)
		struct.pack_into(">L", mesh_data, 0x8, element_mask)

		linker.add_relocation(mesh_blob_name, 0xc, "vcd_table")

		# Calculate stride
		vertex_stride = 0
		for attribute in self.attributes:
			if attribute in VERTEX_ATTRIBUTE_INDEX_ORDER:
				vertex_stride += 2

		for polygon_index, polygon in enumerate(out_polygons):
			polygon_blob_name = mesh_blob_name + ":polygons:" + str(polygon_index)

			# Calculate aligned size
			polygon_data_size = align_up(3 + len(polygon) * vertex_stride, 32)
			polygon_data = bytearray(polygon_data_size)
			struct.pack_into(">B", polygon_data, 0x0, 0x98) # display list leading draw opcode
			struct.pack_into(">H", polygon_data, 0x1, len(polygon)) # vertex count
			
			for vertex_index, vertex in enumerate(polygon):
				vertex_offset = 0x3 + vertex_index * vertex_stride
				for attribute_index, attribute in enumerate(vertex):
					# Fixed 16-bit indices
					attribute_offset = vertex_offset + attribute_index * 2
					struct.pack_into(">H", polygon_data, attribute_offset, attribute)

			# Place Polygon data in same section as mesh data. This is
			# emulating original exporter behavior, however it may not
			# best choice for size and performance.
			linker.add_blob(polygon_blob_name, polygon_data)
			linker.place_blob_in_section(polygon_blob_name, "meshs")

			linker.add_relocation(mesh_blob_name, 0x10 + 8 * polygon_index, polygon_blob_name)
			struct.pack_into(">L", mesh_data, 0x14 + 8 * polygon_index, polygon_data_size)

		linker.add_blob(mesh_blob_name, mesh_data)
		linker.place_blob_in_section(mesh_blob_name, "meshs")
		return mesh_blob_name

	@staticmethod
	def list_from_blender_mesh(blender_mesh, materials):
		material_data = {}
		for blender_polygon in blender_mesh.polygons:
			# Get appropriate material if this mesh has any and we're not doing collision
			dmd_material = None
			if materials != None and len(blender_mesh.materials) > 0:
				blender_material = blender_mesh.materials[blender_polygon.material_index]
				for m in materials:
					if m.name == blender_material.name:
						dmd_material = m
						break
				# Create new material if no existing one
				if dmd_material == None:
					dmd_material = DmdMaterial.from_blender_material(blender_material)
					materials.append(dmd_material)

				attributes_to_store = dmd_material.get_referenced_vertex_attributes()
				blender_uv_layers = dmd_material.get_uv_layer_names()
				blender_color_layers = dmd_material.get_color_layer_names()
				material_name = dmd_material.name
			else:
				# No material, store info necessary for collision detection
				# todo-blender_io_ttyd: Is normal necessary for pure hit meshes?
				attributes_to_store = [
					VERTEX_ATTRIBUTE_POSITION_ID,
					VERTEX_ATTRIBUTE_NORMAL_ID
				]
				blender_uv_layers = []
				blender_color_layers = []
				material_name = ""

			# Setup per-material polygon data
			if material_name not in material_data:
				material_data[material_name] = {
					"polygons": [],
					"attributes": attributes_to_store
				}

			# Tessellate polygons into triangles
			loop_indices = [i for i in blender_polygon.loop_indices]
			polygon_tessellated_loop_indices = []
			if len(loop_indices) == 3:
				polygon_tessellated_loop_indices.append(list(reversed(loop_indices)))
			elif len(loop_indices) > 3:
				# todo-blender_io_ttyd: Tessellate polygons into strips instead
				# of outputting individual triangles
				vertex_positions = []
				for loop_index in loop_indices:
					loop = blender_mesh.loops[loop_index]
					vertex = blender_mesh.vertices[loop.vertex_index]
					vertex_positions.append(vertex.co)
				tessellated_triangles = mathutils.geometry.tessellate_polygon([vertex_positions])
				for tri in tessellated_triangles:
					# todo-blender_io_ttyd: Should blender_polygon.loop_indices
					# not be simply loop_indices here?
					tri_loop_indices = [blender_polygon.loop_indices[i] for i in tri]
					polygon_tessellated_loop_indices.append(tri_loop_indices)

			# Extract attributes
			# We store the raw attribute data; the data is deduplicated when
			# linking in the VCD table

			# Evaluate face normal only once and share index if not smooth shaded
			if (VERTEX_ATTRIBUTE_NORMAL_ID in attributes_to_store
				and not blender_polygon.use_smooth):
				face_normal = tuple(blender_polygon.normal)

			# todo-blender_io_ttyd: Think about removing attribute names from
			# vertices here since they're already stored in the model's field
			for triangle_loop_indices in polygon_tessellated_loop_indices:
				vertices = []
				for loop_index in triangle_loop_indices:
					vertex_attributes = {}
					loop = blender_mesh.loops[loop_index]
					vertex = blender_mesh.vertices[loop.vertex_index]
					assert(VERTEX_ATTRIBUTE_POSITION_ID in attributes_to_store)
					# todo-blender_io_ttyd: Think about flattening this loop
					# into a series of if-statements with loops for
					# colors/texcoords
					for attribute_name in attributes_to_store:
						if attribute_name == VERTEX_ATTRIBUTE_POSITION_ID:
							vertex_attributes[attribute_name] = tuple(vertex.co)
						elif attribute_name == VERTEX_ATTRIBUTE_NORMAL_ID:
							if blender_polygon.use_smooth:
								normal_data = tuple(vertex.normal)
							else:
								normal_data = face_normal

							vertex_attributes[attribute_name] = normal_data
						elif attribute_name.startswith(VERTEX_ATTRIBUTE_TEXCOORD_ID_PREFIX):
							tc_index = int(attribute_name[len(VERTEX_ATTRIBUTE_TEXCOORD_ID_PREFIX):])
							assert(tc_index < VERTEX_ATTRIBUTE_TEXCOORD_MAX_COUNT)

							if tc_index < len(blender_uv_layers):
								tc_layer_name = blender_uv_layers[tc_index]
								tc_layer = blender_mesh.uv_layers[tc_layer_name]
								tc_data = tuple(tc_layer.data[loop_index].uv)
							else:
								# todo-blender_io_ttyd: Figure out if this is a
								# fatal error; probably should be.
								assert(False)
								tc_data = (0.0, 0.0)

							vertex_attributes[attribute_name] = tc_data
						elif attribute_name.startswith(VERTEX_ATTRIBUTE_COLOR_ID_PREFIX):
							color_index = int(attribute_name[len(VERTEX_ATTRIBUTE_COLOR_ID_PREFIX):])
							assert(color_index < VERTEX_ATTRIBUTE_COLOR_MAX_COUNT)

							if color_index < len(blender_color_layers):
								color_layer_name = blender_color_layers[color_index]
								color_layer = blender_mesh.vertex_colors[color_layer_name]

								# No SRGB conversion necessary!
								color_data = tuple(color_layer.data[loop_index].color)

								# todo-blender_io_ttyd: Think about whether
								# this is the best way to handle color
								# quantization.

								# Convert to 0-255 here instead of VCD table as
								# the quantizations there are different in that
								# they multiply by 2**n; for colors however,
								# this does not hold, as 1.0 maps to 255 and
								# not to 256.
								color_data = tuple(int(x * 255) for x in color_data)
							else:
								# todo-blender_io_ttyd: Figure out if this is a
								# probably should be.
								assert(False)
								color_data = (255, 0, 255, 255)

							vertex_attributes[attribute_name] = color_data
					vertices.append(vertex_attributes)
				material_data[material_name]["polygons"].append(vertices)

		# Build final models
		models = []
		for material_name in material_data:
			model = DmdModel()
			model.material_name = material_name
			model.attributes = material_data[material_name]["attributes"]
			model.polygons = material_data[material_name]["polygons"]
			models.append(model)
		return models

class DmdJoint:
	"""DMD File Joint"""

	def __init__(self):
		self.name = ""

		self.children = []
		self.models = []

		self.translation = (0.0, 0.0, 0.0)
		self.rotation = (0.0, 0.0, 0.0)
		self.scale = (1.0, 1.0, 1.0)

	def link(self, linker, vcd_table, parent = None, next = None, prev = None):
		blob_name = "joints:{}".format(self.name)
		joint_data = bytearray(0x60 + len(self.models) * 0x8)

		linker.add_string(blob_name, 0x00, self.name)
		linker.add_string(blob_name, 0x04, "mesh" if len(self.models) > 0 else "null")

		if parent:
			linker.add_relocation(blob_name, 0x08, "joints:{}".format(parent.name))
		if len(self.children) > 0:
			linker.add_relocation(blob_name, 0x0c, "joints:{}".format(self.children[0].name))
		if next:
			linker.add_relocation(blob_name, 0x10, "joints:{}".format(next.name))
		if prev:
			linker.add_relocation(blob_name, 0x14, "joints:{}".format(prev.name))
		
		struct.pack_into(
			">fff", joint_data, 0x18,
			self.scale[0], self.scale[1], self.scale[2]
		)
		struct.pack_into(
			">fff", joint_data, 0x24,
			self.rotation[0], self.rotation[1], self.rotation[2]
		)
		struct.pack_into(
			">fff", joint_data, 0x30,
			self.translation[0], self.translation[1], self.translation[2]
		)

		# Compute local-space bounding box
		bbox = None
		for model in self.models:
			model_bbox = model.get_bbox()
			if bbox == None:
				bbox = (list(model_bbox[0]), list(model_bbox[1]))
				continue
			for i in range(3):
				bbox[0][i] = min(bbox[0][i], model_bbox[0][i])
				bbox[1][i] = max(bbox[1][i], model_bbox[1][i])

		# Default to non-zero bounding box imitating original exporter behavior
		if bbox == None:
			bbox = ((-0.1, -0.1, -0.1), (0.1, 0.1, 0.1))
		else:
			bbox = (tuple(bbox[0]), tuple(bbox[1]))

		struct.pack_into(
			">ffffff", joint_data, 0x3c,
			bbox[0][0], bbox[0][1], bbox[0][2],
			bbox[1][0], bbox[1][1], bbox[1][2]
		)
		struct.pack_into(">L", joint_data, 0x54, 0) # unknown

		# todo-blender_io_ttyd: Further investigate drawmode
		if parent != None: # Root node does not have drawmode
			drawmode_blob_name = "drawmodes:{}".format(self.name)
			linker.add_relocation(blob_name, 0x58, drawmode_blob_name)
			drawmode_data = bytearray(0x14)
			struct.pack_into(">B", drawmode_data, 0x1, 1) # cullMode = back
			linker.add_blob(drawmode_blob_name, drawmode_data)
			linker.place_blob_in_section(drawmode_blob_name, "drawmodes")

		struct.pack_into(">L", joint_data, 0x5c, len(self.models))

		for i in range(len(self.models)):
			model = self.models[i]
			mesh_blob_name = model.link(linker, vcd_table)

			# Only link material if there is a material, hit data may not have one
			if model.material_name != "":
				material_blob_name = "materials:{}".format(model.material_name)
				linker.add_relocation(blob_name, 0x60 + i * 8, material_blob_name)

			linker.add_relocation(blob_name, 0x64 + i * 8, mesh_blob_name)

		linker.add_blob(blob_name, joint_data)
		linker.place_blob_in_section(blob_name, "joints")

		for i in range(len(self.children)):
			if i > 0:
				prev_child = self.children[i - 1]
			else:
				prev_child = None
			if i < len(self.children) - 1:
				next_child = self.children[i + 1]
			else:
				next_child = None
			self.children[i].link(linker, vcd_table, self, next_child, prev_child)
		return blob_name

	@staticmethod
	def from_blender_object(blender_object, materials, global_matrix = None):
		joint = DmdJoint()
		joint.name = blender_object.name

		joint.children = []
		for c in blender_object.children:
			joint.children.append(DmdJoint.from_blender_object(c, materials))

		transform_matrix = blender_object.matrix_local
		if global_matrix != None:
			transform_matrix = global_matrix @ transform_matrix
		translation, rotation, scale = transform_matrix.decompose()
		joint.translation = translation.to_tuple()

		rotation_euler = rotation.to_euler()
		joint.rotation = (
			math.degrees(rotation_euler.x),
			math.degrees(rotation_euler.y),
			math.degrees(rotation_euler.z)
		)

		joint.scale = scale.to_tuple()

		if blender_object.type == "MESH":
			blender_depsgraph = bpy.context.evaluated_depsgraph_get()
			blender_evaluated_object = blender_object.evaluated_get(blender_depsgraph)
			blender_mesh = blender_evaluated_object.to_mesh()
			joint.models = DmdModel.list_from_blender_mesh(blender_mesh, materials)
			blender_evaluated_object.to_mesh_clear()

		return joint

	@staticmethod
	def from_blender_collection(collection, materials, global_matrix = None):
		joint = DmdJoint()
		joint.name = collection.name

		joint.children = []
		first_level_objects = [x for x in collection.all_objects if not x.parent]
		for c in first_level_objects:
			joint.children.append(DmdJoint.from_blender_object(c, materials, global_matrix))
		return joint


def blender_anim_data_from_dmd_object(target):
	anim_data = None
	if isinstance(target, DmdJoint):
		anim_data = bpy.data.objects[target.name].animation_data
	elif isinstance(target, DmdMaterial):
		blender_material = bpy.data.materials[target.name]
		if blender_material.node_tree != None:
			anim_data = blender_material.node_tree.animation_data
	else:
		# todo-blender_io_ttyd: Add light animations
		assert(False)
	return anim_data

def value_keyframe_convert_to_degrees(value_keyframe):
		new_value_keyframe = list(value_keyframe)
		for var_index in range(3):
			new_value_keyframe[var_index] = math.degrees(value_keyframe[var_index])
		return tuple(new_value_keyframe)


class DmdAnimation:
	def __init__(self):
		self.name = ""
		self.length = 0.0
		self.joint_transform_tracks = []
		self.material_uv_tracks = []
		self.material_blend_tracks = [] # TODO
		self.light_transform_tracks = [] # TODO
		self.light_parameter_tracks = [] # TODO

	def link(self, linker):
		anim_blob_name = "animations:{}".format(self.name)
		anim_data = bytearray(0x28)

		linker.add_string(anim_blob_name, 0x00, self.name)

		# Length in frames (float)
		struct.pack_into(">f", anim_data, 0x08, self.length)

		# Helper to pack a keyframe tuple
		def pack_keyframe_into(buffer, offset, data):
			# Value, tangent in, tangent out, unk_0c, is_step
			# unk_0c is zero in all animations in all TTYD maps, so we don't
			# bother storing
			struct.pack_into(
				">fffLL", buffer, offset,
				data[0], data[1], data[2], 0, 1 if data[3] else 0
			)

		# Tuples of (track list, offset to track table pointer in animation,
		# header size, components)
		track_type_descriptions = {
			"joint_transform": (
				self.joint_transform_tracks,
				0x0c, # offset to track table ptr in animation
				[
					("translation", 3),
					("rotation", 3),
					("scale", 3),
					(None, 3 * 4), # Unused fields
				],
				0x58 # header size
			),
			"material_uv": (
				self.material_uv_tracks,
				0x10,
				[
					("translation", 2),
					("scale", 2),
					("rotation", 1),
				],
				0x10
			),
			"material_blend": (
				self.material_blend_tracks,
				0x14,
				[
					("color", 4),
				],
				0x4
			),
			"light_transform": (
				self.light_transform_tracks,
				0x18,
				[
					("translation", 3),
					("rotation", 3),
					("scale", 3),
				],
				0x4
			),
			"light_parameters": (
				self.light_parameter_tracks,
				0x1c,
				[
					("color", 3),
					("spot_angle", 1),
					("angular_attenuation", 3),
				],
				0x4
			),
		}

		all_track_type_blob_names = []
		for track_type_name, track_type_description in track_type_descriptions.items():
			track_type_blob_names = []

			track_list = track_type_description[0]
			animation_track_type_table_offset = track_type_description[1]
			track_components = track_type_description[2]
			track_header_size = track_type_description[3]

			if len(track_list) < 1:
				continue

			track_total_component_count = sum([x[1] for x in track_components])
			# All keyframes are time as float header followed by some amount of
			# component keyframes
			track_keyframe_size = 0x4 + track_total_component_count * 0x14

			for track in track_list:
				track_blob_name = "animation_data:tracks:{}".format(linker.get_uid())
				track_data = bytearray(track_header_size + 0x4 + len(track["keyframes"]) * track_keyframe_size)

				if track_type_name == "joint_transform":
					linker.add_string(track_blob_name, 0x00, track["joint_name"])

					# Translation/rotation/scale origin
					struct.pack_into(
						">fff", track_data, 0x04,
						track["translation_origin"][0],
						track["translation_origin"][1],
						track["translation_origin"][2]
					)
					struct.pack_into(
						">fff", track_data, 0x10,
						track["rotation_origin"][0],
						track["rotation_origin"][1],
						track["rotation_origin"][2]
					)
					struct.pack_into(
						">fff", track_data, 0x1c,
						track["scale_origin"][0],
						track["scale_origin"][1],
						track["scale_origin"][2]
					)

					# Four unused sets, the first and third are sometimes set to
					# 0.408818 on the Y-axis. Presumably these are Maya-specific
					# additional transforms. We do not use this value here since it
					# makes no actual difference and is not consistently used in the
					# original exporter.
					struct.pack_into(">fff", track_data, 0x28, 0.0, 0.0, 0.0)
					struct.pack_into(">fff", track_data, 0x34, 0.0, 0.0, 0.0)
					struct.pack_into(">fff", track_data, 0x40, 0.0, 0.0, 0.0)
					struct.pack_into(">fff", track_data, 0x4c, 0.0, 0.0, 0.0)
				elif track_type_name == "material_uv":
					linker.add_string(track_blob_name, 0x0, track["material_name"])
					struct.pack_into(">L", track_data, 0x4, track["sampler_index"])

					# Scale-independent translation for alignment, not animatable
					struct.pack_into(">ff", track_data, 0x8, track["align"][0], track["align"][1])
				elif track_type_name == "material_blend":
					linker.add_string(track_blob_name, 0x0, track["material_name"])
				elif track_type_name == "light_transform":
					linker.add_string(track_blob_name, 0x0, track["light_name"])
				elif track_type_name == "light_parameters":
					linker.add_string(track_blob_name, 0x0, track["light_name"])

				# Export the actual keyframes
				struct.pack_into(">L", track_data, track_header_size + 0x0, len(track["keyframes"]))

				for keyframe_index, keyframe in enumerate(track["keyframes"]):
					keyframe_offset = track_header_size + 0x4 + keyframe_index * track_keyframe_size

					struct.pack_into(">f", track_data, keyframe_offset, keyframe["time"])

					component_offset = keyframe_offset + 0x4
					for value_index, value_info in enumerate(track_components):
						value_key = value_info[0]
						value_component_count = value_info[1]
						for component_index in range(value_component_count):
							if value_key != None:
								if value_component_count == 1:
									component_source_data = keyframe[value_key]
								else:
									component_source_data = keyframe[value_key][component_index]
							else:
								component_source_data = (0.0, 0.0, 0.0, 0)
							pack_keyframe_into(
								track_data,
								component_offset,
								component_source_data
							)
							component_offset += 0x14
					assert(component_offset == keyframe_offset + track_keyframe_size)

				# We do not place the blob into a section here to emulate
				# original exporter behavior. The original exporter serializes
				# all data referenced by an animation into one contiguous chunk
				# in a section that follows the serialized animations, with the
				# track table located immediately *before* the tracks it
				# contains. To do this, the track table must be placed in this
				# shared section *after* the tracks themselves have been
				# serialized.
				linker.add_blob(track_blob_name, track_data)
				track_type_blob_names.append(track_blob_name)

			# Serialize track table
			track_table_blob_name = "animation_data:tables:{}".format(linker.get_uid())
			track_table_data = bytearray(0x4 + len(track_type_blob_names) * 4)
			struct.pack_into(">L", track_table_data, 0x0, len(track_type_blob_names))
			for track_index, track_blob_name in enumerate(track_type_blob_names):
				linker.add_relocation(track_table_blob_name, 0x4 + track_index * 4, track_blob_name)
			linker.add_relocation(
				anim_blob_name,
				animation_track_type_table_offset,
				track_table_blob_name
			)
			linker.add_blob(track_table_blob_name, track_table_data)
			linker.place_blob_in_section(track_table_blob_name, "animation_data")

			# Place tracks after their respective table emulating original exporter behavior.
			for track_blob_name in track_type_blob_names:
				linker.place_blob_in_section(track_blob_name, "animation_data")
		
		linker.add_blob(anim_blob_name, anim_data)
		linker.place_blob_in_section(anim_blob_name, "animations")
		return anim_blob_name

	@staticmethod
	def list_from_blender_nla_tracks_for_targets(targets, global_matrix = None):
		animation_sources = defaultdict(set)
		for target in targets:
			anim_data = blender_anim_data_from_dmd_object(target)
			if anim_data == None:
				continue

			for track in anim_data.nla_tracks:
				animation_sources[track.name].add(target)

		animations = []
		for name, targets in animation_sources.items():
			animation = DmdAnimation.from_blender_name_and_targets(
				name,
				targets,
				global_matrix
			)
			animations.append(animation)
		return animations

	@staticmethod
	def from_blender_name_and_targets(name, targets, global_matrix = None):
		anim = DmdAnimation()
		anim.name = name
		
		def any_keyframes_exist(target, action, blender_fcurve_mapping):
			# Check if there are any FCurves in this action that matter for
			# this track type
			for fcurve in action.fcurves:
				blender_data_source = (fcurve.data_path, fcurve.array_index)
				if blender_data_source in blender_fcurve_mapping:
					return True
			return False

		def make_keyframes(target, action, keyframe_layout, blender_fcurve_mapping):
			# There may not be keyframes for all attributes in Blender, however
			# the map animations must always have them, so for convenience we
			# insert best-effort keyframes.
			keyframe_times = set()
			for fcurve in action.fcurves:
				for k in fcurve.keyframe_points:
					keyframe_times.add(k.co[0])
			sorted_keyframe_times = sorted(keyframe_times)

			keyframes = []
			for time in sorted_keyframe_times:
				# Init keyframe data
				keyframe = {}
				keyframe["time"] = time

				for name, count in keyframe_layout.items():
					if count == 1:
						keyframe[name] = None
					else:
						keyframe[name] = [None] * count

				# Try and find corresponding keyframe for each fcurve
				for fcurve in action.fcurves:
					# If we don't use this FCurve, don't bother to convert it
					blender_data_source = (fcurve.data_path, fcurve.array_index)
					if blender_data_source not in blender_fcurve_mapping:
						continue

					found_keyframe = False
					for k in fcurve.keyframe_points:
						if k.co[0] == time:
							found_keyframe = True

							value = k.co[1]
							tangent_in = (k.co[1] - k.handle_left[1]) / (k.co[0] - k.handle_left[0])
							tangent_out = (k.handle_right[1] - k.co[1]) / (k.handle_right[0] - k.co[0])
							is_step = False
							break

					# todo-blender_io_ttyd: Handle missing keyframes with
					# better approximation.
					# If there is no keyframe, approximate badly based on curve
					# data
					if not found_keyframe:
						delta_time_step = 1
						value = fcurve.evaluate(time)
						prev_value = fcurve.evaluate(time - delta_time_step) / delta_time_step
						next_value = fcurve.evaluate(time + delta_time_step) / delta_time_step
						tangent_in = value - prev_value
						tangent_out = next_value - value
						is_step = False

					keyframe_component_data = (value, tangent_in, tangent_out, is_step)
					target_key, target_index = blender_fcurve_mapping[blender_data_source]
					if target_index != None:
						keyframe[target_key][target_index] = keyframe_component_data
					else:
						keyframe[target_key] = keyframe_component_data

				keyframes.append(keyframe)

			# Fill any keyframes that were not accounted for by any FCurve with
			# whatever the current value is.
			blender_fcurve_reverse_mapping = {
				v: k for k, v in blender_fcurve_mapping.items()
			}
			for keyframe in keyframes:
				for value_key in keyframe_layout:
					component_count = keyframe_layout[value_key]
					for component_index in range(component_count):
						if component_count == 1:
							if keyframe[value_key] != None:
								continue
						else:
							if keyframe[value_key][component_index] != None:
								continue

						output_path = (value_key, component_index)
						assert(output_path in blender_fcurve_reverse_mapping)
						blender_source_path, blender_source_index = blender_fcurve_reverse_mapping[output_path]

						# Use Blender RNA magic to get the right property.
						# todo-blender_io_ttyd: This shares code with the
						# anim_data lookup.
						blender_source_object = None
						if isinstance(target, DmdJoint):
							blender_source_object = bpy.data.objects[target.name]
						elif isinstance(target, DmdMaterial):
							blender_source_object = bpy.data.materials[target.name].node_tree
						else:
							assert(False)

						blender_source_property = blender_source_object.path_resolve(blender_source_path)
						if blender_source_index != None:
							blender_source_data = blender_source_property[blender_source_index]
						else:
							blender_source_data = blender_source_property
						
						# Constant value keyframe with zero slope
						value_keyframe = (
							blender_source_data,
							0,
							0,
							False
						)
						if component_count == 1:
							keyframe[value_key] = value_keyframe
						else:
							keyframe[value_key][component_index] = value_keyframe

			return keyframes

		def try_make_joint_transform_track(target, action, global_matrix = None):
			blender_fcurve_mapping = {
				("location", 0): ("translation", 0),
				("location", 1): ("translation", 1),
				("location", 2): ("translation", 2),
				("rotation_euler", 0): ("rotation", 0),
				("rotation_euler", 1): ("rotation", 1),
				("rotation_euler", 2): ("rotation", 2),
				("scale", 0): ("scale", 0),
				("scale", 1): ("scale", 1),
				("scale", 2): ("scale", 2),
			}

			if not any_keyframes_exist(target, action, blender_fcurve_mapping):
				return None

			track = {}
			track["joint_name"] = target.name
			track["keyframes"] = make_keyframes(
				target,
				action,
				{
					"translation": 3,
					"rotation": 3,
					"scale": 3,
				},
				blender_fcurve_mapping
			)

			# todo-blender_io_ttyd: Check that this is works correctly in all
			# cases and no additional coordinate transform is required.
			track["translation_origin"] = target.translation
			track["rotation_origin"] = target.rotation
			track["scale_origin"] = target.scale

			# Convert rotation to degrees
			for keyframe in track["keyframes"]:
				for component_index in range(len(keyframe["rotation"])):
					keyframe["rotation"][component_index] = value_keyframe_convert_to_degrees(
						keyframe["rotation"][component_index]
					)

			return track

		def try_make_material_uv_track(target, sampler_index, action):
			# Check if this sampler is actually animated
			mapping_node_name = target.samplers[sampler_index]["mapping_node"]
			
			fcurve_name_fmt = "nodes[\"" + mapping_node_name + "\"].{}"
			translation_fcurve_name = fcurve_name_fmt.format("translation")
			scale_fcurve_name = fcurve_name_fmt.format("scale")
			rotation_fcurve_name = fcurve_name_fmt.format("rotation")

			blender_fcurve_mapping = {
				(translation_fcurve_name, 0): ("translation", 0),
				(translation_fcurve_name, 1): ("translation", 1),
				(scale_fcurve_name, 0): ("scale", 0),
				(scale_fcurve_name, 1): ("scale", 1),
				(rotation_fcurve_name, 2): ("rotation", None),
			}
			if not any_keyframes_exist(target, action, blender_fcurve_mapping):
				return None

			track = {}
			track["material_name"] = target.name
			track["sampler_index"] = sampler_index

			# This effectively defines the rotation pivot point; (1.0, 1.0) is
			# centered. Blender always has this at (0.0, 0.0), which
			# corresponds to (0.0, 2.0) in the coordinate system used for this
			# field.
			track["align"] = (0.0, 2.0)

			track["keyframes"] = make_keyframes(
				target,
				action,
				{
					"translation": 2,
					"scale": 2,
					"rotation": 1,
				},
				blender_fcurve_mapping
			)

			# Convert rotation to degrees and invert
			for keyframe in track["keyframes"]:
				value_keyframe = value_keyframe_convert_to_degrees(keyframe["rotation"])
				keyframe["rotation"] = tuple([-x for x in value_keyframe[:3]] + [value_keyframe[3]])

			return track

		# Find all NLA strips for each target, each of which is converted into
		# one subanimation.
		length = 0.0
		for target in targets:
			anim_data = blender_anim_data_from_dmd_object(target)
			if anim_data == None:
				continue

			for nla_track in anim_data.nla_tracks:
				# Only get relevant tracks for this animation
				# Can't process tracks
				if nla_track.name != name or len(nla_track.strips) != 1:
					continue
				strip = nla_track.strips[0]
				length = max(length, strip.frame_end)
				action = strip.action

				# Try to generate all types of tracks
				if isinstance(target, DmdJoint):
					track = try_make_joint_transform_track(target, action, global_matrix)
					if track != None:
						anim.joint_transform_tracks.append(track)
				elif isinstance(target, DmdMaterial):
					for sampler_index in range(len(target.samplers)):
						track = try_make_material_uv_track(target, sampler_index, action)
						if track != None:
							anim.material_uv_tracks.append(track)
				else:
					assert(False)
		anim.length = length

		# Notably we do not check for animations without tracks here. They do
		# actually occur in TTYD and may be controlled through the animation
		# system in order to then query the progress to drive some other state.
		return anim

class DmdFile:
	def __init__(self):
		self.root_joint = None

		self.map_joint = None
		self.hit_joint = None

		self.materials = []
		self.textures = []
		self.animations = []

	@staticmethod
	def from_blender_scene(scene, settings):
		file = DmdFile()
		file.root_joint = DmdJoint()
		file.root_joint.name = "world_root"

		global_matrix = mathutils.Matrix.Identity(4)
		if "axis_conversion_matrix" in settings:
			global_matrix = settings["axis_conversion_matrix"]
		file.materials = []
		file.map_joint = DmdJoint.from_blender_collection(
			settings["map_root"],
			file.materials,
			global_matrix
		)
		file.hit_joint = DmdJoint.from_blender_collection(
			settings["hit_root"],
			None,
			global_matrix
		)

		file.root_joint.children = [
			file.map_joint,
			file.hit_joint
		]

		# Identify textures and sort into correct order

		# todo-blender_io_ttyd: This style of gathering textures is 
		# inconsistent with the style for gathering materials used a few lines
		# above. They should probably use the same approach, however properties
		# of the data are required for materials as the referenced vertex
		# attributes are necessary for vertex attribute data serialization.
		# Think about this.
		
		texture_order = []
		for material in file.materials:
			samplers = material.samplers
			for sampler in samplers:
				texture_name = sampler["texture_name"]
				if texture_name != "" and texture_name not in texture_order:
					texture_order.append(texture_name)

		# Generate textures
		file.textures = []
		for texture_name in texture_order:
			blender_image = bpy.data.images[texture_name]
			texture = DmdTexture.from_blender_image(blender_image)
			file.textures.append(texture)

		# Identify animations
		def get_all_children_nodes(joint):
			# Post-order DFS
			joint_list = []
			joint_stack = [(joint, 0)]

			while True:
				if len(joint_stack) == 0:
					break

				edge_node, next_child_index = joint_stack[-1]
				
				# Ascend
				if next_child_index == len(edge_node.children):
					del joint_stack[-1]
					continue

				# Descend
				next_child = edge_node.children[next_child_index]
				joint_list.append(next_child)
				joint_stack[-1] = (edge_node, next_child_index + 1)
				joint_stack.append(
					(next_child, 0)
				)

			return joint_list

		potential_animation_targets = []
		potential_animation_targets += get_all_children_nodes(file.map_joint)
		potential_animation_targets += get_all_children_nodes(file.hit_joint)
		potential_animation_targets += file.materials
		# todo-blender_io_ttyd: Light animations

		file.animations = DmdAnimation.list_from_blender_nla_tracks_for_targets(
			potential_animation_targets,
			global_matrix
		)

		return file

	def serialize(self):
		linker = DmdLinker()

		vcd_table = DmdVcdTable()
		root_joint_blob_name = self.root_joint.link(linker, vcd_table)
		vcd_table.link(linker)

		# Helper function to write a table containing references to other elements
		def link_reference_table(linker, blob_name, section_name, references):
			table_data = bytearray(0x4 + len(references) * 0x4)
			struct.pack_into(">L", table_data, 0x0, len(references)) # Element count
			for i, reference in enumerate(references):
				linker.add_relocation(blob_name, 0x4 + i * 0x4, reference)
			linker.add_blob(blob_name, table_data)
			linker.place_blob_in_section(blob_name, section_name)

		# Animation table
		animation_blob_names = [a.link(linker) for a in self.animations]
		link_reference_table(linker, "animation_table", "animation_table", animation_blob_names)

		# Curve table (legacy, always empty)
		link_reference_table(linker, "curve_table", "curve_table", [])

		# Fog table
		# todo-blender_io_ttyd: Expose fog settings to user
		fog_table_blob_name = "fog_table"
		fog_table_data = bytearray(0x14)
		struct.pack_into(">L", fog_table_data, 0x00, 0) # Fog enabled
		struct.pack_into(">L", fog_table_data, 0x04, 0) # Fog mode
		struct.pack_into(">f", fog_table_data, 0x08, 0) # Fog start
		struct.pack_into(">f", fog_table_data, 0x0c, 1000) # Fog end
		struct.pack_into(">L", fog_table_data, 0x10, 0x000000FF) # Fog color
		linker.add_blob(fog_table_blob_name, fog_table_data)
		linker.place_blob_in_section(fog_table_blob_name, "fog_table")

		# Light table
		# todo-blender_io_ttyd: Light support
		link_reference_table(linker, "light_table", "light_table", [])

		# Material name table
		material_name_table_blob_name = "material_name_table"
		material_name_table_data = bytearray(4 + len(self.materials) * 8)
		struct.pack_into(">L", material_name_table_data, 0x0, len(self.materials))
		for i, material in enumerate(self.materials):
			linker.add_string(
				material_name_table_blob_name,
				0x4 + i * 8,
				material.name
			)
			material_blob_name = material.link(linker)
			linker.add_relocation(
				material_name_table_blob_name,
				0x8 + i * 8,
				material_blob_name
			)
		linker.add_blob(material_name_table_blob_name, material_name_table_data)
		linker.place_blob_in_section(material_name_table_blob_name, "material_name_table")

		# Texture table
		texture_table_blob_name = "texture_table"
		texture_table_data = bytearray(0x4 + len(self.textures) * 0x4)
		struct.pack_into(">L", texture_table_data, 0x0, len(self.textures)) # Texture count
		for i, texture in enumerate(self.textures):
			linker.add_string(texture_table_blob_name, 0x4 + i * 0x4, texture.name)
		linker.add_blob(texture_table_blob_name, texture_table_data)
		linker.place_blob_in_section(texture_table_blob_name, "texture_table")

		# Serialize referenced textures
		for texture in self.textures:
			texture.link(linker)

		# Information table
		information_table_blob_name = "information"
		information_table_data = bytearray(0x14)

		linker.add_string(information_table_blob_name, 0x00, "ver1.02") # Version string
		linker.add_relocation(information_table_blob_name, 0x04, root_joint_blob_name) # World root
		linker.add_string(information_table_blob_name, 0x08, self.map_joint.name)
		linker.add_string(information_table_blob_name, 0x0c, self.hit_joint.name)
		date_text = datetime.datetime.utcnow().strftime("%y/%m/%d %H:%M:%S")
		linker.add_string(information_table_blob_name, 0x10, date_text)

		linker.add_blob(information_table_blob_name, information_table_data)
		linker.place_blob_in_section(information_table_blob_name, "information")

		# Place sections and finalize linked data.
		linker.place_section("information")
		linker.place_section("texture_data")
		linker.place_section("sampler_data")
		linker.place_section("vertex_attribute_data", 32) # Align by 32 for vertex cache efficiency
		linker.place_section("materials")
		linker.place_section("meshs", 32) # Align by 32 since this contains display lists
		linker.place_section("joints")
		linker.place_section("vcd_table")
		linker.place_section("material_name_table")
		linker.place_section("light_table")
		linker.place_section("fog_table")
		linker.place_section("texture_table")
		linker.place_section("curve_table")
		linker.place_section("animation_table")
		linker.place_section("animations")
		linker.place_section("animation_data")
		linker.place_section("drawmodes")
		linker.place_section("tev_configs")
		linker.place_section("strings")

		# Generate final data
		assert(linker.resolve_relocations())
		linked_data = linker.serialize()
		print(linker.dump_map())

		# Pad out to multiple of 32 bytes
		linked_data += (align_up(len(linked_data), 32) - len(linked_data)) * b"\x00"

		# Build table infos
		# These appear alphabetically sorted, presumably this was done
		# dynamically by the original exporter. This data is not created using
		# the offset table, but instead hardcoded offsets into the file
		table_order = [
			"animation_table",
			"curve_table",
			"fog_table",
			"information",
			"light_table",
			"material_name_table",
			"texture_table",
			"vcd_table"
		]

		table_info_data = bytearray(len(table_order) * 8)
		table_name_data = bytearray()
		for i, table_name in enumerate(table_order):
			struct.pack_into(
				">L",
				table_info_data,
				0x0 + i * 8,
				linker.get_blob_address(table_name)
			)
			struct.pack_into(">L", table_info_data, 0x4 + i * 8, len(table_name_data)) # Name offset
			encoded_table_name = table_name.encode("shift_jis") + b"\x00"
			table_name_data += encoded_table_name

		# Build offset table
		offsets = []
		for source_name, source_offset, target_name in linker.resolved_relocations:
			offset = linker.get_blob_address(source_name) + source_offset
			offsets.append(offset)

		# Offsets appears sorted in ascending order in table
		offsets.sort()

		# Build final data
		offset_table_data = bytearray(len(offsets) * 4)
		for i, offset in enumerate(offsets):
			struct.pack_into(">L", offset_table_data, i * 4, offset)

		# Build file header
		header_data = bytearray(0x20)
		struct.pack_into(">L", header_data, 0x4, len(linked_data))
		struct.pack_into(">L", header_data, 0x8, len(offsets))
		struct.pack_into(">L", header_data, 0xc, len(table_order))

		# Assemble final file
		# This order is important as only the location of the offset table is
		# encoded in the header and the game assumes the table infos follow.
		final_data = bytearray()
		final_data += header_data
		final_data += linked_data
		final_data += offset_table_data
		final_data += table_info_data
		final_data += table_name_data

		# Add final file size
		struct.pack_into(">L", final_data, 0x0, len(final_data))

		return final_data