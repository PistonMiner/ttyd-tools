#pragma once

#include "types.h"

#include <cstdint>

namespace gc::mtx {

extern "C" {

void PSMTXIdentity(mat3x4 *m);
void PSMTXCopy(mat3x4 *src, mat3x4 *dst);
void PSMTXConcat(mat3x4 *lhs, mat3x4 *rhs, mat3x4 *dst);
void PSMTXInverse(mat3x4 *src, mat3x4 *dst);
void PSMTXInvXpose(mat3x4 *src, mat3x4 *dst);
void PSMTXRotRad(mat3x4 *dst, char axis, float radians);
void PSMTXRotTrig(mat3x4 *dst, char axis, float sin, float cos);
// local: __PSMTXRotAxisRadInternal
void PSMTXRotAxisRad(mat3x4 *dst, vec3 *axis);
void PSMTXTrans(mat3x4 *dst, float tx, float ty, float tz);
void PSMTXTransApply(mat3x4 *src, mat3x4 *dst, float tx, float ty, float tz);
void PSMTXScale(mat3x4 *dst, float sx, float sy, float sz);
void PSMTXScaleApply(mat3x4 *src, mat3x4 *dst, float sx, float sy, float sz);

void C_MTXLookAt(mat3x4 *dst, vec3 *pos, vec3 *up, vec3 *targetPos);

void C_MTXLightFrustum(mat3x4 *dst, float top, float bottom, float left, float right, float near, float su, float sv, float tu, float tv);
void C_MTXLightPerspective(mat3x4 *dst, float fovY, float aspectRatio, float su, float sv, float tu, float tv);
void C_MTXLightOrtho(mat3x4 *dst, float top, float bottom, float left, float right, float su, float sv, float tu, float tv);

void PSMTXMultVec(mat3x4 *m, vec3 *src, vec3 *dst);
void PSMTXMultVecSR(mat3x4 *m, vec3 *src, vec3 *dst);

void C_MTXFrustum(mat4x4 *m, float top, float bottom, float left, float right, float near, float far);
void C_MTXPerspective(mat4x4 *m, float fovY, float aspectRatio, float near, float far);
void C_MTXOrtho(mat4x4 *m, float top, float bottom, float left, float right, float near, float far);

void PSMTX44Copy(mat4x4 *src, mat4x4 *dst);
void PSMTX44Concat(mat4x4 *lhs, mat4x4 *rhs, mat4x4 *dst);
void PSMTX44Trans(mat4x4 *dst, float tx, float ty, float tz);
void PSMTX44Scale(mat4x4 *dst, float sx, float sy, float sz);
void PSMTX44MultVec(mat4x4 *m, vec3 *src, vec3 *dst);

void PSVECAdd(vec3 *lhs, vec3 *rhs, vec3 *dst);
void PSVECSubtract(vec3 *lhs, vec3 *rhs, vec3 *dst);
void PSVECScale(vec3 *src, vec3 *dst, float scale);
void PSVECNormalize(vec3 *src, vec3 *dst);
float PSVECSquareMag(vec3 *v);
float PSVECMag(vec3 *v);
float PSVECDotProduct(vec3 *lhs, vec3 *rhs);
void PSVECCrossProduct(vec3 *lhs, vec3 *rhs, vec3 *dst);
void C_VECHalfAngle(vec3 *lhs, vec3 *rhs, vec3 *dst);
void C_VECReflect(vec3 *incident, vec3 *normal, vec3 *dst);
float PSVECSquareDistance(vec3 *lhs, vec3 *rhs);
float PSVECDistance(vec3 *lhs, vec3 *rhs);

}

}