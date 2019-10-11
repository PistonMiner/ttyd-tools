#pragma once

#include <cstdint>

namespace ttyd::dispdrv {

enum class CameraId : uint8_t
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

typedef void (*PFN_dispCallback)(CameraId cameraId, void *user);

struct DisplayWork
{
	CameraId cameraId;
	uint8_t renderMode;
	uint16_t padding_2;
	float order;
	PFN_dispCallback callback;
	void *user;
} __attribute__((__packed__));

extern "C" {

void dispInit();
void dispReInit();
void dispEntry(CameraId cameraId, uint8_t renderMode, float order, PFN_dispCallback callback, void *user);
void dispSort();
void dispDraw(CameraId cameraId);
// float dispCalcZ(void *vecUnk);
DisplayWork *dispGetCurWork();

}

}
