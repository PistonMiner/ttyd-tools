#include "mod.h"

#include <ttyd/system.h>
#include <ttyd/fontmgr.h>
#include <ttyd/dispdrv.h>

#include "patch.h"

#include <cstdio>

namespace mod {

Mod *gMod = nullptr;

void main()
{
	Mod *mod = new Mod();
	mod->init();
}

Mod::Mod()
{
	
}

void Mod::init()
{
	gMod = this;
	
	mPFN_makeKey_trampoline = patch::hookFunction(ttyd::system::makeKey, []()
	{
		gMod->updateEarly();
	});

	// Initialize typesetting early
	ttyd::fontmgr::fontmgrTexSetup();
	patch::hookFunction(ttyd::fontmgr::fontmgrTexSetup, [](){});
}

static uint16_t getInput()
{
#ifdef TTYD_US
	return *reinterpret_cast<uint16_t *>(0x803CA398);
#else
	#error getInput() not implemented for this version
#endif
}

static uint32_t getThreadExecState()
{
#ifdef TTYD_US
	return *reinterpret_cast<uint32_t *>(0x8041E940);
#else
	#error getThreadExecState() not implemented for this version
#endif
}

void Mod::updateEarly()
{
	// Check for font load
	ttyd::dispdrv::dispEntry(ttyd::dispdrv::DisplayLayer_Dbg3d, 0, [](uint8_t layerId, void *user)
	{
		reinterpret_cast<Mod *>(user)->draw();
	}, this);
	
	// Palace skip timing code
	if (getThreadExecState() == 4)
	{
		// Reset upon pausing
		mPalaceSkipTimer.stop();
		mPalaceSkipTimer.setValue(0);
		mPaused = true;
	}
	else if (getThreadExecState() == 0 && mPaused)
	{
		// Start when unpausing
		mPalaceSkipTimer.start();
		mPaused = false;
	}
	
	if (getInput() & 0x0400)
	{
		// Stop when pressing A or X
		mPalaceSkipTimer.stop();
	}
	mPalaceSkipTimer.tick();
	
	// Call original function
	mPFN_makeKey_trampoline();
}

void Mod::draw()
{
	uint32_t r13 = 0x8041CF20;
	float *marioPos = *reinterpret_cast<float **>(r13 + 0x19E0) + 35;
	float *marioVel = *reinterpret_cast<float **>(r13 + 0x19E0) + 31;
	
	sprintf(mDisplayBuffer, "Pos: %.2f %.2f %.2f\r\nSpdY: %.2f\r\nPST: %lu", marioPos[0], marioPos[1], marioPos[2], marioVel[0], mPalaceSkipTimer.getValue());
	ttyd::fontmgr::FontDrawStart();
	uint32_t color = 0xFFFFFFFF;
	ttyd::fontmgr::FontDrawColor(reinterpret_cast<uint8_t *>(&color));
	ttyd::fontmgr::FontDrawEdge();
	ttyd::fontmgr::FontDrawMessage(-272, -120, mDisplayBuffer);
}

}