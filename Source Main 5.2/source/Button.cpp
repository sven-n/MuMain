//***************************************************************************
// File: Button.cpp
//***************************************************************************
#include "stdafx.h"

#include <algorithm>
#include <array>
#include <vector>

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

namespace
{
constexpr float kDefaultTextOffset = 0.5f;
constexpr float kPressedTextOffset = 1.5f;

bool IsValidStateIndex(int state)
{
    return state >= 0 && state < BTN_IMG_MAX;
}

int ClampStateIndex(int state)
{
    return IsValidStateIndex(state) ? state : BTN_UP;
}
} // namespace

CButton* CButton::m_pBtnHeld = nullptr;

CButton::CButton()
    : m_bEnable(true)
    , m_bActive(true)
    , m_bClick(false)
    , m_bCheck(false)
{
    m_imageFrames.fill(-1);
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

    const int frameCount = nMaxFrame;
    std::vector<SFrameCoord> frameCoords(static_cast<std::size_t>(frameCount > 0 ? frameCount : 0));
    for (int i = 0; i < frameCount; ++i)
    {
        auto& coord = frameCoords[static_cast<std::size_t>(i)];
        coord.nX = 0;
        coord.nY = nHeight * i;
    }

    CSprite::Create(nWidth, nHeight, nTexID, frameCount, frameCoords.data());
    CSprite::SetAction(0, frameCount - 1);

    m_imageFrames.fill(-1);
    m_imageFrames[BTN_UP] = 0;
    m_imageFrames[BTN_DOWN] = (nDownFrame >= 0) ? nDownFrame : m_imageFrames[BTN_UP];
    m_imageFrames[BTN_ACTIVE] = (nActiveFrame >= 0) ? nActiveFrame : m_imageFrames[BTN_UP];
    m_imageFrames[BTN_DISABLE] = nDisableFrame;

    m_imageFrames[BTN_UP_CHECK] = nCheckUpFrame;
    m_imageFrames[BTN_DOWN_CHECK] = (nCheckDownFrame >= 0) ? nCheckDownFrame : m_imageFrames[BTN_UP_CHECK];
    m_imageFrames[BTN_ACTIVE_CHECK] = (nCheckActiveFrame >= 0) ? nCheckActiveFrame : m_imageFrames[BTN_UP_CHECK];
    m_imageFrames[BTN_DISABLE_CHECK] = nCheckDisableFrame;

    m_bClick = false;
    m_bCheck = false;
    m_bEnable = true;
    m_bActive = true;
}

void CButton::Show(bool bShow)
{
    CSprite::Show(bShow);
    if (!bShow)
    {
        m_bClick = false;
    }
}

BOOL CButton::CursorInObject()
{
    if (!m_bActive)
    {
        return FALSE;
    }

    return CSprite::CursorInObject();
}

void CButton::Update()
{
    if (!CSprite::m_bShow)
    {
        return;
    }

    CInput& input = CInput::Instance();
    m_fTextAddYPos = kDefaultTextOffset;

    if (m_bEnable)
    {
        if (CursorInObject() && input.IsLBtnDn())
        {
            m_pBtnHeld = this;
        }

        m_bClick = false;

        if (input.IsLBtnUp())
        {
            if (CursorInObject() && this == m_pBtnHeld)
            {
                m_bClick = true;
                ::PlayBuffer(SOUND_CLICK01);

                if (HasCheckVisuals())
                {
                    m_bCheck = !m_bCheck;
                }
            }

            if (this == m_pBtnHeld)
            {
                m_pBtnHeld = nullptr;
            }
        }

        if (CursorInObject() && m_pBtnHeld == nullptr)
        {
            ApplyVisualState(m_bCheck ? BTN_ACTIVE_CHECK : BTN_ACTIVE);
        }
        else if (CursorInObject() && this == m_pBtnHeld)
        {
            m_fTextAddYPos = kPressedTextOffset;
            ApplyVisualState(m_bCheck ? BTN_DOWN_CHECK : BTN_DOWN);
        }
        else
        {
            ApplyVisualState(m_bCheck ? BTN_UP_CHECK : BTN_UP);
        }
    }
    else
    {
        m_bClick = false;
        if (input.IsLBtnUp() && this == m_pBtnHeld)
        {
            m_pBtnHeld = nullptr;
        }

        ApplyVisualState(m_bCheck ? BTN_DISABLE_CHECK : BTN_DISABLE);
    }

    // Button animation update is intentionally disabled (legacy behavior kept).
}

void CButton::Render()
{
    if (!CSprite::m_bShow)
    {
        return;
    }

    if (!m_bEnable)
    {
        if (m_bCheck && m_imageFrames[BTN_DISABLE_CHECK] < 0)
        {
            return;
        }
        if (!m_bCheck && m_imageFrames[BTN_DISABLE] < 0)
        {
            return;
        }
    }

    CSprite::Render();

    if (m_text.empty())
    {
        return;
    }

    g_pRenderText->SetTextColor(m_textColor);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetFont(g_hFixFont);

    SIZE size{};
    const int textLength = static_cast<int>(m_text.length());
    GetTextExtentPoint32(g_pRenderText->GetFontDC(), m_text.c_str(), textLength, &size);

    const float textRelativeYPos = (static_cast<float>(CSprite::GetHeight()) - size.cy) * 0.5f;
    g_pRenderText->RenderText(
        static_cast<int>(CSprite::GetXPos() / g_fScreenRate_x),
        static_cast<int>((static_cast<float>(CSprite::GetYPos()) + textRelativeYPos) / g_fScreenRate_y + m_fTextAddYPos),
        m_text.c_str(),
        CSprite::GetWidth() / g_fScreenRate_x,
        0,
        RT3_SORT_CENTER);
}

void CButton::ReleaseText()
{
    m_text.clear();
    m_textColors.fill(0);
    m_textColorCount = 0;
    m_textColor = 0;
}

void CButton::SetText(const wchar_t* pszText, DWORD* adwColor)
{
    ReleaseText();

    if (pszText == nullptr)
    {
        return;
    }

    m_text.assign(pszText);

    const std::size_t colorCount = HasCheckVisuals() ? BTN_IMG_MAX : BTN_IMG_MAX / 2;
    if (adwColor != nullptr)
    {
        m_textColorCount = colorCount;
        std::copy_n(adwColor, m_textColorCount, m_textColors.begin());
    }

    ApplyTextColorForState(m_bCheck ? BTN_UP_CHECK : BTN_UP);
}

wchar_t* CButton::GetText() const
{
    if (m_text.empty())
    {
        return nullptr;
    }

    m_textBuffer.assign(m_text.begin(), m_text.end());
    m_textBuffer.push_back(L'\0');
    return m_textBuffer.data();
}

void CButton::ApplyVisualState(int frameIndex)
{
    const int clampedState = ClampStateIndex(frameIndex);
    int spriteFrame = m_imageFrames[clampedState];
    if (spriteFrame < 0)
    {
        spriteFrame = m_imageFrames[BTN_UP];
    }

    CSprite::SetNowFrame(spriteFrame);
    ApplyTextColorForState(clampedState);
}

void CButton::ApplyTextColorForState(int colorIndex)
{
    if (m_textColorCount == 0 || m_text.empty())
    {
        return;
    }

    const int resolvedIndex = ResolveColorIndex(colorIndex);
    if (resolvedIndex < 0)
    {
        return;
    }

    const std::size_t index = static_cast<std::size_t>(resolvedIndex);
    m_textColor = m_textColors[index];
}

bool CButton::HasCheckVisuals() const
{
    return m_imageFrames[BTN_UP_CHECK] >= 0;
}

int CButton::ResolveColorIndex(int requestedIndex) const
{
    if (m_textColorCount == BTN_IMG_MAX)
    {
        return ClampStateIndex(requestedIndex);
    }

    if (m_textColorCount == BTN_IMG_MAX / 2)
    {
        return ClampStateIndex(requestedIndex) % (BTN_IMG_MAX / 2);
    }

    if (m_textColorCount == 0)
    {
        return -1;
    }

    const int clamped = ClampStateIndex(requestedIndex);
    return std::min<int>(clamped, static_cast<int>(m_textColorCount - 1));
}
