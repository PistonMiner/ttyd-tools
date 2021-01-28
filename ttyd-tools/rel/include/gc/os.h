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

// OSError
void OSReport(const char *fmt, ...);
void OSPanic(const char *fmt, ...);

// OSTime
int64_t OSGetTime();
uint32_t OSGetTick();

}

}