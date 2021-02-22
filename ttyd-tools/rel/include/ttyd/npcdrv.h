#pragma once

#include <gc/types.h>
#include <ttyd/database.h>

#include <cstdint>

namespace ttyd::npcdrv {

struct NpcEntry;

struct NpcWork
{
	uint32_t npcCount;
	uint32_t npcMaxCount;
	uint32_t wFlags;
	NpcEntry *entries;
	NpcEntry *wTalkCheckRelatedNpc;
} __attribute__((__packed__));

static_assert(sizeof(NpcWork) == 0x14);

struct NpcTribeDescription
{
	char *nameJp;
	char *modelName;
	char *wInitialAnimation;
	char *stopAnimation;
	char *stayAnimation;
	char *talkAnimation;
	char *walkAnimation;
	char *runAnimation;
	char *damageAnimation;
	char *confuseAnimation;
	float wWidth;
	float height;
	float wShadowOffsetX;
	float wShadowOffsetY;
	float wShadowOffsetZ;
	float runStartSpeed;
	char *moveLeftSfxId;
	char *moveRightSfxId;
	uint16_t unkSfxId;
	uint8_t pad_4a[2];
	char *jumpSfxId;
	char *landingSfxId;
} __attribute__((__packed__));

static_assert(sizeof(NpcTribeDescription) == 0x54);

struct NpcBattleInfo
{
	void *wpStageInfo;
	uint32_t wFromSetup18_dropFlags;
	uint32_t wFromSetup8_drop_fixitem;
	uint32_t wItemDropped;
	uint32_t wHeartsDroppedBaseCount;
	uint32_t wFlowersDroppedBaseCount;
	void *pConfiguration; // Reference to unimplemented type BattleSetupConfiguration
	uint32_t wHeldItems[8];
	uint32_t *pItemDropLists[8];
	int32_t wStolenItems[8];
	uint32_t wBackItemIds[8];
	void *wpBtlSetupWork[4];
	uint32_t wFromSetup1c;
	int8_t audienceCounts[16];
	char *musicName;
	uint8_t ruleCondition;
	uint8_t ruleParameter0;
	uint8_t ruleParameter1;

	uint8_t pad_c7[0x1];
} __attribute__((__packed__));

static_assert(sizeof(NpcBattleInfo) == 0xc8);

enum NpcEntryFlags
{
	NpcEntryFlags_Blur = 0x100,
	NpcEntryFlags_IsVivian = 0x4000000,
	NpcEntryFlags_IgnoreCloudBreath = 0x20000000,
};

enum class NpcTerritoryType : int32_t
{
	kNothing = 0x0,
	kCircle = 0x1,
	kSquare = 0x2,
};

struct NpcEntry
{
	uint32_t flags;
	uint32_t reactionFlags;
	char name[32];
	NpcTribeDescription *tribe;
	char currentAnimation[32];
	char stayAnimation[32];
	char talkAnimation[32];
	gc::vec3 position;
	gc::vec3 previousPosition;
	gc::vec3 positionHistory[5];
	gc::vec3 scale;
	gc::vec3 rotation;
	gc::vec3 rotationOffset;
	uint32_t poseId;
	uint32_t currentMotionAnimationState;
	char *wSomeAnimName_defaultM_I_2;
	int32_t offscreenId;
	gc::color4 color;
	uint32_t initEvtId;
	uint32_t regularEvtId;
	void *initEvtCode;
	void *regularEvtCode;
	void *wSomeEvtCode;
	void *deadEvtCode;
	void *findEvtCode;
	void *lostEvtCode;
	void *returnEvtCode;
	void *blowEvtCode;
	uint32_t wInterruptFlags;
	float rotationY;
	float wFbatSavedRotationY;
	float width;
	float height;
	float wShadowSizeRelated;
	gc::vec3 wJumpStartPosition;
	gc::vec3 wJumpTargetPosition;

	uint8_t gap_170[0x8];

	uint32_t wResetUponEvtJump2;
	uint32_t wResetUponEvtJump3;
	float field_180;

	uint8_t gap_184[0x4];

	int64_t wResetUponEvtJump4;
	int64_t field_190;
	int64_t wSomeTime;
	float field_1a0;
	float wCurrentWalkingSpeed;
	float field_1a8;
	float jumpAngleXZ;
	float wUnkJumpParameter;
	float wUnkJumpRelated6;
	float wUnkJumpRelated5;
	float wUnkJumpRelated7;
	float wUnkJumpRelated4;
	float wUnkJumpRelated3;
	float wUnkJumpRelated2;
	float wUnkJumpRelated1;
	float field_1d0;
	uint32_t wJumpFlags;
	uint16_t wbSoundDataDirty_counter;

	uint8_t gap_1da[0x2];

	float wInitInSetupDataBaseSoundRelated;
	char *moveLeftSfxId;
	char *moveRightSfxId;
	uint16_t unkSfxId_fromTribe;

	uint8_t gap_1ea[0x2];

	char *jumpSfxId;
	char *landingSfxId;
	uint32_t cameraId;
	NpcTerritoryType territoryType;
	gc::vec3 wTerritoryBase;
	gc::vec3 wTerritoryLoiter;
	gc::vec3 wTerritoryHoming;
	float wSearchRange;
	float wSearchAngle;
	float wHomingRange;
	float wHomingAngle;
	NpcBattleInfo battleInfo;
	int32_t wAttackMode;
	uint32_t wWallStop;
	void *wKpaMobjDeadCheck; // Reference to unimplemented type HitEntry
	void *wUnitWork[4];
	uint8_t wDatabaseTagIndex;
	char wBattleCoinDropCount;
	uint8_t wBaseCoinCount;
	uint8_t wBalloonType;
	uint32_t wFbatHitFlags;
	uint16_t wKpaScoreType;
	uint16_t wKpaMobjFlags;
	uint16_t wFbatHitCheckRelated;

	uint8_t gap_322[0x2];

	NpcEntry *linkFrom;
	NpcEntry *linkTo;
	NpcEntry *master;
	NpcEntry *slaves[4];
} __attribute__((__packed__));

static_assert(sizeof(NpcEntry) == 0x340);

struct FbatHitInfo
{
	uint32_t wFlags;
	uint32_t wAttackMode;
	float wDistance;
	uint32_t wUnknownFlags;
} __attribute__((__packed__));

static_assert(sizeof(FbatHitInfo) == 0x10);

struct FbatBattleInformation
{
	uint32_t wMode;
	uint32_t wParty;
	uint32_t wFirstAttack;

	uint8_t gap_c[0x4];

	uint32_t wResult;
	int32_t wCoinDropCount;

	uint8_t gap_18[0x1];

	uint8_t wRuleKeepResult;

	uint8_t gap_1a[0x2];
} __attribute__((__packed__));

static_assert(sizeof(FbatBattleInformation) == 0x1c);

struct FbatDatabaseNpcDeadInfo
{
	char mapName[16];
	uint32_t deadNpcMask;
} __attribute__((__packed__));

static_assert(sizeof(FbatDatabaseNpcDeadInfo) == 0x14);

struct FbatAttackAnnounceInfo
{
	uint32_t enabled;
	uint32_t field_4;
	uint32_t field_8;
	uint32_t field_c;
	float field_10;
	float field_14;
	char *wMsg;
	gc::color4 wColor;
} __attribute__((__packed__));

static_assert(sizeof(FbatAttackAnnounceInfo) == 0x20);

struct FbatData
{
	int16_t mode;
	uint8_t field_2;

	uint8_t gap_3[0x1];

	NpcEntry *wHitNpc;
	NpcEntry *pBattleNpc;
	uint32_t wUnkEvtThreadId;
	FbatHitInfo hitInfo;
	FbatBattleInformation battleInfo;

	uint8_t gap_3c[0xc];

	uint32_t wUnkEvtThreadId_2;
	FbatDatabaseNpcDeadInfo deadInfos[64];
	char wbBattleTripleCoinDrop;
	uint8_t wSandersBombTrigger;

	uint8_t gap_54e[0x2];

	FbatAttackAnnounceInfo attackAnnounceInfo;

	uint8_t gap_570[0x4];

	int32_t wKpaComboLength;
	int32_t wKpaScoreChangeCounter;
	int32_t wKpaTotalScore;
} __attribute__((__packed__));

static_assert(sizeof(FbatData) == 0x580);

extern "C" {

NpcWork *npcGetWorkPtr();

void npcReleaseFiledNpc();
void npcRecoveryFiledNpc();

void npcInit();
void npcReset(bool battle);
int32_t npcGetReactionOfLivingBody(bool battle);

int32_t npcEntry(const char *name, const char *modelName);
NpcTribeDescription *npcGetTribe(const char *tribeName);
void npcDelete(NpcEntry *npc);
void npcDeleteGroup(NpcEntry *npc);

void npcMain();
NpcEntry *npcNameToPtr(const char *name);
NpcEntry *npcNameToPtr_NoAssert(const char *name);

void npcSetMarioAutoTalkPose(const char *stayPose, const char *talkPose);
void npcSetTalkPose(const char *talkPose);
void npcSetStayPose(const char *stayPose);

void npcTuningRy(NpcEntry *npc, float rotationY);
float npcTransRytoFaceDir(const NpcEntry *npc);
// npcHitCheckSide
// npcNearDistCheck

FbatData *fbatGetPointer();
// void fbatChangeMode(uint32_t wMode);
// void fbatSetAttackAnnounce(uint32_t wUnk);
void fbatSetAttackAnnounceEnable();
void fbatEncountCheck();
void fbatTalkMode();
void fbatBattleMode();

void npcGroupDead(NpcEntry *npc, int32_t wKpaScoreType);

NpcEntry *fbatNpcTalkCheck();
NpcEntry *fbatHitCheckAll();
NpcEntry *fbatHitCheck();

bool npcCheckInterrupt(NpcEntry *npc);
// Reference to unimplemented type BattleSetupInfo
void npcSetupBattleInfo(NpcEntry *npc, void *battleInfo);
void npcSetBattleInfo(NpcEntry *npc, int32_t battleInfoId);

void npcStopForEvent();
void npcStopForOneEvent(NpcEntry *npc);
void npcStartForEvent();
void npcStartForOneEvent(NpcEntry *npc);

void npcSetColor(const char *name, const gc::color4 *color);
void npcBlurOn(const char *name);
void npcBlurOff(const char *name);
void *npcGetBtlSetupWork(NpcEntry *npc, int32_t index);
void npcSetBtlSetupWork(NpcEntry *npc, int32_t index, void *btlSetupWork);
void npcSetSlave(NpcEntry *npc, NpcEntry *slave, int32_t index);
void npcSetLink(NpcEntry *npc, NpcEntry *other);

uint32_t dbGetDefData(const database::DatabaseDefinition *definitions, const char *key);

void npcExecAllInitEvt();
bool npcWaitAllInitEvtEnd();
void npcExecAllReglEvt();

void npcGetBackItemEntry(NpcEntry *npc);

void npcClearDeadInfo();
void npcKoopaModeEncountCheck();
// Reference to unimplemented type HitEntry
int32_t npcKoopaModeMobjBoundDeadCheck(void *hit);
void fbatSandersBombTriggerOn();

int32_t npcCalcScore(const NpcEntry *npc);
bool npcCheckBlow(const NpcEntry *npc);

// fbatPreventMarioEventChk

}

}