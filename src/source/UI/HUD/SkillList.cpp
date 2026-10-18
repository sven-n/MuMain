#include "stdafx.h"
#include "UI/HUD/SkillList.h"
#include "GameLogic/Items/ItemCategories.h"
#include <algorithm>
#include "I18N/All.h"

#include "UI/Options/OptionWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Widgets/UIBaseDef.h"
#include "Audio/DSPlaySound.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"

#include "GameLogic/Items/CSItemOption.h"
#include "GameLogic/Events/CSChaosCastle.h"
#include "World/MapInfra/MapManager.h"
#include "Character/CharacterManager.h"
#include "GameLogic/Skills/SkillManager.h"
#include "UI/HUD/Skills/SkillIconAtlas.h"
#include "UI/HUD/Skills/SkillTooltip.h"
#include "UI/Scaling/UITransform.h"
#include "Core/Time/CTimCheck.h"
#include "GameLogic/Social/MonkSystem.h"

#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
#include "GameShop/InGameShopSystem.h"
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME

#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Renderer/MuRenderer.h"
#include "Camera/CameraProjection.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlWorkspaceParticipant.h"
#include "UI/Placement/WindowPlacement.h"
#include "UI/RmlBridge/RmlTooltip.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>
#include "GameLogic/Quests/QuestMng.h"
#include "UI/Social/FriendWindow.h"

mu::ui::window::CSkillList::CSkillList()
{
    m_pNewUIMng = NULL;
    Reset();
}

mu::ui::window::CSkillList::~CSkillList()
{
    Release();
}

bool mu::ui::window::CSkillList::Create(CManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_SKILL_LIST, this);

    LoadImages();

    Show(true);

    return true;
}

void mu::ui::window::CSkillList::Release()
{
    // The tooltip is a plain RmlUi element now, so nothing needs unregistering via
    // UI2DEffectObject/DeleteUI2DEffectObject() here.
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CSkillList::Reset()
{
    m_bSkillList = false;
    m_bHotKeySkillListUp = false;

    for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
    {
        m_iHotKeySkillType[i] = -1;
    }

    m_GridSnapshot.clear();
    m_PetSnapshot.clear();
    m_bTooltipPending = false;
    m_iTooltipSkillIndex = -1;
    m_fTooltipAnchorX = 0.f;
    m_fTooltipAnchorY = 0.f;
    m_iHoveredGridSkillIndex = -1;
}

void mu::ui::window::CSkillList::LoadImages()
{
    // The HUD draws none of these any more (main_frame.rml loads the atlases itself), but the
    // texture slots are shared: CUIMuHelper draws skill icons and boxes from them natively.
    LoadBitmap(L"Interface\\newui_skill.jpg", IMAGE_SKILL1, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_skill2.jpg", IMAGE_SKILL2, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_command.jpg", IMAGE_COMMAND, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_skillbox.jpg", IMAGE_SKILLBOX, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_skillbox2.jpg", IMAGE_SKILLBOX_USE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_non_skill.jpg", IMAGE_NON_SKILL1, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_non_skill2.jpg", IMAGE_NON_SKILL2, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_non_command.jpg", IMAGE_NON_COMMAND, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_skill3.jpg", IMAGE_SKILL3, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_non_skill3.jpg", IMAGE_NON_SKILL3, GL_LINEAR);
}

void mu::ui::window::CSkillList::UnloadImages()
{
    DeleteBitmap(IMAGE_SKILL1);
    DeleteBitmap(IMAGE_SKILL2);
    DeleteBitmap(IMAGE_COMMAND);
    DeleteBitmap(IMAGE_SKILLBOX);
    DeleteBitmap(IMAGE_SKILLBOX_USE);
    DeleteBitmap(IMAGE_NON_SKILL1);
    DeleteBitmap(IMAGE_NON_SKILL2);
    DeleteBitmap(IMAGE_NON_COMMAND);
    DeleteBitmap(IMAGE_SKILL3);
    DeleteBitmap(IMAGE_NON_SKILL3);
}

bool mu::ui::window::CSkillList::UpdateMouseEvent()
{
    // RmlUi's own Context now does hit-testing for the current-skill icon, hotkey row, and
    // grid/pet row (see OnHotkeySlotClick()/OnCurrentSkillClick()/OnGridCellClick()/
    // OnPetCellClick() and their *Hover() counterparts).
    return true;
}

bool mu::ui::window::CSkillList::UpdateKeyEvent()
{
    for (int i = 0; i < 9; ++i)
    {
        if (mu::ui::window::IsPress('1' + i))
        {
            UseHotKey(i + 1);
        }
    }

    if (mu::ui::window::IsPress('0'))
    {
        UseHotKey(0);
    }

    // m_iHoveredGridSkillIndex arms Ctrl+digit assignment; set by OnGridCellHover()/OnPetCellHover(), cleared by OnUnhover().
    if (m_iHoveredGridSkillIndex != -1)
    {
        if (mu::ui::window::IsRepeat(VK_CONTROL))
        {
            for (int i = 0; i < 9; ++i)
            {
                if (mu::ui::window::IsPress('1' + i))
                {
                    SetHotKey(i + 1, m_iHoveredGridSkillIndex);

                    return false;
                }
            }

            if (mu::ui::window::IsPress('0'))
            {
                SetHotKey(0, m_iHoveredGridSkillIndex);

                return false;
            }
        }
    }

    if (mu::ui::window::IsRepeat(VK_SHIFT))
    {
        for (int i = 0; i < 4; ++i)
        {
            if (mu::ui::window::IsPress('1' + i))
            {
                Hero->CurrentSkill = AT_PET_COMMAND_DEFAULT + i;
                return false;
            }
        }
    }

    return true;
}

bool mu::ui::window::CSkillList::IsArrayUp(BYTE bySkill)
{
    for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
    {
        if (m_iHotKeySkillType[i] == bySkill)
        {
            if (i == 0 || i > 5)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    return false;
}

bool mu::ui::window::CSkillList::IsArrayIn(BYTE bySkill)
{
    for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
    {
        if (m_iHotKeySkillType[i] == bySkill)
        {
            return true;
        }
    }

    return false;
}

void mu::ui::window::CSkillList::SetHotKey(int iHotKey, int iSkillType)
{
    for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
    {
        if (m_iHotKeySkillType[i] == iSkillType)
        {
            m_iHotKeySkillType[i] = -1;
            break;
        }
    }

    m_iHotKeySkillType[iHotKey] = iSkillType;
}

int mu::ui::window::CSkillList::GetHotKey(int iHotKey)
{
    return m_iHotKeySkillType[iHotKey];
}

int mu::ui::window::CSkillList::GetSkillIndex(int iSkillType)
{
    // special handling for skills with different skill id for the trigger
    if (iSkillType == AT_SKILL_NOVA_BEGIN)
    {
        iSkillType = AT_SKILL_NOVA;
    }

    int iReturn = -1;
    for (int i = 0; i < MAX_MAGIC; ++i)
    {
        if (CharacterAttribute->Skill[i] == iSkillType)
        {
            iReturn = i;
            break;
        }
    }

    return iReturn;
}

void mu::ui::window::CSkillList::UseHotKey(int iHotKey)
{
    if (m_iHotKeySkillType[iHotKey] != -1)
    {
        if (m_iHotKeySkillType[iHotKey] >= AT_PET_COMMAND_DEFAULT && m_iHotKeySkillType[iHotKey] < AT_PET_COMMAND_END)
        {
            if (Hero->m_pPet == NULL)
            {
                return;
            }
        }

        auto wHotKeySkill = CharacterAttribute->Skill[m_iHotKeySkillType[iHotKey]];

        if (wHotKeySkill == 0)
        {
            return;
        }

        m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];

        Hero->CurrentSkill = m_iHotKeySkillType[iHotKey];

        auto bySkill = CharacterAttribute->Skill[Hero->CurrentSkill];

        if (
            g_pOption->IsAutoAttack() == true
            && gMapManager.WorldActive != WD_6STADIUM
            && gMapManager.InChaosCastle() == false
            && (bySkill == AT_SKILL_TELEPORT || bySkill == AT_SKILL_TELEPORT_ALLY))
        {
            SelectedCharacter = -1;
            Attacking = -1;
        }
    }
}

bool mu::ui::window::CSkillList::Update()
{
    if (IsArrayIn(Hero->CurrentSkill) == true)
    {
        if (IsArrayUp(Hero->CurrentSkill) == true)
        {
            m_bHotKeySkillListUp = true;
        }
        else
        {
            m_bHotKeySkillListUp = false;
        }
    }

    if (Hero->m_pPet == NULL)
    {
        if (Hero->CurrentSkill >= AT_PET_COMMAND_DEFAULT && Hero->CurrentSkill < AT_PET_COMMAND_END)
        {
            Hero->CurrentSkill = 0;
        }
    }

    // Refreshes the RmlUi-facing overlay snapshot while the grid is open; left stale (harmless,
    // hidden) while closed.
    if (m_bSkillList)
    {
        RebuildGridSnapshot();
    }

    return true;
}

bool mu::ui::window::CSkillList::IsHotKeySlotCurrentSkill(int iSlotIndex)
{
    // The hotkey row shows hotkeys 1-5 or 6-9,0 (wraparound), skipping empty slots and pet
    // commands without a pet.
    if (iSlotIndex < 0 || iSlotIndex >= 5)
        return false;

    if (CharacterAttribute->SkillNumber == 0)
        return false;

    int iStartSkillIndex = m_bHotKeySkillListUp ? 6 : 1;
    int iIndex = iStartSkillIndex + iSlotIndex;
    if (iIndex == 10)
        iIndex = 0;

    if (m_iHotKeySkillType[iIndex] == -1)
        return false;

    if (m_iHotKeySkillType[iIndex] >= AT_PET_COMMAND_DEFAULT && m_iHotKeySkillType[iIndex] < AT_PET_COMMAND_END)
    {
        if (Hero->m_pPet == NULL)
            return false;
    }

    return Hero->CurrentSkill == m_iHotKeySkillType[iIndex];
}

int mu::ui::window::CSkillList::GetHotKeySlotNumber(int iSlotIndex)
{
    // Same loop as IsHotKeySlotCurrentSkill(), but doesn't check Hero->CurrentSkill -- the number
    // shows for every occupied slot, not just the active one.
    if (iSlotIndex < 0 || iSlotIndex >= 5)
        return -1;

    if (CharacterAttribute->SkillNumber == 0)
        return -1;

    int iStartSkillIndex = m_bHotKeySkillListUp ? 6 : 1;
    int iIndex = iStartSkillIndex + iSlotIndex;
    if (iIndex == 10)
        iIndex = 0;

    if (m_iHotKeySkillType[iIndex] == -1)
        return -1;

    if (m_iHotKeySkillType[iIndex] >= AT_PET_COMMAND_DEFAULT && m_iHotKeySkillType[iIndex] < AT_PET_COMMAND_END)
    {
        if (Hero->m_pPet == NULL)
            return -1;
    }

    return iIndex;
}

bool mu::ui::window::CSkillList::Render()
{
    // Nothing native: the hotkey row, current-skill slot, grid and pet row are main_frame.rml
    // (icons, legacy box art, hotkey numbers, cooldown wipes), fed by SyncRmlModel().
    return true;
}

float mu::ui::window::CSkillList::GetLayerDepth()
{
    return 5.2f;
}

WORD mu::ui::window::CSkillList::GetHeroPriorSkill()
{
    return m_wHeroPriorSkill;
}

void mu::ui::window::CSkillList::SetHeroPriorSkill(BYTE bySkill)
{
    m_wHeroPriorSkill = bySkill;
}

namespace
{
bool HasOneHandedOrMeleeWeapon()
{
    const int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
    const int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;
    return iTypeR != -1 && (iTypeR < ITEM_STAFF || iTypeR >= ITEM_STAFF + MAX_ITEM_INDEX) &&
           (iTypeL < ITEM_STAFF || iTypeL >= ITEM_STAFF + MAX_ITEM_INDEX);
}

// The original RenderSkillIcon()'s checks that chose the grey icon: equipment, mount, buffs,
// party, map and stats. The atlas cell itself is UI::Skills::Icon::ResolveSkillIcon().
bool IsHudSkillUsable(ActionSkillType bySkillType)
{
    bool bCantSkill = false;

    const BYTE bySkillUseType = SkillAttribute[bySkillType].SkillUseType;

    if (!gSkillManager.AreSkillAttributeRequirementsMet(bySkillType))
    {
        bCantSkill = true;
    }

    if (IsCanBCSkill(bySkillType) == false)
    {
        bCantSkill = true;
    }
    if (g_isCharacterBuff((&Hero->Object), eBuff_AddSkill) && bySkillUseType == SKILL_USE_TYPE_BRAND)
    {
        bCantSkill = true;
    }
    auto isSittingOnPet = GameLogic::Items::IsHornMountModel(Hero->Helper.Type);
    if (bySkillType == AT_SKILL_IMPALE && !isSittingOnPet)
    {
        bCantSkill = true;
    }

    if (bySkillType == AT_SKILL_IMPALE && isSittingOnPet)
    {
        int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
        int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;
        if ((iTypeL < ITEM_SPEAR || iTypeL >= ITEM_BOW) && (iTypeR < ITEM_SPEAR || iTypeR >= ITEM_BOW))
        {
            bCantSkill = true;
        }
    }

    if (isSittingOnPet && ((bySkillType >= AT_SKILL_BLOCKING && bySkillType <= AT_SKILL_SLASH) ||
                           bySkillType == AT_SKILL_FALLING_SLASH_STR || bySkillType == AT_SKILL_LUNGE_STR ||
                           bySkillType == AT_SKILL_CYCLONE_STR || bySkillType == AT_SKILL_CYCLONE_STR_MG ||
                           bySkillType == AT_SKILL_SLASH_STR))
    {
        bCantSkill = true;
    }

    if ((bySkillType == AT_SKILL_POWER_SLASH || bySkillType == AT_SKILL_POWER_SLASH_STR) && isSittingOnPet)
    {
        bCantSkill = true;
    }

    if (bySkillType == AT_SKILL_PARTY_TELEPORT && PartyNumber <= 0)
    {
        bCantSkill = true;
    }

    if (bySkillType == AT_SKILL_PARTY_TELEPORT &&
        (IsDoppelGanger1() || IsDoppelGanger2() || IsDoppelGanger3() || IsDoppelGanger4()))
    {
        bCantSkill = true;
    }

    if (bySkillType == AT_SKILL_EARTHSHAKE || bySkillType == AT_SKILL_EARTHSHAKE_STR ||
        bySkillType == AT_SKILL_EARTHSHAKE_MASTERY)
    {
        BYTE byDarkHorseLife = 0;
        byDarkHorseLife = CharacterMachine->Equipment[EQUIPMENT_HELPER].Durability;
        if (byDarkHorseLife == 0 || Hero->Helper.Type != MODEL_DARK_HORSE_ITEM)
        {
            bCantSkill = true;
        }
    }
#ifdef PJH_FIX_SPRIT
    /*박종훈*/
    if (bySkillType >= AT_PET_COMMAND_DEFAULT && bySkillType < AT_PET_COMMAND_END)
    {
        int iCharisma = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;
        PET_INFO PetInfo;
        giPetManager::GetPetInfo(PetInfo, 421 - PET_TYPE_DARK_SPIRIT);
        int RequireCharisma = (185 + (PetInfo.m_wLevel * 15));
        if (RequireCharisma > iCharisma)
        {
            bCantSkill = true;
        }
    }
#endif // PJH_FIX_SPRIT
    if ((bySkillType == AT_SKILL_INFINITY_ARROW) || (bySkillType == AT_SKILL_INFINITY_ARROW_STR) ||
        (bySkillType == AT_SKILL_EXPANSION_OF_WIZARDRY) || (bySkillType == AT_SKILL_EXPANSION_OF_WIZARDRY_STR) ||
        (bySkillType == AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY))
    {
        if ((g_isCharacterBuff((&Hero->Object), eBuff_InfinityArrow)) ||
            (g_isCharacterBuff((&Hero->Object), eBuff_SwellOfMagicPower)))
        {
            bCantSkill = true;
        }
    }

    if (bySkillType == AT_SKILL_FIRE_SLASH || bySkillType == AT_SKILL_FIRE_SLASH_STR)
    {
        WORD Strength;
        const WORD wRequireStrength = 596;
        Strength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
        if (Strength < wRequireStrength)
        {
            bCantSkill = true;
        }
        int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
        int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;

        if (!(iTypeR != -1 && (iTypeR < ITEM_STAFF || iTypeR >= ITEM_STAFF + MAX_ITEM_INDEX) &&
              (iTypeL < ITEM_STAFF || iTypeL >= ITEM_STAFF + MAX_ITEM_INDEX)))
        {
            bCantSkill = true;
        }
    }

    switch (bySkillType)
    {
        // case AT_SKILL_PIERCING:
    case AT_SKILL_ICE_ARROW:
    case AT_SKILL_ICE_ARROW_STR:
    {
        WORD Dexterity;
        const WORD wRequireDexterity = 646;
        Dexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
        if (Dexterity < wRequireDexterity)
        {
            bCantSkill = true;
        }
    }
    break;
    }

    if (bySkillType == AT_SKILL_TWISTING_SLASH || bySkillType == AT_SKILL_TWISTING_SLASH_STR ||
        bySkillType == AT_SKILL_TWISTING_SLASH_STR_MG || bySkillType == AT_SKILL_TWISTING_SLASH_MASTERY ||
        bySkillType == AT_SKILL_RAGEFUL_BLOW || bySkillType == AT_SKILL_RAGEFUL_BLOW_STR ||
        bySkillType == AT_SKILL_RAGEFUL_BLOW_MASTERY || bySkillType == AT_SKILL_DEATHSTAB ||
        bySkillType == AT_SKILL_DEATHSTAB_STR)
    {
        int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
        int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;

        if (!(iTypeR != -1 && (iTypeR < ITEM_STAFF || iTypeR >= ITEM_STAFF + MAX_ITEM_INDEX) &&
              (iTypeL < ITEM_STAFF || iTypeL >= ITEM_STAFF + MAX_ITEM_INDEX)))
        {
            bCantSkill = true;
        }
    }

    if (gMapManager.InChaosCastle() == true)
    {
        if (bySkillType == AT_SKILL_EARTHSHAKE || bySkillType == AT_SKILL_EARTHSHAKE_STR ||
            bySkillType == AT_SKILL_EARTHSHAKE_MASTERY || bySkillType == AT_SKILL_RIDER ||
            (static_cast<int>(bySkillType) >= static_cast<int>(AT_PET_COMMAND_DEFAULT) &&
             static_cast<int>(bySkillType) <= static_cast<int>(AT_PET_COMMAND_TARGET)))
        {
            bCantSkill = true;
        }
    }
    else
    {
        if (bySkillType == AT_SKILL_EARTHSHAKE || bySkillType == AT_SKILL_EARTHSHAKE_STR ||
            bySkillType == AT_SKILL_EARTHSHAKE_MASTERY)
        {
            BYTE byDarkHorseLife = 0;
            byDarkHorseLife = CharacterMachine->Equipment[EQUIPMENT_HELPER].Durability;
            if (byDarkHorseLife == 0)
            {
                bCantSkill = true;
            }
        }
    }

    if (!g_CMonkSystem.IsSwordformGlovesUseSkill(bySkillType))
    {
        bCantSkill = true;
    }
    if (g_CMonkSystem.IsRideNotUseSkill(bySkillType, Hero->Helper.Type))
    {
        bCantSkill = true;
    }

    ITEM* pLeftRing = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
    ITEM* pRightRing = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];

    if (g_CMonkSystem.IsChangeringNotUseSkill(pLeftRing->Type, pRightRing->Type, pLeftRing->Level, pRightRing->Level) &&
        (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER))
    {
        bCantSkill = true;
    }

    if (!g_csItemOption.IsNonWeaponSkillOrIsSkillEquipped(bySkillType))
    {
        bCantSkill = true;
    }

    if (bySkillType == AT_SKILL_MULTI_SHOT && gCharacterManager.GetEquipedBowType_Skill() == BOWTYPE_NONE)
    {
        bCantSkill = true;
    }

    if (bySkillType == AT_SKILL_FLAME_STRIKE && !HasOneHandedOrMeleeWeapon())
    {
        bCantSkill = true;
    }

    return !bCantSkill;
}
} // namespace

Rml::String mu::ui::window::CSkillList::GetSkillIconDecorator(int iIndex)
{
    auto bySkillType = CharacterAttribute->Skill[iIndex];

    if (bySkillType == 0)
    {
        return "none";
    }

    if (iIndex >= AT_PET_COMMAND_DEFAULT)
    {
        bySkillType = (ActionSkillType)iIndex;
    }

    const UI::Skills::Icon::SkillIcon icon =
        UI::Skills::Icon::ResolveSkillIcon({.skillType = bySkillType,
                                            .skillUseType = SkillAttribute[bySkillType].SkillUseType,
                                            .magicIcon = SkillAttribute[bySkillType].Magic_Icon,
                                            .usable = IsHudSkillUsable(bySkillType)});
    const std::string sprite = UI::Skills::Icon::IconSpriteName(icon);
    return sprite.empty() ? Rml::String("none") : "image(" + sprite + ")";
}

Rml::String mu::ui::window::CSkillList::GetHotKeySlotIconDecorator(int iSlotIndex)
{
    const int iHotKey = GetHotKeySlotNumber(iSlotIndex);
    return iHotKey >= 0 ? GetSkillIconDecorator(m_iHotKeySkillType[iHotKey]) : Rml::String("none");
}

Rml::String mu::ui::window::CSkillList::GetCurrentSkillIconDecorator()
{
    return CharacterAttribute->SkillNumber > 0 ? GetSkillIconDecorator(Hero->CurrentSkill) : Rml::String("none");
}

int mu::ui::window::CSkillList::GetSkillHotKeyNumber(int iIndex)
{
    for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
    {
        if (m_iHotKeySkillType[i] == iIndex)
        {
            return i;
        }
    }
    return -1;
}

namespace
{
    // RenderSkillDelay()'s own fraction math (iSkillDelay/iSkillMaxDelay), draw call replaced with
    // a plain return. Feeds SkillCellEntry::cooldownFraction and
    // GetHotKeySlotCooldownFraction()/GetCurrentSkillCooldownFraction().
    //
    // Known gap: unlike the original, this doesn't suppress the cooldown wipe for
    // AT_SKILL_CHAIN_DRIVE/_STR, AT_SKILL_DRAGON_KICK/_ROAR/_STR when bCantSkill is also true --
    // a minor double-signal for those 4 skills, not a functional bug.
    float ComputeSkillCooldownFraction(int iIndex)
    {
        // Mirrors RenderSkillIcon()'s bySkillType resolution, used only for the 5-skill exclusion
        // gate below. RenderSkillDelay()'s own resolution (further down) is NOT pet-aware -- a
        // pre-existing inconsistency between the two, preserved rather than fixed.
        WORD bySkillTypeForGate = CharacterAttribute->Skill[iIndex];
        if (iIndex >= AT_PET_COMMAND_DEFAULT)
            bySkillTypeForGate = (WORD)iIndex;

        if (bySkillTypeForGate == AT_SKILL_INFINITY_ARROW || bySkillTypeForGate == AT_SKILL_INFINITY_ARROW_STR
            || bySkillTypeForGate == AT_SKILL_EXPANSION_OF_WIZARDRY || bySkillTypeForGate == AT_SKILL_EXPANSION_OF_WIZARDRY_STR
            || bySkillTypeForGate == AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY)
            return 0.f;

        // From here down: RenderSkillDelay()'s own original body, draw call replaced with a
        // fraction return.
        int iSkillDelay = CharacterAttribute->SkillDelay[iIndex];
        if (iSkillDelay <= 0)
            return 0.f;

        int iSkillType = CharacterAttribute->Skill[iIndex];

        if (iSkillType == AT_SKILL_PLASMA_STORM_FENRIR && !CheckAttack())
            return 0.f;

        int iSkillMaxDelay = SkillAttribute[iSkillType].Delay;
        if (iSkillMaxDelay == 0)
            return 0.f; // avoid a divide-by-zero the original's own float division would also hit

        return iSkillDelay / (float)iSkillMaxDelay;
    }
}

namespace
{
Rml::String HotKeyText(int hotkey)
{
    return hotkey >= 0 ? std::to_string(hotkey) : Rml::String();
}
} // namespace

void mu::ui::window::CSkillList::RebuildGridSnapshot()
{
    // The skills the legacy grid loop drew, in its order; the theme lays them out (the original's
    // zig-zag in legacy).
    m_GridSnapshot.clear();
    m_PetSnapshot.clear();

    if (CharacterAttribute->SkillNumber == 0)
        return;

    for (int i = 0; i < MAX_MAGIC; ++i)
    {
        int iSkillType = CharacterAttribute->Skill[i];

        if (iSkillType == 0 || (iSkillType >= AT_SKILL_STUN && iSkillType <= AT_SKILL_REMOVAL_BUFF))
            continue;

        BYTE bySkillUseType = SkillAttribute[iSkillType].SkillUseType;
        if (bySkillUseType == SKILL_USE_TYPE_MASTER || bySkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
            continue;

        SkillCellEntry entry;
        entry.skillIndex = i;
        entry.isPet = false;
        entry.isCurrent = (i == Hero->CurrentSkill);
        entry.cooldownFraction = ComputeSkillCooldownFraction(i);
        entry.icon = GetSkillIconDecorator(i);
        entry.hotkey = HotKeyText(GetSkillHotKeyNumber(i));
        m_GridSnapshot.push_back(entry);
    }

    if (Hero->m_pPet != NULL)
    {
        // The four commands are always all four, so the theme puts each on its own cell; only the
        // zig-zag grid above needs a computed position.
        for (int i = AT_PET_COMMAND_DEFAULT; i < AT_PET_COMMAND_END; ++i)
        {
            SkillCellEntry entry;
            entry.skillIndex = i;
            entry.isPet = true;
            entry.isCurrent = (i == Hero->CurrentSkill);
            entry.cooldownFraction = ComputeSkillCooldownFraction(i);
            entry.icon = GetSkillIconDecorator(i);
            entry.hotkey = HotKeyText(GetSkillHotKeyNumber(i));
            m_PetSnapshot.push_back(entry);
        }
    }
}

// Native CNewUISkillList::RenderSkillInfo() centres the skill tooltip at box x + 10 for the current
// skill and hotkeys, cell x + 15 in the expanded list, anchored 10 above the box.
namespace
{
constexpr float kSlotTooltipOffsetX = 10.f;
constexpr float kGridTooltipOffsetX = 15.f;
constexpr float kTooltipGapAbove = 10.f;
// The original hit-tested the current skill as a 32x38 box around its 20x28 icon (385/431 around
// 392/437) and anchored the hint on that box like a hotkey slot.
constexpr float kCurrentSkillBoxInsetX = 7.f;
constexpr float kCurrentSkillBoxInsetY = 6.f;
} // namespace

void mu::ui::window::CSkillList::QueueTooltip(int iSkillIndex, float x, float y)
{
    m_bTooltipPending = true;
    m_iTooltipSkillIndex = iSkillIndex;
    m_fTooltipAnchorX = x;
    m_fTooltipAnchorY = y;
}

float mu::ui::window::CSkillList::GetHotKeySlotCooldownFraction(int iSlotIndex)
{
    if (iSlotIndex < 0 || iSlotIndex >= 5)
        return 0.f;
    if (CharacterAttribute->SkillNumber == 0)
        return 0.f;

    int iStartSkillIndex = m_bHotKeySkillListUp ? 6 : 1;
    int iIndex = iStartSkillIndex + iSlotIndex;
    if (iIndex == 10)
        iIndex = 0;

    if (m_iHotKeySkillType[iIndex] == -1)
        return 0.f;

    if (m_iHotKeySkillType[iIndex] >= AT_PET_COMMAND_DEFAULT && m_iHotKeySkillType[iIndex] < AT_PET_COMMAND_END)
    {
        if (Hero->m_pPet == NULL)
            return 0.f;
    }

    return ComputeSkillCooldownFraction(m_iHotKeySkillType[iIndex]);
}

float mu::ui::window::CSkillList::GetCurrentSkillCooldownFraction()
{
    return ComputeSkillCooldownFraction(Hero->CurrentSkill);
}

// Click/hover entry points bound from main_frame.rml's data-event-click/mouseover/mouseout (CMainFrameWindow::BuildRmlUi()).
void mu::ui::window::CSkillList::OnHotkeySlotClick(int iSlotIndex)
{
    if (iSlotIndex < 0 || iSlotIndex >= 5)
        return;
    if (CharacterAttribute->SkillNumber == 0)
        return;

    int iStartSkillIndex = m_bHotKeySkillListUp ? 6 : 1;
    int iIndex = iStartSkillIndex + iSlotIndex;
    if (iIndex == 10)
        iIndex = 0;

    if (m_iHotKeySkillType[iIndex] == -1)
        return;

    // Mirrors the legacy mouse-click branch, deliberately NOT UseHotKey() -- the original mouse
    // click never went through it either (only the keyboard 0-9 press does), so its pet-check/
    // auto-attack-cancel rule doesn't apply here.
    WORD bySkillType = CharacterAttribute->Skill[m_iHotKeySkillType[iIndex]];
    if (bySkillType == 0 || (bySkillType >= AT_SKILL_STUN && bySkillType <= AT_SKILL_REMOVAL_BUFF))
        return;
    if (SkillAttribute[bySkillType].SkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
        return;

    m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
    Hero->CurrentSkill = m_iHotKeySkillType[iIndex];
    PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::CSkillList::OnHotkeySlotHover(int iSlotIndex, float slotLeft, float slotTop)
{
    if (iSlotIndex < 0 || iSlotIndex >= 5)
        return;
    if (CharacterAttribute->SkillNumber == 0)
        return;

    int iStartSkillIndex = m_bHotKeySkillListUp ? 6 : 1;
    int iIndex = iStartSkillIndex + iSlotIndex;
    if (iIndex == 10)
        iIndex = 0;

    if (m_iHotKeySkillType[iIndex] == -1)
        return;

    WORD bySkillType = CharacterAttribute->Skill[m_iHotKeySkillType[iIndex]];
    if (bySkillType == 0 || (bySkillType >= AT_SKILL_STUN && bySkillType <= AT_SKILL_REMOVAL_BUFF))
        return;
    if (SkillAttribute[bySkillType].SkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
        return;

    QueueTooltip(m_iHotKeySkillType[iIndex], slotLeft + kSlotTooltipOffsetX, slotTop - kTooltipGapAbove);
}

void mu::ui::window::CSkillList::OnCurrentSkillClick()
{
    m_bSkillList = !m_bSkillList;
    PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::CSkillList::OnCurrentSkillHover(float iconLeft, float iconTop)
{
    QueueTooltip(Hero->CurrentSkill, iconLeft - kCurrentSkillBoxInsetX + kSlotTooltipOffsetX,
                 iconTop - kCurrentSkillBoxInsetY - kTooltipGapAbove);
}

void mu::ui::window::CSkillList::OnGridCellClick(int iSkillIndex)
{
    m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
    Hero->CurrentSkill = iSkillIndex;
    m_bSkillList = false;
    PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::CSkillList::OnGridCellHover(int iSkillIndex, float cellLeft, float cellTop)
{
    m_iHoveredGridSkillIndex = iSkillIndex;
    QueueTooltip(iSkillIndex, cellLeft + kGridTooltipOffsetX, cellTop - kTooltipGapAbove);
}

void mu::ui::window::CSkillList::OnPetCellClick(int iSkillIndex)
{
    m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
    Hero->CurrentSkill = iSkillIndex;
    m_bSkillList = false;
    PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::CSkillList::OnPetCellHover(int iSkillIndex, float cellLeft, float cellTop)
{
    // Pet-row entries arm Ctrl+digit assignment the same way grid entries do (legacy behavior, preserved).
    m_iHoveredGridSkillIndex = iSkillIndex;
    QueueTooltip(iSkillIndex, cellLeft + kGridTooltipOffsetX, cellTop - kTooltipGapAbove);
}

void mu::ui::window::CSkillList::OnUnhover()
{
    m_bTooltipPending = false;
    m_iTooltipSkillIndex = -1;
    m_iHoveredGridSkillIndex = -1;
}

bool mu::ui::window::CSkillList::IsSkillListUp()
{
    return m_bHotKeySkillListUp;
}

void mu::ui::window::CSkillList::ResetMouseLButton()
{
    MouseLButton = false;
    MouseLButtonPop = false;
    MouseLButtonPush = false;
}
