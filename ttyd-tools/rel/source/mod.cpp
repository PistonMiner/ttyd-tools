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
#include <cstring>

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

	// Register draw command
	ttyd::dispdrv::dispEntry(ttyd::dispdrv::DisplayLayer::kDebug3d, 0, [](ttyd::dispdrv::DisplayLayer layerId, void *user)
	{
		reinterpret_cast<Mod *>(user)->draw();
	}, this);
	
	// Keyboard input for prompt
	size_t bufferLen = strlen(mCommandBuffer);
	for (int i = 0; i < mKeyboard->getKeyPressedCount(); ++i)
	{
		KeyCode pressed = mKeyboard->getKeyPressed(i);
		char textInput = Keyboard::getCharForKeycode(
			pressed,
			mKeyboard->isKeyDown(KeyCode::kLeftShift) || mKeyboard->isKeyDown(KeyCode::kRightShift)
		);
		if (textInput != '\0')
		{
			if (bufferLen < sizeof(mCommandBuffer) - 1)
			{
				mCommandBuffer[bufferLen++] = textInput;
				mCommandBuffer[bufferLen] = '\0';
			}
		}
		else if (pressed == KeyCode::kEnter)
		{
			processCommand(mCommandBuffer);
			mCommandBuffer[0] = '\0';
		}
		else if (pressed == KeyCode::kBackspace && bufferLen > 0)
		{
			mCommandBuffer[--bufferLen] = '\0';
		}
	}

	// Call original function
	mPFN_makeKey_trampoline();
}

void Mod::draw()
{
	ttyd::mario::Player *player = ttyd::mario::marioGetPtr();
	sprintf(mDisplayBuffer,
	        "Pos: %.2f %.2f %.2f\r\nSpdY: %.2f\r\nCmd: %s",
	        player->playerPosition[0], player->playerPosition[1], player->playerPosition[2],
	        player->wJumpVelocityY,
	        mCommandBuffer);
	ttyd::fontmgr::FontDrawStart();
	uint32_t color = 0xFFFFFFFF;
	ttyd::fontmgr::FontDrawColor(reinterpret_cast<uint8_t *>(&color));
	ttyd::fontmgr::FontDrawEdge();
	ttyd::fontmgr::FontDrawMessage(-272, -40, mDisplayBuffer);
}

void Mod::processCommand(const char *command)
{

}

}