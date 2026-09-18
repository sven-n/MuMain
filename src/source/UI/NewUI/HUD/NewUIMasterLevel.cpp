#include "stdafx.h"
#include "App/Platform/Windows/Winmain.h"
#include "Render/Textures/ZzzTexture.h"
#include "GameLogic/Items/CSItemOption.h"
#include "UI/Legacy/UIControls.h"
#include "UI/NewUI/NewUISystem.h"
#include "UI/NewUI/HUD/NewUIMasterLevel.h"
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"
#include "UI/NewUI/Dialogs/NewUICommonMessageBox.h"
#include "GameLogic/Skills/SkillManager.h"

namespace 
{
    _MASTER_SKILLTREE_DATA m_stMasterSkillTreeData[MAX_MASTER_SKILL_DATA];
    _MASTER_SKILL_TOOLTIP m_stMasterSkillTooltip[MAX_MASTER_SKILL_DATA];

    using SUGGEST_PRESET = SEASON3B::CNewUIMasterLevel::SUGGEST_PRESET;

    // Build presets. Each entry is a skill and the level the build wants it
    // at; the entries are filled in order, so put the skills the build cares
    // about most first. Skills which are only needed to unlock something else
    // (a parent skill, or the 10 points a rank needs before the next one
    // opens) do not have to be listed - CalcSuggestion adds them by itself.
    const MASTER_SUGGEST_ENTRY g_aBladeMasterPvE[] = {
        { AT_SKILL_DurabilityReduction1, 10 },
        { AT_SKILL_MaximumSDincrease, 20 },
        { AT_SKILL_DefenseIncrease, 20 },
        { AT_SKILL_AttackSuccRateInc, 20 },
        { AT_SKILL_TWISTING_SLASH_STR, 20 },
        { AT_SKILL_TWISTING_SLASH_MASTERY, 20 },
        { AT_SKILL_MaximumLifeIncrease, 20 },
        { AT_SKILL_WeaponMasteryBladeMaster, 20 },
        { AT_SKILL_STRIKE_OF_DESTRUCTION_STR, 20 },
        { AT_SKILL_PvPAttackRate, 10 },
        { AT_SKILL_TwoHandedSwordStrengthener, 20 },
        { AT_SKILL_TwoHandedSwordMaster, 20 },
        { AT_SKILL_SWELL_LIFE_STR, 20 },
        { AT_SKILL_SWELL_LIFE_PROFICIENCY, 20 },
        { AT_SKILL_MinimumAttackPowerInc, 20 },
        { AT_SKILL_MonsterAttackLifeInc, 20 },
    };

    const MASTER_SUGGEST_ENTRY g_aBladeMasterPvP[] = {
        { AT_SKILL_PvPDefenceRateInc, 20 },
        { AT_SKILL_MaximumSDincrease, 20 },
        { AT_SKILL_SdRecoverySpeedInc, 20 },
        { AT_SKILL_DefenseIncrease, 20 },
        { AT_SKILL_DefenseSuccessRateInc, 20 },
        { AT_SKILL_DEATHSTAB_STR, 20 },
        { AT_SKILL_MaximumLifeIncrease, 20 },
        { AT_SKILL_WeaponMasteryBladeMaster, 20 },
        { AT_SKILL_MaximumManaIncrease, 20 },
        { AT_SKILL_PvPAttackRate, 20 },
        { AT_SKILL_OneHandedSwordStrengthener, 20 },
        { AT_SKILL_OneHandedSwordMaster, 10 },
        { AT_SKILL_SWELL_LIFE_STR, 20 },
        { AT_SKILL_SWELL_LIFE_PROFICIENCY, 20 },
        { AT_SKILL_MinimumAttackPowerInc, 20 },
    };

    const MASTER_SUGGEST_ENTRY g_aGrandMasterPvE[] = {
        { AT_SKILL_DurabilityReduction1, 10 },
        { AT_SKILL_MaximumSDincrease, 20 },
        { AT_SKILL_AutomaticManaRecInc, 20 },
        { AT_SKILL_DefenseIncrease, 20 },
        { AT_SKILL_EXPANSION_OF_WIZARDRY_STR, 20 },
        { AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY, 20 },
        { AT_SKILL_EVIL_SPIRIT_STR, 20 },
        { AT_SKILL_MagicMasteryGrandMaster, 20 },
        { AT_SKILL_BLAST_STR, 20 },
        { AT_SKILL_ICE_STR, 20 },
        { AT_SKILL_OneHandedStaffStrengthener, 20 },
        { AT_SKILL_OneHandedStaffMaster, 10 },
        { AT_SKILL_SOUL_BARRIER_STR, 20 },
        { AT_SKILL_SOUL_BARRIER_PROFICIENCY, 20 },
        { AT_SKILL_MinimumWizardryInc, 20 },
    };

    const MASTER_SUGGEST_ENTRY g_aGrandMasterPvP[] = {
        { AT_SKILL_PvPDefenceRateInc, 20 },
        { AT_SKILL_MaximumSDincrease, 20 },
        { AT_SKILL_SdRecoverySpeedInc, 20 },
        { AT_SKILL_DefenseIncrease, 20 },
        { AT_SKILL_DefenseSuccessRateInc, 20 },
        { AT_SKILL_EXPANSION_OF_WIZARDRY_STR, 20 },
        { AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY, 20 },
        { AT_SKILL_EVIL_SPIRIT_STR, 20 },
        { AT_SKILL_MagicMasteryGrandMaster, 20 },
        { AT_SKILL_ShieldStrengthenerGrandMaster, 20 },
        { AT_SKILL_ShieldMasteryGrandMaster, 20 },
        { AT_SKILL_SOUL_BARRIER_STR, 20 },
        { AT_SKILL_SOUL_BARRIER_PROFICIENCY, 20 },
        { AT_SKILL_MinimumWizardryInc, 20 },
    };

    const MASTER_SUGGEST_ENTRY g_aHighElfPvE[] = {
        { AT_SKILL_DurabilityReduction1, 10 },
        { AT_SKILL_MaximumSDincrease, 20 },
        { AT_SKILL_DefenseIncrease, 20 },
        { AT_SKILL_TRIPLE_SHOT_STR, 20 },
        { AT_SKILL_TRIPLE_SHOT_MASTERY, 10 },
        { AT_SKILL_ATTACK_STR, 20 },
        { AT_SKILL_ATTACK_MASTERY, 20 },
        { AT_SKILL_WeaponMasteryHighElf, 20 },
        { AT_SKILL_ICE_ARROW_STR, 20 },
        { AT_SKILL_BowStrengthener, 20 },
        { AT_SKILL_BowMastery, 10 },
        { AT_SKILL_INFINITY_ARROW_STR, 20 },
        { AT_SKILL_MinimumAttPowerInc, 20 },
    };

    const MASTER_SUGGEST_ENTRY g_aHighElfPvP[] = {
        { AT_SKILL_PvPDefenceRateInc, 20 },
        { AT_SKILL_MaximumSDincrease, 20 },
        { AT_SKILL_SdRecoverySpeedInc, 20 },
        { AT_SKILL_DefenseIncrease, 20 },
        { AT_SKILL_DefenseSuccessRateInc, 20 },
        { AT_SKILL_DEFENSE_STR, 20 },
        { AT_SKILL_DEFENSE_MASTERY, 20 },
        { AT_SKILL_ATTACK_STR, 20 },
        { AT_SKILL_ATTACK_MASTERY, 20 },
        { AT_SKILL_WeaponMasteryHighElf, 20 },
        { AT_SKILL_PvPAttackRate, 20 },
        { AT_SKILL_CrossbowStrengthener, 20 },
        { AT_SKILL_CrossbowMastery, 20 },
        { AT_SKILL_INFINITY_ARROW_STR, 20 },
        { AT_SKILL_MinimumAttPowerInc, 20 },
    };

    const MASTER_SUGGEST_ENTRY g_aHighElfSupport[] = {
        { AT_SKILL_HEALING_STR, 20 },
        { AT_SKILL_DEFENSE_STR, 20 },
        { AT_SKILL_DEFENSE_MASTERY, 20 },
        { AT_SKILL_ATTACK_STR, 20 },
        { AT_SKILL_SummonedMonsterStr1, 20 },
        { AT_SKILL_SummonedMonsterStr2, 20 },
        { AT_SKILL_AutomaticManaRecInc, 20 },
        { AT_SKILL_MaximumSDincrease, 20 },
        { AT_SKILL_DefenseIncrease, 20 },
        { AT_SKILL_ShieldStrengthenerHighElf, 20 },
        { AT_SKILL_ShieldMasteryHighElf, 20 },
    };

    const MASTER_SUGGEST_ENTRY g_aDimensionMasterPvE[] = {
        { AT_SKILL_DurabilityReduction1, 10 },
        { AT_SKILL_MaximumSDincrease, 20 },
        { AT_SKILL_AutomaticManaRecInc, 20 },
        { AT_SKILL_DefenseIncrease, 20 },
        { AT_SKILL_LIGHTNING_SHOCK_STR, 20 },
        { AT_SKILL_ALICE_CHAINLIGHTNING_STR, 20 },
        { AT_SKILL_MagicMasterySummoner, 20 },
        { AT_SKILL_LightningTomeStren, 20 },
        { AT_SKILL_LightningTomeMastery, 20 },
        { AT_SKILL_StickStrengthener, 20 },
        { AT_SKILL_StickMastery, 20 },
        { AT_SKILL_ALICE_BERSERKER_STR, 20 },
        { AT_SKILL_BerserkerProficiency, 20 },
        { AT_SKILL_MinimumWizCurseInc, 20 },
    };

    const MASTER_SUGGEST_ENTRY g_aDimensionMasterPvP[] = {
        { AT_SKILL_PvPDefenceRateInc, 20 },
        { AT_SKILL_MaximumSDincrease, 20 },
        { AT_SKILL_SdRecoverySpeedInc, 20 },
        { AT_SKILL_DefenseIncrease, 20 },
        { AT_SKILL_DefenseSuccessRateInc, 20 },
        { AT_SKILL_ALICE_SLEEP_STR, 20 },
        { AT_SKILL_LIGHTNING_SHOCK_STR, 20 },
        { AT_SKILL_MagicMasterySummoner, 20 },
        { AT_SKILL_ALICE_DRAINLIFE_STR, 20 },
        { AT_SKILL_OtherWorldTomeStreng, 20 },
        { AT_SKILL_OtherWorldTomeMastery, 10 },
        { AT_SKILL_ALICE_BERSERKER_STR, 20 },
        { AT_SKILL_BerserkerProficiency, 20 },
        { AT_SKILL_MinimumWizCurseInc, 20 },
    };

    const MASTER_SUGGEST_ENTRY g_aDuelMasterPvE[] = {
        { AT_SKILL_DurabilityReduction1, 10 },
        { AT_SKILL_MaximumSDincrease, 20 },
        { AT_SKILL_DefenseIncrease, 20 },
        { AT_SKILL_TWISTING_SLASH_STR_MG, 20 },
        { AT_SKILL_WeaponMasteryDuelMaster, 20 },
        { AT_SKILL_BLAST_STR_MG, 20 },
        { AT_SKILL_EVIL_SPIRIT_STR_MG, 20 },
        { AT_SKILL_MagicMasteryDuelMaster, 20 },
        { AT_SKILL_ICE_STR_MG, 20 },
        { AT_SKILL_FIRE_SLASH_STR, 20 },
        { AT_SKILL_AutomaticHpRecInc, 20 },
        { AT_SKILL_DefenseSuccessRateInc, 20 },
    };

    const MASTER_SUGGEST_ENTRY g_aDuelMasterPvP[] = {
        { AT_SKILL_PvPDefenceRateInc, 20 },
        { AT_SKILL_MaximumSDincrease, 20 },
        { AT_SKILL_SdRecoverySpeedInc, 20 },
        { AT_SKILL_DefenseIncrease, 20 },
        { AT_SKILL_DefenseSuccessRateInc, 20 },
        { AT_SKILL_POWER_SLASH_STR, 20 },
        { AT_SKILL_TWISTING_SLASH_STR_MG, 20 },
        { AT_SKILL_WeaponMasteryDuelMaster, 20 },
        { AT_SKILL_EVIL_SPIRIT_STR_MG, 20 },
        { AT_SKILL_MagicMasteryDuelMaster, 20 },
        { AT_SKILL_FIRE_SLASH_STR, 20 },
    };

    const MASTER_SUGGEST_ENTRY g_aLordEmperorPvE[] = {
        { AT_SKILL_DurabilityReduction1, 10 },
        { AT_SKILL_MaximumSDincrease, 20 },
        { AT_SKILL_DefenseIncrease, 20 },
        { AT_SKILL_FIREBURST_STR, 20 },
        { AT_SKILL_FIREBURST_MASTERY, 20 },
        { AT_SKILL_EARTHSHAKE_STR, 20 },
        { AT_SKILL_EARTHSHAKE_MASTERY, 20 },
        { AT_SKILL_WeaponMasteryLordEmperor, 20 },
        { AT_SKILL_FIRE_SCREAM_STR, 20 },
        { AT_SKILL_ScepterStrengthener, 20 },
        { AT_SKILL_ScepterMastery, 20 },
        { AT_SKILL_CommandAttackInc, 20 },
        { AT_SKILL_DarkSpiritStr, 20 },
        { AT_SKILL_DarkSpiritStr2, 20 },
        { AT_SKILL_DarkSpiritStr3, 20 },
    };

    const MASTER_SUGGEST_ENTRY g_aLordEmperorPvP[] = {
        { AT_SKILL_PvPDefenceRateInc, 20 },
        { AT_SKILL_MaximumSDincrease, 20 },
        { AT_SKILL_SdRecoverySpeedInc, 20 },
        { AT_SKILL_DefenseIncrease, 20 },
        { AT_SKILL_DefenseSuccessRateInc, 20 },
        { AT_SKILL_ADD_CRITICAL_STR1, 20 },
        { AT_SKILL_ADD_CRITICAL_STR2, 20 },
        { AT_SKILL_ADD_CRITICAL_STR3, 20 },
        { AT_SKILL_FIREBURST_STR, 20 },
        { AT_SKILL_FIREBURST_MASTERY, 20 },
        { AT_SKILL_WeaponMasteryLordEmperor, 20 },
        { AT_SKILL_ShieldStrengthenerLordEmperor, 20 },
        { AT_SKILL_ShieldMastery, 20 },
        { AT_SKILL_CommandAttackInc, 20 },
        { AT_SKILL_DarkSpiritStr, 20 },
        { AT_SKILL_DarkSpiritStr2, 20 },
    };

    const MASTER_SUGGEST_ENTRY g_aTempleKnightPvE[] = {
        { AT_SKILL_DurabilityReduction1FistMaster, 10 },
        { AT_SKILL_IncreaseMaximumSd, 20 },
        { AT_SKILL_IncreasesDefense, 20 },
        { AT_SKILL_IncreaseAttackSuccessRate, 20 },
        { AT_SKILL_KILLING_BLOW_STR, 20 },
        { AT_SKILL_KILLING_BLOW_MASTERY, 20 },
        { AT_SKILL_IncreaseMaximumHp, 20 },
        { AT_SKILL_WeaponMasteryFistMaster, 20 },
        { AT_SKILL_DRAGON_ROAR_STR, 20 },
        { AT_SKILL_CHAIN_DRIVE_STR, 20 },
        { AT_SKILL_IncreasePvPAttackRate, 10 },
        { AT_SKILL_EquippedWeaponStrengthener, 20 },
        { AT_SKILL_EquippedWeaponMastery, 20 },
        { AT_SKILL_IncreaseMinimumAttackPower, 20 },
        { AT_SKILL_RecoverHPfromMonsterKills, 20 },
    };

    const MASTER_SUGGEST_ENTRY g_aTempleKnightPvP[] = {
        { AT_SKILL_IncreasePvPDefenseRate, 20 },
        { AT_SKILL_IncreaseMaximumSd, 20 },
        { AT_SKILL_IncreaseSdRecoveryRate, 20 },
        { AT_SKILL_IncreasesDefense, 20 },
        { AT_SKILL_IncreaseDefenseSuccessRate, 20 },
        { AT_SKILL_BEAST_UPPERCUT_STR, 20 },
        { AT_SKILL_BEAST_UPPERCUT_MASTERY, 20 },
        { AT_SKILL_IncreaseMaximumHp, 20 },
        { AT_SKILL_WeaponMasteryFistMaster, 20 },
        { AT_SKILL_IncreasePvPAttackRate, 20 },
        { AT_SKILL_DEF_UP_OURFORCES_STR, 20 },
        { AT_SKILL_DEF_UP_OURFORCES_MASTERY, 20 },
        { AT_SKILL_HP_UP_OURFORCES_STR, 20 },
        { AT_SKILL_IncreaseMinimumAttackPower, 20 },
    };

#define MASTER_PRESET(namePtr, table) { namePtr, table, static_cast<int>(_countof(table)) }

    const SUGGEST_PRESET g_aPresetBladeMaster[] = {
        MASTER_PRESET(&I18N::Game::PvE, g_aBladeMasterPvE),
        MASTER_PRESET(&I18N::Game::PvP, g_aBladeMasterPvP),
    };

    const SUGGEST_PRESET g_aPresetGrandMaster[] = {
        MASTER_PRESET(&I18N::Game::PvE, g_aGrandMasterPvE),
        MASTER_PRESET(&I18N::Game::PvP, g_aGrandMasterPvP),
    };

    const SUGGEST_PRESET g_aPresetHighElf[] = {
        MASTER_PRESET(&I18N::Game::PvE, g_aHighElfPvE),
        MASTER_PRESET(&I18N::Game::PvP, g_aHighElfPvP),
        MASTER_PRESET(&I18N::Game::Support, g_aHighElfSupport),
    };

    const SUGGEST_PRESET g_aPresetDimensionMaster[] = {
        MASTER_PRESET(&I18N::Game::PvE, g_aDimensionMasterPvE),
        MASTER_PRESET(&I18N::Game::PvP, g_aDimensionMasterPvP),
    };

    const SUGGEST_PRESET g_aPresetDuelMaster[] = {
        MASTER_PRESET(&I18N::Game::PvE, g_aDuelMasterPvE),
        MASTER_PRESET(&I18N::Game::PvP, g_aDuelMasterPvP),
    };

    const SUGGEST_PRESET g_aPresetLordEmperor[] = {
        MASTER_PRESET(&I18N::Game::PvE, g_aLordEmperorPvE),
        MASTER_PRESET(&I18N::Game::PvP, g_aLordEmperorPvP),
    };

    const SUGGEST_PRESET g_aPresetTempleKnight[] = {
        MASTER_PRESET(&I18N::Game::PvE, g_aTempleKnightPvE),
        MASTER_PRESET(&I18N::Game::PvP, g_aTempleKnightPvP),
    };

#undef MASTER_PRESET
}


SEASON3B::CNewUIMasterLevel::CNewUIMasterLevel()
{
    m_pNewUIMng = nullptr;
    this->ConsumePoint = 0;
    this->CurSkillID = 0;
    this->classCode = MASTER_SKILL_TREE_CLASS_NONE;
    this->m_iSuggestPreset = 0;
    this->m_eSuggestPresetClass = MASTER_SKILL_TREE_CLASS_NONE;
    this->CategoryTextIndex = 0;
    this->categoryPos[0] = { 11,55 };
    this->categoryPos[1] = { 221,55 };
    this->categoryPos[2] = { 431,55 };
    this->InitMasterSkillPoint();
    this->ClearSkillTreeData();
    this->ClearSkillTooltipData();
}

SEASON3B::CNewUIMasterLevel::~CNewUIMasterLevel()
{
    this->Release();
}

BYTE SEASON3B::CNewUIMasterLevel::GetConsumePoint() const
{
    return this->ConsumePoint;
}

int SEASON3B::CNewUIMasterLevel::GetCurSkillID() const
{
    return this->CurSkillID;
}

bool SEASON3B::CNewUIMasterLevel::Create(CNewUIManager* pNewUIMng)
{
    if (nullptr == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_MASTER_LEVEL, this);

    this->SetPos();

    this->LoadImages();

    this->m_CloseBT.ChangeButtonImgState(true, IMAGE_MASTER_INTERFACE + 5, false, false, false);

    this->m_CloseBT.ChangeButtonInfo(611, 9, 13, 14);

    this->m_CloseBT.ChangeToolTipText(&I18N::Game::Close388);

    // Suggestion bar, in the free part of the title bar left of the class name.
    this->m_BtnSuggestPreset.ChangeButtonImgState(true, IMAGE_MASTER_BTN_SUGGEST, true);
    this->m_BtnSuggestPreset.ChangeButtonInfo(14, 5, 58, 20);
    this->m_BtnSuggestPreset.ChangeTextBackColor(RGBA(255, 255, 255, 0));
    this->m_BtnSuggestPreset.ChangeText(&I18N::Game::Suggest);
    this->m_BtnSuggestPreset.ChangeToolTipText(&I18N::Game::ClickToSwitchBuildPreset, true);

    this->m_BtnSuggestApply.ChangeButtonImgState(true, IMAGE_MASTER_BTN_SUGGEST, true);
    this->m_BtnSuggestApply.ChangeButtonInfo(76, 5, 58, 20);
    this->m_BtnSuggestApply.ChangeTextBackColor(RGBA(255, 255, 255, 0));
    this->m_BtnSuggestApply.ChangeText(&I18N::Game::Apply);
    this->m_BtnSuggestApply.ChangeToolTipText(&I18N::Game::AddAllPointsUsingTheSuggestedBuild, true);

    for (int i = 0; i < MAX_MASTER_SKILL_CATEGORY; i++)
    {
        this->ButtonX[i] = 0;

        this->ButtonY[i] = 0;
    }

    return true;
}

void SEASON3B::CNewUIMasterLevel::Release()
{
    this->ClearSkillTreeData();
    this->ClearSkillTooltipData();
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = nullptr;
    }
}

void SEASON3B::CNewUIMasterLevel::SetPos()
{
    this->PosX = 0;
    this->PosY = 0;
    this->width = REFERENCE_WIDTH;
    this->height = 428;
}

void SEASON3B::CNewUIMasterLevel::OpenMasterSkillTreeData(const wchar_t* path)
{
    memset(m_stMasterSkillTreeData, 0, sizeof(m_stMasterSkillTreeData));

    FILE* fp = _wfopen(path, L"rb");

    wchar_t Text[256];

    if (fp == nullptr)
    {
        mu_swprintf(Text, L"%ls - File not exist.", path);
        g_ErrorReport.Write(Text);
        MessageBox(g_hWnd, Text, nullptr, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        return;
    }

    constexpr int Size = sizeof(_MASTER_SKILLTREE_DATA);

    auto Buffer = new BYTE[Size * MAX_MASTER_SKILL_DATA];

    fread(Buffer, Size * MAX_MASTER_SKILL_DATA, 1, fp);

    DWORD dwCheckSum;

    fread(&dwCheckSum, sizeof(DWORD), 1u, fp);

    fclose(fp);

    if (dwCheckSum != GenerateCheckSum2(Buffer, 12288, 0x2BC1))
    {
        mu_swprintf(Text, L"%ls - File corrupted.", path);
        g_ErrorReport.Write(Text);
        MessageBox(g_hWnd, Text, nullptr, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        return;
    }

    BYTE* pSeek = Buffer;

    for (int i = 0; i < MAX_MASTER_SKILL_DATA; i++)
    {
        BuxConvert(pSeek, Size);

        memcpy(&m_stMasterSkillTreeData[i], pSeek, Size);

        pSeek += Size;

        if (pSeek == nullptr)
        {
            break;
        }
    }

    delete[] Buffer;
}

void SEASON3B::CNewUIMasterLevel::OpenMasterSkillTooltip(const wchar_t* path)
{
    memset(m_stMasterSkillTooltip, 0, sizeof(m_stMasterSkillTooltip));

    FILE* fp = _wfopen(path, L"rb");

    if (fp == nullptr)
    {
        wchar_t Text[256];
        mu_swprintf(Text, L"%ls - File not exist.", path);
        g_ErrorReport.Write(Text);
        MessageBox(g_hWnd, Text, nullptr, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        return;
    }

    constexpr int record_size = sizeof(_MASTER_SKILL_TOOLTIP_FILE);
    auto file_buffer = new BYTE[record_size * MAX_MASTER_SKILL_DATA];
    fread(file_buffer, record_size * MAX_MASTER_SKILL_DATA, 1, fp);
    DWORD dwCheckSum;
    fread(&dwCheckSum, sizeof(DWORD), 1u, fp);
    fclose(fp);

    BYTE* pSeek = file_buffer;

    for (int i = 0; i < MAX_MASTER_SKILL_DATA; i++)
    {
        BuxConvert(pSeek, record_size);

        _MASTER_SKILL_TOOLTIP_FILE current{ };
        memcpy(&current, pSeek, record_size);

        const auto target = &m_stMasterSkillTooltip[i];
        target->SkillNumber = static_cast<ActionSkillType>(current.SkillNumber);
        target->ClassCode = static_cast<MASTER_SKILL_TREE_CLASS>(current.ClassCode);
        CMultiLanguage::ConvertFromUtf8(target->Info1, current.Info1);
        CMultiLanguage::ConvertFromUtf8(target->Info2, current.Info2);
        CMultiLanguage::ConvertFromUtf8(target->Info3, current.Info3);
        CMultiLanguage::ConvertFromUtf8(target->Info4, current.Info4);
        CMultiLanguage::ConvertFromUtf8(target->Info5, current.Info5);
        CMultiLanguage::ConvertFromUtf8(target->Info6, current.Info6);
        CMultiLanguage::ConvertFromUtf8(target->Info7, current.Info7);

        pSeek += record_size;

        if (pSeek == nullptr)
        {
            break;
        }
    }

    delete[] file_buffer;
}

void SEASON3B::CNewUIMasterLevel::InitMasterSkillPoint()
{
    for (int i = 0; i < 3; i++)
    {
        this->CategoryPoint[i] = 0;
        for (int k = 0; k < 10; k++)
        {
            this->skillPoint[i][k] = 0;
        }
    }
}

void SEASON3B::CNewUIMasterLevel::SetMasterType(CLASS_TYPE Class)
{
    switch (Class)
    {
    case CLASS_GRANDMASTER:
        this->classCode = MASTER_SKILL_TREE_CLASS_GRANDMASTER;
        break;
    case CLASS_BLADEMASTER:
        this->classCode = MASTER_SKILL_TREE_CLASS_BLADEMASTER;
        break;
    case CLASS_HIGHELF:
        this->classCode = MASTER_SKILL_TREE_CLASS_HIGHELF;
        break;
    case CLASS_DUELMASTER:
        this->classCode = MASTER_SKILL_TREE_CLASS_DUELMASTER;
        break;
    case CLASS_LORDEMPEROR:
        this->classCode = MASTER_SKILL_TREE_CLASS_LORDEMPEROR;
        break;
    case CLASS_DIMENSIONMASTER:
        this->classCode = MASTER_SKILL_TREE_CLASS_DIMENSIONMASTER;
        break;
    case CLASS_TEMPLENIGHT:
        this->classCode = MASTER_SKILL_TREE_CLASS_TEMPLEKNIGHT;
        break;
    default:
        break;
    }

    this->SetMasterSkillTreeData();

    this->SetMasterSkillToolTipData();

    switch (Class)
    {
    case CLASS_WIZARD:
    case CLASS_SOULMASTER:
    case CLASS_GRANDMASTER:
        this->CategoryTextIndex = 1751;
        this->ClassNameTextIndex = 1669;
        break;
    case CLASS_KNIGHT:
    case CLASS_BLADEKNIGHT:
    case CLASS_BLADEMASTER:
        this->CategoryTextIndex = 1755;
        this->ClassNameTextIndex = 1668;
        break;
    case CLASS_ELF:
    case CLASS_MUSEELF:
    case CLASS_HIGHELF:
        this->CategoryTextIndex = 1759;
        this->ClassNameTextIndex = 1670;
        break;
    case CLASS_DARK:
    case CLASS_DUELMASTER:
        this->CategoryTextIndex = 1763;
        this->ClassNameTextIndex = 1671;
        break;
    case CLASS_DARK_LORD:
    case CLASS_LORDEMPEROR:
        this->CategoryTextIndex = 1767;
        this->ClassNameTextIndex = 1672;
        break;
    case CLASS_SUMMONER:
    case CLASS_BLOODYSUMMONER:
    case CLASS_DIMENSIONMASTER:
        this->CategoryTextIndex = 3136;
        this->ClassNameTextIndex = 1689;
        break;
    case CLASS_RAGEFIGHTER:
    case CLASS_TEMPLENIGHT:
        this->CategoryTextIndex = 3330;
        this->ClassNameTextIndex = 3151;
        break;
    default:
        return;
    }
}

void SEASON3B::CNewUIMasterLevel::SetMasterSkillTreeData()
{
    this->ClearSkillTreeData();

    for (int i = 0; i < MAX_MASTER_SKILL_DATA; i++)
    {
        if (m_stMasterSkillTreeData[i].Index == 0)
        {
            break;
        }

        if ((this->classCode & m_stMasterSkillTreeData[i].ClassCode) == 0)
        {
            continue;
        }

        if (!this->map_masterData.insert(std::pair<BYTE, _MASTER_SKILLTREE_DATA>(m_stMasterSkillTreeData[i].Index, m_stMasterSkillTreeData[i])).second)
        {
            break;
        }
    }
}

void SEASON3B::CNewUIMasterLevel::SetMasterSkillToolTipData()
{
    this->ClearSkillTooltipData();

    for (int i = 0; i < MAX_MASTER_SKILL_DATA; i++)
    {
        if (m_stMasterSkillTooltip[i].SkillNumber == 0)
        {
            break;
        }

        if ((this->classCode & m_stMasterSkillTooltip[i].ClassCode) == 0)
        {
            continue;
        }

        if (!this->map_masterSkillToolTip.insert(std::pair(m_stMasterSkillTooltip[i].SkillNumber, m_stMasterSkillTooltip[i])).second)
        {
            break;
        }
    }
}

bool SEASON3B::CNewUIMasterLevel::SetMasterSkillTreeInfo(int index, BYTE skillLevel, float value, float nextvalue)
{
    const auto it = this->map_masterData.find(index);

    if (it == this->map_masterData.end())
    {
        return false;
    }

    const CSkillTreeInfo skillInfo = { skillLevel, value, nextvalue };
    CharacterAttribute->MasterSkillInfo[it->second.Skill] = skillInfo;

    this->CategoryPoint[it->second.Group] += skillLevel;

    return true;
}

int SEASON3B::CNewUIMasterLevel::SetDivideString(wchar_t* text, int isItemTollTip, int TextNum, int iTextColor, int iTextBold, bool isPercent)
{
    if (text == nullptr)
    {
        return TextNum;
    }

    constexpr wchar_t alpszDst[10][256] = {};

    int  nLine = 0;

    if (isItemTollTip == 0)
    {
        nLine = DivideStringByPixel((LPTSTR)alpszDst, 10, 256, text, 150, true, 35);
    }
    else if (isItemTollTip == 1)
    {
        nLine = DivideStringByPixel((LPTSTR)alpszDst, 10, 256, text, 200, true, 35);
    }

    for (int i = 0; i < nLine; i++)
    {
        TextListColor[TextNum] = iTextColor;

        TextBold[TextNum] = iTextBold;

        std::wstring cText = alpszDst[i];

        if (isPercent)
        {
            for (int j = cText.find(L"%", 0); j != -1; j = cText.find(L"%", j + 2))
            {
                cText.insert(j, L"%");
            }
        }

        mu_swprintf(TextList[TextNum], cText.c_str());

        TextNum++;
    }

    return TextNum;
}

bool SEASON3B::CNewUIMasterLevel::Render()
{
    EnableAlphaTest();
    RenderImage(IMAGE_MASTER_INTERFACE, this->PosX, this->PosY, Bitmaps[IMAGE_MASTER_INTERFACE].Width, Bitmaps[IMAGE_MASTER_INTERFACE].Height);
    RenderImage(IMAGE_MASTER_INTERFACE + 1, this->PosX + Bitmaps[IMAGE_MASTER_INTERFACE].Width, this->PosY, Bitmaps[IMAGE_MASTER_INTERFACE + 1].Width, Bitmaps[IMAGE_MASTER_INTERFACE + 1].Height);
    this->RenderIcon();
    this->m_CloseBT.Render();
    this->RenderSuggestButtons();
    DisableAlphaBlend();
    this->RenderText();

    return true;
}

bool SEASON3B::CNewUIMasterLevel::Update()
{
    // The preset index points into the table of the class whose tree is
    // loaded, so a class change has to drop the selection.
    if (this->m_eSuggestPresetClass != this->classCode)
    {
        this->ResetSuggestion();
        this->m_eSuggestPresetClass = this->classCode;
    }

    this->CalcSuggestion();

    return true;
}

bool SEASON3B::CNewUIMasterLevel::UpdateMouseEvent()
{
    if (this->m_CloseBT.UpdateMouseEvent() == true)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_MASTER_LEVEL);

        return true;
    }

    int presetCount = 0;
    const bool hasPreset = (this->GetPresetTable(presetCount) != nullptr && presetCount > 0);

    if (hasPreset && this->m_BtnSuggestPreset.UpdateMouseEvent() == true)
    {
        this->CycleSuggestPreset();
        PlayBuffer(SOUND_CLICK01);

        return true;
    }

    if (hasPreset && this->m_iSuggestPreset > 0 && this->m_BtnSuggestApply.UpdateMouseEvent() == true)
    {
        this->ApplySuggestedPoints();
        PlayBuffer(SOUND_CLICK01);

        return true;
    }

    bool result = true;

    if (SEASON3B::IsPress(VK_LBUTTON) == true)
    {
        result = this->CheckMouse(MouseX, MouseY);

        if (result == false)
        {
            PlayBuffer(SOUND_CLICK01);
        }
    }

    for (int i = 0; i < MAX_MASTER_SKILL_CATEGORY; i++)
    {
        if (this->ButtonX[i] == 1 && SEASON3B::IsPress(VK_LBUTTON) == true)
        {
            this->ButtonX[i] = 0;

            return true;
        }
    }

    this->CheckBtn();

    if (SEASON3B::CheckMouseIn(this->PosX, this->PosY, this->width, this->height) == true)
    {
        return false;
    }

    return result;
}

bool SEASON3B::CNewUIMasterLevel::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MASTER_LEVEL) == false || SEASON3B::IsPress(VK_ESCAPE) == false && SEASON3B::IsPress('A') == false)
    {
        return true;
    }

    g_pNewUISystem->Hide(SEASON3B::INTERFACE_MASTER_LEVEL);

    PlayBuffer(SOUND_CLICK01);

    return false;
}

float SEASON3B::CNewUIMasterLevel::GetLayerDepth()
{
    return 10.1000004;
}

void SEASON3B::CNewUIMasterLevel::LoadImages()
{
    LoadBitmap(L"Interface\\new_Master_back01.jpg", IMAGE_MASTER_INTERFACE, GL_LINEAR, GL_REPEAT, true, false);
    LoadBitmap(L"Interface\\new_Master_back02.jpg", IMAGE_MASTER_INTERFACE + 1, GL_LINEAR, GL_REPEAT, true, false);
    LoadBitmap(L"Interface\\new_Master_Icon.jpg", IMAGE_MASTER_INTERFACE + 2, GL_LINEAR, GL_CLAMP, true, false);
    LoadBitmap(L"Interface\\new_Master_Non_Icon.jpg", IMAGE_MASTER_INTERFACE + 3, GL_LINEAR);
    LoadBitmap(L"Interface\\new_Master_box.tga", IMAGE_MASTER_INTERFACE + 4, GL_LINEAR);
    LoadBitmap(L"Interface\\new_Master_exit.jpg", IMAGE_MASTER_INTERFACE + 5, GL_LINEAR);
    LoadBitmap(L"Interface\\new_Master_arrow01.tga", IMAGE_MASTER_INTERFACE + 6, GL_LINEAR);
    LoadBitmap(L"Interface\\new_Master_arrow02.tga", IMAGE_MASTER_INTERFACE + 7, GL_LINEAR);
    LoadBitmap(L"Interface\\new_Master_arrow03.tga", IMAGE_MASTER_INTERFACE + 8, GL_LINEAR);
    LoadBitmap(L"Interface\\new_Master_arrow04.tga", IMAGE_MASTER_INTERFACE + 9, GL_LINEAR);
    LoadBitmap(L"Interface\\new_Master_arrow05.tga", IMAGE_MASTER_INTERFACE + 10, GL_LINEAR);
    LoadBitmap(L"Interface\\new_Master_arrow06.tga", IMAGE_MASTER_INTERFACE + 11, GL_LINEAR);
    LoadBitmap(L"Interface\\new_Master_arrow07.tga", IMAGE_MASTER_INTERFACE + 12, GL_LINEAR);
    LoadBitmap(L"Interface\\new_Master_arrow08.tga", IMAGE_MASTER_INTERFACE + 13, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_btn_empty_very_small.tga", IMAGE_MASTER_BTN_SUGGEST, GL_LINEAR);
}

void SEASON3B::CNewUIMasterLevel::UnloadImages()
{
    for (int i = 0; i < 15; i++)
    {
        DeleteBitmap(i + IMAGE_MASTER_INTERFACE, false);
    }
}

void SEASON3B::CNewUIMasterLevel::RenderText() const
{
    g_pRenderText->SetFont(g_hFont);

    if (SEASON3B::IsPress(VK_LBUTTON) == false && SEASON3B::CheckMouseIn(458, 11, 81, 10) == true)
    {
        TextList[0][0] = 0;
        TextBold[0] = 0;
        TextListColor[0] = 0;
        mu_swprintf(TextList[0], L"%I64d / %I64d", Master_Level_Data.lMasterLevel_Experince, Master_Level_Data.lNext_MasterLevel_Experince);
        RenderTipTextList(466, 26, 1, 0, 3, 0, 1);
    }

    g_pRenderText->SetTextColor(255, 255, 255, 0xFFu);

    g_pRenderText->SetBgColor(0, 0, 0, 1u);

    wchar_t Buffer[256] = {};

    mu_swprintf(Buffer, I18N::Game::MasterLevelD, Master_Level_Data.nMLevel);

    g_pRenderText->RenderText(275, 11, Buffer, 0, 0, 1, 0);

    mu_swprintf(Buffer, I18N::Game::LevelPointD, Master_Level_Data.nMLevelUpMPoint);

    g_pRenderText->RenderText(372, 11, Buffer, 0, 0, 1, 0);

    if (Master_Level_Data.lNext_MasterLevel_Experince != 0)
    {
        const __int64 iTotalLevel = Master_Level_Data.nMLevel + 400;				// 종합레벨 - 400렙이 만렙이기 때문에 더해준다.
        const __int64 iTOverLevel = iTotalLevel - 255;		// 255레벨 이상 기준 레벨
        __int64 iBaseExperience = 0;					// 레벨 초기 경험치

        const __int64 iData_Master =	// A
            (
                (
                    (__int64)9 + (__int64)iTotalLevel
                    )
                * (__int64)iTotalLevel
                * (__int64)iTotalLevel
                * (__int64)10
                )
            +
            (
                (
                    (__int64)9 + (__int64)iTOverLevel
                    )
                * (__int64)iTOverLevel
                * (__int64)iTOverLevel
                * (__int64)1000
                );

        iBaseExperience = (iData_Master - (__int64)3892250000) / (__int64)2;	// B

        // 레벨업 경험치
        const double fNeedExp = (double)Master_Level_Data.lNext_MasterLevel_Experince - (double)iBaseExperience;

        // 현재 획득한 경험치
        const double fExp = (double)Master_Level_Data.lMasterLevel_Experince - (double)iBaseExperience;

        mu_swprintf(Buffer, I18N::Game::EXP62f, fExp / fNeedExp * 100.0);

        g_pRenderText->RenderText(466, 11, Buffer, 0, 0, 1, 0);
    }

    g_pRenderText->RenderText(154, 11, I18N::Game::Lookup(this->ClassNameTextIndex), 0, 0, 1, nullptr);

    g_pRenderText->SetTextColor(255, 155, 0, 0xFFu);

    mu_swprintf(Buffer, I18N::Game::Lookup(this->CategoryTextIndex), this->CategoryPoint[0]);

    g_pRenderText->RenderText(92, 40, Buffer, 0, 0, RT3_SORT_CENTER, 0);

    mu_swprintf(Buffer, I18N::Game::Lookup(this->CategoryTextIndex + 1), this->CategoryPoint[1]);

    g_pRenderText->RenderText(302, 40, Buffer, 0, 0, RT3_SORT_CENTER, 0);

    mu_swprintf(Buffer, I18N::Game::Lookup(this->CategoryTextIndex + 2), this->CategoryPoint[2]);

    g_pRenderText->RenderText(513, 40, Buffer, 0, 0, RT3_SORT_CENTER, 0);
}

void SEASON3B::CNewUIMasterLevel::RenderIcon()
{
    constexpr int SKILL_ICON_WIDTH = 20;
    constexpr int SKILL_ICON_HEIGHT = 28;

    for (auto it = this->map_masterData.begin(); it != this->map_masterData.end(); it++)
    {
        const auto group = it->second.Group;
        const auto skill = it->second.Skill;
        const auto skillAttribute = &SkillAttribute[skill];
        const auto skillLevel = CharacterAttribute->MasterSkillInfo[skill].GetSkillLevel();

        const int index = (it->second.Index - 1) % 4;
        const BYTE rank = skillAttribute->SkillRank;

        const int CalcX = (int)(index * 49.0f + this->categoryPos[group].x);
        const int CalcY = (int)(this->categoryPos[group].y + (skillAttribute->SkillRank - 1) * 41.0f);

        DWORD textColor;

        RenderImage(IMAGE_MASTER_INTERFACE + 4, CalcX, CalcY, 50, 38, 0, 0, 50.f / 64.f, 38.f / 64.f);

        if (!this->CheckParentSkill(it->second)
            || !this->CheckRankPoint(group, rank, skillLevel)
            || !this->CheckBeforeSkill(skill, skillLevel)
            || !g_csItemOption.IsNonWeaponSkillOrIsSkillEquipped(skill)
            )
        {
            textColor = RGBA(120, 120, 120, 255);

            g_pRenderText->SetTextColor(textColor);
            RenderImage(IMAGE_MASTER_INTERFACE + 3, CalcX + 8, CalcY + 5, SKILL_ICON_WIDTH, SKILL_ICON_HEIGHT, (20.f / 512.f) * (skillAttribute->Magic_Icon % 25), ((28.f / 512.f) * ((skillAttribute->Magic_Icon / 25))), 20.f / 512, 28.f / 512.f);
        }
        else
        {
            textColor = RGBA(255, 255, 255, 255);

            g_pRenderText->SetTextColor(textColor);

            RenderImage(IMAGE_MASTER_INTERFACE + 2, CalcX + 8, CalcY + 5, SKILL_ICON_WIDTH, SKILL_ICON_HEIGHT, (20.f / 512.f) * (skillAttribute->Magic_Icon % 25), ((28.f / 512.f) * ((skillAttribute->Magic_Icon / 25))), 20.f / 512.f, 28.f / 512.f);
        }

        if (it->second.ArrowDirection == 1)
            RenderImage(IMAGE_MASTER_INTERFACE + 6, CalcX + 8 + (SKILL_ICON_WIDTH)+2, CalcY + (SKILL_ICON_HEIGHT / 2), 28, 7, 0, 0, 28 / 32.f, 7 / 8.f);
        if (it->second.ArrowDirection == 2)
            RenderImage(IMAGE_MASTER_INTERFACE + 7, CalcX + 8 + (SKILL_ICON_WIDTH)+2, CalcY + (SKILL_ICON_HEIGHT / 2), 28, 7, 0, 0, 28 / 32.f, 7 / 8.f);
        if (it->second.ArrowDirection == 3)
            RenderImage(IMAGE_MASTER_INTERFACE + 8, CalcX + 8 + (SKILL_ICON_WIDTH / 2) - 3.5, CalcY + SKILL_ICON_HEIGHT + 7, 7, 12, 0, 0, 7 / 8.f, 12 / 16.f);
        if (it->second.ArrowDirection == 4)
            RenderImage(IMAGE_MASTER_INTERFACE + 9, CalcX + 8 + (SKILL_ICON_WIDTH / 2) - 3.5, CalcY + SKILL_ICON_HEIGHT + 7, 7, 52, 0, 0, 7 / 8.f, 12 / 16.f);
        if (it->second.ArrowDirection == 5)
            RenderImage(IMAGE_MASTER_INTERFACE + 10, CalcX, CalcY, 42, 31, 0, 0, 42 / 64.f, 31 / 32.f);
        if (it->second.ArrowDirection == 6)
            RenderImage(IMAGE_MASTER_INTERFACE + 11, CalcX, CalcY, 42, 31, 0, 0, 42 / 64.f, 31 / 32.f);
        if (it->second.ArrowDirection == 7)
            RenderImage(IMAGE_MASTER_INTERFACE + 12, CalcX + 8 + (SKILL_ICON_WIDTH / 2) - 1.5, CalcY + SKILL_ICON_HEIGHT + 8, 40, 28, 0, 0, 40 / 64.f, 28 / 32.f);
        if (it->second.ArrowDirection == 8)
            RenderImage(IMAGE_MASTER_INTERFACE + 13, CalcX, CalcY, 40, 28, 0, 0, 40 / 64.f, 28 / 32.f);

        g_pRenderText->RenderText(CalcX + 8 + 30, CalcY + 28 - 5, std::to_wstring(skillLevel).c_str());
    }

    // Suggestion badges on top of the icons, in a second pass so they are not
    // affected by the per-icon text color above.
    if (this->m_iSuggestPreset > 0)
    {
        g_pRenderText->SetFont(g_hFontBold);
        g_pRenderText->SetTextColor(120, 255, 120, 255);
        g_pRenderText->SetBgColor(0, 0, 0, 0);

        for (auto it = this->map_masterData.begin(); it != this->map_masterData.end(); it++)
        {
            const int points = this->GetSuggestedPoints(it->second.Skill);

            if (points <= 0)
            {
                continue;
            }

            const int index = (it->second.Index - 1) % 4;
            const BYTE rank = SkillAttribute[it->second.Skill].SkillRank;
            const int CalcX = (int)(index * 49.0f + this->categoryPos[it->second.Group].x);
            const int CalcY = (int)(this->categoryPos[it->second.Group].y + (rank - 1) * 41.0f);

            wchar_t badge[16] = {};
            mu_swprintf(badge, L"+%d", points);

            constexpr unsigned int BadgeColor = 0xB0000000u;
            RenderColorQuadARGB(CalcX + 8 + SKILL_ICON_WIDTH + 1, CalcY + 2, 22, 13, BadgeColor);
            g_pRenderText->RenderText(CalcX + 8 + SKILL_ICON_WIDTH + 1, CalcY + 3, badge, 22, 0, RT3_SORT_CENTER);
        }

        g_pRenderText->SetFont(g_hFont);
        g_pRenderText->SetTextColor(255, 255, 255, 255);
    }

    this->RenderToolTip();
}

void SEASON3B::CNewUIMasterLevel::RenderToolTip()
{
    for (auto it = this->map_masterData.begin(); it != this->map_masterData.end(); it++)
    {
        const BYTE group = it->second.Group;

        auto Skill = it->second.Skill;

        SKILL_ATTRIBUTE* p = &SkillAttribute[Skill];

        if (p == nullptr)
        {
            break;
        }

        const int index = (it->second.Index - 1) % 4;

        const int CalcX = (int)(index * 49.0f + this->categoryPos[group].x);

        const int CalcY = (int)(this->categoryPos[group].y + (p->SkillRank - 1) * 41.0f);

        if (SEASON3B::IsPress(VK_LBUTTON) == true || SEASON3B::CheckMouseIn(CalcX + 8, CalcY + 5, 20, 28) == false)
        {
            continue;
        }

        auto mtit = this->map_masterSkillToolTip.find(Skill);

        if (mtit == this->map_masterSkillToolTip.end())
        {
            return;
        }

        auto skillInfo = CharacterAttribute->MasterSkillInfo[Skill];
        const auto skillLevel = skillInfo.GetSkillLevel();
        auto skillValue = skillInfo.GetSkillValue();
        const auto skillNextValue = skillInfo.GetSkillNextValue();

        for (int i = 0; i < 30; i++)
        {
            TextList[i][0] = 0;
        }

        memset(TextBold, 0, sizeof(TextBold));

        for (int i = 0; i < 30; i++)
        {
            TextListColor[i] = i == 0 ? TEXT_COLOR_YELLOW : TEXT_COLOR_WHITE;
        }

        int lineCount = 0;

        mu_swprintf(TextList[lineCount], L"%ls", p->Name);

        TextBold[lineCount] = true;

        lineCount++;

        mu_swprintf(TextList[lineCount], mtit->second.Info1, p->SkillRank, skillLevel, it->second.MaxLevel);

        lineCount++;

        wchar_t buffer[512] = {};

        if (it->second.DefValue == -1.0f)
        {
            mu_swprintf(buffer, mtit->second.Info2);
        }
        else
        {
            mu_swprintf(buffer, mtit->second.Info2, skillLevel != 0 ? skillValue : it->second.DefValue);
        }

        lineCount = this->SetDivideString(buffer, 0, lineCount, 0, 0, true);

        if (skillLevel != 0 && skillLevel < it->second.MaxLevel)
        {
            mu_swprintf(buffer, I18N::Game::NextLevel);

            lineCount = this->SetDivideString(buffer, 0, lineCount, 4, 0, true);

            TextBold[lineCount] = 1;

            mu_swprintf(buffer, mtit->second.Info2, skillNextValue);

            lineCount = this->SetDivideString(buffer, 0, lineCount, 0, 0, true);
        }

        if (skillLevel < it->second.MaxLevel)
        {
            mu_swprintf(buffer, I18N::Game::Requirements3329);

            lineCount = this->SetDivideString(buffer, 0, lineCount, 1, 0, true);

            TextBold[lineCount] = 1;

            mu_swprintf(buffer, mtit->second.Info3, it->second.RequiredPoints);

            if (it->second.RequiredPoints <= Master_Level_Data.nMLevelUpMPoint)
            {
                lineCount = this->SetDivideString(buffer, 0, lineCount, 0, 0, true);
            }
            else
            {
                lineCount = this->SetDivideString(buffer, 0, lineCount, 2, 0, true);
            }
        }

        int iTextColor = this->CheckBeforeSkill(Skill, skillLevel) == true ? 0 : 2;

        mu_swprintf(buffer, mtit->second.Info4);

        lineCount = this->SetDivideString(buffer, 0, lineCount, iTextColor, 0, true);

        if (skillLevel < it->second.MaxLevel && p->SkillRank != 1)
        {
            iTextColor = this->CheckRankPoint(group, p->SkillRank, skillLevel) == true ? 0 : 2;

            mu_swprintf(buffer, mtit->second.Info5);

            lineCount = this->SetDivideString(buffer, 0, lineCount, iTextColor, 0, true);

            for (int i = 0; i < MAX_MASTER_SKILL_REQUIRES; i++)
            {
                const auto RequireSkill = it->second.RequireSkill[i];

                if (RequireSkill >= AT_SKILL_MASTER_BEGIN && RequireSkill <= AT_SKILL_MASTER_END)
                {
                    auto requiredSkill = CharacterAttribute->MasterSkillInfo[RequireSkill];
                    iTextColor = requiredSkill.GetSkillValue() < 10 ? 2 : 0;
                    mu_swprintf(buffer, i == 0 ? mtit->second.Info6 : mtit->second.Info7);
                    lineCount = this->SetDivideString(buffer, 0, lineCount, iTextColor, 0, true);
                }
            }
        }

        if (CalcY > 300)
        {

            RenderTipTextList(CalcX + 8, CalcY + 33, lineCount, 0, 3, STRP_BOTTOMCENTER, 1);
        }
        else
        {
            RenderTipTextList(CalcX + 8, CalcY + 33, lineCount, 0, 3, 0, 1);
        }

    }
}

bool SEASON3B::CNewUIMasterLevel::CheckMouse(int posx, int posy)
{
    constexpr POINT position[3] = { {185,65},{385,65},{585,65} };

    for (int i = 0; i < MAX_MASTER_SKILL_CATEGORY; i++)
    {
        if (SEASON3B::CheckMouseIn(position[i].x + this->PosX, this->ButtonY[i] + position[i].y + this->PosY, 15, 30) == true && this->ButtonX[i] == 0)
        {
            this->ButtonY[i] = 1;

            return false;
        }
    }

    return true;
}

bool SEASON3B::CNewUIMasterLevel::CheckBtn()
{
    constexpr int posX = 220;

    for (int i = 0; i < MAX_MASTER_SKILL_CATEGORY; i++)
    {
        if (this->ButtonX[i] == 1 && SEASON3B::IsRelease(VK_LBUTTON))
        {
            this->ButtonX[i] = 0;
            return false;
        }

        if (this->ButtonX[i] == 1)
        {
            this->ButtonY[i] = MouseY - 65;

            if (this->ButtonY[i] > posX)
            {
                this->ButtonY[i] = posX;
            }
            else if (this->ButtonY[i] <= 0)
            {
                this->ButtonY[i] = 0;
            }
        }
    }

    for (auto it = this->map_masterData.begin(); it != this->map_masterData.end(); it++)
    {
        auto selectedSkill = it->second;
        
        switch (selectedSkill.Group)
        {
        case 0:
        case 1:
        case 2:
            this->CheckAttributeArea(selectedSkill);
            break;
        }
    }

    return true;
}

bool SEASON3B::CNewUIMasterLevel::CheckAttributeArea(const _MASTER_SKILLTREE_DATA& skillData)
{
    if (skillData.Group < 0 || skillData.Group >= 3)
    {
        return false;
    }

    const auto lpskill = &SkillAttribute[skillData.Skill];

    if (lpskill == nullptr)
    {
        return false;
    }

    const int tindex = (skillData.Index - 1) % 4;

    const int posX = (int)((double)this->categoryPos[skillData.Group].x + tindex * 49.0);

    const int posY = (int)((double)this->categoryPos[skillData.Group].y + (lpskill->SkillRank - 1) * 41.0);

    if (!SEASON3B::IsPress(VK_LBUTTON) || SEASON3B::CheckMouseIn(posX + 8, posY + 5, 20, 28) == false)
    {
        return true;
    }

    const auto skillPoint = CharacterAttribute->MasterSkillInfo[skillData.Skill].GetSkillLevel();

    PlayBuffer(SOUND_CLICK01);

    if (!this->CheckSkillPoint(Master_Level_Data.nMLevelUpMPoint, skillData, skillPoint))
    {
        return true;
    }

    if (!g_csItemOption.IsNonWeaponSkillOrIsSkillEquipped(skillData.Skill))
    {
        SEASON3B::CreateOkMessageBox(I18N::Game::YouNeedToWearTheRequiredEquipmentToLevelUpThisSkill);
        return true;
    }

    if (!this->CheckParentSkill(skillData)
        || !this->CheckRankPoint(skillData.Group, lpskill->SkillRank, skillPoint)
        || !this->CheckBeforeSkill(skillData.Skill, skillPoint))
    {
        SEASON3B::CreateOkMessageBox(I18N::Game::YouMustMeetAllSkillRequirements);

        return true;
    }

    this->ConsumePoint = skillData.RequiredPoints;
    
    this->CurSkillID = skillData.Skill;

    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CMaster_Level_Interface));

    MouseLButton = false;

    MouseLButtonPop = false;

    MouseLButtonPush = false;

    return true;
}

bool SEASON3B::CNewUIMasterLevel::CheckSkillPoint(WORD mLevelUpPoint, const _MASTER_SKILLTREE_DATA& skillData, BYTE skillLevel)
{

    if (skillLevel >= skillData.MaxLevel)
    {
        SEASON3B::CreateOkMessageBox(I18N::Game::YouCanTRaiseAnyMoreLevels);
        return false;
    }

    if (mLevelUpPoint >= skillData.RequiredPoints)
    {
        return true;
    }

    wchar_t Buffer[358] = {};

    mu_swprintf(Buffer, I18N::Game::YouCanTRaiseAnyMoreLevels, skillData.RequiredPoints - mLevelUpPoint);

    SEASON3B::CreateOkMessageBox(Buffer);

    return false;
}

bool SEASON3B::CNewUIMasterLevel::CheckParentSkill(const _MASTER_SKILLTREE_DATA& masterSkill)
{
    for (int i = 0; i < MAX_MASTER_SKILL_REQUIRES; i++)
    {
        const auto requiredSkill = masterSkill.RequireSkill[i];
        if (requiredSkill == AT_SKILL_UNDEFINED)
        {
            return true;
        }

        if (requiredSkill < AT_SKILL_MASTER_BEGIN || requiredSkill > AT_SKILL_MASTER_END)
        {
            return true;
        }

        const auto reqSkillLevel = CharacterAttribute->MasterSkillInfo[requiredSkill].GetSkillLevel();
        if (reqSkillLevel < MASTER_SKILL_LEVEL_REQ_FOR_NEXT_RANK)
        {
            return false;
        }
    }

    return true;
}

bool SEASON3B::CNewUIMasterLevel::CheckRankPoint(BYTE group, BYTE rank, BYTE skillLevel)
{
    if (this->skillPoint[group][rank] < skillLevel)
    {
        this->skillPoint[group][rank] = skillLevel;
    }

    if (rank == 1)
    {
        return true;
    }

    return this->skillPoint[group][rank - 1] >= 10;
}

bool SEASON3B::CNewUIMasterLevel::CheckBeforeSkill(ActionSkillType skill, BYTE skillLevel)
{
    if (skillLevel != 0)
    {
        return true;
    }

    const auto Index = SkillAttribute[skill].SkillBrand;

    if (Index == 0)
    {
        return true;
    }

    const SKILL_ATTRIBUTE* lpSkill = &SkillAttribute[Index];

    if (lpSkill == nullptr)
    {
        return false;
    }

    if (lpSkill->SkillUseType == 4)
    {
        return true;
    }

    for (int i = 0; i < MAX_MAGIC; i++)
    {
        if (CharacterAttribute->Skill[i] == Index)
        {
            return true;
        }
    }

    return false;
}

void SEASON3B::CNewUIMasterLevel::SkillUpgrade(int index, BYTE skillLevel, float value, float nextValue)
{
    const auto it = this->map_masterData.find(index);
    if (it == this->map_masterData.end())
    {
        return;
    }

    const auto realSkill = it->second.Skill;
    const int oldLevel = CharacterAttribute->MasterSkillInfo[realSkill].GetSkillLevel();

    const CSkillTreeInfo skillTreeInfo = { skillLevel, value, nextValue };
    CharacterAttribute->MasterSkillInfo[realSkill] = skillTreeInfo;

    // And update the category points
    const int addedPoints = skillLevel - oldLevel;
    this->CategoryPoint[it->second.Group] += addedPoints;
}

const SEASON3B::CNewUIMasterLevel::SUGGEST_PRESET* SEASON3B::CNewUIMasterLevel::GetPresetTable(int& count) const
{
    switch (this->classCode)
    {
    case MASTER_SKILL_TREE_CLASS_BLADEMASTER:
        count = static_cast<int>(_countof(g_aPresetBladeMaster));
        return g_aPresetBladeMaster;
    case MASTER_SKILL_TREE_CLASS_GRANDMASTER:
        count = static_cast<int>(_countof(g_aPresetGrandMaster));
        return g_aPresetGrandMaster;
    case MASTER_SKILL_TREE_CLASS_HIGHELF:
        count = static_cast<int>(_countof(g_aPresetHighElf));
        return g_aPresetHighElf;
    case MASTER_SKILL_TREE_CLASS_DIMENSIONMASTER:
        count = static_cast<int>(_countof(g_aPresetDimensionMaster));
        return g_aPresetDimensionMaster;
    case MASTER_SKILL_TREE_CLASS_DUELMASTER:
        count = static_cast<int>(_countof(g_aPresetDuelMaster));
        return g_aPresetDuelMaster;
    case MASTER_SKILL_TREE_CLASS_LORDEMPEROR:
        count = static_cast<int>(_countof(g_aPresetLordEmperor));
        return g_aPresetLordEmperor;
    case MASTER_SKILL_TREE_CLASS_TEMPLEKNIGHT:
        count = static_cast<int>(_countof(g_aPresetTempleKnight));
        return g_aPresetTempleKnight;
    default:
        break;
    }

    count = 0;

    return nullptr;
}

const _MASTER_SKILLTREE_DATA* SEASON3B::CNewUIMasterLevel::FindSkillData(ActionSkillType skill) const
{
    for (auto it = this->map_masterData.begin(); it != this->map_masterData.end(); it++)
    {
        if (it->second.Skill == skill)
        {
            return &it->second;
        }
    }

    return nullptr;
}

int SEASON3B::CNewUIMasterLevel::GetSuggestedPoints(ActionSkillType skill) const
{
    int points = 0;

    for (const auto& entry : this->m_vSuggestPoint)
    {
        if (entry.SkillNumber == skill)
        {
            points += entry.Points;
        }
    }

    return points;
}

int SEASON3B::CNewUIMasterLevel::GetSimulatedLevel(ActionSkillType skill) const
{
    return CharacterAttribute->MasterSkillInfo[skill].GetSkillLevel() + this->GetSuggestedPoints(skill);
}

int SEASON3B::CNewUIMasterLevel::GetSimulatedRankLevel(BYTE group, BYTE rank) const
{
    int highest = 0;

    for (auto it = this->map_masterData.begin(); it != this->map_masterData.end(); it++)
    {
        if (it->second.Group != group || SkillAttribute[it->second.Skill].SkillRank != rank)
        {
            continue;
        }

        const int level = this->GetSimulatedLevel(it->second.Skill);

        if (level > highest)
        {
            highest = level;
        }
    }

    return highest;
}

void SEASON3B::CNewUIMasterLevel::AddSuggestedPoints(ActionSkillType skill, int points)
{
    // Consecutive points for the same skill are one request; a skill which is
    // filled again later (because something else needed it first) gets its own
    // entry, so the order the server sees stays the order we simulated.
    if (!this->m_vSuggestPoint.empty() && this->m_vSuggestPoint.back().SkillNumber == skill)
    {
        this->m_vSuggestPoint.back().Points += points;

        return;
    }

    this->m_vSuggestPoint.push_back({ skill, points });
}

bool SEASON3B::CNewUIMasterLevel::EnsureRequirements(const _MASTER_SKILLTREE_DATA& skillData, int& freePoints, int depth)
{
    // Same three conditions the tree itself checks before it lets a point in,
    // but against the simulated levels instead of the current ones.
    if (!this->CheckBeforeSkill(skillData.Skill, static_cast<BYTE>(this->GetSimulatedLevel(skillData.Skill))))
    {
        return false;
    }

    if (!g_csItemOption.IsNonWeaponSkillOrIsSkillEquipped(skillData.Skill))
    {
        return false;
    }

    for (int i = 0; i < MAX_MASTER_SKILL_REQUIRES; i++)
    {
        const auto requiredSkill = skillData.RequireSkill[i];

        if (requiredSkill < AT_SKILL_MASTER_BEGIN || requiredSkill > AT_SKILL_MASTER_END)
        {
            continue;
        }

        if (this->GetSimulatedLevel(requiredSkill) < MASTER_SKILL_LEVEL_REQ_FOR_NEXT_RANK)
        {
            this->SpendSuggestedPoints(requiredSkill, MASTER_SKILL_LEVEL_REQ_FOR_NEXT_RANK, freePoints, depth + 1);
        }

        if (this->GetSimulatedLevel(requiredSkill) < MASTER_SKILL_LEVEL_REQ_FOR_NEXT_RANK)
        {
            return false;
        }
    }

    const BYTE rank = SkillAttribute[skillData.Skill].SkillRank;

    if (rank <= 1)
    {
        return true;
    }

    if (this->GetSimulatedRankLevel(skillData.Group, rank - 1) >= MASTER_SKILL_LEVEL_REQ_FOR_NEXT_RANK)
    {
        return true;
    }

    // The rank above only opens once some skill of the rank below is at 10.
    // The preset does not have to name one - take the first one of that rank
    // which can be filled.
    for (auto it = this->map_masterData.begin(); it != this->map_masterData.end(); it++)
    {
        if (it->second.Group != skillData.Group || SkillAttribute[it->second.Skill].SkillRank != rank - 1)
        {
            continue;
        }

        this->SpendSuggestedPoints(it->second.Skill, MASTER_SKILL_LEVEL_REQ_FOR_NEXT_RANK, freePoints, depth + 1);

        if (this->GetSimulatedRankLevel(skillData.Group, rank - 1) >= MASTER_SKILL_LEVEL_REQ_FOR_NEXT_RANK)
        {
            return true;
        }
    }

    return false;
}

void SEASON3B::CNewUIMasterLevel::SpendSuggestedPoints(ActionSkillType skill, int targetLevel, int& freePoints, int depth)
{
    if (freePoints <= 0 || depth > MAX_MASTER_TREE_RANK)
    {
        return;
    }

    const auto* skillData = this->FindSkillData(skill);

    if (skillData == nullptr)
    {
        // Not part of this class' tree - the preset lists a skill the loaded
        // tree data does not have.
        return;
    }

    int level = this->GetSimulatedLevel(skill);
    const int maxLevel = std::min(targetLevel, static_cast<int>(skillData->MaxLevel));

    if (level >= maxLevel)
    {
        return;
    }

    if (!this->EnsureRequirements(*skillData, freePoints, depth))
    {
        return;
    }

    // Learning a skill costs its required points and gives that many levels at
    // once - exactly what the server does - every level after that costs one.
    while (level < maxLevel)
    {
        const int cost = (level == 0) ? std::max(1, static_cast<int>(skillData->RequiredPoints)) : 1;

        if (freePoints < cost || level + cost > skillData->MaxLevel)
        {
            break;
        }

        freePoints -= cost;
        level += cost;

        this->AddSuggestedPoints(skill, cost);
    }
}

void SEASON3B::CNewUIMasterLevel::ResetSuggestion()
{
    this->m_iSuggestPreset = 0;
    this->m_BtnSuggestPreset.ChangeText(&I18N::Game::Suggest);
    this->m_vSuggestPoint.clear();
}

void SEASON3B::CNewUIMasterLevel::CycleSuggestPreset()
{
    int presetCount = 0;
    const SUGGEST_PRESET* presets = this->GetPresetTable(presetCount);

    if (presets == nullptr || presetCount <= 0)
    {
        this->ResetSuggestion();

        return;
    }

    // 0 is "no suggestion", so the cycle runs off -> preset 1 -> ... -> off.
    this->m_iSuggestPreset = (this->m_iSuggestPreset + 1) % (presetCount + 1);

    if (this->m_iSuggestPreset == 0)
    {
        this->m_BtnSuggestPreset.ChangeText(&I18N::Game::Suggest);
    }
    else
    {
        this->m_BtnSuggestPreset.ChangeText(presets[this->m_iSuggestPreset - 1].s_pNameSlot);
    }
}

void SEASON3B::CNewUIMasterLevel::CalcSuggestion()
{
    this->m_vSuggestPoint.clear();

    int presetCount = 0;
    const SUGGEST_PRESET* presets = this->GetPresetTable(presetCount);

    if (this->m_iSuggestPreset <= 0 || presets == nullptr || this->m_iSuggestPreset > presetCount)
    {
        return;
    }

    int freePoints = Master_Level_Data.nMLevelUpMPoint;

    if (freePoints <= 0)
    {
        return;
    }

    const SUGGEST_PRESET& preset = presets[this->m_iSuggestPreset - 1];

    // Rank by rank, because a rank only opens once the one below it has a
    // skill at level 10 - filling the whole preset top down would waste the
    // points on skills the character cannot reach yet. Within a rank the
    // preset order decides.
    for (BYTE rank = 1; rank <= MAX_MASTER_TREE_RANK && freePoints > 0; rank++)
    {
        for (int i = 0; i < preset.s_iEntryCount && freePoints > 0; i++)
        {
            const MASTER_SUGGEST_ENTRY& entry = preset.s_pEntries[i];

            if (SkillAttribute[entry.SkillNumber].SkillRank != rank)
            {
                continue;
            }

            this->SpendSuggestedPoints(entry.SkillNumber, entry.TargetLevel, freePoints, 0);
        }
    }
}

void SEASON3B::CNewUIMasterLevel::ApplySuggestedPoints()
{
    if (this->m_iSuggestPreset <= 0 || Master_Level_Data.nMLevelUpMPoint <= 0)
    {
        return;
    }

    // One packet per skill instead of one per point. The order matters: the
    // server checks every request against the tree, so a parent skill has to
    // arrive before the skill which requires it.
    for (const auto& entry : this->m_vSuggestPoint)
    {
        if (entry.Points <= 0)
        {
            continue;
        }

        SocketClient->ToGameServer()->SendAddMasterSkillPointMultiple(
            static_cast<uint16_t>(entry.SkillNumber), static_cast<BYTE>(entry.Points));
    }

    this->m_vSuggestPoint.clear();
}

void SEASON3B::CNewUIMasterLevel::RenderSuggestButtons()
{
    int presetCount = 0;

    if (this->GetPresetTable(presetCount) == nullptr || presetCount <= 0)
    {
        return;
    }

    this->m_BtnSuggestPreset.Render();

    if (this->m_iSuggestPreset > 0)
    {
        this->m_BtnSuggestApply.Render();
    }
}

void SEASON3B::CNewUIMasterLevel::ClearSkillTreeData()
{
    if (!map_masterSkillToolTip.empty())
        this->map_masterData.clear();
}

void SEASON3B::CNewUIMasterLevel::ClearSkillTooltipData()
{
    if (!map_masterSkillToolTip.empty())
        this->map_masterSkillToolTip.clear();
}
