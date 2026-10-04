#include "stdafx.h"

#include "UI/HUD/HudUpdates.h"

#include "UI/Core/WindowSystem.h"
#include "UI/HUD/GensRanking.h"
#include "UI/HUD/MasterLevel.h"
#include "UI/HUD/UIMapName.h"

#include <string>

extern CUIMapName* g_pUIMapName;

namespace UI::Hud
{
void SetGensStanding(int contribution, int ranking, int nextContribution)
{
    g_pNewUIGensRanking->SetContribution(contribution);
    g_pNewUIGensRanking->SetRanking(ranking);
    g_pNewUIGensRanking->SetNextContribution(nextContribution);
}

void ShowMapName()
{
    g_pUIMapName->ShowMapName();
}

void ShowExperienceGain(std::int64_t previous, std::int64_t gained, bool master)
{
    if (master)
    {
        g_pMainFrame->SetPreExp_Wide(previous);
        g_pMainFrame->SetGetExp_Wide(gained);
    }
    else
    {
        g_pMainFrame->SetPreExp(previous);
        g_pMainFrame->SetGetExp(gained);
    }
}

void ClearSkillHotkeys()
{
    g_pMainFrame->ResetSkillHotKey();
}

void SetSkillHotkey(int hotkey, int skillIndex)
{
    g_pMainFrame->SetSkillHotKey(hotkey, skillIndex);
}

void SetItemHotkey(ItemHotkey hotkey, int itemType, int itemLevel)
{
    static constexpr int keys[] = { mu::ui::window::HOTKEY_Q, mu::ui::window::HOTKEY_W, mu::ui::window::HOTKEY_E,
                                    mu::ui::window::HOTKEY_R };
    g_pMainFrame->SetItemHotKey(keys[static_cast<int>(hotkey)], itemType, itemLevel);
}

void AddSlideNotice(int loopCount, int loopDelay, std::wstring_view text, int type, float speed,
                    std::uint32_t color)
{
    std::wstring notice(text);
    g_pSlideHelpMgr->AddSlide(loopCount, loopDelay, notice.c_str(), type, speed, color);
}

void SetGameOver(bool gameOver)
{
    g_pNewUIHotKey->SetStateGameOver(gameOver);
}

void SetMoveCommandKey(std::uint32_t key)
{
    g_pMoveCommandWindow->SetMoveCommandKey(key);
}

void ReplaceMasterSkills(CLASS_TYPE heroClass, std::span<const MasterSkill> skills)
{
    auto tree = mu::ui::window::CSystem::GetInstance()->GetUI_NewMasterLevelInterface();
    tree->SetMasterType(heroClass);
    tree->InitMasterSkillPoint();
    for (const MasterSkill& skill : skills)
        tree->SetMasterSkillTreeInfo(skill.index, skill.level, skill.value, skill.nextValue);
}

void UpgradeMasterSkill(const MasterSkill& skill)
{
    auto tree = mu::ui::window::CSystem::GetInstance()->GetUI_NewMasterLevelInterface();
    tree->SkillUpgrade(skill.index, skill.level, skill.value, skill.nextValue);
}
}
