#ifndef __WINMAIN_H__
#define __WINMAIN_H__

#define FOR_WORK
/*--------------------------------------------------------------------------------------*/

#define LANGUAGE_KOREAN			( 0 )
#define LANGUAGE_ENGLISH		( 1 )
#define LANGUAGE_TAIWANESE		( 2 )
#define LANGUAGE_CHINESE		( 3 )
#define LANGUAGE_JAPANESE		( 4 )
#define LANGUAGE_THAILAND		( 5 )
#define LANGUAGE_PHILIPPINES	( 6 )
#define LANGUAGE_VIETNAMESE		( 7 )
#define NUM_LANGUAGE			( 8 )

#define SELECTED_LANGUAGE	( LANGUAGE_ENGLISH )

#ifdef _DEBUG
#define ENABLE_EDIT
#define ENABLE_EDIT2
//#define DEBUG_BITMAP_CACHE
#endif // _DEBUG

#ifdef FOR_WORK
#ifdef _DEBUG

#define CSK_LH_DEBUG_CONSOLE
#ifdef CSK_LH_DEBUG_CONSOLE
#define CONSOLE_DEBUG
#endif // CSK_LH_DEBUG_CONSOLE

#define CSK_DEBUG_MAP_ATTRIBUTE

#define CSK_DEBUG_RENDER_BOUNDINGBOX

#define CSK_DEBUG_MAP_PATHFINDING

#endif // _DEBUG
#endif // FOR_WORK

#define WINDOWMODE

#define ACTIVE_FOCUS_OUT

//#define DEVIAS_XMAS_EVENT  //more snow in devias
//#define GUILD_WAR_EVENT
#define DUEL_SYSTEM
//#define CAMERA_TEST


#define WM_RECEIVE_BUFFER	( WM_USER + 2)
#define WM_NPROTECT_EXIT_TWO  (WM_USER + 10001)

extern bool ashies;
extern int weather;
extern float Time_Effect;
extern HWND      g_hWnd;
extern HINSTANCE g_hInst;
extern HDC       g_hDC;
extern HGLRC     g_hRC;
extern HFONT     g_hFont;
extern HFONT     g_hFontBold;
extern HFONT     g_hFontBig;
extern HFONT     g_hFixFont;
extern bool      Destroy;
extern int       RandomTable[];

extern wchar_t m_ID[11];
extern wchar_t m_Password[21];
extern wchar_t m_Version[];
extern int  m_SoundOnOff;
extern int  m_MusicOnOff;
extern int  m_Resolution;
extern int m_nColorDepth;
extern int m_RememberMe;
extern int g_MaxMessagePerCycle;
extern double CPU_AVG;

extern void SetMaxMessagePerCycle(int messages);
extern void CheckHack(void);
extern DWORD GetCheckSum(WORD wKey);
extern void StopMp3(char* Name, BOOL bEnforce = false);
extern void PlayMp3(char* Name, BOOL bEnforce = false);
extern bool IsEndMp3();
extern int GetMp3PlayPosition();

extern GLvoid KillGLWindow(GLvoid);
extern void DestroyWindow();
extern void DestroySound();

//#if defined _DEBUG || defined PBG_LOG_PACKET_WINSOCKERROR
//	#include "./Utilities/Log/DebugAngel.h"
//#define ExecutionLog	DebugAngel_Write
//#else
//#define ExecutionLog	{}
//#endif //_DEBUG

#define FAKE_CODE( pos)\
	_asm { jmp pos };\
	_asm { __emit 0xFF };\
	_asm { __emit 0x15 }

#endif
