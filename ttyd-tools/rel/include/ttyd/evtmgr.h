#pragma once

#include <cstdint>

namespace ttyd::evtmgr {

struct EvtEntry;

struct EvtEntry
{
	int64_t timeSinceStart;
	uint8_t flags;
	int8_t paramCount;
	uint8_t opcode;
	int8_t executionOrder;
	uint8_t typeMask;
	bool sleeping;
	uint8_t loopStackIndex;
	uint8_t switchStackIndex;
	int8_t wNpcEventType;
	uint8_t pad_10[3];
	int32_t nextCommandPtr;
	void *currentCommandArguments;
	int8_t labelIdTable[16];
	void *labelAddressTable[16];
	EvtEntry *waitingEvt;
	EvtEntry *waitingOnEvt;
	EvtEntry *parentEvt;
	int32_t sleepTimeMs;
	int64_t sleepStartTime;
	int32_t wInterpolationStartedNpcFlag;
	int64_t wInterpRelatedTime;
	int32_t unk_90;
	int32_t unk_94;
	int32_t currentSleepingFuncPtr;
	int32_t lwData[16];
	uint32_t lfData;
	int32_t unk_e0;
	int32_t unk_e4;
    void *loopStartAddressStack[8];
    int32_t loopIterationsLeftStack[8];
    int8_t switchStateStack[8];
    int32_t switchValueStack[8];
    void* memoryCmdBase;
	void *uwBase;
	void *ufBase;
	int32_t threadId;
	void *wActorThisPtr;
	float timescale;
	int32_t timeScheduledToRun;
	int32_t unk_16c;
	void *wThisPtr;
	void *wThisObjPtr;
	int32_t wActiveMsgWindowId;
	int32_t unk_17c;
	int32_t unk_180;
	int32_t unk_184;
	int32_t unk_188;
	int32_t unk_18c;
	int32_t msgPriority;
	int32_t unk_194;
	int64_t wInterpolationStartTime2;
	void *restartFromLocation;
	char *name;
	void *wCurrentCommandPtr;
	int32_t unk_1ac;
} __attribute__((__packed__));

static_assert(sizeof(EvtEntry) == 0x1b0);

struct EvtWork
{
	int32_t entryCount;
	uint32_t gwData[32];
	uint32_t gfData[3];
	EvtEntry *entries;
	uint32_t pad_94;
	int64_t currentEvtTime;
} __attribute__((__packed__));

static_assert(sizeof(EvtWork) == 0xa0);

extern "C" {

void evtmgrInit();
void evtmgrReInit();
void evtmgrMain();

EvtWork *evtGetWork();

EvtEntry *evtEntry(void *evtCode, int32_t executionOrder, uint32_t flags);
EvtEntry *evtEntryType(void *evtCode, int8_t priority, uint32_t flags, uint32_t types);
EvtEntry *evtChildEntry(EvtEntry *parentEvt, void *evtCode, uint32_t flags);
EvtEntry *evtBrotherEntry(EvtEntry *parentEvt, void *evtCode, uint32_t flags);
EvtEntry *evtRestart(EvtEntry *evt);
void evtDelete(EvtEntry *evt);
void evtDeleteID(int32_t threadId);
bool evtCheckID(int32_t threadId);
void evtSetPri(EvtEntry *evt, int32_t priority);
void evtSetSpeed(EvtEntry *evt, float timescale);
void evtSetType(EvtEntry *evt, uint8_t typeMask);
void evtStop(EvtEntry *evt, uint8_t typeMask);
void evtStart(EvtEntry *evt, uint8_t typeMask);
void evtStopID(int32_t threadId);
void evtStartID(int32_t threadId);
void evtStopAll(uint8_t typeMask);
void evtStartAll(uint8_t typeMask);
void evtStopOther(EvtEntry *evt, uint8_t typeMask);
void evtStartOther(EvtEntry *evt, uint8_t typeMask);
EvtEntry *evtGetPtr(int32_t index);
EvtEntry *evtGetPtrID(int32_t threadId);

// Local functions
// void make_pri_table();
// void evtEntryRunCheck(EvtEntry *evt);

}

#define EVT_DECLARE_USER_FUNC(name, parameter_count) \
	constexpr int name##_parameter_count = (parameter_count); \
	int32_t name(ttyd::evtmgr::EvtEntry *evt, bool isFirstCall);

#define EVT_DEFINE_USER_FUNC(name) \
	int32_t name(ttyd::evtmgr::EvtEntry *evt, bool isFirstCall)

#define EVT_DECLARE(name) \
	extern const int32_t name[];

}