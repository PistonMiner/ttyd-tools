#pragma once

#include <cstdint>

namespace ttyd::dvdmgr {

struct DvdMgrFile
{
	char path[64];
	uint8_t fileInfo[0x3c];
	void *readTargetAddr;
	int32_t readSize;
	int32_t readBaseOffset;
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

void *DVDMgrOpen(const char *path, int priority, uint16_t wZero);
uint32_t DVDMgrRead(void *file, void *dst, int size, int offset);
void DVDMgrReadAsync(void *file, void *dst, int size, int offset, void (*pfnReadDoneCb)(int result, void *fileInfo));
void DVDMgrClose(void *file);
uint32_t DVDMgrGetLength(void *file);

void DVDMgrSetupCallback(void *(cb)());

}