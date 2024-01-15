# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright 2019 Linus S. (aka PistonMiner)

import math

from .dxt1 import *
from .util import *

TPL_FORMAT_IDS = {
	"I4": 0,
	"I8": 1,
	"IA4": 2,
	"IA8": 3,
	"RGB565": 4,
	"RGB5A3": 5,
	"RGBA32": 6,
	"C4": 8,
	"C8": 9,
	"C14X2": 10,
	"CMPR": 14,
}

TPL_FORMAT_BLOCK_SIZES = {
	"I4": (8, 8),
	"I8": (8, 4),
	"IA4": (8, 4),
	"IA8": (4, 4),
	"RGB565": (4, 4),
	"RGB5A3": (4, 4),
	"RGBA32": (4, 4),
	"C4": (8, 8),
	"C8": (8, 4),
	"C14X2": (4, 4),
	"CMPR": (8, 8),
}

TPL_WRAP_MODES = {
	"CLAMP": 0,
	"REPEAT": 1,
	"MIRROR": 2,
}

def get_block(pixels, size, block_size, block_x, block_y):
	# Find block bounds backed with image data
	image_block_start_x = block_x * block_size[0]
	image_block_start_y = block_y * block_size[1]
	image_block_end_x = min(size[0], image_block_start_x + block_size[0])
	image_block_end_y = min(size[1], image_block_start_y + block_size[1])
	block_backed_size_x = image_block_end_x - image_block_start_x
	block_backed_size_y = image_block_end_y - image_block_start_y
	#print("TPL: Block {}/{} - actual size {}/{}".format(block_x, block_y, block_backed_size_x, block_backed_size_y))

	# Extract rows
	buffer_block_start = image_block_start_y * size[0] + image_block_start_x
	block = []
	for block_row_index in range(block_size[1]):
		if block_row_index >= block_backed_size_y:
			block.extend([None] * block_size[0])
			continue
					
		buffer_row_start = buffer_block_start + block_row_index * size[0]
		buffer_row_end = buffer_row_start + block_backed_size_x

		block.extend(pixels[buffer_row_start:buffer_row_end])
		if block_backed_size_x < block_size[0]:
			block.extend([None] * (block_size[0] - block_backed_size_x))

	return block

def downsample_pixels(pixels, size):
	# Box-filter four pixels into one.
	block_size = (2, 2)
	block_count_x = math.ceil(size[0] / block_size[0])
	block_count_y = math.ceil(size[1] / block_size[1])

	downsampled_pixels = []
	for block_y in range(block_count_y):
		for block_x in range(block_count_x):
			block = get_block(pixels, size, block_size, block_x, block_y)
			
			real_pixels = [p for p in block if p != None]
			filtered_pixel = [0.0, 0.0, 0.0, 0.0]
			for pixel in real_pixels:
				filtered_pixel = [pixel[i] / len(real_pixels) + filtered_pixel[i] for i in range(4)]
			downsampled_pixels.append(tuple(filtered_pixel))

	return downsampled_pixels

def encode_pixels(pixels, size, image_format):
	texture_data = bytearray()

	# We process one block at a time for memory reasons.
	block_size = TPL_FORMAT_BLOCK_SIZES[image_format]
	block_count_x = math.ceil(size[0] / block_size[0])
	block_count_y = math.ceil(size[1] / block_size[1])
	#print("TPL: Block count {}/{}".format(block_count_x, block_count_y))
	for block_y in range(block_count_y):
		for block_x in range(block_count_x):
			block = get_block(pixels, size, block_size, block_x, block_y)

			if image_format == "I4":
				block_data = bytearray(len(block) / 2)
				for i in range(len(block) / 2):
					high_pixel = block[i * 2 + 0]
					high_data = float_to_quantized(high_pixel[0], 4) if high_pixel else 0
					low_pixel = block[i * 2 + 1]
					low_data = float_to_quantized(low_pixel[0], 4) if low_pixel else 0
					struct.pack_into(">B", block_data, i * 2, high_data << 4 | low_data)
			elif image_format == "I8":
				block_data = bytearray(len(block))
				for i, pixel in enumerate(block):
					pixel_data = float_to_quantized(pixel[0], 8) if pixel else 0
					struct.pack_into(">B", block_data, i, pixel_data)
			elif image_format == "IA4":
				block_data = bytearray(len(block))
				for i, pixel in enumerate(block):
					if pixel:
						intensity_data = float_to_quantized(pixel[0], 4)
						alpha_data = float_to_quantized(pixel[3], 4)
					else:
						intensity_data = 0
						alpha_data = 0
					struct.pack_into(">B", block_data, i, intensity_data << 4 | alpha_data)
			elif image_format == "IA8":
				block_data = bytearray(len(block) * 2)
				for i, pixel in enumerate(block):
					if pixel:
						intensity_data = float_to_quantized(pixel[0], 8)
						alpha_data = float_to_quantized(pixel[3], 8)
					else:
						intensity_data = 0
						alpha_data = 0
					struct.pack_into(">BB", block_data, i * 2, intensity_data, alpha_data)
			elif image_format == "RGB565":
				block_data = bytearray(len(block) * 2)
				for i, pixel in enumerate(block):
					if pixel:
						red_data = float_to_quantized(pixel[0], 5)
						green_data = float_to_quantized(pixel[1], 6)
						blue_data = float_to_quantized(pixel[2], 5)
					else:
						red_data = 0
						green_data = 0
						blue_data = 0
					final_data = red_data << 11 | green_data << 5 | blue_data
					struct.pack_into(">H", block_data, i * 2, final_data)
			elif image_format == "RGB5A3":
				block_data = bytearray(len(block) * 2)
				for i, pixel in enumerate(block):
					if pixel:
						has_alpha = pixel[3] < (224 / 255)
						if has_alpha:
							alpha_data = float_to_quantized(pixel[3], 3)
							red_data = float_to_quantized(pixel[0], 4)
							green_data = float_to_quantized(pixel[1], 4)
							blue_data = float_to_quantized(pixel[2], 4)

							packed_data = 0
							packed_data |= alpha_data << 12
							packed_data |= red_data << 8
							packed_data |= green_data << 4
							packed_data |= blue_data
						else:
							red_data = float_to_quantized(pixel[0], 5)
							green_data = float_to_quantized(pixel[1], 5)
							blue_data = float_to_quantized(pixel[2], 5)

							packed_data = 1 << 15
							packed_data |= red_data << 10
							packed_data |= green_data << 5
							packed_data |= blue_data
					else:
						packed_data = 0
					struct.pack_into(">H", block_data, i * 2, packed_data)
			elif image_format == "RGBA32":
				low_block_data = bytearray(len(block) * 2)
				high_block_data = bytearray(len(block) * 2)
				for i, pixel in enumerate(block):
					if pixel:
						#print(pixel)
						red_data = float_to_quantized(pixel[0], 8)
						green_data = float_to_quantized(pixel[1], 8)
						blue_data = float_to_quantized(pixel[2], 8)
						alpha_data = float_to_quantized(pixel[3], 8)

						packed_low_data = alpha_data << 8 | red_data
						packed_high_data = green_data << 8 | blue_data
					else:
						packed_low_data = 0
						packed_high_data = 0

					struct.pack_into(">H", low_block_data, i * 2, packed_low_data)
					struct.pack_into(">H", high_block_data, i * 2, packed_high_data)
				block_data = low_block_data + high_block_data
			elif image_format == "C4":
				# todo-blender_io_ttyd: Add support for TPL palette image formats
				assert(False)
			elif image_format == "C8":
				assert(False)
			elif image_format == "C14X2":
				assert(False)
			elif image_format == "CMPR":
				#print("TPL - CMPR: Block {}".format(block))
				block_data = bytearray()
				for subblock_y in range(2):
					for subblock_x in range(2):
						subblock = get_block(block, block_size, (4, 4), subblock_x, subblock_y)
						subblock_data = dxt1_compress_block(subblock)
						block_data += subblock_data
			else:
				assert(False)

			# Write back block
			texture_data += block_data

	return texture_data

class TplTexture:
	def __init__(self):
		self.size = (0, 0)
		self.wrap = 1 # repeat
		self.texture_data = bytearray()
		self.format = ""

	def link(self, linker):
		texture_id = linker.get_uid()
		texture_info_blob_name = "texture_infos:" + str(texture_id)
		texture_info_data = bytearray(0x8)

		texture_header_blob_name = "texture_headers:" + str(texture_id)
		texture_header_data = bytearray(0x24)

		struct.pack_into(">HH", texture_header_data, 0x00, self.size[0], self.size[1])
		struct.pack_into(">L", texture_header_data, 0x04, TPL_FORMAT_IDS[self.format])

		wrap_mode = TPL_WRAP_MODES[self.wrap]
		struct.pack_into(">L", texture_header_data, 0x0c, wrap_mode) # wrap S
		struct.pack_into(">L", texture_header_data, 0x10, wrap_mode) # wrap T

		# Min/Mag filter set to linear, unused for map materials
		struct.pack_into(">L", texture_header_data, 0x14, 1) # min filter
		struct.pack_into(">L", texture_header_data, 0x18, 1) # mag filter

		# Serialize image data
		texture_data_blob_name = "texture_data:" + str(texture_id)

		texture_data = self.texture_data.copy()
		texture_data.extend(b"\x00" * align_up(len(texture_data), 32))

		linker.add_blob(texture_data_blob_name, texture_data)
		linker.place_blob_in_section(texture_data_blob_name, "texture_data")

		linker.add_relocation(
			texture_header_blob_name,
			0x08,
			texture_data_blob_name
		)

		linker.add_relocation(texture_info_blob_name, 0x0, texture_header_blob_name)

		linker.add_blob(texture_header_blob_name, texture_header_data)
		linker.place_blob_in_section(texture_header_blob_name, "texture_headers")

		linker.add_blob(texture_info_blob_name, texture_info_data)
		linker.place_blob_in_section(texture_info_blob_name, "texture_infos")
		return texture_info_blob_name

	@staticmethod
	def from_blender_image(blender_image, blender_extension = 'REPEAT', format = None):
		texture = TplTexture()
		texture.size = tuple(blender_image.size)

		# Convert wrap mode
		if blender_extension == 'REPEAT':
			texture.wrap = "REPEAT"
		elif blender_extension == 'EXTEND':
			texture.wrap = "CLAMP"
		else:
			# clip is not supported
			assert(False)

		# Store image data
		channel_count = blender_image.channels
		raw_pixel_data = tuple(blender_image.pixels)
		pixel_count = len(raw_pixel_data) // channel_count

		# Convert N-channel pixels to fixed RGBA
		pixels = []
		default_pixel_data = (0.0, 0.0, 0.0, 1.0)
		for i in range(pixel_count):
			# Get pixel dat from blender
			pixel_start_offset = i * channel_count
			pixel_end_offset = pixel_start_offset + min(channel_count, len(default_pixel_data))
			backed_pixel_data = blender_image.pixels[pixel_start_offset:pixel_end_offset]

			# Extend pixel data if the image has less than four channels.
			extended_pixel_data = backed_pixel_data
			#for j in range(len(backed_pixel_data), len(default_pixel_data)):
			#	extended_pixel_data.append(default_pixel_data[j])

			#print(extended_pixel_data)
			pixels.append(tuple(extended_pixel_data))

		# Convert texture data to target format now to save memory
		if format == None:
			format = "RGB5A3"
		texture.format = format
		texture.texture_data = encode_pixels(pixels, texture.size, format)
		return texture

class TplFile:
	def __init__(self):
		self.textures = []

	def serialize(self):
		linker = Linker()

		header_blob_name = "header"
		header_data = bytearray(0xc)
		struct.pack_into(">L", header_data, 0x0, 0x0020af30) # magic
		struct.pack_into(">L", header_data, 0x4, len(self.textures)) # texture count

		linker.add_blob(header_blob_name, header_data)
		linker.place_blob_in_section(header_blob_name, "header")

		for i, texture in enumerate(self.textures):
			texture_blob_name = texture.link(linker)
			if i == 0:
				linker.add_relocation(header_blob_name, 0x8, texture_blob_name)

		# Place sections and serialize
		linker.place_section("header")
		linker.place_section("texture_infos")
		linker.place_section("texture_headers")
		linker.place_section("texture_data", 0x20)
		assert(linker.resolve_relocations())
		data = linker.serialize()
		return data