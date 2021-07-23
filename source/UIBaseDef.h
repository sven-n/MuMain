//*****************************************************************************
// File: UIBaseDef.h
//
// Desc: UI 기본 정의 모음.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#ifndef _UI_BASE_DEF_H_
#define _UI_BASE_DEF_H_

#include "UsefulDef.h"

// 색상 정의.
// DWORD 타잎.
#define	CLRDW_WHITE			ARGB(255, 255, 255, 255)// 흰색.
#define CLRDW_BR_GRAY		ARGB(255, 226, 226, 226)// 밝은 회색.
#define CLRDW_GRAY			ARGB(255, 119, 119, 119)// 회색.
#define CLRDW_YELLOW		ARGB(255, 255, 255, 121)// 노란색.
#define CLRDW_BR_YELLOW		ARGB(255, 255, 238, 193)// 밝은 노란색.
#define CLRDW_BR_ORANGE		ARGB(255, 255, 217, 39)	// 밝은 오렌지색.
#define CLRDW_ORANGE		ARGB(255, 255, 180, 0)	// 오렌지색.
#define CLRDW_DARKYELLOW	ARGB(255, 255, 255, 0)	// 진한 노랑색
// COLORREF 타잎.
#define	CLRREF_WHITE		RGB(240, 240, 240)		// 흰색.
#define	CLRREF_BR_GRAY		RGB(210, 210, 210)		// 밝은 회색.
#define	CLRREF_GRAY			RGB(127, 127, 127)		// 회색.
#define	CLRREF_BLACK		RGB(  0,   0,   0)		// 검정색.
#define	CLRREF_RED			RGB(252,  60,  60)		// 빨간색.
#define	CLRREF_BR_RED		RGB(252, 128, 128)		// 밝은 빨간색.
#define	CLRREF_ORANGE		RGB(220, 170, 100)		// 주황색.
#define	CLRREF_YELLOW		RGB(210, 210, 100)		// 노란색.
#define	CLRREF_BR_YELLOW	RGB(240, 240, 170)		// 밝은 노란색.
#define	CLRREF_GREEN		RGB(100, 230, 100)		// 초록색.
#define	CLRREF_DK_BLUE		RGB( 50,  40, 255)		// 어두운 파란색.
#define	CLRREF_BLUE			RGB(101,  89, 255)		// 파란색.
#define	CLRREF_BR_BLUE		RGB(168, 148, 255)		// 밝은 파란색.
#define	CLRREF_VIOLET		RGB(210, 100, 210)		// 보라색.
#define	CLRREF_BR_OCHER		RGB(250, 225, 200)		// 밝은 황토색.

// 버텍스 위치. (좌상, 좌하, 우하, 우상, 최대값. GL_TRIANGLE_FAN 랜더 순서.)
enum VERTEX_POS { LT, LB, RB, RT, POS_MAX };
/// 변경할 매개 변수.
enum CHANGE_PRAM { X = 1, Y, XY };

struct SScrCoord	// 스크린 좌표.
{
	float	fX, fY;
};

struct STexCoord	// 텍스처 좌표.
{
	float	fTU, fTV;
};

struct SFrameCoord	// 택스처 내의 프래임 시작 픽셀 좌표.
{
	int		nX, nY;
};

struct SImgInfo		// 이미지 정보.
{
	int		nTexID;				// 텍스처 ID.
	int		nX, nY;				// 텍스처 내의 이미지 위치.
	int		nWidth, nHeight;	// 텍스처 내의 이미지 크기.
};

#endif // _UI_BASE_DEF_H_