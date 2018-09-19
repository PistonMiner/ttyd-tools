#include "mod.h"

#include <ttyd/system.h>
#include <ttyd/mariost.h>
#include <ttyd/fontmgr.h>
#include <ttyd/dispdrv.h>
#include <ttyd/seqdrv.h>
#include <ttyd/seq_logo.h>
#include <ttyd/mario.h>

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

	// Keyboard on controller port 1
	mKeyboard = new Keyboard(1);
}

void Mod::updateEarly()
{
	// Keyboard code
	mKeyboard->update();

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
	char keyDownString[4] = "";
	for (int i = 0; i < mKeyboard->getKeyDownCount(); ++i)
	{
		KeyCode k = mKeyboard->getKeyDown(i);
		keyDownString[i] = Keyboard::getCharForKeycode(k);
	}

	char keyPressedString[4] = "";
	for (int i = 0; i < mKeyboard->getKeyPressedCount(); ++i)
	{
		KeyCode k = mKeyboard->getKeyPressed(i);
		keyPressedString[i] = Keyboard::getCharForKeycode(k);
	}

	char keyReleasedString[4] = "";
	for (int i = 0; i < mKeyboard->getKeyReleasedCount(); ++i)
	{
		KeyCode k = mKeyboard->getKeyReleased(i);
		keyReleasedString[i] = Keyboard::getCharForKeycode(k);
	}

	ttyd::mario::Player *player = ttyd::mario::marioGetPtr();
	sprintf(mDisplayBuffer,
	        "Pos: %.2f %.2f %.2f\r\nSpdY: %.2f\r\nPST: %lu\r\nKBD: %s\r\nKBP: %s\r\nKBR: %s",
	        player->playerPosition[0], player->playerPosition[1], player->playerPosition[2],
	        player->wJumpVelocityY,
	        mPalaceSkipTimer.getValue(),
	        keyDownString,
	        keyPressedString,
	        keyReleasedString);
	ttyd::fontmgr::FontDrawStart();
	uint32_t color = 0xFFFFFFFF;
	ttyd::fontmgr::FontDrawColor(reinterpret_cast<uint8_t *>(&color));
	ttyd::fontmgr::FontDrawEdge();
	ttyd::fontmgr::FontDrawMessage(-272, -40, mDisplayBuffer);
}

}