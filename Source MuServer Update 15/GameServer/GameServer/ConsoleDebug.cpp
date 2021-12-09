#include "stdafx.h"
#include "ConsoleDebug.h"
#include "WindowsConsole.h"
#include "Util.h"

CmuConsoleDebug::CmuConsoleDebug() : m_bInit(false)
{
#ifdef CSK_LH_DEBUG_CONSOLE
	if (leaf::OpenConsoleWindow("Mu Debug Console Window"))
	{
		leaf::ActivateCloseButton(false);
		leaf::ShowConsole(true);
		m_bInit = true;
		LogAdd(LOG_BLACK, "Mu Debug Console Window Init - completed(Handle:0x%00000008X)\r\n", leaf::GetConsoleWndHandle());
	}
#endif
}

CmuConsoleDebug::~CmuConsoleDebug()
{
#ifdef CSK_LH_DEBUG_CONSOLE
	leaf::CloseConsoleWindow();
#endif
}

CmuConsoleDebug* CmuConsoleDebug::GetInstance()
{
#ifdef CSK_LH_DEBUG_CONSOLE
	static CmuConsoleDebug sInstance;
	return &sInstance;
#else
	return 0;
#endif 
}

void CmuConsoleDebug::Write(int iType, const char* pStr, ...)
{
#ifdef CSK_LH_DEBUG_CONSOLE
	if (m_bInit)
	{
		switch (iType)
		{
		case MCD_SEND:
			leaf::SetConsoleTextColor(leaf::COLOR_OLIVE);
			break;
		case MCD_RECEIVE:
			leaf::SetConsoleTextColor(leaf::COLOR_DARKGREEN);
			break;
		case MCD_ERROR:
			leaf::SetConsoleTextColor(leaf::COLOR_WHITE, leaf::COLOR_DARKRED);
			break;
		case MCD_NORMAL:
			leaf::SetConsoleTextColor(leaf::COLOR_GRAY);
			break;
		}

		char szBuffer[256] = "";
		va_list	pArguments;

		va_start(pArguments, pStr);
		vsprintf(szBuffer, pStr, pArguments);
		va_end(pArguments);

		std::cout << szBuffer << std::endl;
	}
#endif
}
