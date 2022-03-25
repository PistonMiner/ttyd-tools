#pragma once

#include <cstdint>

namespace gc::demo {

extern "C" {

void DEMOInitCaption(int32_t type, int32_t width, int32_t height);
void DEMOPuts(int16_t x, int16_t y, int16_t z, const char *text);
void DEMOPrintf(int16_t x, int16_t y, int16_t z, char const* fmt, ...);
// DEMOWriteStats
// DEMOUpdateStats
// DEMOPrintStats

}

}