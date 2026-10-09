
#include "stdafx.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"
#include "UI/Inventory/ItemEnduranceInfo.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Placement/WindowPlacement.h"
#include "UI/Scaling/UITransform.h"
#include "I18N/All.h"

#include "Character/CharacterManager.h"
#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>

#include <string>
#include <utility>

#ifdef PJH_FIX_SPRIT
#include "GameLogic/Pets/GIPetManager.h"
#endif //PJH_FIX_SPRIT



using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
// The equipped helper that has an HP frame (guardian angel, imp, horns, dark horse, pets).
bool HasHelperLifeFrame()
{
    return Hero->Helper.Type >= MODEL_HELPER && Hero->Helper.Type <= MODEL_DARK_HORSE_ITEM ||
           Hero->Helper.Type == MODEL_DEMON || Hero->Helper.Type == MODEL_SPIRIT_OF_GUARDIAN ||
           Hero->Helper.Type == MODEL_PET_RUDOLF || Hero->Helper.Type == MODEL_PET_PANDA ||
           Hero->Helper.Type == MODEL_PET_UNICORN || Hero->Helper.Type == MODEL_PET_SKELETON ||
           Hero->Helper.Type == MODEL_HORN_OF_FENRIR;
}

// The HP frame's name of the equipped helper (HasHelperLifeFrame()).
void HelperLifeFrameName(wchar_t* szText)
{
    switch (Hero->Helper.Type)
    {
    case MODEL_HELPER:
    {
        mu_swprintf(szText, I18N::Game::GuardianAngel);
    }
    break;
    case MODEL_IMP:
    {
        ITEM_ATTRIBUTE* p = &ItemAttribute[Hero->Helper.Type - MODEL_SWORD];
        mu_swprintf(szText, p->Name);
    }
    break;
    case MODEL_HORN_OF_UNIRIA:
    {
        mu_swprintf(szText, I18N::Game::Uniria);
    }
    break;
    case MODEL_HORN_OF_DINORANT:
    {
        mu_swprintf(szText, I18N::Game::Dinorant);
    }
    break;
    case MODEL_DARK_HORSE_ITEM:
    {
        mu_swprintf(szText, I18N::Game::DarkHorse);
    }
    break;
    case MODEL_HORN_OF_FENRIR:
    {
        mu_swprintf(szText, I18N::Game::Fenrir);
    }
    break;
    case MODEL_DEMON:
    {
        mu_swprintf(szText, ItemAttribute[ITEM_DEMON].Name);
    }
    break;
    case MODEL_SPIRIT_OF_GUARDIAN:
    {
        mu_swprintf(szText, ItemAttribute[ITEM_SPIRIT_OF_GUARDIAN].Name);
    }
    break;
    case MODEL_PET_RUDOLF:
    {
        mu_swprintf(szText, ItemAttribute[ITEM_PET_RUDOLF].Name);
    }
    break;
    case MODEL_PET_PANDA:
    {
        mu_swprintf(szText, ItemAttribute[ITEM_PET_PANDA].Name);
    }
    break;
    case MODEL_PET_UNICORN:
    {
        mu_swprintf(szText, ItemAttribute[ITEM_PET_UNICORN].Name);
    }
    break;
    case MODEL_PET_SKELETON:
    {
        mu_swprintf(szText, ItemAttribute[ITEM_PET_SKELETON].Name);
    }
    break;
    }
}

// The elf's arrow / bolt count line ("Arrows %d (%d)"), false when none is shown.
bool ArrowCountText(int arrowType, int bowArrows, int crossbowBolts, wchar_t* szText)
{
    int iNumEquipedArrowDurability = 0;
    int iNumArrowSetInInven = 0;

    if (arrowType == bowArrows)
    {
        iNumArrowSetInInven = g_pMyInventory->GetNumItemByType(bowArrows);

        if (CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type == ITEM_ARROWS)
        {
            iNumEquipedArrowDurability = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Durability;
        }

        if ((iNumArrowSetInInven == 0) && (CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type != ITEM_ARROWS))
            return false;

        mu_swprintf(szText, I18N::Game::ArrowsDD, iNumEquipedArrowDurability, iNumArrowSetInInven);
        return true;
    }
    if (arrowType == crossbowBolts)
    {
        iNumArrowSetInInven = g_pMyInventory->GetNumItemByType(crossbowBolts);

        if (CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type == ITEM_BOLT)
        {
            iNumEquipedArrowDurability = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Durability;
        }

        if ((iNumArrowSetInInven == 0) && (CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type != ITEM_BOLT))
            return false;

        mu_swprintf(szText, I18N::Game::BoltsDD, iNumEquipedArrowDurability, iNumArrowSetInInven);
        return true;
    }
    return false;
}
} // namespace

CItemEnduranceInfo::CItemEnduranceInfo()
{
    m_iCurArrowType = ARROWTYPE_NONE;
    m_iTooltipIndex = -1;
}

CItemEnduranceInfo::~CItemEnduranceInfo()
{
    Release();
}

bool mu::ui::window::CItemEnduranceInfo::Create(CManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_ITEM_ENDURANCE_INFO, this);

    InitImageIndex();
    Show(true);
    return true;
}

void mu::ui::window::CItemEnduranceInfo::Release()
{
    // Hidden directly: RmlUi renders last in the frame regardless of scene.
    if (m_RmlView.Document() != nullptr)
        m_RmlView.Document()->Hide();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    m_RmlView.Release();
}

// The frames hold the pointer, and the icon under it shows its tooltip, by where they are drawn.
bool mu::ui::window::CItemEnduranceInfo::UpdateMouseEvent()
{
    Rml::ElementDocument* document = m_RmlView.Document();
    if (document == nullptr)
        return true;

    Rml::ElementList frames;
    document->QuerySelectorAll(frames, ".pet");
    for (Rml::Element* frame : frames)
    {
        if (UI::RmlBridge::IsPointerWithin(frame))
            return false;
    }

    // Each icon's tint is its hit box: the whole icon, or its half when both rings share one.
    Rml::ElementList tints;
    document->QuerySelectorAll(tints, "#icon_column .icon-tint");
    for (size_t i = 0; i < tints.size() && i < m_IconSlots.size(); ++i)
    {
        if (UI::RmlBridge::IsPointerWithin(tints[i]))
        {
            m_iTooltipIndex = m_IconSlots[i];
            return false;
        }
    }
    return true;
}
bool mu::ui::window::CItemEnduranceInfo::UpdateKeyEvent()
{
    return true;
}

bool mu::ui::window::CItemEnduranceInfo::Update()
{
    if (!IsVisible())
        return true;
    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF)
    {
        if (gCharacterManager.GetEquipedBowType() == BOWTYPE_BOW)
        {
            m_iCurArrowType = ARROWTYPE_BOW;
        }
        else if (gCharacterManager.GetEquipedBowType() == BOWTYPE_CROSSBOW)
        {
            m_iCurArrowType = ARROWTYPE_CROSSBOW;
        }
        else
        {
            m_iCurArrowType = ARROWTYPE_NONE;
        }
    }

    return true;
}

bool mu::ui::window::CItemEnduranceInfo::Render()
{
    BuildRmlUi();
    if (m_RmlView.Document() != nullptr)
        SyncView();
    return true;
}
//---------------------------------------------------------------------------------------------

bool mu::ui::window::CItemEnduranceInfo::BtnProcess()
{
    return false;
}

float mu::ui::window::CItemEnduranceInfo::GetLayerDepth()
{
    return 3.5f;
}

void mu::ui::window::CItemEnduranceInfo::OpenningProcess()
{
}

void mu::ui::window::CItemEnduranceInfo::ClosingProcess()
{
}

void mu::ui::window::CItemEnduranceInfo::InitImageIndex()
{
    m_iItemDurImageIndex[EQUIPMENT_WEAPON_RIGHT] = IMAGE_ITEM_DUR_WEAPON;
    m_iItemDurImageIndex[EQUIPMENT_WEAPON_LEFT] = IMAGE_ITEM_DUR_SHIELD;
    m_iItemDurImageIndex[EQUIPMENT_HELM] = IMAGE_ITEM_DUR_CAP;
    m_iItemDurImageIndex[EQUIPMENT_ARMOR] = IMAGE_ITEM_DUR_UPPER;
    m_iItemDurImageIndex[EQUIPMENT_PANTS] = IMAGE_ITEM_DUR_LOWER;
    m_iItemDurImageIndex[EQUIPMENT_GLOVES] = IMAGE_ITEM_DUR_GLOVES;
    m_iItemDurImageIndex[EQUIPMENT_BOOTS] = IMAGE_ITEM_DUR_BOOTS;
    m_iItemDurImageIndex[EQUIPMENT_WING] = IMAGE_ITEM_DUR_WING;
    m_iItemDurImageIndex[EQUIPMENT_HELPER] = -1;
    m_iItemDurImageIndex[EQUIPMENT_AMULET] = IMAGE_ITEM_DUR_NECKLACE;
    m_iItemDurImageIndex[EQUIPMENT_RING_RIGHT] = IMAGE_ITEM_DUR_RING;
    m_iItemDurImageIndex[EQUIPMENT_RING_LEFT] = IMAGE_ITEM_DUR_RING;
}

//---------------------------------------------------------------------------------------------
// RmlUi view (item_endurance.rml)

namespace
{
template <typename T>
void SyncItemEnduranceField(RmlModelBinder<UI::ItemEndurance::ItemEnduranceRmlModel>& binder,
                            T UI::ItemEndurance::ItemEnduranceRmlModel::* field, const char* name, T value)
{
    auto& model = binder.GetModel();
    if (model.*field == value)
        return;
    model.*field = std::move(value);
    binder.MarkDirty(name);
}

// The durability band the original's tint and tooltip colours follow.
const char* DurabilityBand(int durability, int maxDurability)
{
    if (durability <= 0)
        return "zero";
    if (durability <= maxDurability * 0.2f)
        return "fifth";
    if (durability <= maxDurability * 0.3f)
        return "third";
    return "half";
}
} // namespace

void mu::ui::window::CItemEnduranceInfo::SyncDocVisibility(bool sceneAllowsShow)
{
    m_sceneAllowsShow = sceneAllowsShow;
    UI::RmlBridge::SyncDocumentVisibilityBehind(m_RmlView.Document(), IsVisible() && sceneAllowsShow);
}

void mu::ui::window::CItemEnduranceInfo::BindRmlModel(Rml::DataModelConstructor& c, UI::ItemEndurance::ItemEnduranceRmlModel& model)
{
    using namespace UI::ItemEndurance;
    c.Bind("text_px", &model.textPx);
    c.Bind("line_height_px", &model.lineHeightPx);
    c.Bind("bold_text_px", &model.boldTextPx);
    c.Bind("bold_line_height_px", &model.boldLineHeightPx);
    c.Bind("arrows", &model.arrows);

    auto pet = c.RegisterStruct<PetFrameEntry>();
    pet.RegisterMember("top", &PetFrameEntry::top);
    pet.RegisterMember("bar_width", &PetFrameEntry::barWidth);
    pet.RegisterMember("name", &PetFrameEntry::name);
    c.RegisterArray<std::vector<PetFrameEntry>>();
    c.Bind("pets", &model.pets);

    auto icon = c.RegisterStruct<DurabilityIconEntry>();
    icon.RegisterMember("cell", &DurabilityIconEntry::cell);
    icon.RegisterMember("image", &DurabilityIconEntry::image);
    icon.RegisterMember("tint_half", &DurabilityIconEntry::tintHalf);
    icon.RegisterMember("band", &DurabilityIconEntry::band);
    c.RegisterArray<std::vector<DurabilityIconEntry>>();
    c.Bind("icons", &model.icons);

    c.Bind("tooltip", &model.tooltip);
    c.Bind("tooltip_band", &model.tooltipBand);
    c.Bind("tooltip_centre_x", &model.tooltipCentreX);
    c.Bind("tooltip_top", &model.tooltipTop);
}

void mu::ui::window::CItemEnduranceInfo::BuildRmlUi()
{
    m_RmlView.Ensure();
}

// Render() under the dock-right transform the window manager set, like the original's.
void mu::ui::window::CItemEnduranceInfo::SyncView()
{
    SyncDocVisibility(m_sceneAllowsShow);
    if (!m_RmlView.Document()->IsVisible())
        return;

    SyncLeftColumn();
    SyncIcons();
    SyncTooltip();
}

// The original's RenderLeft(): the elf's arrow count line (11 high), then the HP frames of the
// equipped helper, the dark lord's raven and the elf's summon, 24 apart. The theme places the
// column (the original's (2, 26) in the screen's stretch); the tops here are from its top. Each
// frame: the fill, the frame art, the bar (life / max of 49 texels) and the name.
void mu::ui::window::CItemEnduranceInfo::SyncLeftColumn()
{
    using namespace UI::ItemEndurance;
    SyncItemEnduranceField(m_RmlView.Binder(), &ItemEnduranceRmlModel::textPx, "text_px",
                           UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Normal));
    SyncItemEnduranceField(m_RmlView.Binder(), &ItemEnduranceRmlModel::lineHeightPx, "line_height_px",
                           CUIRenderTextSDLTtf::LineHeightPx(UI::Scaling::FontRole::Normal));

    int iNextPosY = 0;
    wchar_t szText[256] = {};

    Rml::String arrows;
    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF &&
        ArrowCountText(m_iCurArrowType, ARROWTYPE_BOW, ARROWTYPE_CROSSBOW, szText))
    {
        arrows = StringUtils::WideToNarrow(szText);
        iNextPosY += (UI_INTERVAL_HEIGHT + 10);
    }
    SyncItemEnduranceField(m_RmlView.Binder(), &ItemEnduranceRmlModel::arrows, "arrows", std::move(arrows));

    std::vector<PetFrameEntry> pets;
    const auto addFrame = [&](const wchar_t* name, int life, int maxLife)
    {
        PetFrameEntry frame;
        frame.top = static_cast<float>(iNextPosY);
        frame.barWidth = (static_cast<float>(life) / static_cast<float>(maxLife)) * static_cast<float>(PETHP_BAR_WIDTH);
        frame.name = StringUtils::WideToNarrow(name);
        pets.push_back(std::move(frame));
        iNextPosY += (static_cast<int>(UI_INTERVAL_HEIGHT) + PETHP_FRAME_HEIGHT);
    };

    if (HasHelperLifeFrame())
    {
        szText[0] = L'\0';
        HelperLifeFrameName(szText);
        addFrame(szText, CharacterMachine->Equipment[EQUIPMENT_HELPER].Durability, 255);
    }
    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD && Hero->m_pPet != nullptr)
    {
        addFrame(I18N::Game::DarkRaven, CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Durability, 255);
    }
    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_ELF && SummonLife > 0)
    {
        addFrame(I18N::Game::SummonedMonsterHP, SummonLife, 100);
    }

    SyncItemEnduranceField(m_RmlView.Binder(), &ItemEnduranceRmlModel::pets, "pets", std::move(pets));
}
// The original's RenderItemEndurance(): every worn item at most half durable, in slot order, two
// per column leftwards from the theme's column (the original's (W - 25, 140) on the dock), 25 apart; the rings share one
// icon, each tinting its half (right ring the left half). None while the trade window is open.
void mu::ui::window::CItemEnduranceInfo::SyncIcons()
{
    using namespace UI::ItemEndurance;
    std::vector<DurabilityIconEntry> icons;
    m_IconSlots.clear();
    if (!g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_TRADE))
    {
        int icntItemDurIcon = 0;
        bool bRenderRingWarning = false;

        for (int i = EQUIPMENT_WEAPON_RIGHT; i < MAX_EQUIPMENT; ++i)
        {
            ITEM* pItem = &CharacterMachine->Equipment[i];
            int iImageIndex = m_iItemDurImageIndex[i];

            if ((pItem->bPeriodItem == true) && (pItem->bExpiredPeriod == false))
                continue;
            if (i == EQUIPMENT_HELPER || pItem->Type == -1)
                continue;
            if (i == EQUIPMENT_WEAPON_RIGHT && pItem->Type == ITEM_ARROWS)
                continue;
            if (i == EQUIPMENT_WEAPON_LEFT)
            {
                if (gCharacterManager.GetEquipedBowType(pItem) == BOWTYPE_BOW)
                    iImageIndex = m_iItemDurImageIndex[EQUIPMENT_WEAPON_RIGHT];
                if (pItem->Type == ITEM_BOLT)
                    continue;
            }

            int iLevel = pItem->Level;
            if ((i == EQUIPMENT_RING_LEFT || i == EQUIPMENT_RING_RIGHT) &&
                (pItem->Type == ITEM_WIZARDS_RING && iLevel == 1 || iLevel == 2))
                continue;

            ITEM_ATTRIBUTE* pItemAtt = &ItemAttribute[pItem->Type];
            int iMaxDurability = CalcMaxDurability(pItem, pItemAtt, iLevel);
            if (pItem->Durability > iMaxDurability * 0.5f)
                continue;

            DurabilityIconEntry icon;
            icon.cell = icntItemDurIcon;
            icon.band = DurabilityBand(pItem->Durability, iMaxDurability);
            if (i != EQUIPMENT_RING_LEFT || bRenderRingWarning != true)
            {
                switch (iImageIndex)
                {
                case IMAGE_ITEM_DUR_BOOTS:
                    icon.image = "boots";
                    break;
                case IMAGE_ITEM_DUR_CAP:
                    icon.image = "cap";
                    break;
                case IMAGE_ITEM_DUR_GLOVES:
                    icon.image = "gloves";
                    break;
                case IMAGE_ITEM_DUR_LOWER:
                    icon.image = "lower";
                    break;
                case IMAGE_ITEM_DUR_NECKLACE:
                    icon.image = "necklace";
                    break;
                case IMAGE_ITEM_DUR_RING:
                    icon.image = "ring";
                    break;
                case IMAGE_ITEM_DUR_SHIELD:
                    icon.image = "shield";
                    break;
                case IMAGE_ITEM_DUR_UPPER:
                    icon.image = "upper";
                    break;
                case IMAGE_ITEM_DUR_WEAPON:
                    icon.image = "weapon";
                    break;
                case IMAGE_ITEM_DUR_WING:
                    icon.image = "wing";
                    break;
                default:
                    break;
                }
            }

            // Both rings warning at once share one icon, each tinting its own half of it.
            if (i == EQUIPMENT_RING_RIGHT)
            {
                bRenderRingWarning = true;
                icon.tintHalf = "left";
            }
            else if (i == EQUIPMENT_RING_LEFT)
            {
                icon.tintHalf = "right";
                bRenderRingWarning = false;
            }
            icons.push_back(std::move(icon));
            m_IconSlots.push_back(i);

            // A shared ring icon stays in the cell the right ring took; everything else moves on.
            if (bRenderRingWarning == false)
                icntItemDurIcon++;
        }
    }
    SyncItemEnduranceField(m_RmlView.Binder(), &ItemEnduranceRmlModel::icons, "icons", std::move(icons));
}

// The original's tooltip of the icon UpdateMouseEvent() found under the pointer: "name (dur/max)",
// bold, centred 10 above the pointer and kept inside the screen's right edge, coloured by band. In
// window pixels.
void mu::ui::window::CItemEnduranceInfo::SyncTooltip()
{
    using namespace UI::ItemEndurance;
    SyncItemEnduranceField(m_RmlView.Binder(), &ItemEnduranceRmlModel::boldTextPx, "bold_text_px",
                           UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Bold));
    SyncItemEnduranceField(m_RmlView.Binder(), &ItemEnduranceRmlModel::boldLineHeightPx, "bold_line_height_px",
                           CUIRenderTextSDLTtf::LineHeightPx(UI::Scaling::FontRole::Bold));

    Rml::String tooltip;
    Rml::String band;
    if (m_iTooltipIndex != -1 && !g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_TRADE))
    {
        const ITEM* pItem = &CharacterMachine->Equipment[m_iTooltipIndex];
        ITEM_ATTRIBUTE* pItemAtt = &ItemAttribute[pItem->Type];
        const int iMaxDurability = CalcMaxDurability(pItem, pItemAtt, pItem->Level);
        if (pItem->Durability <= (iMaxDurability * 0.5f))
        {
            wchar_t szText[256] = {};
            mu_swprintf(szText, L"%ls (%d/%d)", pItemAtt->Name, pItem->Durability, iMaxDurability);
            float tooltipWidth = 0.f;
            {
                const UI::Scaling::ScopedWindowPixels pixels(WindowWidth, WindowHeight);
                g_pRenderText->SetFont(g_hFontBold);
                tooltipWidth = static_cast<float>(g_pRenderText->MeasureText(szText, static_cast<int>(wcslen(szText))).cx);
            }
            const float centreX = std::min(g_fWindowMouseX, static_cast<float>(WindowWidth) - tooltipWidth / 2.f);
            const float above = 10.f * UI::Scaling::TypographyScale(static_cast<int>(WindowWidth), static_cast<int>(WindowHeight));

            tooltip = StringUtils::WideToNarrow(szText);
            band = DurabilityBand(pItem->Durability, iMaxDurability);
            SyncItemEnduranceField(m_RmlView.Binder(), &ItemEnduranceRmlModel::tooltipCentreX, "tooltip_centre_x", centreX);
            SyncItemEnduranceField(m_RmlView.Binder(), &ItemEnduranceRmlModel::tooltipTop, "tooltip_top",
                                   g_fWindowMouseY - above);
            m_iTooltipIndex = -1;
        }
    }
    SyncItemEnduranceField(m_RmlView.Binder(), &ItemEnduranceRmlModel::tooltip, "tooltip", std::move(tooltip));
    SyncItemEnduranceField(m_RmlView.Binder(), &ItemEnduranceRmlModel::tooltipBand, "tooltip_band", std::move(band));
}
