#include "stdafx.h"

#include "UI/Combat/SiegeUpdates.h"

#include "UI/Core/WindowAccess.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/UIManager.h"
#include "UI/Combat/GuardWindow.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "UI/NPCs/UIGateKeeper.h"
#include "I18N/All.h"

namespace UI::Siege
{
void ResetMiniMap()
{
    if (g_pSiegeWarfare)
        g_pSiegeWarfare->InitMiniMapUI();
}

void ShowMiniMap()
{
    if (g_pSiegeWarfare)
        g_pSiegeWarfare->CreateMiniMapUI();
}

void EnsureLocalPlayerMiniMap()
{
    if (!g_pSiegeWarfare || g_pSiegeWarfare->IsCreated())
        return;

    g_pSiegeWarfare->InitMiniMapUI();
    g_pSiegeWarfare->SetGuildData(Hero);
    g_pSiegeWarfare->CreateMiniMapUI();
}

void SetBattleSkillsActive(bool active)
{
    if (active)
        g_pSiegeWarfare->InitSkillUI();
    else
        g_pSiegeWarfare->ReleaseSkillUI();
}

void SetCommanderMapInfo(std::uint8_t team, std::uint8_t x, std::uint8_t y, std::uint8_t command)
{
    if (!g_pSiegeWarfare)
        return;

    GuildCommander mapInfo = {team, x, y, command};
    g_pSiegeWarfare->SetMapInfo(mapInfo);
}

void ReplaceMemberLocations(std::span<const MapLocation> locations)
{
    if (g_pSiegeWarfare->GetCurSiegeWarType() != TYPE_GUILD_COMMANDER)
        return;

    g_pSiegeWarfare->ClearGuildMemberLocation();
    for (const MapLocation& location : locations)
        g_pSiegeWarfare->SetGuildMemberLocation(0, location.x, location.y);
}

void AddNpcLocations(std::span<const MapLocation> locations)
{
    if (g_pSiegeWarfare->GetCurSiegeWarType() != TYPE_GUILD_COMMANDER)
        return;

    for (const MapLocation& location : locations)
        g_pSiegeWarfare->SetGuildMemberLocation(location.type + 1, location.x, location.y);
}

void SetMatchTime(std::uint8_t hour, std::uint8_t minute)
{
    g_pSiegeWarfare->SetTime(hour, minute);
}

void ShowGuardStatus(const GuardStatus& status)
{
    UI::Windows::Show(mu::ui::window::INTERFACE_GUARDSMAN);
    g_pGuardWindow->SetData(status);
}

void ReplaceDeclarations(std::span<const DeclarationGuild> guilds)
{
    g_pGuardWindow->ClearDeclareGuildList();
    for (const DeclarationGuild& guild : guilds)
        g_pGuardWindow->AddDeclareGuildList(guild.name, guild.markCount, guild.gaveUp, guild.sequence);
    g_pGuardWindow->SortDeclareGuildList();
}

void ReplaceAttackingGuilds(std::span<const AttackingGuild> guilds)
{
    g_pGuardWindow->ClearGuildList();
    for (const AttackingGuild& guild : guilds)
        g_pGuardWindow->AddGuildList(guild.name, guild.side, guild.involvement, guild.score);
}

void ClearCrownNotices()
{
    g_MessageBox->PopAllMessageBoxes();
}

void ShowCrownNotice(CrownNotice notice, std::wstring_view actor,
                     std::wstring_view switchOwner, std::uint32_t accessTimeMs)
{
    using namespace mu::ui::window;
    CProgressMsgBox* message = nullptr;
    switch (notice)
    {
    case CrownNotice::SwitchReleased:
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CCrownSwitchPopLayout));
        break;
    case CrownNotice::SwitchActivated:
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CCrownSwitchPushLayout));
        break;
    case CrownNotice::SwitchActivatedByOther:
    {
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CCrownSwitchOtherPushLayout), &message);
        if (!message)
            break;
        wchar_t text[256];
        const std::wstring actorName(actor);
        const std::wstring ownerName(switchOwner);
        if (!actor.empty())
            mu_swprintf(text, I18N::Game::CharacterSIs, actorName.c_str());
        else
            mu_swprintf(text, I18N::Game::CharacterIs);
        message->AddMsg(text);
        mu_swprintf(text, I18N::Game::AlreadyPressingS, ownerName.c_str());
        message->AddMsg(text);
        break;
    }
    case CrownNotice::RegistrationStarted:
    case CrownNotice::RegistrationFailed:
    {
        if (notice == CrownNotice::RegistrationStarted)
            CreateMessageBox(MSGBOX_LAYOUT_CLASS(CSealRegisterStartLayout), &message);
        else
            CreateMessageBox(MSGBOX_LAYOUT_CLASS(CSealRegisterFailLayout), &message);
        if (!message)
            break;
        wchar_t text[256];
        int seconds = static_cast<int>(accessTimeMs / 1000);
        if (seconds >= 59)
            seconds = 59;
        mu_swprintf(text, I18N::Game::SAccumulatedHourDseconds,
                    notice == CrownNotice::RegistrationStarted
                        ? I18N::Game::OfficialSealRegistrationWillStart
                        : I18N::Game::OfficialSealRegistrationIsFailed,
                    seconds);
        message->AddMsg(text);
        if (notice == CrownNotice::RegistrationStarted)
            message->SetElapseTime(60000 - accessTimeMs);
        break;
    }
    case CrownNotice::RegistrationSucceeded:
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CSealRegisterSuccessLayout));
        break;
    case CrownNotice::RegistrationByOther:
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CSealRegisterOtherLayout));
        break;
    case CrownNotice::RegistrationByOtherCamp:
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CSealRegisterOtherCampLayout));
        break;
    case CrownNotice::DefenseRemoved:
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CCrownDefenseRemoveLayout));
        break;
    case CrownNotice::DefenseActivated:
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CCrownDefenseCreateLayout));
        break;
    }
}

void OpenHuntZone(std::uint8_t type, bool enabled, int currentPrice, int unitPrice, int maxPrice)
{
    g_pUIGateKeeper->SetInfo(type, enabled, currentPrice, unitPrice, maxPrice);
    UI::Windows::Show(mu::ui::window::INTERFACE_GATEKEEPER);
}

void SetHuntZoneEntranceFee(int fee)
{
    g_pUIGateKeeper->SetEntranceFee(fee);
}

void SetHuntZonePublic(std::uint8_t enabled)
{
    g_pUIGateKeeper->SetPublic(enabled);
}

void OpenCatapult(int key, std::uint8_t weaponType)
{
    UI::Windows::Show(mu::ui::window::INTERFACE_CATAPULT);
    g_pCatapultWindow->Init(key, weaponType);
}

void CatapultFired(int key, std::uint8_t result, std::uint8_t weaponType, int targetX, int targetY)
{
    g_pCatapultWindow->DoFire(key, result, weaponType, targetX, targetY);
}

void CatapultFiredAtPlayer(std::uint8_t weaponType, int targetX, int targetY)
{
    g_pCatapultWindow->DoFireFixStartPosition(weaponType, targetX, targetY);
}
}
