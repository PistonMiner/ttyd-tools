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

void Mod::init()
{
	gMod = this;
	
	mPFN_makeKey_trampoline = patch::hookFunction(ttyd::system::makeKey, []()
	{
		gMod->updateEarly();
	});
}

extern "C" {

extern void _sprintf(const char *fmt, ...);

}


void Mod::updateEarly()
{
	// Check for font load
	if (*reinterpret_cast<void **>(0x8041E978) != nullptr)
	{
		ttyd::dispdrv::dispEntry(ttyd::dispdrv::DisplayLayer_Dbg3d, 0, [](uint8_t layerId, void *user)
		{
			reinterpret_cast<Mod *>(user)->draw();
		}, this);		
	}
		
	// Call original function
	mPFN_makeKey_trampoline();
}

void Mod::draw()
{
	uint32_t r13 = 0x8041CF20;
	float *marioPos = *reinterpret_cast<float **>(r13 + 0x19E0) + 35;
	float *marioVel = *reinterpret_cast<float **>(r13 + 0x19E0) + 31;
	
	_sprintf(mDisplayBuffer, "Pos: %.2f %.2f %.2f\r\nSpdY: %.2f", marioPos[0], marioPos[1], marioPos[2], marioVel[0]);
	ttyd::fontmgr::FontDrawStart();
	ttyd::fontmgr::FontDrawMessage(-272, -120, mDisplayBuffer);
}

}