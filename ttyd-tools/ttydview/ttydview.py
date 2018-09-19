"""
File: ttydview.py
Author: Brett B.
Revision: 1.0.1
Purpose: Create .obj file from map collision data for viewing
"""

import os
import sys
import math
import numpy
import struct

inputPath = os.path.join(os.path.dirname(__file__), "map_data") + "/"
inputFile = inputPath + sys.argv[1]
outputPath = os.path.join(os.path.dirname(__file__), "obj_files") + "/"
outputFile = outputPath + input("Enter the name of the output file: ") + ".obj"

f = open(inputFile, "rb")
binaryData = f.read()
f.close()

class Vector3f(object):
	def __init__(self, address, x = 0.0, y = 0.0, z = 0.0):
		if address == None:
			self.x = float(x)
			self.y = float(y)
			self.z = float(z)
		else:
			self.x = struct.unpack_from(">f", binaryData, address)[0]
			self.y = struct.unpack_from(">f", binaryData, address + 0x4)[0]
			self.z = struct.unpack_from(">f", binaryData, address + 0x8)[0]

class Box(object):
	def __init__(self, maximum, minimum):
		self.max = maximum
		self.min = minimum

class Mesh(object):
	def __init__(self, parentClass, address):
		self.parentClass    = parentClass
		self.unk_00         = struct.unpack_from(">L", binaryData, address)[0]
		self.polygonCount   = struct.unpack_from(">L", binaryData, address + 0x4)[0]
		self.elementMask    = struct.unpack_from(">L", binaryData, address + 0x8)[0]
		self.vcdTableOffset = struct.unpack_from(">L", binaryData, address + 0xC)[0]
		self.polygonInfo    = []

		for i in range(self.polygonCount):
			self.polygonInfo.append(PolygonInfo(self, address + 0x10 + (0x8*i)))

class MeshDescriptor(object):
	def __init__(self, address):
		self.materialOffset = struct.unpack_from(">L", binaryData, address)[0]
		self.meshOffset     = struct.unpack_from(">L", binaryData, address + 0x4)[0]

class PolygonInfo(object):
	def __init__(self, parentClass, address):
		self.parentClass = parentClass
		self.offset      = struct.unpack_from(">L", binaryData, address)[0]
		self.size        = struct.unpack_from(">L", binaryData, address + 0x4)[0]
		self.data        = Polygon(self, 0x20 + self.offset)

class Polygon(object):
	def __init__(self, parentClass, address):
		self.parentClass = parentClass
		self.unk_00      = struct.unpack_from(">H", binaryData, address)[0]
		self.vertexCount = struct.unpack_from(">B", binaryData, address + 0x2)[0]
		self.vertices    = []

		vertexSize = 0x2*bin(parentClass.parentClass.elementMask).count("1")

		for i in range(self.vertexCount):
			self.vertices.append(Vertex(self, address + 0x3 + (vertexSize*i)))

class Vertex(object):
	def __init__(self, parentClass, address):
		self.parentClass = parentClass

		if (parentClass.parentClass.parentClass.elementMask & 0x001):
			self.positionIndex = struct.unpack_from(">H", binaryData, address)[0]
		
		if (parentClass.parentClass.parentClass.elementMask & 0x002):
			self.normalIndex   = struct.unpack_from(">H", binaryData, address + 0x2)[0]
		
		if (parentClass.parentClass.parentClass.elementMask & 0x004):
			self.colorIndex0   = struct.unpack_from(">H", binaryData, address + 0x4)[0]
		
		if (parentClass.parentClass.parentClass.elementMask & 0x008):
			self.colorIndex1   = struct.unpack_from(">H", binaryData, address + 0x6)[0]
		
		if (parentClass.parentClass.parentClass.elementMask & 0x010):
			self.textureCoordinateIndex0 = struct.unpack_from(">H", binaryData, address + 0x8)[0]
		
		if (parentClass.parentClass.parentClass.elementMask & 0x020):
			self.textureCoordinateIndex1 = struct.unpack_from(">H", binaryData, address + 0xA)[0]
		
		if (parentClass.parentClass.parentClass.elementMask & 0x040):
			self.textureCoordinateIndex2 = struct.unpack_from(">H", binaryData, address + 0xC)[0]
		
		if (parentClass.parentClass.parentClass.elementMask & 0x080):
			self.textureCoordinateIndex3 = struct.unpack_from(">H", binaryData, address + 0xE)[0]
		
		if (parentClass.parentClass.parentClass.elementMask & 0x100):
			self.textureCoordinateIndex4 = struct.unpack_from(">H", binaryData, address + 0x10)[0]
		
		if (parentClass.parentClass.parentClass.elementMask & 0x200):
			self.textureCoordinateIndex5 = struct.unpack_from(">H", binaryData, address + 0x12)[0]
		
		if (parentClass.parentClass.parentClass.elementMask & 0x400):
			self.textureCoordinateIndex6 = struct.unpack_from(">H", binaryData, address + 0x14)[0]
		
		if (parentClass.parentClass.parentClass.elementMask & 0x800):
			self.textureCoordinateIndex7 = struct.unpack_from(">H", binaryData, address + 0x16)[0]

class Node(object):
	def __init__(self, address):
		self.nameOffset   = struct.unpack_from(">L", binaryData, address)[0]
		self.name         = struct.unpack_from(">{}s".format(getStringLength(0x20 + self.nameOffset)), binaryData, 0x20 + self.nameOffset)[0].decode("utf-8")
		self.typeOffset   = struct.unpack_from(">L", binaryData, address + 0x4)[0]
		self.type         = struct.unpack_from(">{}s".format(getStringLength(0x20 + self.typeOffset)), binaryData, 0x20 + self.typeOffset)[0].decode("utf-8")
		self.parentOffset = struct.unpack_from(">L", binaryData, address + 0x8)[0]
		self.childOffset  = struct.unpack_from(">L", binaryData, address + 0xC)[0]
		self.nextOffset   = struct.unpack_from(">L", binaryData, address + 0x10)[0]
		self.prevOffset   = struct.unpack_from(">L", binaryData, address + 0x14)[0]
		self.scale        = Vector3f(address + 0x18)
		self.rotation     = Vector3f(address + 0x24)
		self.translation  = Vector3f(address + 0x30)
		self.bbox         = Box(Vector3f(address + 0x3C), Vector3f(address + 0x48))
		self.unk_54       = struct.unpack_from(">L", binaryData, address + 0x54)[0]
		self.unkOffset    = struct.unpack_from(">L", binaryData, address + 0x58)[0]
		self.meshCount    = struct.unpack_from(">L", binaryData, address + 0x5C)[0]
		self.parentNode   = None
		self.children     = []
		
		self.meshDescriptors = []
		self.meshes = []

		if self.meshCount > 0:
			for i in range(self.meshCount):
				self.meshDescriptors.append(MeshDescriptor(address + 0x60 + (0x8*i)))

			for j in range(self.meshCount):
				self.meshes.append(Mesh(self, 0x20 + self.meshDescriptors[j].meshOffset))

	def add_child(self, address):
		childNode = Node(address)
		self.children.append(childNode)
		childNode.parentNode = self

class VCDTable(object):
	def __init__(self, address):
		self.positionOffset = struct.unpack_from(">L", binaryData, address)[0]
		self.normalOffset   = struct.unpack_from(">L", binaryData, address + 0x4)[0]
		self.unk08          = struct.unpack_from(">L", binaryData, address + 0x8)[0]
		self.colorOffset0   = struct.unpack_from(">L", binaryData, address + 0xC)[0]
		self.colorOffset1   = struct.unpack_from(">L", binaryData, address + 0x10)[0]
		self.unk_14         = struct.unpack_from(">L", binaryData, address + 0x14)[0]
		self.textureCoordinateOffsets  = []
		self.unk_38         = struct.unpack_from(">L", binaryData, address + 0x38)[0]
		self.unk_3C         = struct.unpack_from(">L", binaryData, address + 0x3C)[0]
		self.unk_40         = struct.unpack_from(">L", binaryData, address + 0x40)[0]
		self.positionQuantizationShift = struct.unpack_from(">L", binaryData, address + 0x44)[0]
		self.textureCoordinateQuantizationShifts = []

		for i in range(8):
			self.textureCoordinateOffsets.append(struct.unpack_from(">L", binaryData, address + 0x18 + (0x4*i))[0])
			self.textureCoordinateQuantizationShifts.append(struct.unpack_from(">L", binaryData, address + 0x44 + (0x4*i))[0])

		positionCount = struct.unpack_from(">L", binaryData, 0x20 + self.positionOffset)[0]
		self.positions = []

		for i in range(positionCount):
			x = rawToFloat(0x24 + self.positionOffset + (0x6*i), self.positionQuantizationShift)
			y = rawToFloat(0x26 + self.positionOffset + (0x6*i), self.positionQuantizationShift)
			z = rawToFloat(0x28 + self.positionOffset + (0x6*i), self.positionQuantizationShift)
			self.positions.append(Vector3f(None, x, y, z))

def rawToFloat(address, scale):
	num = struct.unpack_from(">h", binaryData, address)[0]
	return '{:.6f}'.format(num / (2**scale))

def getStringLength(address):
	charCount = 0
	curChar = struct.unpack_from(">b", binaryData, address)[0]
	while curChar: 
		charCount += 1
		curChar = struct.unpack_from(">b", binaryData, address + charCount)[0]
	return charCount
	
def fillScene(curNode):
	if curNode.nextOffset:
		curNode.parentNode.add_child(0x20 + curNode.nextOffset)
		fillScene(curNode.parentNode.children[-1])
	if curNode.childOffset:
		curNode.add_child(0x20 + curNode.childOffset)
		fillScene(curNode.children[-1])

sceneGraphRootOffset = struct.unpack_from(">L", binaryData, 0x24)[0]
curAddress = 0x20 + sceneGraphRootOffset

rootNode = Node(curAddress)
fillScene(rootNode)


def getTransformationMatrix(curNode):
	radX = math.radians(curNode.rotation.x)
	radY = math.radians(curNode.rotation.y)
	radZ = math.radians(curNode.rotation.z)
	
	scaleMatrix  = numpy.array([[curNode.scale.x, 0, 0, 0], [0, curNode.scale.y, 0, 0], [0, 0, curNode.scale.z, 0], [0, 0, 0, 1]])
	rotationMatX = numpy.array([[1, 0, 0, 0], [0, math.cos(radX), -math.sin(radX), 0], [0, math.sin(radX), math.cos(radX), 0], [0, 0, 0, 1]])
	rotationMatY = numpy.array([[math.cos(radY), 0, math.sin(radY), 0], [0, 1, 0, 0], [-math.sin(radY), 0, math.cos(radY), 0], [0, 0, 0, 1]])
	rotationMatZ = numpy.array([[math.cos(radZ), -math.sin(radZ), 0, 0], [math.sin(radZ), math.cos(radZ), 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]])
	translationMatrix = numpy.array([[1, 0, 0, curNode.translation.x], [0, 1, 0, curNode.translation.y], [0, 0, 1, curNode.translation.z], [0, 0, 0, 1]])

	return translationMatrix @ (rotationMatZ @ rotationMatY @ rotationMatX) @ scaleMatrix

def vectorToNumpy(vector):
	return numpy.array([(vector.x), (vector.y), (vector.z), 1])

def DFS(curNode):
	global vcdTable
	matrixStack.append(matrixStack[-1] @ getTransformationMatrix(curNode))
	for node in curNode.children:
		DFS(node)
	if curNode.meshCount:
		writeGroup(curNode)
	for mesh in curNode.meshes:
		if not vertexCount:
			vcdTable = VCDTable(0x20 + mesh.vcdTableOffset)
		for polyInfo in mesh.polygonInfo:
			for vertex in polyInfo.data.vertices:
				absolutePos = matrixStack[-1] @ vectorToNumpy(vcdTable.positions[vertex.positionIndex])
				writeVertex(absolutePos)
			writeFace(len(polyInfo.data.vertices))
	matrixStack.pop()

def writeVertex(vertex):
	global vertexCount
	vertexCount += 1
	out.write("v " + str('{:.6f}'.format(vertex[0])) +  " " + str('{:.6f}'.format(vertex[1])) + " " + str('{:.6f}'.format(vertex[2])) + "\n")

def writeFace(vCount):
	global curFaceStart
	for v in range(curFaceStart, curFaceStart + vCount-2, 1):
		out.write("f " + str(v+1) + " " + str(v+2) + " " + str(v+3) + "\n")
	curFaceStart = vertexCount

def writeGroup(node):
	out.write("g " + node.name + "\n")

vcdTable = None

matrixStack = []
identityMatrix = numpy.array([[1, 0, 0, 0], [0, 1, 0, 0], [0, 0, 1, 0], [0, 0, 0, 1]])
matrixStack.append(identityMatrix)

curFaceStart = 0
vertexCount = 0

out = open(outputFile, "w")
DFS(rootNode)
out.close()