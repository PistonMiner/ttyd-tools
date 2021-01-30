#pragma once

#include <cstdint>

namespace gc::os {

// OSAlloc
struct ChunkInfo
{
	ChunkInfo *prev;
	ChunkInfo *next;
	uint32_t size;
} __attribute__((__packed__));

struct HeapInfo
{
	uint32_t capacity;
	ChunkInfo *firstFree;
	ChunkInfo *firstUsed;
} __attribute__((__packed__));

extern "C" {

extern HeapInfo *OSAlloc_HeapArray;
extern int OSAlloc_NumHeaps;

}

// OSCache
extern "C" {

void L2GlobalInvalidate();
void LCDisable();

void ICEnable();
void ICFlashInvalidate();
void ICInvalidateRange(void *base, uint32_t size);

void DCEnable();
void DCInvalidateRange(void *base, uint32_t size);
void DCFlushRange(void *base, uint32_t size);
void DCStoreRange(void *base, uint32_t size);
void DCFlushRangeNoSync(void *base, uint32_t size);
void DCStoreRangeNoSync(void *base, uint32_t size);

}

// OSContext
struct OSContext
{
	uint32_t gpr[32];
	uint32_t cr;
	uint32_t lr;
	uint32_t ctr;
	uint32_t xer;
	double fpr[32];
	double fpcsr;
	uint32_t srr0;
	uint32_t srr1;

	uint16_t mode;
	uint16_t state;

	uint32_t gqr[8];
	
	uint32_t pad_1c4;

	double fpr_ps[32];
} __attribute__((__packed__));

extern "C" {

void OSInitContext(OSContext *context, uint32_t srr0, uint32_t r1);
void OSClearContext(OSContext *context);

int32_t OSSaveContext(OSContext *context);
void OSLoadContext(OSContext *context);

OSContext *OSGetCurrentContext();
void OSSetCurrentContext(OSContext *context);

void OSSaveFPUContext(OSContext *context);
void OSFillFPUContext(OSContext *context);
void __OSSaveFPUContext(uint32_t, uint32_t, OSContext *context);
void __OSLoadFPUContext(uint32_t, OSContext *context);

void OSDumpContext(OSContext *context);

void *OSGetStackPointer(OSContext *context);

// OSError
void OSReport(const char *fmt, ...);
void OSPanic(const char *fmt, ...);

// OSTime
int64_t OSGetTime();
uint32_t OSGetTick();

}

}