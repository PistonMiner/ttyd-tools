# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright 2019 Linus S. (aka PistonMiner)

from collections import OrderedDict
import struct

def align_up(value, alignment):
	return (value + alignment - 1) & ~(alignment - 1)

def get_bbox(points):
	if len(points) == 0:
		return None

	dimensions = len(points[0])
	bbox_min = points[0]
	bbox_max = points[0]
	for point in points:
		bbox_min = [min(bbox_min[i], point[i]) for i in range(dimensions)]
		bbox_max = [max(bbox_max[i], point[i]) for i in range(dimensions)]

	return (bbox_min, bbox_max)

def merge_bboxes(bboxes):
	merged_bbox = None
	for bbox in bboxes:
		if merged_bbox == None:
			merged_bbox = (list(bbox[0]), list(bbox[1]))
			continue
		for i in range(3):
			merged_bbox[0][i] = min(merged_bbox[0][i], bbox[0][i])
			merged_bbox[1][i] = max(merged_bbox[1][i], bbox[1][i])
	return merged_bbox

class Linker:
	def __init__(self):
		self.blob_data = {}
		self.blob_addresses = {}

		self.section_addresses = {}
		self.section_blobs = {}

		self.resolved_relocations = []
		self.unresolved_relocations = []

		self.current_uid = 0

	def place_blob_in_section(self, blob_name, section_name):
		if not section_name in self.section_blobs:
			self.section_blobs[section_name] = []
		self.section_blobs[section_name].append(blob_name)

	def get_blob_address(self, blob_name):
		return self.blob_addresses[blob_name] if blob_name in self.blob_addresses else None

	def get_section_blob_count(self, section_name):
		if section_name in self.section_blobs:
			return len(self.section_blobs[section_name])
		else:
			return 0

	def place_section(self, section_name, align = 1):
		if not section_name in self.section_blobs:
			return
		if not len(self.section_blobs[section_name]):
			return

		next_free = 0
		for blob_name in self.blob_addresses:
			blob_size = len(self.blob_data[blob_name])
			blob_end = self.blob_addresses[blob_name] + blob_size
			next_free = max(next_free, blob_end)

		next_free = align_up(next_free, align)
		self.place_section_at(section_name, next_free)

	def place_section_at(self, section_name, address):
		current_address = address
		for blob_name in self.section_blobs[section_name]:
			self.blob_addresses[blob_name] = current_address
			current_address += len(self.blob_data[blob_name])

	def get_section_address(self, section_name):
		if section_name not in self.section_blobs:
			return 0

		if len(self.section_blobs[section_name]) < 1:
			return 0

		first_blob = self.section_blobs[section_name][0]
		assert(first_blob in self.blob_addresses)
		return self.blob_addresses[first_blob]

	def blob_exists(self, blob_name):
		return blob_name in self.blob_data

	def add_blob(self, name, data):
		# Catch name collisions here
		if name in self.blob_data:
			print("Name collision on blob {}!".format(name))
			assert(False)
		self.blob_data[name] = data

	def add_relocation(self, source_name, source_offset, target_name):
		relocation = (source_name, source_offset, target_name)
		self.unresolved_relocations.append(relocation)

	def get_uid(self):
		self.current_uid += 1
		return self.current_uid

	def resolve_relocations(self):
		while True:
			resolved_any = False
			i = 0
			while i < len(self.unresolved_relocations):
				source_name, source_offset, target_name = self.unresolved_relocations[i]
				if target_name in self.blob_addresses:
					target_address = self.blob_addresses[target_name]
					struct.pack_into(">L", self.blob_data[source_name], source_offset, target_address)
					self.resolved_relocations.append(self.unresolved_relocations.pop(i))
					resolved_any = True
				else:
					i += 1
			if not resolved_any:
				break
		
		# Debug logging
		if len(self.unresolved_relocations) != 0:
			for source_name, source_offset, target_name in self.unresolved_relocations:
				print("Unresolved relocation from {}+0x{:x} to {}".format(source_name, source_offset, target_name))

		return len(self.unresolved_relocations) == 0

	def dump_map(self):
		entries = []
		for blob_name in self.blob_addresses:
			address = self.blob_addresses[blob_name]
			data = self.blob_data[blob_name]
			entries.append((address, blob_name, len(data)))
		
		entries.sort(key=lambda x: x[0])

		map_text = "Linker map:\n"
		for address, name, size in entries:
			map_text += "{:8x}: {} (size {})\n".format(address, name, size)
		return map_text

	def serialize(self):
		blocks_to_write = []
		for blob_name in self.blob_addresses:
			address = self.blob_addresses[blob_name]
			data = self.blob_data[blob_name]
			blocks_to_write.append((address, data))
		blocks_to_write.sort(key=lambda x: x[0])

		last_block = blocks_to_write[-1]
		end_address = last_block[0] + len(last_block[1])

		serialized_data = bytearray(end_address)
		for address, data in blocks_to_write:
			serialized_data[address:address + len(data)] = data

		return serialized_data