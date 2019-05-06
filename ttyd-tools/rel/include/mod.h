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
	void processCommand(const char *command);
	
	void updateHeapInfo();

private:
	void (*mPFN_makeKey_trampoline)() = nullptr;
	
	char mCommandBuffer[256] = "";
	int mBackspaceHoldTimer = 0;

	int mDebugHeapId = -1;
	char mDebugHeapText[64];

	Keyboard *mKeyboard = nullptr;
};

}