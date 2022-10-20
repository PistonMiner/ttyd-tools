#pragma once

#include <cstdint>

namespace ttyd::dvdmgr {

enum DvdMgrFile_Flags
{
	DvdMgrFile_ReadPending = 0x1,
	DvdMgrFile_ReadDone = 0x2,
	DvdMgrFile_ClosePending = 0x8,

	DvdMgrFile_HasError = 0x20,
	DvdMgrFile_Suspended = 0x80,

	DvdMgrFile_Used = 0x8000,
};

struct DvdMgrFile
{
	char path[64];
	uint8_t fileInfo[0x3c];
	void *readTargetAddr;
	int32_t readSize;
	int32_t readBaseOffset;
	int32_t readBlockOffset;
	void (*pfnReadDoneCb)(int result, void *fileInfo);
	uint16_t flags;
	uint16_t priority;
	uint16_t wOpenArg3;
	uint16_t pad_96;
} __attribute__((__packed__));

static_assert(sizeof(DvdMgrFile) == 0x98);

extern "C" {

// local: proc_main
// DVDMgrInit
// DVDMgrDelete
// local: compare
// DVDMgrMain

DvdMgrFile *DVDMgrOpen(const char *path, int priority, uint16_t wZero);
uint32_t DVDMgrRead(DvdMgrFile *file, void *dst, int size, int offset);
void DVDMgrReadAsync(DvdMgrFile *file, void *dst, int size, int offset, void (*pfnReadDoneCb)(int result, void *fileInfo));
void DVDMgrClose(DvdMgrFile *file);
uint32_t DVDMgrGetLength(DvdMgrFile *file);

void DVDMgrSetupCallback(void (*cb)());

}

}