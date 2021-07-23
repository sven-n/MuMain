// NewUIHelpWindow.cpp: implementation of the CNewUIHelpWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIHelpWindow.h"
#include "NewUISystem.h"
#include "ZzzInventory.h"
#include "DSPlaySound.h"


using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUIHelpWindow::CNewUIHelpWindow()
{
	m_pNewUIMng = NULL;
	m_Pos.x = 0;
	m_Pos.y = 0;

	m_iIndex = 0;
}

SEASON3B::CNewUIHelpWindow::~CNewUIHelpWindow()
{
	Release();
}

bool SEASON3B::CNewUIHelpWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if( NULL == pNewUIMng )
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_HELP, this);
	
	SetPos(x, y);

	Show(false);

	return true;
}

void SEASON3B::CNewUIHelpWindow::Release()
{
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}	
}

void SEASON3B::CNewUIHelpWindow::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool SEASON3B::CNewUIHelpWindow::UpdateMouseEvent()
{
	return true;
}

bool SEASON3B::CNewUIHelpWindow::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_HELP))
	{
		if(IsPress(VK_F1) == true)
		{
#ifdef PSW_ADD_FOREIGN_HELPWINDOW
			if(++m_iIndex > 1)
#else //PSW_ADD_FOREIGN_HELPWINDOW
			if(++m_iIndex > 3)
#endif //PSW_ADD_FOREIGN_HELPWINDOW			
			{
				g_pNewUISystem->Hide(SEASON3B::INTERFACE_HELP);
				PlayBuffer(SOUND_CLICK01);
			}

			return false;
		}
		
		if(IsPress(VK_ESCAPE) == true)
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_HELP);
			PlayBuffer(SOUND_CLICK01);

			return false;
		}
	}

	return true;
}

bool SEASON3B::CNewUIHelpWindow::Update()
{
	return true;
}

bool SEASON3B::CNewUIHelpWindow::Render()
{
	EnableAlphaTest();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	extern char TextList[30][100];
	extern int TextListColor[30];
	extern int TextBold[30];
	
	if(m_iIndex == 0)
	{
		int iTextNum = 0;

		unicode::_sprintf(TextList[iTextNum], "\n");
		iTextNum++;

		// 120 "키 사용법"
		unicode::_strcpy(TextList[iTextNum], GlobalText[120]);
		TextListColor[iTextNum] = TEXT_COLOR_BLUE;
		TextBold[iTextNum] = true;
		iTextNum++;

     	unicode::_sprintf(TextList[iTextNum], "\n");
		iTextNum++;

		// 121 "F1 : 도움말 On/Off"
		// 122 "F2 : 채팅창 On/Off"
		// 123 "F3 : 귓속말 On/Off"
		// 124 "F4 : 채팅창 크기 조정"
		// 125 "Enter : 채팅모드"
		// 126 "C : 케릭터정보창"
		// 127 "I,V : 인벤토리창"
		// 128 "P : 파티창"
		// 129 "G : 길드창"
		// 130 "Q : 회복물약 사용"
		// 131 "W : 마나물약 사용"
		// 132 "E : 해독물약 사용"
		// 133 "Shift : 케릭터 고정키"
		// 134 "Ctrl+숫자키 : 스킬핫키 등록"
		// 135 "숫자키 : 핫키로 등록된 스킬 선택"
		// 136 "Alt : 바닥에 떨어진 아이템 보기"
		// 137 "Alt+아이템 : 아이템을 우선순위로 선택"
		// 138 "Ctrl+케릭터 : PK 모드"
		// 139 "PrintScreen : 스크린샷 저장"
		for(int i=0; i<19; ++i)
		{
			unicode::_strcpy(TextList[iTextNum], GlobalText[121+i]);
			TextListColor[iTextNum] = TEXT_COLOR_WHITE;
			TextBold[iTextNum] = false;
			iTextNum++;
		}

     	unicode::_sprintf(TextList[iTextNum],"\n");
		iTextNum++;

		RenderTipTextList(1, 1, iTextNum, 0, RT3_SORT_CENTER);
	}
	else if(m_iIndex == 1)
	{
		int iTextNum = 0;

		unicode::_sprintf(TextList[iTextNum], "\n");
		iTextNum++;

		// 140 "채팅모드 키 사용법"
		unicode::_strcpy(TextList[iTextNum], GlobalText[140]);
		TextListColor[iTextNum] = TEXT_COLOR_BLUE;
		TextBold[iTextNum] = true;
		iTextNum++;

     	unicode::_sprintf(TextList[iTextNum], "\n");
		iTextNum++;

		// 141 "Tab : 귓속말 아이디 입력창 전환"
		// 142 "채팅글 오른쪽클릭 : 귓속말 ID 지정"
		// 143 "상하 방향키 : 채팅창 히스토리 기능"
		// 144 "PageUP, PageDN : 지나간 글 보기"
		// 145 "글앞에 ~를 붙임 : 파티원들에게 채팅"
		// 146 "글앞에 @를 붙임 : 길드원들에게 채팅"
		// 147 "글앞에 @>를 붙임 : 길드원들에게 공지"
		// 148 "글앞에 #를 붙임 : 글이 오랫동안 유지"
		// 149 "상대방에 마우스를 올리고 /거래: 거래"
		// 150 "상대방에 마우스를 올리고 /파티 : 파티"
		// 151 "길드마스터에 마우스를 올리고 /길드 : 길드"
		// 152 "/전쟁 길드명 : 길드전 신청"
		// 153 "/아이템이름 : 아이템 설명"
		// 154 "/이동 월드이름 : 월드로 순간이동"
		// 155 "/필터 단어1 단어2 : 단어 있는 채팅만 보기"
		// 156 "아래로 마우스 커서 이동 -> 채팅창 조절버튼"
		for(int i=0;i<16; ++i)
		{
#ifdef CSK_MOD_MOVE_COMMAND_WINDOW
			if(i == 13)
			{
				continue;
			}
#endif // CSK_MOD_MOVE_COMMAND_WINDOW
			unicode::_strcpy(TextList[iTextNum], GlobalText[141+i]);
			TextListColor[iTextNum] = TEXT_COLOR_WHITE;
			TextBold[iTextNum] = false;
			iTextNum++;
		}

     	unicode::_sprintf(TextList[iTextNum],"\n");
		iTextNum++;

		RenderTipTextList(1, 1, iTextNum, 0, RT3_SORT_CENTER);
	}
	else if(m_iIndex == 2)
	{
		int iTextNum = 0;

		unicode::_sprintf(TextList[iTextNum], "\n");
		iTextNum++;

		// 2421 "<AM>                  <PM>"	
		unicode::_strcpy(TextList[iTextNum], GlobalText[2421]);
		TextListColor[iTextNum] = TEXT_COLOR_BLUE; 
		TextBold[iTextNum] = true;
		iTextNum++;

		// 2422 "0:30  환영사원             12:30   환영사원"
		// 2423 "1:00  카오스캐슬(PC)   13:00   카오스캐슬"
		// 2424 "1:30  블러드캐슬          13:30   블러드캐슬"
		// 2425 "2:00  악마의광장          14:00   악마의광장"
		// 2426 "2:30  환영사원             14:30   환영사원"
		// 2427 "3:00  -                        15:00   카오스캐슬(PC)"
		// 2428 "3:30  블러드캐슬          15:30   블러드캐슬"
		// 2429 "4:00  -                        16:00   카오스캐슬"
		// 2430 "4:30  환영사원                 16:30   환영사원"
		// 2431 "5:00  악마의광장          17:00   악마의광장"
		// 2432 "5:30  블러드캐슬          17:30   블러드캐슬"
		// 2433 "6:00  -                        18:00   카오스캐슬(PC)"
		// 2434 "6:30  -                        18:30   환영사원"
		// 2435 "7:00  -                        19:00   카오스캐슬"
		// 2436 "7:30  블러드캐슬          19:30   블러드캐슬"
		// 2437 "8:00  악마의광장          20:00   악마의광장"
		// 2438 "8:30  환영사원                 20:30   환영사원"
		// 2439 "9:00  -                        21:00   카오스캐슬"
		// 2440 "9:30  블러드캐슬          21:30   블러드캐슬"
		// 2441 "10:00 카오스캐슬          22:00   카오스캐슬(PC)"
		// 2442 "10:30 카오스캐슬(PC)   22:30   환영사원"
		// 2443 "11:00 악마의광장          23:00   악마의광장"
		// 2444 "11:30 블러드캐슬          23:30   블러드캐슬"
		// 2445 "12:00 카오스캐슬(PC)   24:00   카오스캐슬"
		for(int i = 0; i<24; ++i)
		{
			unicode::_strcpy(TextList[iTextNum], GlobalText[2422+i]);
			TextListColor[iTextNum] = TEXT_COLOR_WHITE; 
			TextBold[iTextNum] = false;
			iTextNum++;
		}

		unicode::_sprintf(TextList[iTextNum], "\n");
		iTextNum++;

		RenderTipTextList(1, 1, iTextNum, 0, RT3_SORT_LEFT);
	}
	else if(m_iIndex == 3)
	{
		int iTextNum = 0;
		
		unicode::_sprintf(TextList[iTextNum], "\n");
		iTextNum++;

		// 2446 " << 카오스캐슬 입장레벨 >>     << 악마의광장 입장레벨 >>"		
		unicode::_strcpy(TextList[iTextNum], GlobalText[2446]);
		TextListColor[iTextNum] = TEXT_COLOR_BLUE; 
		TextBold[iTextNum] = true;
		iTextNum++;

// 		2447 "등급   일반      2차      등급   일반       2차"     
// 		2448 " 1    15-49     15-29      1    15-130    10-110"
// 		2449 " 2    50-119    30-99      2    131-180   111-160"
// 		2450 " 3    120-179   100-159    3    181-230   161-210"
// 		2451 " 4    180-239   160-219    4    231-280   211-260"
// 		2452 " 5    240-299   220-279    5    281-330   261-310"
// 		2453 " 6    300-400   280-400    6    331-400   311-400"
// 		2454 " 7    마스터    마스터      7    마스터    마스터"
// 		2455 " << 블러드캐슬 >>         << 환영사원 >>"
// 		2456 "등급   일반      2차      등급   일반       2차" 
// 		2457 " 1    15-80     10-60          1    220-270"	
// 		2458 " 2    81-130    61-110        2    271-320"	
// 		2459 " 3    131-180   111-160      3    321-350"	
// 		2460 " 4    181-230   161-210      4    351-380"	
// 		2461 " 5    231-280   211-260      5    381-400"	
// 		2462 " 6    281-330   261-310      6    마스터"			
// 		2463 " 7    331-400   311-400" 
// 		2464 " 8    마스터    마스터"

		for( int i=0; i<18; ++i )
		{
			unicode::_strcpy(TextList[iTextNum], GlobalText[2447+i]);
			if(i == 0 || i == 8 || i == 9)
			{
				TextListColor[iTextNum] = TEXT_COLOR_BLUE; 
				TextBold[iTextNum] = true;
				iTextNum++;
			}
			else
			{
				TextListColor[iTextNum] = TEXT_COLOR_WHITE; 
				TextBold[iTextNum] = false;
				iTextNum++;
			}
		}

		unicode::_sprintf(TextList[iTextNum],"\n");
		iTextNum++;

		RenderTipTextList(1, 1, iTextNum, 0, RT3_SORT_LEFT);
	}


	DisableAlphaBlend();
	return true;
}

float SEASON3B::CNewUIHelpWindow::GetLayerDepth()
{
#ifdef PJH_FIX_HELP_LAYER
	return 8.2f;
#else
	return 7.1f;
#endif //PJH_FIX_HELP_LAYER
}

float SEASON3B::CNewUIHelpWindow::GetKeyEventOrder()
{
	return 10.f;
}

void SEASON3B::CNewUIHelpWindow::OpenningProcess()
{
	m_iIndex = 0;
}

void SEASON3B::CNewUIHelpWindow::ClosingProcess()
{

}

void SEASON3B::CNewUIHelpWindow::AutoUpdateIndex()
{
#ifdef PSW_ADD_FOREIGN_HELPWINDOW
	if(++m_iIndex > 1) {
#else //PSW_ADD_FOREIGN_HELPWINDOW
	if(++m_iIndex > 3) {
#endif //PSW_ADD_FOREIGN_HELPWINDOW
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_HELP);
	}
}
