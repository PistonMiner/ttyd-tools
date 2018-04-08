#pragma once

#include "seqdrv.h"

#include <cstdint>

namespace ttyd::seq_logo {

extern "C" {

void seq_logoInit(seqdrv::SeqInfo *info);
void seq_logoExit(seqdrv::SeqInfo *info);
void seq_logoMain(seqdrv::SeqInfo *info);

}

}