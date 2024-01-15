# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright 2024 Linus S. (aka PistonMiner)

import numpy as np

import math
import struct

from .util import *

def color_to_rgb565(color):
	red_data = float_to_quantized(float(color[0]), 5)
	green_data = float_to_quantized(float(color[1]), 6)
	blue_data = float_to_quantized(float(color[2]), 5)
	packed_data = red_data << 11 | green_data << 5 | blue_data
	return packed_data

def dxt1_compress_block_range(block):
	# PCA Range-Fit DXT1 encoder
	# https://www.sjbrown.co.uk/posts/dxt-compression-techniques/
	# https://developer.download.nvidia.com/compute/cuda/1.1-Beta/x86_website/projects/dxtc/doc/cuda_dxtc.pdf
	block_size = 16
	assert(len(block) == block_size)

	# Find relevant pixels
	opaque_pixel_indices = np.array([i for i in range(block_size) if block[i] != None and block[i][3] >= 0.5])
	has_alpha = any([p != None and p[3] < 0.5 for p in block])

	opaque_pixel_count = len(opaque_pixel_indices)
	opaque_pixels = np.array([block[i][:3] for i in opaque_pixel_indices])

	# Empty block?
	if opaque_pixel_count == 0:
		# Return all-transparent, black block
		return b"\x00\x00\x00\x00\xff\xff\xff\xff"

	# Find principal axis
	center = np.mean(opaque_pixels, axis=0)
	centered_pixels = opaque_pixels - center
	if opaque_pixel_count != 1:
		cov_matrix = np.dot(centered_pixels.T, centered_pixels)
	else:
		# Pick any axis
		cov_matrix = np.identity(3)

	cov_eig_vals, cov_eig_vecs = np.linalg.eigh(cov_matrix) # cov is symmetric so we can use eigh
	principal_axis = cov_eig_vecs[:,-1] # eigh returns eigenvectors in ascending order

	# Project pixels onto principal axis
	opaque_pixel_projs = np.dot(centered_pixels, principal_axis)

	# Sort pixels along principal axis
	opaque_pixel_order = np.argsort(opaque_pixel_projs)

	# Find endpoints
	ep1_proj = opaque_pixel_projs[opaque_pixel_order[0]]
	ep2_proj = opaque_pixel_projs[opaque_pixel_order[-1]]

	# Construct palette
	if has_alpha:
		palette_projs = [
			ep1_proj,
			ep2_proj,
			ep1_proj * 1/2 + ep2_proj * 1/2,
		]
	else:
		palette_projs = [
			ep1_proj,
			ep2_proj,
			# GameCube-accurate blend values
			ep1_proj * 5/8 + ep2_proj * 3/8,
			ep1_proj * 3/8 + ep2_proj * 5/8,
		]

	# Assign pixels to palette
	opaque_pixel_pal_indices = np.argmin(
		np.abs(opaque_pixel_projs[:, np.newaxis] - palette_projs),
		axis=1
	)

	# Build final indices
	pal_indices = np.full(block_size, 3, dtype=np.uint8)
	pal_indices[opaque_pixel_indices] = opaque_pixel_pal_indices

	# Compute endpoint colors
	ep1_color = ep1_proj * principal_axis + center
	ep2_color = ep2_proj * principal_axis + center

	# Clamp endpoints inside sRGB-cube
	# Note: This ought to happen before endpoint assignment, but that would
	# complicate everything and this is a rare enough case.
	# PERF: np.clip is slow on small arrays currently, so use min()
	# https://github.com/numpy/numpy/issues/14281
	#np.clip(ep1_color, 0.0, 1.0, out=ep1_color)
	#np.clip(ep2_color, 0.0, 1.0, out=ep2_color)
	np.minimum(ep1_color, 1.0, out=ep1_color)
	np.minimum(ep2_color, 1.0, out=ep2_color)
	np.maximum(ep1_color, 0.0, out=ep1_color)
	np.maximum(ep2_color, 0.0, out=ep2_color)

	# Compute final block
	block_data = bytearray(8)
	ep1_color_packed = color_to_rgb565(ep1_color)
	ep2_color_packed = color_to_rgb565(ep2_color)

	swap_indices = False
	if not has_alpha and ep1_color_packed == ep2_color_packed:
		# Handle equal endpoints (flat surface) on opaque palette by switching
		# to transparent a
		pal_indices.fill(0)
	elif has_alpha != (ep1_color_packed <= ep2_color_packed):
		# Swap indices if necessary
		ep1_color_packed, ep2_color_packed = ep2_color_packed, ep1_color_packed
		swap_indices = True

	struct.pack_into(">HH", block_data, 0x0, ep1_color_packed, ep2_color_packed)
	
	packed_indices = 0
	for i, palette_index in enumerate(pal_indices):
		packed_indices |= (int(palette_index) << ((15 - i) * 2))

	# Swap if necessary
	if swap_indices:
		if has_alpha:
			packed_indices ^= (~packed_indices >> 1) & 0x55555555
		else:
			packed_indices ^= 0x55555555
	struct.pack_into(">L", block_data, 0x4, packed_indices)

	return block_data

def dxt1_compress_block(block):
	return dxt1_compress_block_range(block)