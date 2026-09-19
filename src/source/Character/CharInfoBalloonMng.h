//*****************************************************************************
// File: CharInfoBalloonMng.h
//
// Desc: interface for the CCharInfoBalloonMng class.
//		 캐릭터 정보 풍선 관리 클래스.(캐릭터 선택씬에서 쓰임)
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_CHARINFOBALLOONMNG_H__37129186_F7FE_4FBC_87BD_189E01191E8F__INCLUDED_)
#define AFX_CHARINFOBALLOONMNG_H__37129186_F7FE_4FBC_87BD_189E01191E8F__INCLUDED_

#pragma once

#include <array>

#include "CharInfoBalloon.h"

class CCharInfoBalloonMng
{
protected:
    static constexpr std::size_t kBalloonCount = 5;
    std::array<CCharInfoBalloon, kBalloonCount> m_charInfoBalloons{};
    bool m_isInitialized{false};

public:
    CCharInfoBalloonMng() = default;
    virtual ~CCharInfoBalloonMng();

    void Release();
    void Create();
    void Render();
    void UpdateDisplay();
};

#endif // !defined(AFX_CHARINFOBALLOONMNG_H__37129186_F7FE_4FBC_87BD_189E01191E8F__INCLUDED_)
