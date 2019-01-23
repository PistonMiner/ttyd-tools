import sys
import os
import struct
import ctypes
import math
from datetime import datetime

inputFilename = sys.argv[1]
inputFile = open(inputFilename, "rb")
inputBuffer = ctypes.create_string_buffer(inputFile.read())[:-1]
inputFile.close()

# Load banner and icon
bannerFile = open(sys.argv[5], "rb")
bannerBuffer = ctypes.create_string_buffer(bannerFile.read())[:-1]
if len(bannerBuffer) != 0x1800:
	print("Warning: banner size mismatch (should be 96x32 RGB5A3)")
bannerFile.close()

iconFile = open(sys.argv[6], "rb")
iconBuffer = ctypes.create_string_buffer(iconFile.read())[:-1]
if len(iconBuffer) != 0x800:
	print("Warning: icon size mismatch %d (should be 32x32 RGB5A3)" % len(iconBuffer))
iconFile.close()

# Comment
commentBuffer = ctypes.create_string_buffer(0x40)
struct.pack_into("32s", commentBuffer, 0x00, sys.argv[3].encode())
struct.pack_into("32s", commentBuffer, 0x20, sys.argv[4].encode())

# File info
fileInfoBuffer = ctypes.create_string_buffer(0x200 - 0x40)
struct.pack_into(">L", fileInfoBuffer, 0, len(inputBuffer))

# Pad to block boundary
fileLength = len(bannerBuffer) + len(iconBuffer) + len(commentBuffer) + len(fileInfoBuffer) + len(inputBuffer)
blockCount = math.ceil(fileLength / 0x2000)
paddingLength = blockCount * 0x2000 - fileLength
paddingBuffer = ctypes.create_string_buffer(paddingLength)

# Create header
headerBuffer = ctypes.create_string_buffer(0x40)
struct.pack_into("4s",  headerBuffer,  0x00, sys.argv[7].encode()) # game code
struct.pack_into(">H",  headerBuffer,  0x04, 0x3031)		# maker code
struct.pack_into(">B",  headerBuffer,  0x06, 0xFF)			# unused
struct.pack_into(">B",  headerBuffer,  0x07, 2)				# banner flags (RGB5A3)
struct.pack_into("32s", headerBuffer,  0x08, sys.argv[2].encode())	# filename
struct.pack_into(">L",  headerBuffer,  0x28, int((datetime.utcnow() - datetime(2000, 1, 1)).total_seconds())) # modified time
struct.pack_into(">L",  headerBuffer,  0x2C, 0)				# image offset
struct.pack_into(">H",  headerBuffer,  0x30, 2)				# icon format
struct.pack_into(">H",  headerBuffer,  0x32, 3)				# animation speed (1 icon for 12 frames)
struct.pack_into(">B",  headerBuffer,  0x34, 4)				# permissions
struct.pack_into(">B",  headerBuffer,  0x35, 0)				# copy counter
struct.pack_into(">H",  headerBuffer,  0x36, 0)				# first block number
struct.pack_into(">H",  headerBuffer,  0x38, blockCount)	# block count
struct.pack_into(">H",  headerBuffer,  0x3A, 0xFF)			# unused
struct.pack_into(">L",  headerBuffer,  0x3C, 0x2000)		# comment address

outputFilename = os.path.splitext(inputFilename)[0] + ".gci"
outputFile = open(outputFilename, "wb")
outputFile.write(bytearray(headerBuffer))
outputFile.write(bytearray(bannerBuffer))
outputFile.write(bytearray(iconBuffer))
outputFile.write(bytearray(commentBuffer))
outputFile.write(bytearray(fileInfoBuffer))
outputFile.write(bytearray(inputBuffer))
outputFile.write(bytearray(paddingBuffer))
outputFile.close()