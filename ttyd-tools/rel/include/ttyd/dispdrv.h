#pragma once

#include <cstdint>

namespace ttyd::dispdrv {

enum class DisplayLayer : uint8_t
{
	kOffscreen = 0,
	kOffscreen2,
	kShadow,
	kBackground,
	k3d,
	k3dEffectA,
	k3dImage,
	k3dEffectB,
	k2d,
	kFade,
	kFade2,
	kDebug,
	kDebug3d,
};

typedef void (*PFN_dispCallback)(DisplayLayer layerId, void *user);

struct DisplayWork
{
	DisplayLayer layer;
	uint8_t renderMode;
	uint16_t padding_2;
	float unk_4;
	PFN_dispCallback callback;
	void *user;
} __attribute__((__packed__));

extern "C" {

void dispInit();
void dispReInit();
void dispEntry(DisplayLayer layerId, uint8_t renderMode, PFN_dispCallback callback, void *user);
void dispSort();
void dispDraw(DisplayLayer layerId);
// float dispCalcZ(void *vecUnk);
DisplayWork *dispGetCurWork();

}

}
