#pragma once

#include <cstdint>

namespace ttyd::event {

struct EventStageEventDescription
{
	uint8_t entryMotionType;
	uint8_t party0Id;
	uint8_t party1Id;
	uint8_t pad_3;
	uint16_t gsw0;
	uint16_t pad_6;
	const char *textId;
	const char *nameJp;
#if !TTYD_JP
	const char *nameEn;
#endif
	const char *map;
	const char *bero;
	void (*pfnInit)();
} __attribute__((packed));

#if !TTYD_JP
static_assert(sizeof(EventStageEventDescription) == 0x20);
#else
static_assert(sizeof(EventStageEventDescription) == 0x1c);
#endif

struct EventStageDescription
{
	const char *nameJp;
#if !TTYD_JP
	const char *nameEn;
#endif
	EventStageEventDescription *events;
	uint32_t eventCount;
} __attribute__((packed));

#if !TTYD_JP
static_assert(sizeof(EventStageDescription) == 0x10);
#else
static_assert(sizeof(EventStageDescription) == 0xc);
#endif

extern "C" {

EventStageDescription *eventStgDtPtr(int stageId);
int eventStgNum();

}

}