#pragma once

#include "database.h"

#include <cstdint>

namespace ttyd::mapdata {

extern "C"
{

void relSetBtlAddr(const char *areaName, const void *battleInfos, const database::DatabaseDefinition *nameToInfoIdTable);
void relSetEvtAddr(const char *mapName, const void *pInitEvtCode);
void **areaDataPtr(const char *areaName);
void **mapDataPtr(const char *mapName);

}

}