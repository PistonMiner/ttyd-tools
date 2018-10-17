#pragma once

#include <cstdint>

namespace gc::si {

extern "C" {

bool SIBusy();
bool SIIsChanBusy(uint32_t channel);
// local: CompleteTransfer
// local: SIInterruptHandler
// local: SIEnablePollingInterrupt
bool SIRegisterPollingHandler(void *handler);
bool SIUnregisterPollingHandler(void *handler);
void SIInit();
// local: __SITransfer
uint32_t SIGetStatus(uint32_t channel);
void SISetCommand(uint32_t channel, uint32_t command);
void SITransferCommands();
uint32_t SISetXY(uint16_t lineInterval, uint8_t count);
uint32_t SIEnablePolling(uint32_t pollMask);
uint32_t SIDisablePolling(uint32_t pollMask);
// local: SIGetResponseRaw
uint32_t SIGetResponse(uint32_t channel, void *buffer);
// local: AlarmHandler
uint32_t SITransfer(uint32_t channel, const void *bufferOut, uint32_t lengthOut, void *bufferIn, uint32_t lengthIn, void *callback);
// local: GetTypeCallback
uint32_t SIGetType(uint32_t channel);
uint32_t SIGetTypeAsync(uint32_t channel, void *callback);
uint32_t SIDecodeType(uint32_t value);
uint32_t SIProbe(uint32_t channel);
void SISetSamplingRate(uint32_t rate);
void SIRefreshSamplingRate();

}

}