#pragma once

namespace mod {

class Mod
{
public:
	void init();
	
private:
	void updateEarly();
	void draw();
	
private:
	void (*mPFN_makeKey_trampoline)() = nullptr;
	char mDisplayBuffer[256];
};

}