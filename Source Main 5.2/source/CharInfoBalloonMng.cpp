//*****************************************************************************
// File: CharInfoBalloonMng.cpp
//
// Desc: implementation of the CCharInfoBalloonMng class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "CharInfoBalloonMng.h"

#include "CharInfoBalloon.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCharInfoBalloonMng::CCharInfoBalloonMng() : m_pCharInfoBalloon(NULL)
{
}

CCharInfoBalloonMng::~CCharInfoBalloonMng()
{
    Release();
}

void CCharInfoBalloonMng::Release()
{
    SAFE_DELETE_ARRAY(m_pCharInfoBalloon);
}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 캐릭터 정보 풍선 매니저 생성.
//			   (캐릭터 선택씬에서 쓰임. 풍선 5개 생성.)
//*****************************************************************************
void CCharInfoBalloonMng::Create()
{
    if (NULL == m_pCharInfoBalloon)
        m_pCharInfoBalloon = new CCharInfoBalloon[5];

    for (int i = 0; i < 5; ++i)
        m_pCharInfoBalloon[i].Create(&CharactersClient[i]);
}

//*****************************************************************************
// 함수 이름 : Render()
// 함수 설명 : 캐릭터 정보 풍선들 렌더.
//*****************************************************************************
void CCharInfoBalloonMng::Render()
{
    if (NULL == m_pCharInfoBalloon)
        return;

    for (int i = 0; i < 5; ++i)
        m_pCharInfoBalloon[i].Render();
}

//*****************************************************************************
// 함수 이름 : UpdateDisplay()
// 함수 설명 : 캐릭터 정보를 업데이트.
//*****************************************************************************
void CCharInfoBalloonMng::UpdateDisplay()
{
    if (NULL == m_pCharInfoBalloon)
        return;

    for (int i = 0; i < 5; ++i)
        m_pCharInfoBalloon[i].SetInfo();
}