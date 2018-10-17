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
	
private:
	void (*mPFN_makeKey_trampoline)() = nullptr;
	char mCommandBuffer[256] = "";
	char mDisplayBuffer[256];

	Keyboard *mKeyboard = nullptr;
};

}