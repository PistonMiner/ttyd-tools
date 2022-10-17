#pragma once

#include <cstdint>

namespace ttyd::hitdrv {

struct HitCheckQuery
{
	uint32_t bCurrentHitSingleSided;
	uint32_t wUser0;
	uint32_t wUser1;
	gc::vec3 targetPosition;
	gc::vec3 targetDirection;
	gc::vec3 hitPosition;
	gc::vec3 hitNormal;
	float inOutTargetDistance;
} __attribute__((__packed__));

static_assert(sizeof(HitCheckQuery) == 0x40);

struct HitEntryTriangleData
{
	gc::vec3 worldSpaceVertex0Position;
	gc::vec3 worldSpaceVertex1Position;
	gc::vec3 worldSpaceVertex2Position;
	gc::vec3 edgeV0V2;
	gc::vec3 edgeV1V0;
	gc::vec3 edgeV2V1;
	gc::vec3 realNormal;
} __attribute__((__packed__));

static_assert(sizeof(HitEntryTriangleData) == 0x54);

struct HitDamageReturn
{
	const char *mapobjName;
	gc::vec3 returnPosition;
} __attribute__((__packed__));

static_assert(sizeof(HitDamageReturn) == 0x10);

enum HitEntryFlags
{
	HitEntryFlags_ShipOnly = 0x80,

	HitEntryFlags_MaterialShallowWater = 0x100,
	HitEntryFlags_WaterHazard = 0x200,
	HitEntryFlags_Unk1Hazard = 0x400,
	HitEntryFlags_Unk2Hazard = 0x800,
	HitEntryFlags_UnkMaterial3_waterRelated = 0x1000,

	HitEntryFlags_UnkMaterial1 = 0x100000,
	HitEntryFlags_UnkMaterial2 = 0x200000,
	HitEntryFlags_MaterialMetal = 0x400000,

	HitEntryFlags_Bero = 0x800000,

	HitEntryFlags_DamageReturnSet = 0x40000000,
};

struct HitEntry
{
	uint16_t flags;

	uint8_t pad_2[0x2];

	uint32_t attributes;
	void *pSourceJoint; // Reference to unimplemented type MapDataFileJoint
	gc::mat3x4 localSpaceToWorldSpaceMatrix;
	gc::mat3x4 wTransformMtx_2;

	uint8_t gap_6c[0x30];

	gc::vec3 localSpaceOrigin;
	uint16_t triangleDataCount;
	int16_t mapSubgroupIndex;
	HitEntryTriangleData *pTriangleData;
	HitDamageReturn *pDamageReturn;

	uint8_t gap_b4[0xc];

	gc::vec3 worldSpaceOrigin;
	float worldSpaceBboxRadius;
	void *pMobj;  // Reference to unimplemented type MobjEntry
	HitEntry *pParent;
	HitEntry *pChild;
	HitEntry *pNext;
	HitEntry *pNextActive;
} __attribute__((__packed__));

static_assert(sizeof(HitEntry) == 0xe4);

typedef int32_t (* PFN_HitFilterFunction)(HitCheckQuery* query, HitEntry* hit);

extern "C" {

void hitInit();
void hitReInit();

// local: _hitEnt
// local: hitEntrySub
HitEntry *hitEntry(void *pJoint, gc::mat3x4 *mat, int supgroupIndex);
HitEntry *hitEntryMOBJ(void *pJoint, gc::mat3x4 *mat);
void hitDelete(const char *mapobjName);

void hitMain();

void hitObjFlagOn(const char *mapobjName, uint16_t flags);
void hitObjFlagOff(const char *mapobjName, uint16_t flags);
// local: hitFlgOn
// local: hitFlgOff
void hitGrpFlagOn(const char *mapobjName, uint16_t flags);
void hitGrpFlagOff(const char *mapobjName, uint16_t flags);
void hitObjAttrOn(const char* mapobjName, uint32_t attr);
void hitObjAttrOff(const char* mapobjName, uint32_t attr);
// local: hitAtrOn
// local: hitAtrOff
void hitGrpAttrOn(const char* mapobjName, uint32_t attr);
void hitGrpAttrOff(const char* mapobjName, uint32_t attr);

void hitReCalcMatrix(HitEntry *hit, gc::mat3x4 *mat);
// local: hitReCalcMatrix2
// local: hitCalcVtxPosition

HitEntry* hitCheckVecFilter(
	HitCheckQuery* pQuery,
	PFN_HitFilterFunction filterFunction
);
HitEntry* hitCheckFilter(
	PFN_HitFilterFunction filterFunction,
	float* pHitX, float* pHitY, float* pHitZ,
	float* pInOutDistance,
	float* pHitNX, float* pHitNY, float* pHitNZ,
	float x, float y, float z,
	float nx, float ny, float nz
);
// local: checkTriVec_xz
int hitCheckVecHitObjXZ(HitCheckQuery *pQuery, HitEntry *hit);
// local: chkFilterAttr
HitEntry* hitCheckAttr(
	uint32_t bitMask,
	float* pOutPositionX, float* pOutPositionY, float* pOutPositionZ,
	float* pInOutDistance,
	float* pOutNormalX, float* pOutNormalY, float* pOutNormalZ,
	float targetPosX, float targetPosY, float targetPosZ,
	float targetNormX, float targetNormY, float targetNormZ
);
// hitCheckSphereFilter

HitEntry *hitNameToPtr(const char *mapobjName);

// local: hitObjGetPosSub
void hitObjGetPos(const char *mapobjName, gc::vec3 *pOutPosition);
void hitObjGetNormal(const char *mapobjName, gc::vec3 *pOutNormal);
char *hitGetName(HitEntry *hit);
uint32_t hitGetAttr(HitEntry *hit);

// local: hitDamageReturnSet
void hitGrpDamageReturnSet(const char *mapobjName, HitDamageReturn *pDamageReturn);
gc::vec3 *hitGetDamageReturnPos(HitEntry *hit);

void hitBindMapObj(const char *mapobjName, const char *bindMapObjName);
void hitBindUpdate(const char *mapobjName);

}

}