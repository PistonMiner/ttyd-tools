#include "mod.h"
#include "console.h"

#include <gc/os.h>

#include <cstdio>

namespace mod {

ConCommand perf_fake_spike("perf_fake_spike", [](const char *args) {
	uint32_t durationMs = 0;
	if (sscanf(args, "%lu", &durationMs) == 1)
	{
		// Cap to avoid double overflow
		if (durationMs > 100000)
		{
			durationMs = 100000;
		}
		uint32_t durationTicks = durationMs * 40500;

		// Busy loop!
		uint32_t startTick = gc::os::OSGetTick();
		do {} while (gc::os::OSGetTick() - startTick < durationTicks);
	}
});

#if TTYD_US
ConIntVar perf_show("perf_show", 0);
MOD_UPDATE_FUNCTION()
{
	if (!perf_show.value)
		return;

	uint32_t ticksProcessing = *(uint32_t *)0x803dbf2c;
	uint32_t ticksTotal = *(uint32_t *)0x803dbf30;
	
	uint32_t tbRate = util::GetTbRate();
	float msProcessing = (1000.f * ticksProcessing) / tbRate;
	float msTotal = (1000.f * ticksTotal) / tbRate;

	gConsole->overlay("proc: %.3fms\ntotal: %.3fms\n", msProcessing, msTotal);
}
#endif

ConIntVar mem_debug_heap("mem_debug_heap", -1);
MOD_UPDATE_FUNCTION()
{
	int heapId = mem_debug_heap.value;
	if (heapId == -1)
		return;

	if (heapId >= gc::os::OSAlloc_NumHeaps || heapId < 0)
	{
		gConsole->overlay("Heap: %d is not a valid heap\n", heapId);
		return;
	}

	const gc::os::HeapInfo &heap = gc::os::OSAlloc_HeapArray[heapId];

	// Check heap integrity
	bool valid = true;
	gc::os::ChunkInfo *currentChunk = nullptr;
	gc::os::ChunkInfo *prevChunk = nullptr;
	for (currentChunk = heap.firstUsed; currentChunk; currentChunk = currentChunk->next)
	{
		// Check pointer sanity
		auto pointerIsValid = [](void *ptr)
		{
			uint32_t pointerRaw = reinterpret_cast<uint32_t>(ptr);
			return pointerRaw >= 0x80000000 && pointerRaw <= 0x817fffff;
		};
		if (!pointerIsValid(currentChunk))
		{
			valid = false;
			break;
		}

		// Sanity check size
		if (currentChunk->size > 0x17fffff)
		{
			valid = false;
			break;
		}

		// Check linked list integrity
		if (prevChunk != currentChunk->prev)
		{
			valid = false;
			break;
		}

		prevChunk = currentChunk;
	}
	if (!valid)
	{
		gConsole->overlay(
			"Heap: %d corrupt at %08lx\n",
			heapId,
			reinterpret_cast<uint32_t>(currentChunk)
		);
		return;
	}

	// Accumulate used memory
	int usage = 0;
	int chunks = 0;
	for (gc::os::ChunkInfo *chunk = heap.firstUsed; chunk; chunk = chunk->next)
	{
		usage += chunk->size;
		++chunks;
	}

	gConsole->overlay(
		"Heap: id %d, %.2f/%.2fkb, %d cks\n",
		heapId,
		usage / 1024.f,
		heap.capacity / 1024.f,
		chunks
	);
}

}