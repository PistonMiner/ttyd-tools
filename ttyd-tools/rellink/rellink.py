import sys
import os
import struct
import ctypes

# read the data in.
filename_in = sys.argv[1]
input = open(filename_in, "rb")
file_data = ctypes.create_string_buffer(input.read())
input.close()

baseAddress = int(sys.argv[2], 16) #0x805ba9a0 #0x804FFF3C #

curOffset = 0x0
relID = struct.unpack(">L", file_data[curOffset:curOffset+0x4])[0]
curOffset = 0xC
section_count = struct.unpack(">L", file_data[curOffset:curOffset+0x4])[0]
curOffset = 0x10
section_info_offset = struct.unpack(">L", file_data[curOffset:curOffset+0x4])[0]
curOffset = 0x28
import_offset = struct.unpack(">L", file_data[curOffset:curOffset+0x4])[0]
curOffset = 0x2C
import_size = struct.unpack(">L", file_data[curOffset:curOffset+0x4])[0]

# set imports and relocations to invalid since we are doing them
struct.pack_into(">L", file_data, 0x24, 0) # reloc offset
struct.pack_into(">L", file_data, 0x28, 0) # import offset
struct.pack_into(">L", file_data, 0x2C, 0) # import count

sections = [] # [list of [offset, size]]
imports = [] # [list of [id, offset]]

for i in xrange(section_count):
	curOffset = section_info_offset + 8 * i

	offset = (struct.unpack(">L", file_data[curOffset:curOffset+0x4])[0]) & ~1 # remove bit 0 (exec bit)
	size = struct.unpack(">L", file_data[curOffset+0x4:curOffset+0x8])[0]

	sections.append([offset, size])

print str(section_count) + " sections"

print sections

for i in xrange(import_size / 8):
	curOffset = import_offset + 8 * i
	
	id = struct.unpack(">L", file_data[curOffset:curOffset+0x4])[0]
	offset = struct.unpack(">L", file_data[curOffset+0x4:curOffset+0x8])[0]
	
	imports.append([id, offset])

print str(import_size / 8) + " import lists"

print imports


for import_entry in imports:
	curOffset = import_entry[1] # offset
	if 1: #relID == import_entry[0] # self-relocations / or others apparently cause they only do it for fancy import entries.
		curRelOffset = 0
		curRelSection = 0
		while 1:
			curRelOffset	+= struct.unpack(">H", file_data[curOffset:curOffset+0x2])[0] # add offset
			operation		= struct.unpack(">B", file_data[curOffset+0x2:curOffset+0x3])[0]
			targetSection	= struct.unpack(">B", file_data[curOffset+0x3:curOffset+0x4])[0]
			addend			= struct.unpack(">L", file_data[curOffset+0x4:curOffset+0x8])[0]
			curOffset += 8
			
			print "Processing import entry: " + format(curRelOffset, "x") + " / " + format(operation, "x") + " / " + format(targetSection, "x") + " / " + format(addend, "x")
			
			effectiveOffset = sections[curRelSection][0] + curRelOffset
			if relID == import_entry[0]:
				targetAddress = sections[targetSection][0] + addend + baseAddress
			else:
				targetAddress = addend
			
			print format(effectiveOffset, "x") + " / " + format(targetAddress, "x")
			
			#if operation == 0 or operation == 201: # R_PPC_NONE || R_DOLPHIN_NOP
			#	dummy = 0
			if operation == 202:	# R_DOLPHIN_SECTION
				curRelSection = targetSection
				curRelOffset = 0
			elif operation == 1:	# R_PPC_ADDR32
				struct.pack_into(">L", file_data, effectiveOffset, targetAddress)
			elif operation == 4:	# R_PPC_ADDR16_LO
				struct.pack_into(">H", file_data, effectiveOffset, targetAddress & 0xFFFF)
			elif operation == 6:	# R_PPC_ADDR16_HA
				if (targetAddress & 0x8000) == 0x8000:
					targetAddress += 0x00010000
				
				struct.pack_into(">H", file_data, effectiveOffset, (targetAddress >> 16) & 0xFFFF)
			elif operation == 10:	# R_PPC_REL24
				value = addend
				value -= (effectiveOffset + baseAddress) 
				orig = struct.unpack(">L", file_data[effectiveOffset:effectiveOffset+0x4])[0]
				orig &= 0xFC000003
				orig |= value & 0x03FFFFFC
				struct.pack_into(">L", file_data, effectiveOffset, orig)
			elif operation == 203:	# R_DOLPHIN_END
				break
			else:
				print "Unknown relocation operation " + format(opcode, "x")

output_data = str(bytearray(file_data))

filename_out = filename_in + ".linked"
output = open(filename_out , "wb")
output.write(output_data)
output.close()