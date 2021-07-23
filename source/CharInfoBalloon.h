//*****************************************************************************
// File: CharInfoBalloon.h
//
// Desc: interface for the CCharInfoBalloon class.
//		 캐릭터 정보 풍선 클래스.(CCharInfoBalloonMng가 관리함.)
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_CHARINFOBALLOON_H__DC2BBC6F_834B_4738_AB09_361BF8484977__INCLUDED_)
#define AFX_CHARINFOBALLOON_H__DC2BBC6F_834B_4738_AB09_361BF8484977__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Sprite.h"

#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"

class CCharInfoBalloon : public CSprite  
{
protected:
	CHARACTER*	m_pCharInfo;	// 표시할 캐릭터 정보.
	DWORD		m_dwNameColor;	// 이름 색.
	char		m_szName[64];	// 이름.
	char		m_szGuild[64];	// 길드.
	char		m_szClass[64];	// 클래스(직업).

public:
	CCharInfoBalloon();
	virtual ~CCharInfoBalloon();

	void Create(CHARACTER* pCharInfo);
	void Render();

	void SetInfo();
};

#endif // !defined(AFX_CHARINFOBALLOON_H__DC2BBC6F_834B_4738_AB09_361BF8484977__INCLUDED_)
