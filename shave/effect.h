

#ifndef _EFFECT_H_
#define _EFFECT_H_

// 1: Includes
// ----------------------------------------------------------------------------
#include <mv_types.h>

// 2:  Exported Global Data (generally better to avoid)
// ----------------------------------------------------------------------------
// 3:  Exported Functions (non-inline)
// ----------------------------------------------------------------------------
void Brightness(u8* inPlane, u8* outPlane, int width, int height);
void BrightenLineAsm(u8* inLine, u8* outLine, int width);
//void CopyPlane(u8* inPlane, u8* outPlane, int width, int height);

#endif//__EFFECT_H__
