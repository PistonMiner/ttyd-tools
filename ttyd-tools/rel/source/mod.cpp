#include "mod.h"

#include <ttyd/system.h>
#include <ttyd/mariost.h>
#include <ttyd/fontmgr.h>
#include <ttyd/dispdrv.h>
#include <ttyd/seqdrv.h>
#include <ttyd/seq_logo.h>

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

	// Skip the logo
	patch::hookFunction(ttyd::seq_logo::seq_logoMain, [](ttyd::seqdrv::SeqInfo *)
	{
		ttyd::seqdrv::seqSetSeq(ttyd::seqdrv::SeqIndex::kTitle, nullptr, nullptr);
	});
}

void Mod::updateEarly()
{
	// Check for font load
	ttyd::dispdrv::dispEntry(ttyd::dispdrv::DisplayLayer::kDebug3d, 0, [](ttyd::dispdrv::DisplayLayer layerId, void *user)
	{
		reinterpret_cast<Mod *>(user)->draw();
	}, this);
	
	// Palace skip timing code
	if (ttyd::mariost::marioStGetSystemLevel() == 0xF)
	{
		// Reset upon pausing
		mPalaceSkipTimer.stop();
		mPalaceSkipTimer.setValue(0);
		mPaused = true;
	}
	else if (ttyd::mariost::marioStGetSystemLevel() == 0 && mPaused)
	{
		// Start when unpausing
		mPalaceSkipTimer.start();
		mPaused = false;
	}
	
	if (ttyd::system::keyGetButtonTrg(0) & 0x0400)
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
#ifdef TTYD_US
	uint32_t r13 = 0x8041CF20;
	float *marioPos = *reinterpret_cast<float **>(r13 + 0x19E0) + 35;
	float *marioVel = *reinterpret_cast<float **>(r13 + 0x19E0) + 31;
#else
	#error Mario position and velocity not implemented for this version
#endif
	
	sprintf(mDisplayBuffer,
	        "Pos: %.2f %.2f %.2f\r\nSpdY: %.2f\r\nPST: %lu",
	        marioPos[0], marioPos[1], marioPos[2],
	        marioVel[0],
	        mPalaceSkipTimer.getValue());
	ttyd::fontmgr::FontDrawStart();
	uint32_t color = 0xFFFFFFFF;
	ttyd::fontmgr::FontDrawColor(reinterpret_cast<uint8_t *>(&color));
	ttyd::fontmgr::FontDrawEdge();
	ttyd::fontmgr::FontDrawMessage(-272, -100, mDisplayBuffer);
}

}