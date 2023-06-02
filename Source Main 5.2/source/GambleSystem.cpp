// GambleSystem.cpp: implementation of the GembleSystem class.
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "GambleSystem.h"

GambleSystem& GambleSystem::Instance()
{
    static GambleSystem s_GambleSys;
    return s_GambleSys;
}

GambleSystem::~GambleSystem()
{
}

void GambleSystem::Init()
{
    m_isGambleShop = false;
    m_byBuyItemPos = 0;
}