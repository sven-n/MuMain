#ifndef __ZZZTEXTURE_H__
#define __ZZZTEXTURE_H__

#include "GlobalBitmap.h"
//extern CGlobalBitmap Bitmaps;

extern bool OpenJpegBuffer(char *filename,float *BufferFloat);
extern bool WriteJpeg(char *filename,int Width,int Height,unsigned char *Buffer,int quality);
extern void SaveImage(int HeaderSize,char *Ext,char *filename,BYTE *PakBuffer,int Size);

#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
bool LoadBitmap(const char* szFileName, GLuint uiTextureIndex, GLuint uiFilter=GL_NEAREST, GLuint uiWrapMode= GL_CLAMP_TO_EDGE, bool bCheck = true, bool bFullPath = false);
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM
bool LoadBitmap(const char* szFileName, GLuint uiTextureIndex, GLuint uiFilter=GL_NEAREST, GLuint uiWrapMode=GL_CLAMP_TO_EDGE, bool bCheck = true);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
void DeleteBitmap(GLuint uiTextureIndex, bool bForce = false);
void PopUpErrorCheckMsgBox(const char* szErrorMsg, bool bForceDestroy = false);

#endif// __ZZZTEXTURE_H__
