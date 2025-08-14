//*****************************************************************************
// File: Button.cpp
//*****************************************************************************

#include "stdafx.h"
#include "Button.h"
#include "Input.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "DSPlaySound.h"
#include "UIControls.h"




CButton* CButton::m_pBtnHeld;

CButton::CButton() : m_szText(NULL), m_adwTextColorMap(NULL)
{
}

CButton::~CButton()
{
    Release();
}

void CButton::Release()
{
    ReleaseText();
}

void CButton::Create(int nWidth, int nHeight, int nTexID, int nMaxFrame, int nDownFrame, int nActiveFrame, int nDisableFrame, int nCheckUpFrame, int nCheckDownFrame, int nCheckActiveFrame, int nCheckDisableFrame)
{
    Release();

    auto* aFrameCoord = new SFrameCoord[nMaxFrame];
    for (int i = 0; i < nMaxFrame; ++i)
    {
        aFrameCoord[i].nX = 0;
        aFrameCoord[i].nY = nHeight * i;
    }

    CSprite::Create(nWidth, nHeight, nTexID, nMaxFrame, aFrameCoord);

    delete[] aFrameCoord;

    CSprite::SetAction(0, nMaxFrame - 1);

    m_anImgMap[BTN_UP] = 0;
    m_anImgMap[BTN_DOWN] = nDownFrame > -1 ? nDownFrame : 0;
    m_anImgMap[BTN_ACTIVE] = nActiveFrame > -1 ? nActiveFrame : 0;
    m_anImgMap[BTN_DISABLE] = nDisableFrame;

    m_anImgMap[BTN_UP_CHECK] = nCheckUpFrame;
    m_anImgMap[BTN_DOWN_CHECK] = nCheckDownFrame > -1 ? nCheckDownFrame : m_anImgMap[BTN_UP_CHECK];
    m_anImgMap[BTN_ACTIVE_CHECK] = nCheckActiveFrame > -1 ? nCheckActiveFrame : m_anImgMap[BTN_UP_CHECK];
    m_anImgMap[BTN_DISABLE_CHECK] = nCheckDisableFrame;

    m_bClick = m_bCheck = false;
    m_bEnable = m_bActive = true;
}

void CButton::Show(bool bShow)
{
    CSprite::Show(bShow);
    if (!bShow)
        m_bClick = false;
}

BOOL CButton::CursorInObject()
{
    if (!m_bActive)
        return FALSE;

    return CSprite::CursorInObject();
}

void CButton::Update()
{
    if (!CSprite::m_bShow)
        return;

    CInput& rInput = CInput::Instance();

    m_fTextAddYPos = 0.5f;

    if (m_bEnable/* && m_bActive*/)
    {
        if (CursorInObject() && rInput.IsLBtnDn())
            m_pBtnHeld = this;

        m_bClick = false;

        if (rInput.IsLBtnUp())
        {
            if (CursorInObject() && this == m_pBtnHeld)
            {
                m_bClick = true;

                ::PlayBuffer(SOUND_CLICK01);

                if (-1 < m_anImgMap[BTN_UP_CHECK])
                    m_bCheck = !m_bCheck;
            }

            if (this == m_pBtnHeld)
                m_pBtnHeld = NULL;
        }

        if (CursorInObject() && NULL == m_pBtnHeld)
            if (m_bCheck)
            {
                CSprite::SetNowFrame(m_anImgMap[BTN_ACTIVE_CHECK]);
                if (NULL != m_szText)
                    m_dwTextColor = m_adwTextColorMap[BTN_ACTIVE_CHECK];
            }
            else
            {
                CSprite::SetNowFrame(m_anImgMap[BTN_ACTIVE]);
                if (NULL != m_szText)
                    m_dwTextColor = m_adwTextColorMap[BTN_ACTIVE];
            }
        else if (CursorInObject() && this == m_pBtnHeld)
        {
            m_fTextAddYPos = 1.5f;

            if (m_bCheck)
            {
                CSprite::SetNowFrame(m_anImgMap[BTN_DOWN_CHECK]);
                if (NULL != m_szText)
                    m_dwTextColor = m_adwTextColorMap[BTN_DOWN_CHECK];
            }
            else
            {
                CSprite::SetNowFrame(m_anImgMap[BTN_DOWN]);
                if (NULL != m_szText)
                    m_dwTextColor = m_adwTextColorMap[BTN_DOWN];
            }
        }
        else
            if (m_bCheck)
            {
                CSprite::SetNowFrame(m_anImgMap[BTN_UP_CHECK]);
                if (NULL != m_szText)
                    m_dwTextColor = m_adwTextColorMap[BTN_UP_CHECK];
            }
            else
            {
                CSprite::SetNowFrame(m_anImgMap[BTN_UP]);
                if (NULL != m_szText)
                    m_dwTextColor = m_adwTextColorMap[BTN_UP];
            }
    }
    else
    {
        m_bClick = false;
        if (rInput.IsLBtnUp() && this == m_pBtnHeld)
            m_pBtnHeld = NULL;

        if (m_bCheck)
        {
            CSprite::SetNowFrame(m_anImgMap[BTN_DISABLE_CHECK]);
            if (NULL != m_szText)
                m_dwTextColor = m_adwTextColorMap[BTN_DISABLE_CHECK];
        }
        else
        {
            CSprite::SetNowFrame(m_anImgMap[BTN_DISABLE]);
            if (NULL != m_szText)
                m_dwTextColor = m_adwTextColorMap[BTN_DISABLE];
        }
    }

    //	CSprite::Update(dDeltaTick);	// ¹öÆ° Animation.
}

void CButton::Render()
{
    if (!CSprite::m_bShow)
        return;

    if (!m_bEnable)
    {
        if (m_bCheck && m_anImgMap[BTN_DISABLE_CHECK] < 0)
            return;
        if (!m_bCheck && m_anImgMap[BTN_DISABLE] < 0)
            return;
    }

    CSprite::Render();

    if (NULL == m_szText)
        return;

    g_pRenderText->SetTextColor(m_dwTextColor);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetFont(g_hFixFont);

    SIZE size;
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), m_szText, ::wcslen(m_szText), &size);

    float fTextRelativeYPos = ((CSprite::GetHeight() - size.cy) / 2.0f);

    g_pRenderText->RenderText(int(CSprite::GetXPos() / g_fScreenRate_x), int(((float)CSprite::GetYPos() + fTextRelativeYPos) / g_fScreenRate_y + m_fTextAddYPos), m_szText, CSprite::GetWidth() / g_fScreenRate_x, 0, RT3_SORT_CENTER);
}

void CButton::ReleaseText()
{
    SAFE_DELETE_ARRAY(m_szText);
    SAFE_DELETE_ARRAY(m_adwTextColorMap);
}

void CButton::SetText(const wchar_t* pszText, DWORD* adwColor)
{
    ReleaseText();

    m_szText = new wchar_t[wcslen(pszText) + 1];
    wcscpy(m_szText, pszText);

    int nTextColor = -1 < m_anImgMap[BTN_UP_CHECK] ? BTN_IMG_MAX : BTN_IMG_MAX / 2;

    m_adwTextColorMap = new DWORD[nTextColor];
    memcpy(m_adwTextColorMap, adwColor, sizeof(DWORD) * nTextColor);
}