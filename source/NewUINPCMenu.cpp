//*****************************************************************************
// File: NewUINPCMenu.cpp
//*****************************************************************************

#include "stdafx.h"
#include <crtdbg.h>
#include "NewUINPCMenu.h"
#include "NewUISystem.h"
#include "wsclientinline.h"

#ifdef ASG_ADD_UI_NPC_MENU

using namespace SEASON3B;

#define NM_TEXT_GAP					16

CNewUINPCMenu::CNewUINPCMenu()
{
	m_pNewUIMng = NULL;
	m_pNewUI3DRenderMng = NULL;
	m_Pos.x = m_Pos.y = 0;
}

CNewUINPCMenu::~CNewUINPCMenu()
{
	Release();
}

bool CNewUINPCMenu::Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng,
						   int x, int y)
{
	if (NULL == pNewUIMng || NULL == pNewUI3DRenderMng)
		return false;

	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_NPC_MENU, this);

	m_pNewUI3DRenderMng = pNewUI3DRenderMng;
	m_pNewUI3DRenderMng->Add3DRenderObj(this, INVENTORY_CAMERA_Z_ORDER);

	SetPos(x, y);

	LoadImages();

	m_btnClose.ChangeButtonImgState(true, IMAGE_NM_BTN_CLOSE);
	m_btnClose.ChangeButtonInfo(x + 13, y + 392, 36, 29);
	m_btnClose.ChangeToolTipText(GlobalText[1002], true);

	m_nSelTextCount = 0;

	Show(false);

	return true;
}

void CNewUINPCMenu::Release()
{
	UnloadImages();

	if (m_pNewUI3DRenderMng)
	{
		m_pNewUI3DRenderMng->Remove3DRenderObj(this);
		m_pNewUI3DRenderMng = NULL;
	}
	
	if (m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj(this);
		m_pNewUIMng = NULL;
	}
}

void CNewUINPCMenu::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool CNewUINPCMenu::UpdateMouseEvent()
{
	if (ProcessBtns())
		return false;

	if (UpdateSelTextMouseEvent())
		return false;

	if(CheckMouseIn(m_Pos.x, m_Pos.y, NM_WIDTH, NM_HEIGHT))
		return false;

	return true;
}

bool CNewUINPCMenu::ProcessBtns()
{
	if (m_btnClose.UpdateMouseEvent())
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPC_MENU);
		return true;
	}
	else if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(m_Pos.x+169, m_Pos.y+7, 13, 12))
	{
		g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPC_MENU);
		return true;
	}

	return false;
}

bool CNewUINPCMenu::UpdateSelTextMouseEvent()
{
	if (!m_bCanClick)
		return false;

	m_nSelText = 0;
	if (MouseX < m_Pos.x+11 || MouseX > m_Pos.x+179)
		return false;

	int nTopY;
	int nBottomY = m_Pos.y+216;
	int i;
	for (i = 0; i < m_nSelTextCount; ++i)
	{
		nTopY = nBottomY;
		nBottomY += m_anSelTextLine[i] * NM_TEXT_GAP;

		if (nTopY <= MouseY && MouseY < nBottomY)
		{
			m_nSelText = i + 1;
			if (SEASON3B::IsRelease(VK_LBUTTON))
			{
				SendQuestSelection(m_adwQuestIndex[i], (BYTE)m_nSelText);
				::PlayBuffer(SOUND_CLICK01);
				m_bCanClick = false;
				if (0 == HIWORD(m_adwQuestIndex[i]))
				{
					if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPC_MENU))
						g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPC_MENU);
				}
				return true;
			}
			break;
		}
	}

	return false;
}

bool CNewUINPCMenu::UpdateKeyEvent()
{
	if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPC_MENU))
	{
		if (SEASON3B::IsPress(VK_ESCAPE))
		{
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPC_MENU);
			return false;
		}
	}

	return true;
}

bool CNewUINPCMenu::Update()
{
	return true;
}

bool CNewUINPCMenu::Render()
{
	::EnableAlphaTest();

	::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	RenderBackImage();
	RenderSelTextBlock();

	RenderText();

	m_btnClose.Render();

	::DisableAlphaBlend();
	
	return true;
}

void CNewUINPCMenu::RenderBackImage()
{
	RenderImage(IMAGE_NM_BACK, m_Pos.x, m_Pos.y, float(NM_WIDTH), float(NM_HEIGHT));
	RenderImage(IMAGE_NM_TOP, m_Pos.x, m_Pos.y, float(NM_WIDTH), 64.f);
	RenderImage(IMAGE_NM_LEFT, m_Pos.x, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_NM_RIGHT, m_Pos.x+NM_WIDTH-21, m_Pos.y+64, 21.f, 320.f);
	RenderImage(IMAGE_NM_BOTTOM, m_Pos.x, m_Pos.y+NM_HEIGHT-45, float(NM_WIDTH), 45.f);

	RenderImage(IMAGE_NM_LINE, m_Pos.x+1, m_Pos.y+196, 188.f, 21.f);
#ifdef ASG_ADD_UI_QUEST_PROGRESS
#ifndef ASG_MOD_3D_CHAR_EXCLUSION_UI
	RenderImage(IMAGE_NM_BOX_NPC, m_Pos.x+(NM_WIDTH-86)/2, m_Pos.y+33, 86.f, 96.f);
#endif	// ASG_MOD_3D_CHAR_EXCLUSION_UI
#endif	// ASG_ADD_UI_QUEST_PROGRESS
}

void CNewUINPCMenu::RenderSelTextBlock()
{
	if (0 == m_nSelText)
		return;

	int nBlockPosY = m_Pos.y+216;
	int i;
	for (i = 0; i < m_nSelText - 1; ++i)
		nBlockPosY += NM_TEXT_GAP * m_anSelTextLine[i];

	::glColor4f(0.5f, 0.7f, 0.3f, 0.5f);
	::RenderColor(m_Pos.x+11, nBlockPosY, 168.f, NM_TEXT_GAP * m_anSelTextLine[m_nSelText - 1]);
	::EndRenderColor();
}

void CNewUINPCMenu::RenderText()
{
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetBgColor(0);

	g_pRenderText->SetTextColor(150, 255, 240, 255);
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+12, g_QuestMng.GetNPCName(),
		NM_WIDTH, 0, RT3_SORT_CENTER);

	g_pRenderText->SetFont(g_hFont);
	g_pRenderText->SetTextColor(255, 230, 210, 255);

	int i;
	for (i = 0; i < NM_NPC_LINE_MAX; ++i)
#ifdef ASG_MOD_3D_CHAR_EXCLUSION_UI
		g_pRenderText->RenderText(m_Pos.x+13, m_Pos.y+84+(18*i), m_aszNPCWords[i], 0, 0,
#else	// ASG_MOD_3D_CHAR_EXCLUSION_UI
		g_pRenderText->RenderText(m_Pos.x+13, m_Pos.y+136+(NM_TEXT_GAP*i), m_aszNPCWords[i], 0, 0,
#endif	// ASG_MOD_3D_CHAR_EXCLUSION_UI
			RT3_SORT_LEFT);

	g_pRenderText->SetTextColor(255, 230, 210, 255);
	for (i = 0; i < NM_SEL_TEXT_LINE_MAX; ++i)
		g_pRenderText->RenderText(m_Pos.x+13, m_Pos.y+220+(NM_TEXT_GAP*i), m_aszSelText[i], 0, 0,
			RT3_SORT_LEFT);
}

void CNewUINPCMenu::Render3D()
{
	//	RenderItem3D();
}

bool CNewUINPCMenu::IsVisible() const
{ return CNewUIObj::IsVisible(); }

float CNewUINPCMenu::GetLayerDepth()
{
	return 3.1f;
}

void CNewUINPCMenu::LoadImages()
{
	LoadBitmap("Interface\\newui_msgbox_back.jpg", IMAGE_NM_BACK, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back01.tga", IMAGE_NM_TOP, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-L.tga", IMAGE_NM_LEFT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back02-R.tga", IMAGE_NM_RIGHT, GL_LINEAR);
	LoadBitmap("Interface\\newui_item_back03.tga", IMAGE_NM_BOTTOM, GL_LINEAR);

	LoadBitmap("Interface\\newui_myquest_Line.tga", IMAGE_NM_LINE, GL_LINEAR);
#ifdef ASG_ADD_UI_QUEST_PROGRESS
#ifndef ASG_MOD_3D_CHAR_EXCLUSION_UI
	LoadBitmap("Interface\\Quest_box_user.tga", IMAGE_NM_BOX_NPC, GL_LINEAR);
#endif	// ASG_MOD_3D_CHAR_EXCLUSION_UI
#endif	// ASG_ADD_UI_QUEST_PROGRESS
	LoadBitmap("Interface\\newui_exit_00.tga", IMAGE_NM_BTN_CLOSE, GL_LINEAR);
}

void CNewUINPCMenu::UnloadImages()
{
	DeleteBitmap(IMAGE_NM_BTN_CLOSE);
#ifdef ASG_ADD_UI_QUEST_PROGRESS
#ifndef ASG_MOD_3D_CHAR_EXCLUSION_UI
	DeleteBitmap(IMAGE_NM_BOX_NPC);
#endif	// ASG_MOD_3D_CHAR_EXCLUSION_UI
#endif	// ASG_ADD_UI_QUEST_PROGRESS
	DeleteBitmap(IMAGE_NM_LINE);

	DeleteBitmap(IMAGE_NM_BOTTOM);
	DeleteBitmap(IMAGE_NM_RIGHT);
	DeleteBitmap(IMAGE_NM_LEFT);
	DeleteBitmap(IMAGE_NM_TOP);
	DeleteBitmap(IMAGE_NM_BACK);
}

void CNewUINPCMenu::ProcessOpening()
{
	::PlayBuffer(SOUND_INTERFACE01);
}

bool CNewUINPCMenu::ProcessClosing()
{
	SendExitInventory();
	::PlayBuffer(SOUND_CLICK01);
	return true;
}

void CNewUINPCMenu::SetContents(DWORD* adwSrcQuestIndex, int nIndexCount)
{
	SetNPCWords();
	SetSelText(adwSrcQuestIndex, nIndexCount);

	m_bCanClick = true;
	m_nSelText = 0;

	if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPC_MENU))
		g_pNewUISystem->Show(SEASON3B::INTERFACE_NPC_MENU);
}

extern float g_fScreenRate_x;

void CNewUINPCMenu::SetNPCWords()
{
	int nNPCWordsIndex = 0;

	switch (g_QuestMng.GetNPCIndex())
	{
	case 257:
		nNPCWordsIndex = 296;
		break;
	default:
		return;
	}

	::memset(m_aszNPCWords[0], 0, sizeof(char) * NM_NPC_LINE_MAX * NM_TEXT_ROW_MAX);

	g_pRenderText->SetFont(g_hFont);
	::DivideStringByPixel(&m_aszNPCWords[0][0], NM_NPC_LINE_MAX, NM_TEXT_ROW_MAX,
		g_QuestMng.GetWords(nNPCWordsIndex), 160);
}

void CNewUINPCMenu::SetSelText(DWORD* adwSrcQuestIndex, int nIndexCount)
{
	_ASSERT(0 < nIndexCount && nIndexCount <= NM_QUEST_INDEX_MAX_COUNT);

	::memset(m_adwQuestIndex, 0, sizeof(DWORD)*NM_QUEST_INDEX_MAX_COUNT);
	::memcpy(m_adwQuestIndex, adwSrcQuestIndex, sizeof(DWORD) * nIndexCount);

	m_nSelTextCount = nIndexCount;

	::memset(m_aszSelText[0], 0, sizeof(char) * NM_SEL_TEXT_LINE_MAX * NM_TEXT_ROW_MAX);
	::memset(m_anSelTextLine, 0, sizeof(int) * NM_SEL_TEXT_LINE_MAX);

	char szSelText[2 * NM_TEXT_ROW_MAX];
	const char* pszSelText;
	int nSelTextRow = 0;
	g_pRenderText->SetFont(g_hFont);
	int i;
	for (i = 0; i < nIndexCount; ++i)
	{
		if (0 == HIWORD(m_adwQuestIndex[i]))
			::sprintf(szSelText, "%d.", i + 1);
		else
			::sprintf(szSelText, "%d. [Q]", i + 1);

		pszSelText = g_QuestMng.GetSubject(m_adwQuestIndex[i]);
		if (NULL == pszSelText)
			break;
		::strcat(szSelText, pszSelText);

		m_anSelTextLine[i] = ::DivideStringByPixel(&m_aszSelText[nSelTextRow][0],
			2, NM_TEXT_ROW_MAX, szSelText, 160, false);
		
		nSelTextRow += m_anSelTextLine[i];
		
		if (NM_SEL_TEXT_LINE_MAX <= nSelTextRow)
			break;
	}
}

#endif	// ASG_ADD_UI_NPC_MENU