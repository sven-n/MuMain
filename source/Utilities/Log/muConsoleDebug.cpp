// muDebugHelper.cpp: implementation of the CmuConsoleDebug class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "muConsoleDebug.h"	// self
#include <iostream>
#include "ZzzInterface.h"
#include "ZzzOpenglUtil.h"
#include "WindowsConsole.h"
#include "./Utilities/Log/ErrorReport.h"
#include "GlobalBitmap.h"
#include "ZzzTexture.h"
#ifdef CSK_DEBUG_MAP_PATHFINDING
#include "ZzzPath.h"
#endif // CSK_DEBUG_MAP_PATHFINDING
#ifdef CSK_HACK_TEST
#include "wsclientinline.h"
#include "HackTest.h"
#endif // CSK_HACK_TEST


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef CSK_LH_DEBUG_CONSOLE

extern CGlobalBitmap Bitmaps;

CmuConsoleDebug::CmuConsoleDebug() : m_bInit(false)
{
	if(leaf::OpenConsoleWindow("Mu Debug Console Window"))
	{
		leaf::ActivateCloseButton(false);
		leaf::ShowConsole(false);
		m_bInit = true;
		
		g_ErrorReport.Write("Mu Debug Console Window Init - completed(Handle:0x%00000008X)\r\n", leaf::GetConsoleWndHandle());
	}
}

CmuConsoleDebug::~CmuConsoleDebug()
{
	leaf::CloseConsoleWindow();
}

CmuConsoleDebug* CmuConsoleDebug::GetInstance()
{
	static CmuConsoleDebug sInstance;
	return &sInstance;
}

void CmuConsoleDebug::UpdateMainScene()
{
	if(m_bInit)
	{
		//. 콘솔창 토글
#ifdef KWAK_FIX_KEY_STATE_RUNTIME_ERR
		if(SEASON3B::IsPress(VK_SHIFT) == TRUE)
#else // KWAK_FIX_KEY_STATE_RUNTIME_ERR
		if(HIBYTE(GetAsyncKeyState(VK_SHIFT)) == 128)
#endif // KWAK_FIX_KEY_STATE_RUNTIME_ERR
		{
			if(PressKey(VK_F7))
			{
				leaf::ShowConsole(!leaf::IsConsoleVisible());
			}
		}
	}
}

bool CmuConsoleDebug::CheckCommand(const std::string& strCommand)
{
	if(!m_bInit)
		return false;

	if(strCommand.compare("$open") == NULL)
	{
		leaf::ShowConsole(true);
		return true;
	}
	else if(strCommand.compare("$close") == NULL)
	{
		leaf::ShowConsole(false);
		return true;
	}
	else if(strCommand.compare("$clear") == NULL)
	{
		leaf::SetConsoleTextColor();
		leaf::ClearConsoleScreen();
		return true;
	}
#ifdef CSK_DEBUG_MAP_ATTRIBUTE
	else if(strCommand.compare("$mapatt on") == NULL)
	{
		EditFlag = EDIT_WALL;
		return true;
	}
	else if(strCommand.compare("$mapatt off") == NULL)
	{
		EditFlag = EDIT_NONE;
		return true;
	}
#endif // CSK_DEBUG_MAP_ATTRIBUTE
#ifdef CSK_DEBUG_MAP_PATHFINDING
	else if(strCommand.compare("$path on") == NULL)
	{
		g_bShowPath = true;
	}
	else if(strCommand.compare("$path off") == NULL)
	{
		g_bShowPath = false;	
	}
#endif // CSK_DEBUG_MAP_PATHFINDING
#ifdef CSK_DEBUG_RENDER_BOUNDINGBOX
	else if(strCommand.compare("$bb on") == NULL)
	{
		g_bRenderBoundingBox = true;
	}
	else if(strCommand.compare("$bb off") == NULL)
	{
		g_bRenderBoundingBox = false;	
	}
#endif // CSK_DEBUG_RENDER_BOUNDINGBOX
#if defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)
	else if(strCommand.compare("$unfixframe on") == NULL)
	{
		g_bUnfixedFixedFrame = true;
	}
	else if(strCommand.compare("$unfixframe off") == NULL)
	{
		g_bUnfixedFixedFrame = false;
	}
#endif // defined(LDS_FOR_DEVELOPMENT_TESTMODE) || defined(LDS_UNFIXED_FIXEDFRAME_FORDEBUG)
	else if(strCommand.compare("$type_test") == NULL)
	{
		Write(MCD_SEND, "MCD_SEND");
		Write(MCD_RECEIVE, "MCD_RECEIVE");
		Write(MCD_ERROR, "MCD_ERROR");
		Write(MCD_NORMAL, "MCD_NORMAL");
		return true;
	}
	else if(strCommand.compare("$texture_info") == NULL)
	{
		Write(MCD_NORMAL, "Texture Number : %d", Bitmaps.GetNumberOfTexture());
		Write(MCD_NORMAL, "Texture Memory : %dKB", Bitmaps.GetUsedTextureMemory()/1024);
		return true;
	}
	else if(strCommand.compare("$color_test") == NULL)
	{
		leaf::SetConsoleTextColor(leaf::COLOR_DARKRED);
		std::cout << "color test: dark red" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_DARKGREEN);
		std::cout << "color test: dark green" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_DARKBLUE);
		std::cout << "color test: dark blue" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_RED);
		std::cout << "color test: red" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_GREEN);
		std::cout << "color test: green" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_BLUE);
		std::cout << "color test: blue" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_OLIVE);
		std::cout << "color test: olive" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_PURPLE);
		std::cout << "color test: purple" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_TEAL);
		std::cout << "color test: teal" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_GRAY);
		std::cout << "color test: gray" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_AQUA);
		std::cout << "color test: aqua" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_FUCHSIA);
		std::cout << "color test: fuchsia" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_YELLOW);
		std::cout << "color test: yellow" << std::endl;
		leaf::SetConsoleTextColor(leaf::COLOR_WHITE);
		std::cout << "color test: white" << std::endl;
		return true;
	}
#ifdef CSK_HACK_TEST
	else
	{
		char* pToken = NULL;
		char seps[] = " ";
		char str[256];
		strcpy(str, strCommand.c_str());
		pToken = strtok(str, seps);

		if(strcmp(pToken, "pos") == 0)
		{
			pToken = strtok(NULL, seps);
			int x = atoi(pToken);
			pToken = strtok(NULL, seps);
			int y = atoi(pToken);
			
			g_pHackTest->MovePos(x, y);
		}
		else if(strcmp(pToken, "movemon") == 0)
		{
			g_pHackTest->MoveMonster();
		}
		else if(strcmp(pToken, "start1") == 0)
		{
			g_pHackTest->StartAuto(CHackTest::HACK_AUTOATTACK);
		}
		else if(strcmp(pToken, "start2") == 0)
		{
			g_pHackTest->StartAuto(CHackTest::HACK_PILLAR_OF_FIRE);
		}
		else if(strcmp(pToken, "start3") == 0)
		{
			g_pHackTest->StartAuto(CHackTest::HACK_EVILSPIRIT);
		}
		else if(strcmp(pToken, "start4") == 0)
		{
			g_pHackTest->StartAuto(CHackTest::HACK_DABAL);
		}
		else if(strcmp(pToken, "end") == 0)
		{	
			g_pHackTest->EndAuto();
		}
		else if(strcmp(pToken, "startskill") == 0)
		{
			g_pHackTest->StartSkill();
		}
		else if(strcmp(pToken, "endskill") == 0)
		{
			g_pHackTest->EndSkill();
		}
	}
#endif // CSK_HACK_TEST

	return false;
}

void CmuConsoleDebug::Write(int iType, const char* pStr, ...)
{
	if(m_bInit)
	{
		switch(iType)
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
		
		char szBuffer[256]="";
		va_list	pArguments;
		
		va_start(pArguments, pStr);
		vsprintf(szBuffer, pStr, pArguments);
		va_end(pArguments);
		
		std::cout << szBuffer << std::endl;
	}
}

CmuSimpleLog::CmuSimpleLog(void)
{
	m_bLogfirst = true;
	m_strFilename = "클라판별스킬분석로그.txt";
	m_pFile = NULL;
}

CmuSimpleLog::~CmuSimpleLog(void)
{
}

void CmuSimpleLog::setFilename(const char* strFilename)
{
	m_strFilename = strFilename;
}

void CmuSimpleLog::log(char *str, ...)
{
	if(m_bLogfirst) 
	{
		m_pFile = fopen(m_strFilename.c_str(), "a");	// 쓰기전용으로
		fclose(m_pFile);
		m_bLogfirst = false;
	}
	
	m_pFile = fopen(m_strFilename.c_str(), "a");	// 읽기 전용으로
	
	va_list ap;
	
	va_start (ap, str);
	vfprintf (m_pFile, str, ap);
	va_end (ap);
	
	fclose(m_pFile);
}
#endif // CSK_LH_DEBUG_CONSOLE
