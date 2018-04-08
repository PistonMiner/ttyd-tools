#pragma once

#include <cstdint>

namespace ttyd::seqdrv {

struct SeqInfo
{
	uint32_t seq;
	uint32_t state;
	const char *map;
	const char *bero;
	uint32_t counter;
	uint32_t unk_14;
	uint32_t unk_18;
	uint32_t unk_1c;
} __attribute__((__packed__));

extern "C" {

void seqInit_MARIOSTORY();
void seqMain();
void seqSetSeq(uint32_t seq, const char *mapName, const char *beroName);
uint32_t seqGetSeq();
uint32_t seqGetPrevSeq();
uint32_t seqGetNextSeq();
bool seqCheckSeq();

}

}