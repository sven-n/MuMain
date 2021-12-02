#ifndef __WINMAIN_H__
#define __WINMAIN_H__

#define FOR_LAST_TEST
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



#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
#define ACTIVE_FOCUS_OUT
#endif	// USER_WINDOW_MODE

// 	#define LIMIT_CHAOS_MIX
// 	#define LIMIT_SKILL
	#define GRAMMAR_OF


//#define CSK_EVENT_HALLOWEEN_MAP
//#define DEVIAS_XMAS_EVENT
//#define LENA_EXCHANGE_ZEN
#define BLOODCASTLE_2ND_PATCH
#define USE_EVENT_ELDORADO
#define SCRATCH_TICKET
#define FRIEND_EVENT
#define FRIENDLYSTONE_EXCHANGE_ZEN

#define CASTLE_EVENT
#ifdef CASTLE_EVENT
	#undef FRIEND_EVENT
	#undef FRIENDLYSTONE_EXCHANGE_ZEN
	#undef LENA_EXCHANGE_ZEN
	#define DISABLE_MUTO
#endif

#define LOREN_RAVINE_EVENT
//#define GUILD_WAR_EVENT
//#define MYSTERY_BEAD

#define	PCROOM_EVENT
#define DUEL_SYSTEM
#define PET_SYSTEM
#define PROTECT_SYSTEMKEY
#define CHANGE_RATE
//#define SYSTEM_NOT_RENDER_TEXT
//#define ON_AIR_BLOODCASTLE
//#define CAMERA_TEST
//#define CAMERA_TEST_FPS
//#define DONTRENDER_TERRAIN


#define CRYINGWOLF_2NDMVP
#define GIFT_BOX_EVENT
#define BUGFIX_CRYWOLF_END_COUNT
#define WM_USER_MEMORYHACK	( WM_USER + 1)
#define WM_NPROTECT_EXIT_TWO  (WM_USER + 10001)

//#define PBG_ADD_INGAMESHOP_FONT

extern bool ashies;
extern int weather;
extern int Time_Effect;
extern HWND      g_hWnd;
extern HINSTANCE g_hInst;
extern HDC       g_hDC;
extern HGLRC     g_hRC;
extern HFONT     g_hFont;
extern HFONT     g_hFontBold;
extern HFONT     g_hFontBig;
extern HFONT     g_hFixFont;
#ifdef PBG_ADD_INGAMESHOP_FONT
extern HFONT	 g_hInGameShopFont;
#endif //PBG_ADD_INGAMESHOP_FONT
//extern HFONT     g_hFontSmall;
extern bool      Destroy;
extern int       RandomTable[];

extern char m_ID[];
extern char m_Version[];
extern int  m_SoundOnOff;
extern int  m_MusicOnOff;
extern int  m_Resolution;
extern int m_nColorDepth;	

extern void CheckHack( void);
extern DWORD GetCheckSum( WORD wKey);
extern void StopMp3(char *Name, BOOL bEnforce=false);
extern void PlayMp3(char *Name, BOOL bEnforce=false);
extern bool IsEndMp3();
#ifdef YDG_ADD_DOPPELGANGER_SOUND
extern int GetMp3PlayPosition();
#endif	// YDG_ADD_DOPPELGANGER_SOUND
extern unsigned int GenID();

extern void CloseMainExe( void);
extern GLvoid KillGLWindow(GLvoid);
extern void DestroyWindow();
extern void DestroySound();

#if defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)
	extern bool g_bUnfixedFixedFrame;
#endif // defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)

#if defined _DEBUG || defined PBG_LOG_PACKET_WINSOCKERROR
	#include "./Utilities/Log/DebugAngel.h"
#define ExecutionLog	DebugAngel_Write
#else
#define ExecutionLog	{}
#endif //_DEBUG

#define FAKE_CODE( pos)\
	_asm { jmp pos };\
	_asm { __emit 0xFF };\
	_asm { __emit 0x15 }


#endif
