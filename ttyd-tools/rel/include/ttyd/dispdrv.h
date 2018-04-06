#pragma once

#include <cstdint>

namespace ttyd::dispdrv {

enum DisplayLayer
{
	DisplayLayer_Off = 0,
	DisplayLayer_Off2,
	DisplayLayer_Shadow,
	DisplayLayer_Bg,
	DisplayLayer_3d,
	DisplayLayer_3deff_A,
	DisplayLayer_3dimg,
	DisplayLayer_3deff_B,
	DisplayLayer_2d,
	DisplayLayer_Fade,
	DisplayLayer_Fade2,
	DisplayLayer_Dbg,
	DisplayLayer_Dbg3d,
};

typedef void (*PFN_dispCallback)(uint8_t layerId, void *user);

struct DisplayWork
{
	uint8_t layer;
	uint8_t renderMode;
	uint16_t padding_2;
	float unk_4;
	PFN_dispCallback callback;
	void *user;
} __attribute__((__packed__));

extern "C" {

void dispInit();
void dispReInit();
void dispEntry(uint8_t layerId, uint8_t renderMode, PFN_dispCallback callback, void *user);
void dispSort();
void dispDraw(uint8_t layerId);
// float dispCalcZ(void *vecUnk);
DisplayWork *dispGetCurWork();

}

}
