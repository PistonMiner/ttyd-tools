#pragma once

namespace ttyd::mario_motion
{

extern "C" {

void marioChgMoveMotion();
void marioChgStayMotion();
void marioChgTalkMotion();
void marioChgGetItemMotion();
void marioChgShipMotion();
void marioChgRollMotion();
void marioChgJumpStandMotion(float direction);
void marioChgSmallJumpMotion();

bool marioChkItemGetMotion();
bool marioChkTalkable();

// marioBoots
// marioMotion

void marioChgMot(int motionId);
void marioChgMotSub(int motionId, bool unk);
void marioChgMot2(int motionId);

void marioClearJumpPara();
void marioSetJumpPara();
void marioSetFallPara();
float marioGetFallSpd();
float marioMakeJumpPara();

// marioJump
// marioFall
// marioLandOn

bool marioChkJump();
bool marioChkTransform();
bool marioChkItemMotion();
// L_marioChkRub

int marioRollChgChk();

void marioChgMotJump2();


}

}