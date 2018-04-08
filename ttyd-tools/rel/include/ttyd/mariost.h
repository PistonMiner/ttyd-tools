#pragma once

namespace ttyd::mariost {
	
extern "C" {

void marioStInit();
void marioStMain();
void marioStDisp();
void marioStSystemLevel(uint32_t level);
uint32_t marioStGetSystemLevel();
void viPostCallback(uint32_t retraceCount);
void gcDvdCheckThread();
void gcRumbleCheck();

}

}