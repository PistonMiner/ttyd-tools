#pragma once

#include "timer.h"
#include "keyboard.h"

#include <cstdint>

namespace mod {

class Mod
{
public:
	Mod();
	void init();
	
private:
	void updateEarly();
	void draw();

	void updateConsole();
	void processCommand(const char *command);
	void updateHeapInfo();
	
	void drawConsole();
	void drawMovementInfo();
	void drawHeapInfo();

private:
	void (*mPFN_makeKey_trampoline)() = nullptr;
	
	bool mShowUi = true;
	char mDisplayBuffer[256];

	bool mShowMovementInfo = false;

	char mCommandBuffer[64] = "";
	int mBackspaceHoldTimer = 0;

	bool mConsoleActive = false;

	int mDebugHeapId = -1;
	char mDebugHeapText[64];

	Keyboard *mKeyboard = nullptr;
};

}