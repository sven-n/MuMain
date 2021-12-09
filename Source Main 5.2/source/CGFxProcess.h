#pragma once

#ifdef LDK_ADD_SCALEFORM

#include "GSysFile.h"
#include "GFxPlayer.h"
#include "CGFxFontConfigParser.h"

static struct
{
	WPARAM winKey;
	GFxKey::Code appKey;
} KeyCodeMap[] =
{
	{VK_BACK,    GFxKey::Backspace},
	{VK_TAB,     GFxKey::Tab},
	{VK_CLEAR,   GFxKey::Clear},
	{VK_RETURN,  GFxKey::Return},
	{VK_SHIFT,   GFxKey::Shift},
	{VK_CONTROL, GFxKey::Control},
	{VK_MENU,    GFxKey::Alt},
	{VK_PAUSE,   GFxKey::Pause},
	{VK_CAPITAL, GFxKey::CapsLock},
	{VK_ESCAPE,  GFxKey::Escape},
	{VK_SPACE,   GFxKey::Space},
	{VK_PRIOR,   GFxKey::PageUp},
	{VK_NEXT,    GFxKey::PageDown},
	{VK_END,     GFxKey::End},
	{VK_HOME,    GFxKey::Home},
	{VK_LEFT,    GFxKey::Left},
	{VK_UP,      GFxKey::Up},
	{VK_RIGHT,   GFxKey::Right},
	{VK_DOWN,    GFxKey::Down},
	{VK_INSERT,  GFxKey::Insert},
	{VK_DELETE,  GFxKey::Delete},
	{VK_HELP,    GFxKey::Help},
	{VK_NUMLOCK, GFxKey::NumLock},
	{VK_SCROLL,  GFxKey::ScrollLock},

	{VK_OEM_1,     GFxKey::Semicolon},
	{VK_OEM_PLUS,  GFxKey::Equal},
	{VK_OEM_COMMA, GFxKey::Comma},
	{VK_OEM_MINUS, GFxKey::Minus},
	{VK_OEM_PERIOD,GFxKey::Period},
	{VK_OEM_2,     GFxKey::Slash},
	{VK_OEM_3,     GFxKey::Bar},
	{VK_OEM_4,     GFxKey::BracketLeft},
	{VK_OEM_5,     GFxKey::Backslash},
	{VK_OEM_6,     GFxKey::BracketRight},
	{VK_OEM_7,     GFxKey::Quote}
};

enum KeyModifiers
{
	KM_Control = 0x1,
	KM_Alt     = 0x2,
	KM_Shift   = 0x4,
	KM_Num     = 0x8,
	KM_Caps    = 0x10,
	KM_Scroll  = 0x20
};

// member Class  등록 타입
enum GFxRegistType
{
	eGFxRegist_NONE = 0,

	eGFxRegist_MainUI,
	eGFxRegist_InfoPupup,

	eGFxRegist_ImageTest,

	eGFxRegist_MAX_COUNT,
};

class CGFXBase
{
public:
	CGFXBase() {}
	virtual ~CGFXBase() {}

	virtual bool OnCreateDevice(SInt bufw, SInt bufh, SInt left, SInt top, SInt w, SInt h, UInt flags = 0) { return FALSE; }
	virtual bool OnResetDevice()	{ return FALSE; }
	virtual bool OnLostDevice()		{ return FALSE; }
	virtual bool OnDestroyDevice()	{ return FALSE; }

	virtual bool Init()		{ return FALSE; }
	virtual bool Update()	{ return FALSE; }
	virtual bool Render()	{ return FALSE; }
	virtual bool RenderModel()	{ return FALSE; }

	virtual GFxMovieView* GetMovie()	{ return NULL; }

	virtual bool IsVisible()				{ return FALSE; }
	virtual void SetVisible(bool _visible)	{}

	virtual bool IsLock()					{ return FALSE; }
	virtual void SetUnLockVisible()			{}
	virtual void SetLockVisible(bool _visible) {}
};

//--------------------------------------------------------------------------------------

typedef map< GFxRegistType, CGFXBase* > mapGFXContainer;

class GFxProcess
{
public:
	static GFxProcess* GetInstancePtr();
	virtual ~GFxProcess();

private:
	GFxProcess();

public:
	CGFXBase* Find(GFxRegistType key);

public:
	void OnCreateDevice(SInt bufw, SInt bufh, SInt left, SInt top, SInt w, SInt h, UInt flags = 0);
	void OnResetDevice();
	void OnLostDevice();
	void OnDestroyDevice();

	bool GFxInit();
	void GFxDestroy();
	void GFxUpdate(int _scene);
	void GFxRender();

	void GFxProcessEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool *pbNoFurtherProcessing);

	//핫키 등록
	//skill
	void SetSkillClearHotKey();
	void SetSkillHotKey(int iHotKey, int _skillType);
	int GetSkillHotKey(int iHotKey);

	//item
	void SetItemClearHotKey();
	void SetItemHotKey(int iHotKey, int iItemType, int iItemLevel);
	int GetItemHotKey(int iHotKey);
	int GetItemHotKeyLevel(int iHotKey);

	//uiSelection
	void SetUISelect(int _iUISelection) { m_iUISelection = _iUISelection; }
	int GetUISelect() { return m_iUISelection; }

private:
	//폰트 로딩
	void LoadFontConfigs(ConfigParser *parser);
	int GetFontConfigIndexByName(const char* pname);
	FontConfig* GetCurrentFontConfig();
	bool LoadDefaultFontConfigFromPath( const GString& path );

	//scene 관리
	void GFxSceneInit_LogIn(GFxRegistType _type, CGFXBase* _gfx);
	void GFxSceneInit_Char(GFxRegistType _type, CGFXBase* _gfx);
	void GFxSceneInit_Main(GFxRegistType _type, CGFXBase* _gfx);

	void GFxSceneControll_LogIn(GFxRegistType _type, CGFXBase* _gfx);
	void GFxSceneControll_Char(GFxRegistType _type, CGFXBase* _gfx);
	void GFxSceneControll_Main(GFxRegistType _type, CGFXBase* _gfx);

private:
	mapGFXContainer m_mapGfxContainer;
	bool m_isCompletedLoad;

	int m_iNowSceneFlag;

	//해상도 정보
	int m_iWidth;
	int m_iHeight;

	//ui = 0 : old ui / ui = 1 : new ui
	int m_iUISelection;

	//폰트 관련 변수등
	bool m_bNoFontConfig;
	// FontConfigFile
	GString		m_FontConfigFilePath;
	GFileStat	m_FontConfigFileStats;
	// Font configurations, if specified.
	FontConfigSet m_FontConfigs;
	// Index of currently applied FontConfig, -1 for none.
	SInt m_FontConfigIndex;
};

//--------------------------------------------------------------------------------------

inline void EncodeUtf8(char* _out, const char* _in)
{
	GString _temp = _in;
	wchar_t *chTemp;

	int strSize = MultiByteToWideChar(CP_ACP, 0, _temp.ToCStr(), -1, NULL, NULL);
	chTemp = new WCHAR[strSize];
	MultiByteToWideChar(CP_ACP, 0, _temp.ToCStr(), _temp.GetSize()+1, chTemp, strSize);

	GUTF8Util::EncodeString(_out, chTemp, strSize);
	delete chTemp; 
	chTemp = NULL;
}

inline void DecodeUtf8(char* _out, const wchar_t* _in)
{
//	GUTF8Util::DecodeString(wTemp, _temp.ToCStr(), strSize);
}

static int GetModifiers()
{
	int new_mods = 0;
	if (::GetKeyState(VK_SHIFT) & 0x8000)
		new_mods |= KM_Shift;
	if (::GetKeyState(VK_CONTROL) & 0x8000)
		new_mods |= KM_Control;
	if (::GetKeyState(VK_MENU) & 0x8000)
		new_mods |= KM_Alt;
	if (::GetKeyState(VK_NUMLOCK) & 0x1)
		new_mods |= KM_Num;
	if (::GetKeyState(VK_CAPITAL) & 0x1)
		new_mods |= KM_Caps;
	if (::GetKeyState(VK_SCROLL) & 0x1)
		new_mods |= KM_Scroll;
	return new_mods;
}

inline void KeyEvent(GFxMovieView *pMovie, GFxKey::Code keyCode, unsigned char asciiCode,
					 unsigned int wcharCode, unsigned int mods, bool down)
{
	if(pMovie == NULL)
		return;

	if (keyCode != GFxKey::VoidSymbol)
	{
		GFxKeyEvent keyEvent(down ? GFxEvent::KeyDown : GFxKeyEvent::KeyUp, keyCode, asciiCode, wcharCode);
		keyEvent.SpecialKeysState.SetShiftPressed(mods & KM_Shift ? 1 : 0);
		keyEvent.SpecialKeysState.SetCtrlPressed(mods & KM_Control ? 1 : 0);
		keyEvent.SpecialKeysState.SetAltPressed(mods & KM_Alt ? 1 : 0);
		keyEvent.SpecialKeysState.SetNumToggled(mods & KM_Num ? 1 : 0);
		keyEvent.SpecialKeysState.SetCapsToggled(mods & KM_Caps ? 1 : 0);
		keyEvent.SpecialKeysState.SetScrollToggled(mods & KM_Scroll ? 1 : 0);
		pMovie->HandleEvent(keyEvent);
	}
}

inline void OnKey(GFxMovieView *pMovie, GFxKey::Code keyCode, unsigned char asciiCode,
				  unsigned int wcharCode, unsigned int mods, bool downFlag)
{
	if(pMovie == NULL)
		return;

	//제한적인 키사용을 원한다면 ....
// 	// See if CTRL is held down. If the event is the acual CTRL key going
// 	// up or down, ignore it.
// 	bool ctrl = mods & KM_Control;
// // 	if(keyCode == GFxKey::Control)
// // 		return;
// 
// 	if (ctrl && downFlag)
// 	{
// 		// Todo: Application can handle Ctrl-Key combinations here
// 	}
// 
// 	// Change to if(!ctrl && downFlag) to filter out key up events
// 	if(!ctrl)
// 	{
// 		// Inform the player about keystroke
// 		if (!ctrl)
// 			KeyEvent(pMovie, (GFxKey::Code)keyCode, asciiCode, wcharCode, mods, downFlag);
// 	}

	//모든키 사용가능
	KeyEvent(pMovie, (GFxKey::Code)keyCode, asciiCode, wcharCode, mods, downFlag);

}

inline void ProcessKeyEvent(GFxMovieView *pMovie, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(pMovie == NULL)
		return;

	if(uMsg == WM_CHAR)
	{
		UInt32 wcharCode = (UInt32)wParam;
		GFxCharEvent charEvent(wcharCode);
		pMovie->HandleEvent(charEvent);
		return;
	}

	GFxKey::Code kc = GFxKey::VoidSymbol;
	if (wParam >= 'A' && wParam <= 'Z')
	{
		kc = (GFxKey::Code) ((wParam - 'A') + GFxKey::A);
	}
	else if (wParam >= VK_F1 && wParam <= VK_F15)
	{
		kc = (GFxKey::Code) ((wParam - VK_F1) + GFxKey::F1);
	}
	else if (wParam >= '0' && wParam <= '9')
	{
		kc = (GFxKey::Code) ((wParam - '0') + GFxKey::Num0);
	}
	else if (wParam >= VK_NUMPAD0 && wParam <= VK_DIVIDE)
	{
		kc = (GFxKey::Code) ((wParam - VK_NUMPAD0) + GFxKey::KP_0);
	}
	else
	{
		for (int i = 0; KeyCodeMap[i].winKey != 0; i++)
		{
			if (wParam == (UInt)KeyCodeMap[i].winKey)
			{
				kc = KeyCodeMap[i].appKey;
				break;
			}
		}
	}
	unsigned char asciiCode = 0;
	if (kc != GFxKey::VoidSymbol)
	{
		// get the ASCII code, if possible.
		UINT uScanCode = ((UINT)lParam >> 16) & 0xFF; // fetch the scancode
		BYTE ks[256];
		WORD charCode;

		// Get the current keyboard state
		::GetKeyboardState(ks);

		if (::ToAscii((UINT)wParam, uScanCode, ks, &charCode, 0) > 0)
		{
			asciiCode = LOBYTE (charCode);
		}
	}
	bool downFlag = (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) ? 1 : 0;
	OnKey(pMovie, kc, asciiCode, 0, GetModifiers(), downFlag);
}

#endif //LDK_ADD_SCALEFORM
