#pragma once

namespace ttyd::animdrv {

struct AnimWork;
struct AnimPose;

extern "C" {

AnimWork *animGetPtr();
int64_t animTimeGetTime(int32_t bBattle);

void animInit();
void animMain();
void animPoseRefresh();
void animPoseBattleInit();

int32_t animPoseEntry(const char *agbName, int32_t group);
int32_t animPaperPoseEntry(const char *agbName, int32_t group);

bool animEffectAsync(const char *agbName, int32_t group); // XXX: Bool return okay?

void animPosePeraOn(int32_t poseId);
void animPosePeraOff(int32_t poseId);
void animPosePaperPeraOn(int32_t poseId);
void animPosePaperPeraOff(int32_t poseId);

void animPoseSetLocalTimeRate(int32_t poseId, float localTimeRate);
void animPoseSetLocalTime(int32_t poseId, float localTimeFrames);
void animPoseSetStartTime(int32_t poseId, uint32_t unused, int64_t startTime); // XXX: Confirm this does correct parm layout

void animPoseSetAnim(int32_t poseId, const char *animName, int32_t bForceReset);
int32_t animPaperPoseGetId(const char *agbName, int32_t group);
void animPoseSetPaperAnimGroup(int32_t poseId, const char *agbName, int32_t bUnkOn);
void animPoseSetPaperAnim(int32_t poseId, const char *animName);
void animPoseSetEffect(int32_t poseId, const char *agbName, int32_t wbUnk);
void animPoseSetEffectAnim(int32_t poseId, const char *animName, int32_t bForceReset);
void animPoseSetGXFunc(int32_t poseId, void (*gxCb)(int32_t wXluStage), int32_t wbDisableDraw);

float animPoseGetRadius(int32_t poseId);
float animPoseGetHeight(int32_t poseId);
float animPoseGetLoopTimes(int32_t poseId);
bool animPoseGetPeraEnd(int32_t poseId);

void animPoseSetMaterialFlagOn(int32_t poseId, uint32_t mask);
void animPoseSetMaterialFlagOff(int32_t poseId, uint32_t mask);
void animPoseSetMaterialLightFlagOn(int32_t poseId, uint32_t mask);
void animPoseSetMaterialLightFlagOff(int32_t poseId, uint32_t mask);
void animPoseSetMaterialEvtColor(int32_t poseId, const gc::color4 *color);
uint32_t animPoseGetMaterialFlag(int32_t poseId);
uint32_t animPoseGetMaterialLightFlag(int32_t poseId);
gc::color4 animPoseGetMaterialEvtColor(int32_t poseId);

void animPoseMain(int32_t poseId);
void animPoseDraw(int32_t poseId, int32_t wXluStage, float x, float y, float z, float rotY, float scale);
void _animPoseDrawMtx(AnimPose *pose, const gx::mat3x4 *mat, int32_t wXluStage, float rotY, float scale);
void animPoseDrawMtx(int32_t poseId, const gc::mat3x4 *mat, int32_t wXluStage, float rotY, float scale);
// animSetPaperTexObj

void animPoseRelease(int32_t poseId);
void animPaperPoseRelease(int32_t poseId);
void animPoseAutoRelease(int32_t group);

// animPoseDisp_MakeExtTexture
void animSetPaperTexMtx(const gc::mat3x4 *mat0, const gc::mat3x4 *mat1, const gc::mat3x4 *mat2);

bool animGroupBaseAsync(const char *name, int32_t group, void (*agbFileLoadedCb)(void *file));

AnimPose *animPoseGetAnimPosePtr(int32_t poseId);
void *animPoseGetAnimDataPtr(int32_t poseId);
void *animPoseGetAnimBaseDataPtr(int32_t poseId);
char *animPoseGetCurrentAnim(int32_t poseId);
bool animPoseTestXLU(int32_t poseId);

void animPoseWorldPositionEvalOn(int32_t poseId);
void animPoseWorldPositionEvalOff(int32_t poseId);
void animPoseWorldMatrixEvalOn(int32_t poseId);

void animPoseVivianMain(int32_t poseId, const gc::vec3 *position);
int32_t animPoseGetVivianType(int32_t poseId);

void animPoseSetDispCallBack(int32_t poseId, void (*dispCb)(void *user, int32_t groupId, gx::mat3x4 *mat), void *user);
void animPoseDrawShape(int32_t poseId, int32_t shapeId);
int32_t animPoseGetShapeIdx(int32_t poseId, const char *shapeName);
int32_t animPoseGetGroupIdx(int32_t poseId, const char *groupName);
char *animPoseGetGroupName(int32_t poseId, int32_t groupId);

}

}