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
	
private:
	Timer<uint32_t> mPalaceSkipTimer;
	bool mPaused = false;
	
	void (*mPFN_makeKey_trampoline)() = nullptr;
	char mDisplayBuffer[256];

	Keyboard *mKeyboard = nullptr;
};

}