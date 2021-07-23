// w_Text.cpp: implementation of the Text class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_Text.h"

#ifdef NEW_USER_INTERFACE_UISYSTEM

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace ui;

#include "ZzzInfomation.h"

Text::Text(const string& rstrName) : UIWnd(rstrName)
{
	Initialize();
	RegisterDrawEvent(eDraw_Image);
}

Text::~Text()
{
	Destroy();
}

void Text::Initialize()
{
	m_pTextBase = BoostSmart_Ptr(CText)(new CText);
}

void Text::Destroy()
{
	m_pTextBase->Release();
}

void Text::Make(int nFontHeight, int nTextMaxLen, TEXT_STYLE eStyle, TEXT_TYPE eType)
{
	m_pTextBase->Create(GlobalText[0], nFontHeight, nTextMaxLen, eStyle, eType);
	m_pTextBase->Show();
}

void Text::SetColor(const Color& rColor)
{
	m_pTextBase->SetColor(rColor.Red(), rColor.Green(), rColor.Blue());
	m_pTextBase->SetAlpha(rColor.Alpha());
}

void Text::SetAlign(BYTE byAlign)
{
	m_pTextBase->SetAlign(byAlign);
}

void Text::SetContent(const string& rstrContent)
{
	m_pTextBase->SetText(rstrContent.c_str());
	Resize(Dim(m_pTextBase->GetTextWidth(), m_pTextBase->GetTextHeight()));
}

void Text::OnMove(const Coord& rPrevPos)
{
	UIWnd::OnMove(rPrevPos);

	Coord pos = GetScreenPosition();
	m_pTextBase->SetPosition(pos.x, pos.y);
}

void Text::OnResize(const Dim& rPrevSize)
{
	UIWnd::OnResize(rPrevSize);
}

void Text::OnMouseOver()
{
	UIWnd::OnMouseOver();
}

void Text::OnMouseOut()
{
	UIWnd::OnMouseOut();
}

void Text::OnChangeOpacity(BYTE byPrevOpacity)
{
	UIWnd::OnChangeOpacity(byPrevOpacity);
}

void Text::Process(int nDelta)
{
	
}

void Text::DrawImage()
{
	Coord pos  = GetScreenPosition();
	//텍스트 셋팅

	//칼라값 셋팅

	//랜더링
	m_pTextBase->Render();
}

#endif //NEW_USER_INTERFACE_UISYSTEM
