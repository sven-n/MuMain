#ifndef _WINDOWSCONSOLE_H_
#define _WINDOWSCONSOLE_H_

#pragma once

#include <windows.h>
#include "./Time/Timer.h"

#pragma warning(disable : 4786)
#include <string>

namespace leaf {

	enum COLOR_INDEX
	{
		COLOR_BLACK = 0,
			
		COLOR_DARKBLUE	= FOREGROUND_BLUE,
		COLOR_DARKGREEN = FOREGROUND_GREEN,
		COLOR_DARKRED	= FOREGROUND_RED,
		COLOR_INTENSITY = FOREGROUND_INTENSITY,
			
		COLOR_BLUE	= COLOR_DARKBLUE | COLOR_INTENSITY,
		COLOR_GREEN = COLOR_DARKGREEN | COLOR_INTENSITY,
		COLOR_RED	= COLOR_DARKRED | COLOR_INTENSITY,
		COLOR_OLIVE = COLOR_DARKRED | COLOR_DARKGREEN,
		COLOR_TEAL = COLOR_DARKGREEN | COLOR_DARKBLUE,
		COLOR_PURPLE = COLOR_DARKRED | COLOR_DARKBLUE,
		COLOR_GRAY = COLOR_DARKRED | COLOR_DARKGREEN | COLOR_DARKBLUE,
		COLOR_YELLOW = COLOR_OLIVE | COLOR_INTENSITY,
		COLOR_AQUA = COLOR_TEAL | COLOR_INTENSITY,
		COLOR_FUCHSIA = COLOR_PURPLE | COLOR_INTENSITY,
		COLOR_WHITE = COLOR_GRAY | COLOR_INTENSITY,

		COLOR_ERROR = 0xFFFF
	};


	/* Public Console Functions */

	bool OpenConsoleWindow(const std::string& title);
	void CloseConsoleWindow();
	
	bool SetConsoleTitle(const std::string& title);
	const std::string& GetConsoleTitle();
	
	HWND GetConsoleWndHandle();

	bool IsConsoleVisible();
	void ShowConsole(bool bShow = true);
	
	void ClearConsoleScreen();

	WORD GetConsoleTextColorIndex(WORD* pwBgColorIndex = NULL);
	void SetConsoleTextColor(WORD wTextColorIndex = COLOR_WHITE, WORD wBgColorIndex = COLOR_BLACK);
	
	void ActivateCloseButton(bool bActive = true);
	bool IsActiveCloseButton();

	bool SaveConsoleScreenBuffer(const std::string& filename);


	/* CConsoleWindow singleton class */

	class CConsoleWindow
	{
		CTimer2	m_LimitTimer;
		HWND	m_hWnd;
		bool	m_bActiveCloseButton;
		
	public:
		~CConsoleWindow();

		bool Open(const std::string& title);
		void Close();

		bool SetTitle(const std::string& title);
		const std::string& GetTitle();

		HWND GetWndHandle();

		bool IsVisible();
		void Show(bool bShow = true);

		void ClearScreen();

		WORD GetTextColorIndex(WORD* pwBgColorIndex = NULL);
		void SetTextColor(WORD wTextColorIndex = COLOR_WHITE, WORD wBgColorIndex = COLOR_BLACK);
		
		void ActivateCloseButton(bool bActive = true);
		bool IsActiveCloseButton() const;
		
		bool SaveScreenBuffer(const std::string& filename);

		static CConsoleWindow* GetInstance();
			
	protected:
		CConsoleWindow();	//. Ban to create instance

		void SetWndHandle(HWND hWnd);
		DWORD Get32ColorFromColorIndex(WORD wColorIndex);
		static BOOL CALLBACK EnumChildProc(HWND hWnd, LPARAM lParam);
		static BOOL CALLBACK EnumThreadWndProc(HWND hWnd, LPARAM lParam);
	};
	
}

#endif	// _WINDOWSCONSOLE_H_
