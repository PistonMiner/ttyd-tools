#pragma once

#include <cstdint>

namespace gc::exi {

extern "C" {

uint32_t EXIGetState(int32_t chan);
int32_t EXIGetID(int32_t chan, int32_t dev, uint32_t *outId);
int32_t EXIProbe(int32_t chan);
int32_t EXIProbeEx(int32_t chan);

int32_t EXIAttach(int32_t chan, void *detachCb);
int32_t EXIDetach(int32_t chan);

int32_t EXILock(int32_t chan, int32_t dev, void *unlockCb);
int32_t EXIUnlock(int32_t chan);

int32_t EXISelect(int32_t chan, int32_t dev, int32_t freq);
int32_t EXIDeselect(int32_t chan);

int32_t EXISync(int32_t chan);
uint32_t EXIClearInterrupts(int32_t chan, int32_t exi, int32_t tc, int32_t ext);
void *EXISetExiCallback(int32_t chan, void *interruptCb);

int32_t EXIImm(int32_t chan, void *data, int32_t len, int32_t mode, void *completionCb);
int32_t EXIImmEx(int32_t chan, void *data, int32_t len, int32_t mode);
int32_t EXIDma(int32_t chan, void *data, int32_t len, int32_t mode, void *completionCb);

}

}