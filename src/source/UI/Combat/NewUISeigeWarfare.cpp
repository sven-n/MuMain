// CSiegeWarfare.cpp: implementation of the CSiegeWarfare class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/Combat/NewUISeigeWarfare.h"
#include "UI/Core/NewUISystem.h"
#include "UI/Combat/NewUISiegeWarCommander.h"
#include "UI/Combat/NewUISiegeWarSoldier.h"
#include "UI/Combat/NewUISiegeWarObserver.h"
#include "Engine/Object/ZzzInventory.h"
#include "Guild/UIGuildInfo.h"
#include "World/MapInfra/MapManager.h"

using namespace SEASON3B;
using namespace mu::ui::window;

mu::ui::window::CSiegeWarfare::CSiegeWarfare()
{
    m_pNewUIMng = NULL;
    m_pSiegeWarUI = NULL;
    m_iCurSiegeWarType = SIEGEWAR_TYPE_NONE;
    m_byGuildStatus = G_NONE;
    m_sGuildMarkIndex = -1;

    m_iHour = 0;
    m_iMinute = 0;
    m_iSecond = 0;
    m_dwSyncTime = 0;

    m_bCreated = true;

    memset(&m_Pos, 0, sizeof(POINT));
}

mu::ui::window::CSiegeWarfare::~CSiegeWarfare()
{
    Release();
}

bool mu::ui::window::CSiegeWarfare::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_SIEGEWARFARE, this);

    Show(true);

    SetPos(x, y);

    return true;
}

void mu::ui::window::CSiegeWarfare::Release()
{
    if (m_pSiegeWarUI)
    {
        m_pSiegeWarUI->UnLoadImages();
        m_pSiegeWarUI->Release();
        SAFE_DELETE(m_pSiegeWarUI);
    }

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CSiegeWarfare::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool mu::ui::window::CSiegeWarfare::UpdateMouseEvent()
{
    if (m_pSiegeWarUI)
    {
        if (!m_pSiegeWarUI->UpdateMouseEvent())
            return false;
    }

    return true;
}

bool mu::ui::window::CSiegeWarfare::UpdateKeyEvent()
{
    if (m_pSiegeWarUI)
    {
        if (!m_pSiegeWarUI->UpdateKeyEvent())
            return false;
    }

    return true;
}

bool mu::ui::window::CSiegeWarfare::Update()
{
    if (IsVisible() == false)
        return true;

    if (m_pSiegeWarUI == NULL)
        return true;

    if (gMapManager.InBattleCastle() && battleCastle::IsBattleCastleStart() == true)
    {
        m_iSecond = m_iSecond - (GetTickCount() - m_dwSyncTime);
        if (m_iSecond <= 0)
        {
            if (m_iMinute <= 0)
            {
                if (m_iHour <= 0)
                {
                    m_iSecond = 0;
                    m_iMinute = 0;
                    m_iHour = 0;
                }
                else
                {
                    --m_iHour;
                    m_iMinute = m_iMinute + 60;
                }
            }
            else
            {
                --m_iMinute;
                m_iSecond = m_iSecond + 60000;
            }
        }

        m_dwSyncTime = GetTickCount();

        m_pSiegeWarUI->SetTime(m_iHour, m_iMinute);
    }

    m_pSiegeWarUI->Update();

    return true;
}

bool mu::ui::window::CSiegeWarfare::Render()
{
    if (m_pSiegeWarUI == NULL || gMapManager.InBattleCastle() == false)
    {
        return true;
    }

    m_pSiegeWarUI->Render();

    return true;
}

float mu::ui::window::CSiegeWarfare::GetLayerDepth()
{
    return 1.6f;
}

void mu::ui::window::CSiegeWarfare::OpenningProcess()
{
}

void mu::ui::window::CSiegeWarfare::ClosingProcess()
{
}

void mu::ui::window::CSiegeWarfare::SetGuildData(const CHARACTER* pCharacter)
{
    m_sGuildMarkIndex = pCharacter->GuildMarkIndex;
    m_byGuildStatus = pCharacter->GuildStatus;
}

bool mu::ui::window::CSiegeWarfare::CreateMiniMapUI()
{
    if (m_pSiegeWarUI != NULL)
    {
        InitMiniMapUI();
    }

    if (!(Hero->EtcPart == PARTS_ATTACK_TEAM_MARK
        || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK2
        || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK3
        || Hero->EtcPart == PARTS_DEFENSE_TEAM_MARK)
        || Hero->GuildStatus == G_PERSON)
    {
        m_byGuildStatus = G_NONE;
    }

    switch (m_byGuildStatus)
    {
    case G_NONE:
    {
        m_pSiegeWarUI = new CSiegeWarObserver;		// Observer
        m_iCurSiegeWarType = SIEGEWAR_TYPE_OBSERVER;
        m_bCreated = false;
    }break;
    case G_MASTER:
    {
        if (wcscmp(GuildMark[m_sGuildMarkIndex].UnionName, L"") == 0
            || wcscmp(GuildMark[m_sGuildMarkIndex].GuildName, GuildMark[m_sGuildMarkIndex].UnionName) == 0)
        {
            m_pSiegeWarUI = new CSiegeWarCommander;	// Commander
            m_iCurSiegeWarType = SIEGEWAR_TYPE_COMMANDER;
        }
        else
        {
            m_pSiegeWarUI = new CSiegeWarSoldier;	// Soldier
            m_iCurSiegeWarType = SIEGEWAR_TYPE_SOLDIER;
        }
        m_bCreated = true;
    }break;
    default:
    {
        m_pSiegeWarUI = new CSiegeWarSoldier;	// Soldier
        m_iCurSiegeWarType = SIEGEWAR_TYPE_SOLDIER;
        m_bCreated = true;
    }break;
    }

    m_pSiegeWarUI->LoadImages();
    m_pSiegeWarUI->Create(m_Pos.x, m_Pos.y);
    Show(true);

    return true;
}

void mu::ui::window::CSiegeWarfare::ClearGuildMemberLocation(void)
{
    if (m_iCurSiegeWarType == SIEGEWAR_TYPE_COMMANDER)
    {
        ((CSiegeWarCommander*)m_pSiegeWarUI)->ClearGuildMemberLocation();
    }
}

void mu::ui::window::CSiegeWarfare::SetGuildMemberLocation(BYTE type, int x, int y)
{
    if (m_iCurSiegeWarType == SIEGEWAR_TYPE_COMMANDER)
    {
        ((CSiegeWarCommander*)m_pSiegeWarUI)->SetGuildMemberLocation(type, x, y);
    }
}

void mu::ui::window::CSiegeWarfare::InitMiniMapUI()
{
    if (m_pSiegeWarUI == NULL)
    {
        return;
    }

    m_pSiegeWarUI->UnLoadImages();
    m_pSiegeWarUI->Release();

    SAFE_DELETE(m_pSiegeWarUI);

    m_iCurSiegeWarType = SIEGEWAR_TYPE_NONE;
    m_byGuildStatus = G_NONE;
    m_sGuildMarkIndex = -1;
}

void  mu::ui::window::CSiegeWarfare::SetTime(BYTE byHour, BYTE byMinute)
{
    m_iHour = (int)byHour;
    m_iMinute = (int)byMinute;
    m_iSecond = 60000;
    m_dwSyncTime = GetTickCount();
}

void mu::ui::window::CSiegeWarfare::SetMapInfo(GuildCommander& data)
{
    if (m_pSiegeWarUI == NULL)
    {
        return;
    }

    m_pSiegeWarUI->SetMapInfo(data);
}

void mu::ui::window::CSiegeWarfare::InitSkillUI()
{
    if (m_pSiegeWarUI == NULL)
    {
        return;
    }

    m_pSiegeWarUI->InitBattleSkill();
}

void mu::ui::window::CSiegeWarfare::ReleaseSkillUI()
{
    if (m_pSiegeWarUI == NULL)
    {
        return;
    }

    m_pSiegeWarUI->ReleaseBattleSkill();
}