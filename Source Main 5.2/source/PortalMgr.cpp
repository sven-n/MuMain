//////////////////////////////////////////////////////////////////////
// PortalMgr.cpp: implementation of the CPortalMgr class.
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "PortalMgr.h"
#include "ZzzBMD.h"
#include "ZzzCharacter.h"
#include "MapManager.h"

CPortalMgr g_PortalMgr;

CPortalMgr::CPortalMgr()
{
    Reset();
}

CPortalMgr::~CPortalMgr()
{
    Reset();
}

void CPortalMgr::Reset()
{
    ResetPortalPosition();
    ResetRevivePosition();
}

void CPortalMgr::ResetPortalPosition()
{
    m_iPortalWorld = -1;
    m_iPortalPosition_x = 0;
    m_iPortalPosition_y = 0;
}

void CPortalMgr::ResetRevivePosition()
{
    m_iReviveWorld = -1;
    m_iRevivePosition_x = 0;
    m_iRevivePosition_y = 0;
}

BOOL CPortalMgr::IsPortalUsable()
{
    switch (gMapManager.WorldActive)
    {
    case WD_6STADIUM:
    case WD_0LORENCIA:
    case WD_3NORIA:
    case WD_51HOME_6TH_CHAR:
    case WD_2DEVIAS:
    case WD_1DUNGEON:
    case WD_7ATLANSE:
    case WD_4LOSTTOWER:
    case WD_8TARKAN:
    case WD_33AIDA:
    case WD_10HEAVEN:
    case WD_37KANTURU_1ST:
    case WD_38KANTURU_2ND:
    case WD_57ICECITY:
    case WD_56MAP_SWAMP_OF_QUIET:
        return TRUE;
    }
    return FALSE;
}

void CPortalMgr::SavePortalPosition()
{
    m_iPortalWorld = gMapManager.WorldActive;
    m_iPortalPosition_x = Hero->PositionX;
    m_iPortalPosition_y = Hero->PositionY;
}

void CPortalMgr::SaveRevivePosition()
{
    m_iReviveWorld = gMapManager.WorldActive;
    m_iRevivePosition_x = Hero->PositionX;
    m_iRevivePosition_y = Hero->PositionY;
}

BOOL CPortalMgr::IsPortalPositionSaved()
{
    return (m_iPortalWorld != -1);
}

BOOL CPortalMgr::IsRevivePositionSaved()
{
    return (m_iReviveWorld != -1);
}

void CPortalMgr::GetPortalPositionText(wchar_t* pszOut)
{
    if (pszOut == NULL) return;

    if (m_iPortalWorld == -1)
    {
        assert(!"이동 위치가 지정되지 않은 상태임");
    }
    else
    {
        swprintf(pszOut, L"%s (%d, %d)", gMapManager.GetMapName(m_iPortalWorld), m_iPortalPosition_x, m_iPortalPosition_y);
    }
}

void CPortalMgr::GetRevivePositionText(wchar_t* pszOut)
{
    if (pszOut == NULL) return;

    if (m_iReviveWorld == -1)
    {
        assert(!"이동 위치가 지정되지 않은 상태임");
    }
    else
    {
        swprintf(pszOut, L"%s (%d, %d)", gMapManager.GetMapName(m_iReviveWorld), m_iRevivePosition_x, m_iRevivePosition_y);
    }
}