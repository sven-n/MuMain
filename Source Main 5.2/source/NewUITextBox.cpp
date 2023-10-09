//=============================================================================
//	NewUITextBox.cpp
//=============================================================================
#pragma once
#include "StdAfx.h"
#include "NewUITextBox.h"
#include "UIControls.h"
#include "UsefulDef.h"


using namespace SEASON3B;

const int iMAX_TEXT_LINE = 512;
const int iLINE_INTERVAL = 2;

CNewUITextBox::CNewUITextBox()
{
    m_iWidth = 0;
    m_iHeight = 0;
    memset(&m_ptPos, 0, sizeof(POINT));

    m_iTextHeight = 0;
    m_iTextLineHeight = 1;
    m_iLimitLine = 0;

    m_iMaxLine = 0;
    m_iCurLine = 0;
}

CNewUITextBox::~CNewUITextBox()
{
    Release();
}

bool CNewUITextBox::Create(int iX, int iY, int iWidth, int iHeight)
{
    SetPos(iX, iY, iWidth, iHeight);
    Show(true);

    return true;
}

void CNewUITextBox::SetPos(int iX, int iY, int iWidth, int iHeight)
{
    m_ptPos.x = iX;
    m_ptPos.y = iY;
    m_iWidth = iWidth;
    m_iHeight = iHeight;

    SIZE Fontsize;
    g_pRenderText->SetFont(g_hFont);

    std::wstring strTemp = L"A";

    GetTextExtentPoint32(g_pRenderText->GetFontDC(), strTemp.c_str(), strTemp.size(), &Fontsize);

    m_iTextHeight = Fontsize.cy;
    m_iTextLineHeight = m_iTextHeight + iLINE_INTERVAL;

    m_iLimitLine = m_iHeight / m_iTextLineHeight;

    m_iMaxLine = 0;
    m_iCurLine = 0;
}

void CNewUITextBox::Release()
{
}

float CNewUITextBox::GetLayerDepth()
{
    return 4.4f;
}

bool CNewUITextBox::UpdateMouseEvent()
{
    return true;
}

bool CNewUITextBox::UpdateKeyEvent()
{
    return true;
}

bool CNewUITextBox::Update()
{
    return true;
}

bool CNewUITextBox::Render()
{
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    for (int iIndex = 0; iIndex < m_iLimitLine; iIndex++)
    {
        int iLineIndex = m_iCurLine + iIndex;

        if (GetLineText(iLineIndex).empty() == false)
        {
            g_pRenderText->SetFont(g_hFont);
            //g_pRenderText->SetBgColor( 0, 0, 0, 0 );
            g_pRenderText->SetTextColor(255, 255, 255, 255);
            g_pRenderText->RenderText(m_ptPos.x, m_ptPos.y + iIndex * m_iTextLineHeight, GetLineText(iLineIndex).c_str(),
                m_iWidth, 0, RT3_SORT_LEFT);
        }
    }

    return true;
}

void CNewUITextBox::AddText(wchar_t* strText)
{
    wchar_t strTemp[iMAX_TEXT_LINE][iMAX_TEXT_LINE];
    ::memset(strTemp[0], 0, sizeof(char) * iMAX_TEXT_LINE * iMAX_TEXT_LINE);

    int iTextLine = ::DivideStringByPixel(&strTemp[0][0], iMAX_TEXT_LINE, iMAX_TEXT_LINE, strText, m_iWidth, true, '#');

    for (int iIndex = 0; iIndex < iTextLine; iIndex++)
    {
        m_vecText.push_back(strTemp[iIndex]);
    }
}

void CNewUITextBox::AddText(const wchar_t* strText)
{
    wchar_t strTempText[iMAX_TEXT_LINE] = { 0, };
    swprintf(strTempText, strText);

    AddText(strTempText);
}

std::wstring CNewUITextBox::GetFullText()
{
   std::wstring strTemp;

    auto vi = m_vecText.begin();
    for (; vi != m_vecText.end(); vi++)
    {
        strTemp += (*vi);
    }

    return strTemp;
}

std::wstring CNewUITextBox::GetLineText(int iLineIndex)
{
    if (0 > iLineIndex || (int)m_vecText.size() <= iLineIndex)
        return L"";

    return m_vecText[iLineIndex];
}

int CNewUITextBox::GetMoveableLine()
{
    int iMoveableLine = m_vecText.size() - m_iLimitLine;
    if (iMoveableLine <= 0)
        iMoveableLine = 0;

    return iMoveableLine;
}