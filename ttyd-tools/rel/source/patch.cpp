#include "patch.h"

#include <cstdint>
#include <cstddef>

namespace mod::patch {

// TODO: Add checks against invalid arguments
constexpr static uint32_t assemble_stw(int rS, int rA, int d)
{
	uint16_t encoded_disp = d & 0xffff;
	return 36u << 26 | rS << 21 | rA << 16 | encoded_disp;
}

constexpr static uint32_t assemble_mfspr(int rD, int spr)
{
	uint32_t encoded_spr = 0;
	encoded_spr |= (spr & 0x1f) << 5;
	encoded_spr |= (spr >> 5) & 0x1f;
	return 31u << 26 | rD << 21 | encoded_spr << 11 | 339 << 1;
}

constexpr static uint32_t assemble_b(void *target, void *source, bool link = false)
{
	uint32_t delta = (uint32_t)target - (uint32_t)source;
	delta &= 0x03fffffc;
	return 18u << 26 | delta | (link ? 1 : 0);
}

void writeBranch(void *ptr, void *destination)
{
	uint32_t value = assemble_b(destination, ptr);
	*(uint32_t *)ptr = value;

	// Make visible
	gc::os::DCFlushRange(ptr, sizeof(uint32_t));
	gc::os::ICInvalidateRange(ptr, sizeof(uint32_t));
}

extern "C" void patchInstructionHookSaveContextAndHandle();

void hookInstruction(void *location, InstructionHookHandler handler, void *user)
{
	using namespace gc::os;

	// These must be kept in sync with the assembly!
	constexpr int kIhStackframeUserR3 = 0x8;
	constexpr int kIhStackframeUserSize = 0x20;
	constexpr int kIhStackframeSize = kIhStackframeUserSize + sizeof(OSContext);
	static_assert(kIhStackframeSize == 0x2e8);

	// Construct trampoline
	constexpr int kTrampolineSize = 7;
	uint32_t *trampoline = new uint32_t[kTrampolineSize];
	// ppc: stw r3, (-IH_STACKFRAME_SIZE + IH_STACKFRAME_R3)(r1)
	trampoline[0] = assemble_stw(3, 1, -kIhStackframeSize + kIhStackframeUserR3);
	// ppc: mflr r3
	trampoline[1] = assemble_mfspr(3, 8);
	// ppc: bl to second stage handler
	trampoline[2] = assemble_b((void *)patchInstructionHookSaveContextAndHandle, &trampoline[2], true);
	// ppc: replaced instruction
	trampoline[3] = *(uint32_t *)location;
	// ppc: branch back
	// Hooked instruction location can be extracted by inspecting this
	// instruction (which can be located via SRR0)
	trampoline[4] = assemble_b((uint8_t *)location + 4, &trampoline[4], false);

	// The offsets of these relative to the bl to the handler must be kept in
	// sync with the assembly!
	// handler fn ptr
	trampoline[5] = (uint32_t)handler;
	// user data
	trampoline[6] = (uint32_t)user;

	DCFlushRange(trampoline, kTrampolineSize * sizeof(uint32_t));
	ICInvalidateRange(trampoline, kTrampolineSize * sizeof(uint32_t));

	writeBranch(location, trampoline);
}

}