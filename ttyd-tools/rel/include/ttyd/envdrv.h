#pragma once

#include <gc/types.h>

#include <cstdint>

namespace ttyd::envdrv {

enum EnvWorkGlareFlags
{
	EnvWorkGlareFlags_DepthTest = 0x1,
};

struct EnvWorkGlare
{
	uint16_t flags;
	int16_t glareType;
	int16_t top;
	int16_t bottom;
	int16_t left;
	int16_t right;
	float zCutoff;
} __attribute__((__packed__));

static_assert(sizeof(EnvWorkGlare) == 0x10);

enum EnvWorkBlurFlags
{
	EnvWorkBlurFlags_FullRangeStatic = 0x1,
};

struct EnvWorkBlur
{
	uint16_t flags;

	uint8_t pad_2[6];

	int64_t startTime;
	uint32_t duration;
} __attribute__((__packed__));

static_assert(sizeof(EnvWorkBlur) == 0x14);

enum EnvWorkDepthOfFieldFlags
{
	EnvWorkDepthOfFieldFlags_Near = 0x1,
	EnvWorkDepthOfFieldFlags_Far = 0x2,
};

struct EnvWorkDepthOfField
{
	uint16_t flags;
	uint8_t nearThreshold;
	uint8_t farThreshold;
	float nearWidth;
	float farWidth;
} __attribute__((__packed__));

static_assert(sizeof(EnvWorkDepthOfField) == 0xc);

enum EnvWorkFlags
{
	EnvWorkFlags_CaptureColor = 0x1,
	EnvWorkFlags_CaptureDepth = 0x2,
	EnvWorkFlags_CaptureColor2 = 0x4,
	EnvWorkFlags_Color2Captured = 0x8,
	EnvWorkFlags_FF = 0x10000000,
	EnvWorkFlags_Glare = 0x20000000,
	EnvWorkFlags_Blur = 0x40000000,
	EnvWorkFlags_DepthOfField = 0x80000000,
};

struct EnvWork
{
	uint32_t flags;
	// Filtered half-width RGBA8 after glare
	void *pCaptureColorImageAllocation;
	// Filtered half-width middle 8 bits of depth after glare as I8
	void *pCaptureDepthImageAllocation;
	uint8_t captureColorImageTexObj[32];
	uint8_t captureDepthImageTexObj[32];
	// Filtered half-width RGB565 captured after all post effects
	void *pCapture2ImageData;
	uint8_t capture2ImageTexObj[32];

	gc::mat3x4 *field_70;
	gc::mat3x4 *field_74;
	gc::mat3x4 *field_78;
	float field_7c;
	float field_80;
	float field_84;

	EnvWorkDepthOfField dof;

	uint8_t gap_94[0x4];

	EnvWorkBlur blur;

	uint8_t gap_ac[0x4];

	EnvWorkGlare glare;

	gc::mat3x4 yamiViewMat;
} __attribute__((__packed__));

static_assert(sizeof(EnvWork) == 0xf0);

extern "C" {

void envInit();
void envReInit();
void envTexSetup();
void envMain();

// If `bStatic` is true, fade a static capture from fully opaque from fully
// transparent. Otherwise, fade from 97% opacity down to 50% opacity on a
// dynamic capture.
void envBlurOn(int32_t bStatic, int32_t duration);
void envBlurOff();

void envGlareFilter(int32_t type, int16_t top, int16_t left, int16_t bottom, int16_t right);
void envGlareFilterZ(float z);

// void envAddTev();
// void envSetWater();
// void envSetYamiView();
// void envSetYami();

}

}