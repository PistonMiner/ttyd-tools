#pragma once

namespace ttyd::fontmgr {

extern "C" {

// fontmgrInit
// fontmgrTexSetup
void FontDrawStart();
// FontDrawStart_alpha
// FontDrawEdge
// FontDrawEdgeOff
// FontDrawRainbowColor
// FontDrawRainbowColorOff
// FontDrawNoise
// FontDrawNoiseOff
// FontDrawColorIDX
// FontDrawColor
// FontDrawColor_
// FontGetDrawColor
// FontDrawScale
// FontDrawScaleVec
// FontDrawCode
// FontDrawCodeMtx
// FontDrawString
// FontDrawStringPitch
// FontDrawStringVecPitch
// FontDrawStringMtx
// FontDrawStringCenterMtx
// FontDrawStringShake
void FontDrawMessage(int x, int y, const char *message);
// FontDrawMessageMtx
// hankakuSearch
// kanjiSearch
// kanjiGetWidth
// FontGetMessageWidthLine
// FontGetMessageWidth
// HSV2RGB

// JUTFontSetup
// JUTFont_Free
// JUTFont_CodeToGlyph
// JUTFont_DrawStart
// _JUTFont_DrawPos

}

}
