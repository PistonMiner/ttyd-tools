#include "platform.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static unsigned int sOldCodePageID;

void setupConsoleCodePage()
{
	sOldCodePageID = GetConsoleOutputCP();
	SetConsoleOutputCP(CP_UTF8);
}

void resetConsoleCodePage()
{
	SetConsoleOutputCP(sOldCodePageID);
}