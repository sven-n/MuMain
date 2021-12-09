

#ifndef _PROTECTSYSKEY_H_
#define _PROTECTSYSKEY_H_

//. soyaviper

namespace ProtectSysKey {

	bool AttachProtectSysKey(HINSTANCE hInst, HWND hWnd);
	void DetachProtectSysKey();

	typedef struct tagKBDLLHOOKSTRUCT {
		DWORD   vkCode;
		DWORD   scanCode;
		DWORD   flags;
		DWORD   time;
		DWORD   dwExtraInfo;
	} KBDLLHOOKSTRUCT, FAR *LPKBDLLHOOKSTRUCT, *PKBDLLHOOKSTRUCT;

	class CProtectSysKey {
		HHOOK	m_hKeyboardHook;
		HWND	m_hWnd;
	public:
		~CProtectSysKey();
		
		bool AttachProtectSysKey(HINSTANCE hInst, HWND hWnd);
		void DetachProtectSysKey();

		HHOOK GetHookHandle() const;
		
		static CProtectSysKey* GetObjPtr();
		static LRESULT CALLBACK LowLevelKeyHookProc(int nCode, WPARAM wParam, LPARAM lParam);

	private:
		CProtectSysKey();	//. ban create instance
	};
}

#endif // _PROTECTSYSKEY_H_

