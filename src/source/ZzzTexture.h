#ifndef __ZZZTEXTURE_H__
#define __ZZZTEXTURE_H__

#include "GlobalBitmap.h"
//extern CGlobalBitmap Bitmaps;

bool OpenJpegBuffer(wchar_t* filename, float* BufferFloat);
bool WriteJpeg(wchar_t* filename, int Width, int Height, unsigned char* Buffer, int quality);
void SaveImage(int HeaderSize, wchar_t* Ext, wchar_t* filename, BYTE* PakBuffer, int Size);

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
bool LoadBitmap(const wchar_t* szFileName, GLuint uiTextureIndex, GLuint uiFilter = GL_NEAREST, GLuint uiWrapMode = GL_CLAMP_TO_EDGE, bool bCheck = true, bool bFullPath = false);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
bool LoadBitmap(const wchar_t* szFileName, GLuint uiTextureIndex, GLuint uiFilter = GL_NEAREST, GLuint uiWrapMode = GL_CLAMP_TO_EDGE, bool bCheck = true);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
void DeleteBitmap(GLuint uiTextureIndex, bool bForce = false);
void PopUpErrorCheckMsgBox(const wchar_t* szErrorMsg, bool bForceDestroy = false);

#endif// __ZZZTEXTURE_H__
