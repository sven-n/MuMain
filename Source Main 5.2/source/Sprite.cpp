//*****************************************************************************
// File: Sprite.cpp
//*****************************************************************************

#include "stdafx.h"
#include "Sprite.h"

#include "Input.h"

#include "ZzzOpenglUtil.h"

#include <crtdbg.h>

CSprite::CSprite()
{
    m_aFrameTexCoord = NULL;
    m_pTexture = NULL;
}

CSprite::~CSprite()
{
    Release();
}

void CSprite::Release()
{
    m_pTexture = NULL;
    SAFE_DELETE_ARRAY(m_aFrameTexCoord);
}

void CSprite::Create(int nOrgWidth, int nOrgHeight, int nTexID, int nMaxFrame, SFrameCoord* aFrameCoord, int nDatumX, int nDatumY, bool bTile, int nSizingDatums, float fScaleX, float fScaleY)
{
    Release();

    m_fOrgWidth = (float)nOrgWidth;
    m_fOrgHeight = (float)nOrgHeight;
    m_nTexID = nTexID;
    m_pTexture = Bitmaps.FindTexture(m_nTexID);

    m_fScrHeight = (float)WindowHeight / fScaleY;

    m_aScrCoord[LT].fX = 0.0f;
    m_aScrCoord[LT].fY = m_fScrHeight;
    m_aScrCoord[LB].fX = 0.0f;
    m_aScrCoord[LB].fY = m_fScrHeight - m_fOrgHeight;
    m_aScrCoord[RB].fX = m_fOrgWidth;
    m_aScrCoord[RB].fY = m_fScrHeight - m_fOrgHeight;
    m_aScrCoord[RT].fX = m_fOrgWidth;
    m_aScrCoord[RT].fY = m_fScrHeight;

    m_nNowFrame = -1;

    if (-1 < m_nTexID)
    {
        m_aTexCoord[LT].fTU = 0.5f / m_pTexture->Width;
        m_aTexCoord[LT].fTV = 0.5f / m_pTexture->Height;
        m_aTexCoord[LB].fTU = 0.5f / m_pTexture->Width;
        m_aTexCoord[LB].fTV = (m_fOrgHeight - 0.5f) / m_pTexture->Height;
        m_aTexCoord[RB].fTU = (m_fOrgWidth - 0.5f) / m_pTexture->Width;
        m_aTexCoord[RB].fTV = (m_fOrgHeight - 0.5f) / m_pTexture->Height;
        m_aTexCoord[RT].fTU = (m_fOrgWidth - 0.5f) / m_pTexture->Width;
        m_aTexCoord[RT].fTV = 0.5f / m_pTexture->Height;

        if (NULL != aFrameCoord)
        {
            _ASSERT(0 < nMaxFrame);

            m_nMaxFrame = nMaxFrame;

            m_aFrameTexCoord = new STexCoord[m_nMaxFrame];

            for (int i = 0; i < m_nMaxFrame; ++i)
            {
                m_aFrameTexCoord[i].fTU = ((float)aFrameCoord[i].nX + 0.5f) / m_pTexture->Width;
                m_aFrameTexCoord[i].fTV = ((float)aFrameCoord[i].nY + 0.5f) / m_pTexture->Height;
            }

            m_nStartFrame = m_nEndFrame = 0;
            SetNowFrame(0);
            m_bTile = false;
        }
        else
        {
            m_nMaxFrame = 0;
            m_nStartFrame = m_nEndFrame = -1;
            m_bTile = bTile;
        }
    }
    else
    {
        ::memset(m_aTexCoord, 0, sizeof(STexCoord) * POS_MAX);

        m_nMaxFrame = 0;
        m_nStartFrame = m_nEndFrame = -1;
        m_bTile = false;
    }

    m_byAlpha = m_byRed = m_byGreen = m_byBlue = 255;

    m_fDatumX = (float)nDatumX;
    m_fDatumY = (float)nDatumY;

    m_bRepeat = false;
    m_dDelayTime = m_dDeltaTickSum = 0.0;
    m_nSizingDatums = nSizingDatums;
    m_fScaleX = fScaleX;
    m_fScaleY = fScaleY;
    m_bShow = false;
}

void CSprite::Create(SImgInfo* pImgInfo, int nDatumX, int nDatumY, bool bTile,
    int nSizingDatums, float fScaleX, float fScaleY)
{
    if (pImgInfo->nX == 0 && pImgInfo->nY == 0)
        Create(pImgInfo->nWidth, pImgInfo->nHeight, pImgInfo->nTexID, 0, NULL, nDatumX, nDatumY, bTile, nSizingDatums, fScaleX, fScaleY);
    else
    {
        SFrameCoord frameCoord = { pImgInfo->nX, pImgInfo->nY };
        Create(pImgInfo->nWidth, pImgInfo->nHeight, pImgInfo->nTexID, 1,
            &frameCoord, nDatumX, nDatumY, bTile, nSizingDatums, fScaleX,
            fScaleY);
    }
}

void CSprite::SetPosition(int nXCoord, int nYCoord, CHANGE_PRAM eChangedPram)
{
    if (eChangedPram & X)
    {
        float fWidth = m_aScrCoord[RT].fX - m_aScrCoord[LT].fX;

        if (IS_SIZING_DATUMS_R(m_nSizingDatums))
        {
            m_aScrCoord[RT].fX = m_aScrCoord[RB].fX = (float)nXCoord + m_fOrgWidth - m_fDatumX;
            m_aScrCoord[LT].fX = m_aScrCoord[LB].fX = m_aScrCoord[RT].fX - fWidth;
        }
        else
        {
            m_aScrCoord[LT].fX = m_aScrCoord[LB].fX = (float)nXCoord - m_fDatumX;
            m_aScrCoord[RT].fX = m_aScrCoord[RB].fX = m_aScrCoord[LT].fX + fWidth;
        }
    }

    if (eChangedPram & Y)
    {
        float fHeight = m_aScrCoord[LT].fY - m_aScrCoord[LB].fY;

        if (IS_SIZING_DATUMS_B(m_nSizingDatums))
        {
            m_aScrCoord[LB].fY = m_aScrCoord[RB].fY = m_fScrHeight - (float)nYCoord - m_fOrgHeight + m_fDatumY;

            m_aScrCoord[LT].fY = m_aScrCoord[RT].fY = m_aScrCoord[LB].fY + fHeight;
        }
        else
        {
            m_aScrCoord[LT].fY = m_aScrCoord[RT].fY = m_fScrHeight - (float)nYCoord + m_fDatumY;
            m_aScrCoord[LB].fY = m_aScrCoord[RB].fY = m_aScrCoord[LT].fY - fHeight;
        }
    }
}

void CSprite::SetSize(int nWidth, int nHeight, CHANGE_PRAM eChangedPram)
{
    if (eChangedPram & X)
    {
        if (IS_SIZING_DATUMS_R(m_nSizingDatums))
        {
            m_aScrCoord[LT].fX = m_aScrCoord[LB].fX = m_aScrCoord[RT].fX - (float)nWidth;
            if (m_bTile)
                m_aTexCoord[LT].fTU = m_aTexCoord[LB].fTU = m_aTexCoord[RT].fTU - nWidth / m_pTexture->Width;
        }
        else
        {
            m_aScrCoord[RT].fX = m_aScrCoord[RB].fX = m_aScrCoord[LT].fX + (float)nWidth;
            if (m_bTile)
                m_aTexCoord[RT].fTU = m_aTexCoord[RB].fTU = nWidth / m_pTexture->Width;
        }
    }
    if (eChangedPram & Y)
    {
        if (IS_SIZING_DATUMS_B(m_nSizingDatums))
        {
            m_aScrCoord[LT].fY = m_aScrCoord[RT].fY = m_aScrCoord[LB].fY + (float)nHeight;
            if (m_bTile)
                m_aTexCoord[LT].fTV = m_aTexCoord[RT].fTV = m_aTexCoord[LB].fTV - nHeight / m_pTexture->Height;
        }
        else
        {
            m_aScrCoord[LB].fY = m_aScrCoord[RB].fY = m_aScrCoord[LT].fY - (float)nHeight;
            if (m_bTile)
                m_aTexCoord[LB].fTV = m_aTexCoord[RB].fTV = nHeight / m_pTexture->Height;
        }
    }
}

BOOL CSprite::PtInSprite(long lXPos, long lYPos)
{
    if (!m_bShow)
        return FALSE;

    POINT pt = { lXPos, lYPos };

    RECT rc = {
        long(m_aScrCoord[LT].fX * m_fScaleX),
        long((m_fScrHeight - m_aScrCoord[LT].fY) * m_fScaleY),
        long(m_aScrCoord[RB].fX * m_fScaleX),
        long((m_fScrHeight - m_aScrCoord[RB].fY) * m_fScaleY)
    };

    return ::PtInRect(&rc, pt);
}

BOOL CSprite::CursorInObject()
{
    CInput& rInput = CInput::Instance();

    return PtInSprite(rInput.GetCursorX(), rInput.GetCursorY());
}

void CSprite::SetAction(int nStartFrame, int nEndFrame, double dDelayTime,
    bool bRepeat)
{
    if (1 >= m_nMaxFrame)
        return;

    _ASSERT(nStartFrame <= nEndFrame && nStartFrame >= 0
        && nEndFrame < m_nMaxFrame);

    m_nStartFrame = m_nNowFrame = nStartFrame;
    m_nEndFrame = nEndFrame;
    m_bRepeat = bRepeat;
    m_dDelayTime = dDelayTime;
}

void CSprite::SetNowFrame(int nFrame)
{
    if (NULL == m_aFrameTexCoord || nFrame == m_nNowFrame)
        return;

    if (nFrame < m_nStartFrame || nFrame > m_nEndFrame)
        return;

    m_nNowFrame = nFrame;

    float fTUWidth = m_aTexCoord[RT].fTU - m_aTexCoord[LT].fTU;
    float fTVHeight = m_aTexCoord[LB].fTV - m_aTexCoord[LT].fTV;

    m_aTexCoord[LT] = m_aFrameTexCoord[m_nNowFrame];

    m_aTexCoord[RT].fTU = m_aFrameTexCoord[m_nNowFrame].fTU + fTUWidth;
    m_aTexCoord[RT].fTV = m_aFrameTexCoord[m_nNowFrame].fTV;

    m_aTexCoord[LB].fTU = m_aFrameTexCoord[m_nNowFrame].fTU;
    m_aTexCoord[LB].fTV = m_aFrameTexCoord[m_nNowFrame].fTV + fTVHeight;

    m_aTexCoord[RB].fTU = m_aTexCoord[RT].fTU;
    m_aTexCoord[RB].fTV = m_aTexCoord[LB].fTV;
}

void CSprite::Update(double dDeltaTick)
{
    if (!m_bShow)
        return;

    if (1 >= m_nMaxFrame)
        return;

    m_dDeltaTickSum += dDeltaTick;

    if (m_dDeltaTickSum >= m_dDelayTime)
    {
        int nFrame = m_nNowFrame;

        if (m_bRepeat)
            nFrame = ++nFrame > m_nEndFrame ? m_nStartFrame : nFrame;
        else
            nFrame = ++nFrame > m_nEndFrame ? m_nEndFrame : nFrame;

        SetNowFrame(nFrame);

        m_dDeltaTickSum = 0.0;
    }
}

void CSprite::Render()
{
    if (!m_bShow)
        return;

    if (-1 < m_nTexID)
    {
        if (!TextureEnable)
        {
            TextureEnable = true;
            ::glEnable(GL_TEXTURE_2D);
        }

        BindTexture(m_nTexID);

        ::glBegin(GL_TRIANGLE_FAN);

        ::glColor4ub(m_byRed, m_byGreen, m_byBlue, m_byAlpha);

        for (int i = LT; i < POS_MAX; ++i)
        {
            ::glTexCoord2f(m_aTexCoord[i].fTU, m_aTexCoord[i].fTV);
            ::glVertex2f(m_aScrCoord[i].fX * m_fScaleX,
                m_aScrCoord[i].fY * m_fScaleY);
        }

        ::glEnd();
    }
    else
    {
        if (TextureEnable)
        {
            TextureEnable = false;
            ::glDisable(GL_TEXTURE_2D);
        }

        ::glBegin(GL_TRIANGLE_FAN);

        ::glColor4ub(m_byRed, m_byGreen, m_byBlue, m_byAlpha);
        for (int i = LT; i < POS_MAX; ++i)
            ::glVertex2f(m_aScrCoord[i].fX * m_fScaleX,
                m_aScrCoord[i].fY * m_fScaleY);

        ::glEnd();
    }
}