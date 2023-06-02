//*****************************************************************************
// File: Sprite.h
//*****************************************************************************

#if !defined(AFX_SPRITE_H__1696B800_F6E1_4AB2_AA02_F67BBA8EFD2E__INCLUDED_)
#define AFX_SPRITE_H__1696B800_F6E1_4AB2_AA02_F67BBA8EFD2E__INCLUDED_

#pragma once

#include "ZzzTexture.h"
#include "UIBaseDef.h"

#define	SPR_SIZING_DATUMS_LT	0x00
#define	SPR_SIZING_DATUMS_LB	0x01
#define	SPR_SIZING_DATUMS_RT	0x02
#define	SPR_SIZING_DATUMS_RB	0x03
#define	IS_SIZING_DATUMS_R(v)	(v & 0x02)
#define	IS_SIZING_DATUMS_B(v)	(v & 0x01)

class CSprite
{
protected:
    float		m_fScrHeight;

    BITMAP_t* m_pTexture;
    int			m_nTexID;
    float		m_fOrgWidth;
    float		m_fOrgHeight;

    SScrCoord	m_aScrCoord[POS_MAX];
    STexCoord	m_aTexCoord[POS_MAX];
    float		m_fDatumX;
    float		m_fDatumY;

    BYTE		m_byAlpha;
    BYTE		m_byRed;
    BYTE		m_byGreen;
    BYTE		m_byBlue;

    int			m_nMaxFrame;
    int			m_nNowFrame;
    int			m_nStartFrame;
    int			m_nEndFrame;
    STexCoord* m_aFrameTexCoord;
    bool		m_bRepeat;
    double		m_dDelayTime;
    double		m_dDeltaTickSum;

    float		m_fScaleX;
    float		m_fScaleY;
    bool		m_bTile;
    int			m_nSizingDatums;
    bool		m_bShow;

public:
    CSprite();
    virtual ~CSprite();

    void Release();
    void Create(int nOrgWidth, int nOrgHeight, int nTexID = -1,
        int nMaxFrame = 0, SFrameCoord* aFrameCoord = NULL, int nDatumX = 0,
        int nDatumY = 0, bool bTile = false,
        int nSizingDatums = SPR_SIZING_DATUMS_LT, float fScaleX = 1.0f,
        float fScaleY = 1.0f);
    void Create(SImgInfo* pImgInfo, int nDatumX = 0, int nDatumY = 0,
        bool bTile = false, int nSizingDatums = SPR_SIZING_DATUMS_LT,
        float fScaleX = 1.0f, float fScaleY = 1.0f);
    void SetPosition(int nXCoord, int nYCoord, CHANGE_PRAM eChangedPram = XY);
    int GetXPos() { return (int)m_aScrCoord[LT].fX; }
    int GetYPos() { return int(m_fScrHeight - m_aScrCoord[LT].fY); }
    void SetSize(int nWidth, int nHeight, CHANGE_PRAM eChangedPram = XY);
    int GetWidth() { return int(m_aScrCoord[RT].fX - m_aScrCoord[LT].fX); }
    int GetHeight() { return int(m_aScrCoord[LT].fY - m_aScrCoord[LB].fY); }
    int GetTexID() { return m_nTexID; };
    int GetTexWidth()
    {
        return -1 < m_nTexID ? (int)m_pTexture->Width : 0;
    }
    int GetTexHeight()
    {
        return -1 < m_nTexID ? (int)m_pTexture->Height : 0;
    }

    float GetScaleX() { return m_fScaleX; }
    float GetScaleY() { return m_fScaleY; }
    void Show(bool bShow = true) { m_bShow = bShow; }
    bool IsShow() { return m_bShow; }
    int GetSizingDatums() { return m_nSizingDatums; }
    BOOL PtInSprite(long lXPos, long lYPos);
    BOOL CursorInObject();
    void SetAlpha(BYTE byAlpha) { m_byAlpha = byAlpha; }
    BYTE GetAlpha() { return m_byAlpha; }
    void SetColor(BYTE byRed, BYTE byGreen, BYTE byBlue)
    {
        m_byRed = byRed;	m_byGreen = byGreen;	m_byBlue = byBlue;
    }
    void SetAction(int nStartFrame, int nEndFrame, double dDelayTime = 0.0,
        bool bRepeat = true);
    void SetNowFrame(int nFrame);
    void Update(double dDeltaTick = 0.0);
    void Render();
};

#endif // !defined(AFX_SPRITE_H__1696B800_F6E1_4AB2_AA02_F67BBA8EFD2E__INCLUDED_)
