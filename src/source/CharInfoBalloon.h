//*****************************************************************************
// File: CharInfoBalloon.h
//*****************************************************************************

#if !defined(AFX_CHARINFOBALLOON_H__DC2BBC6F_834B_4738_AB09_361BF8484977__INCLUDED_)
#define AFX_CHARINFOBALLOON_H__DC2BBC6F_834B_4738_AB09_361BF8484977__INCLUDED_

#pragma once

#include "Sprite.h"

#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"

class CCharInfoBalloon : public CSprite
{
protected:
    CHARACTER* m_pCharInfo;
    DWORD		m_dwNameColor;
    wchar_t		m_szName[64];
    wchar_t		m_szGuild[64];
    wchar_t		m_szClass[64];

public:
    CCharInfoBalloon();
    virtual ~CCharInfoBalloon();

    void Create(CHARACTER* pCharInfo);
    void Render();

    void SetInfo();
};

#endif // !defined(AFX_CHARINFOBALLOON_H__DC2BBC6F_834B_4738_AB09_361BF8484977__INCLUDED_)
