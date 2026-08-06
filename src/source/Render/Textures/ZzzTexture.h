#ifndef __ZZZTEXTURE_H__
#define __ZZZTEXTURE_H__

#include "Render/Sprites/GlobalBitmap.h"
//extern CGlobalBitmap Bitmaps;

bool OpenJpegBuffer(wchar_t* filename, float* BufferFloat);
// DXP-12: bottomUp defaults true to match every pre-existing caller's row order
// (raw glReadPixels/legacy buffers are bottom-up); pass false for a buffer that's
// already top-down (e.g. RHI::ReadColorFramebuffer's contract).
bool WriteJpeg(wchar_t* filename, int Width, int Height, unsigned char* Buffer, int quality, bool bottomUp = true);
void SaveImage(int HeaderSize, wchar_t* Ext, wchar_t* filename, BYTE* PakBuffer, int Size);

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
bool LoadBitmap(const wchar_t* szFileName, GLuint uiTextureIndex, GLuint uiFilter = GL_NEAREST, GLuint uiWrapMode = GL_CLAMP_TO_EDGE, bool bCheck = true, bool bFullPath = false);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
bool LoadBitmap(const wchar_t* szFileName, GLuint uiTextureIndex, GLuint uiFilter = GL_NEAREST, GLuint uiWrapMode = GL_CLAMP_TO_EDGE, bool bCheck = true);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
void DeleteBitmap(GLuint uiTextureIndex, bool bForce = false);
void PopUpErrorCheckMsgBox(const wchar_t* szErrorMsg, bool bForceDestroy = false);

#endif// __ZZZTEXTURE_H__
