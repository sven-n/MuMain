
#include "stdafx.h"
#include "I18N/All.h"

#include "UI/Events/CursedTempleEnter.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "UI/Widgets/UIBaseDef.h"
#include "Audio/DSPlaySound.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"

#include "Character/CharacterManager.h"
#include "GameLogic/Items/CSItemOption.h"
#include "GameLogic/Events/CSChaosCastle.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlPointer.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
    const int EnterLevelCount = 5;
    const int EnterMinLevel[EnterLevelCount] = { 220, 271, 321, 351, 381 };
    const int EnterMaxLevel[EnterLevelCount] = { 270, 320, 350, 380, 400 };

    // RenderText() shrinks a text wider than its box to fit it: the size it drew `text` at.
    float TextPxInBox(UI::Scaling::FontRole role, const wchar_t* text, float boxWidth)
    {
        g_pRenderText->SetFont(role == UI::Scaling::FontRole::Bold ? g_hFontBold : g_hFont);
        const int width = g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text))).cx;
        return UI::RmlBridge::NativeTextPxInBox(role, static_cast<float>(width), boxWidth);
    }
}

bool mu::ui::window::CCursedTempleEnter::Create(CManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_CURSEDTEMPLE_NPC, this);

    BuildRmlUi();

    Show(false);

    return true;
}

mu::ui::window::CCursedTempleEnter::CCursedTempleEnter() : m_pNewUIMng(NULL), m_EnterTime(0), m_EnterCount(0)
{
    Initialize();
}

mu::ui::window::CCursedTempleEnter::~CCursedTempleEnter()
{
    Destroy();
}

void mu::ui::window::CCursedTempleEnter::Initialize()
{
}

void mu::ui::window::CCursedTempleEnter::Destroy()
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

bool mu::ui::window::CCursedTempleEnter::CheckEnterLevel(int& enterlevel)
{
    if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
    {
        enterlevel = 6;
        return true;
    }

    int HeroLevel = CharacterAttribute->Level;

    for (int i = 0; i < EnterLevelCount; ++i)
    {
        if (HeroLevel >= EnterMinLevel[i] && HeroLevel <= EnterMaxLevel[i])
        {
            enterlevel = i + 1;
            return true;
        }
    }

    return false;
}

bool mu::ui::window::CCursedTempleEnter::CheckEnterItem(ITEM* p, int enterlevel)
{
    if (p->Type == ITEM_ILLUSION_TEMPLE_TICKET)
    {
        if (!CheckEnterLevel(enterlevel)) return false;
    }
    else
    {
        if (p->Type != ITEM_SCROLL_OF_BLOOD)
            return false;

        int itemLevel = p->Level;

        if (itemLevel != enterlevel)
            return false;
    }

    if (p->Durability < 1) return false;

    return true;
}

bool mu::ui::window::CCursedTempleEnter::CheckInventory(BYTE& itempos, int enterlevel)
{
    int pos = 0;

    if (enterlevel == -1) {
        return false;
    }

    pos = g_pMyInventory->GetInventoryCtrl()->FindItemIndex(ITEM_SCROLL_OF_BLOOD, enterlevel);
    if (pos != -1) {
        itempos = pos;
        return true;
    }

    pos = g_pMyInventory->GetInventoryCtrl()->FindItemIndex(ITEM_ILLUSION_TEMPLE_TICKET, -1);
    if (pos != -1) {
        itempos = pos;
        return true;
    }
    return false;
}

bool mu::ui::window::CCursedTempleEnter::UpdateMouseEvent()
{
    // The Enter and Close buttons are RmlUi's (see Update()); the window keeps the pointer.
    return !UI::RmlBridge::IsPointerOver(m_RmlView.Document());
}

bool mu::ui::window::CCursedTempleEnter::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_CURSEDTEMPLE_NPC) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_CURSEDTEMPLE_NPC);
            return false;
        }
    }

    return true;
}

bool mu::ui::window::CCursedTempleEnter::Update()
{
    SyncRmlModel();

    // Clicks RmlUi reported (the original's button handling in UpdateMouseEvent()).
    const bool enter = m_PendingEnter;
    const bool close = m_PendingClose;
    m_PendingEnter = m_PendingClose = false;
    if (!IsVisible())
        return true;
    if (enter)
    {
        int EnterLevel = -1;
        if (CheckEnterLevel(EnterLevel))
        {
            SocketClient->ToGameServer()->SendIllusionTempleEnterRequest(static_cast<BYTE>(EnterLevel), 0xFF);
        }
        else
        {
            g_pSystemLogBox->AddText(I18N::Game::TheAdmissionAndScrollLevelsDoNotMatch,
                                     mu::ui::window::TYPE_ERROR_MESSAGE);
        }
        return true;
    }
    if (close)
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_CURSEDTEMPLE_NPC);
    return true;
}

bool mu::ui::window::CCursedTempleEnter::Render()
{
    // Nothing native left: the frame, the texts and the buttons are RmlUi. Kept because CObject
    // requires the override.
    return true;
}

//ServerMessage
void mu::ui::window::CCursedTempleEnter::SetEntryOffer(std::uint8_t remainingTime, std::uint8_t entryCount)
{
    m_EnterTime = remainingTime;
    m_EnterCount = entryCount;
}

void mu::ui::window::CCursedTempleEnter::SetEntryCounts(std::span<const std::uint8_t, 6> counts)
{
    int enterlevel = -1;
    if (CheckEnterLevel(enterlevel) && enterlevel > 0)
        m_EnterCount = counts[enterlevel - 1];
}

void mu::ui::window::CCursedTempleEnter::BindRmlModel(Rml::DataModelConstructor& c, CursedTempleEnterRmlModel& model)
{
    c.Bind("text_px", &model.textPx);
    c.Bind("bold_text_px", &model.boldTextPx);
    c.Bind("line_height_px", &model.lineHeightPx);
    c.Bind("title", &model.title);
    c.Bind("eligible", &model.eligible);
    auto line = c.RegisterStruct<CursedTempleEnterLine>();
    line.RegisterMember("text", &CursedTempleEnterLine::text);
    line.RegisterMember("text_px", &CursedTempleEnterLine::textPx);
    c.Bind("temple_line", &model.templeLine);
    c.Bind("members_line", &model.membersLine);
    c.Bind("notice", &model.notice);
    auto band = c.RegisterStruct<CursedTempleEnterBand>();
    band.RegisterMember("text", &CursedTempleEnterBand::text);
    band.RegisterMember("text_px", &CursedTempleEnterBand::textPx);
    band.RegisterMember("hero", &CursedTempleEnterBand::hero);
    c.RegisterArray<std::vector<CursedTempleEnterBand>>();
    c.Bind("bands", &model.bands);
    c.Bind("enter_text", &model.enterText);
    c.Bind("close_text", &model.closeText);
    c.Bind("label_line_px", &model.labelLinePx);
    c.BindEventCallback("temple_enter", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { m_PendingEnter = true; });
    c.BindEventCallback("temple_close", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { m_PendingClose = true; });
}

void mu::ui::window::CCursedTempleEnter::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void mu::ui::window::CCursedTempleEnter::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    // Layer depth 10.3: over the HUD and the panels.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_RmlView.Document(), IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());
    SyncLines();
}

void mu::ui::window::CCursedTempleEnter::SyncLines()
{
    CursedTempleEnterRmlModel updated = m_RmlView.GetModel();
    updated.boldTextPx = UI::RmlBridge::NativeTextPx(UI::Scaling::FontRole::Bold);
    updated.lineHeightPx = CUIRenderTextSDLTtf::LineHeightPx(UI::Scaling::FontRole::Normal);
    updated.labelLinePx = updated.lineHeightPx;
    updated.title = StringUtils::WideToNarrow(I18N::Game::DoYouWishToGoToTheIllusionTemple);
    updated.enterText = StringUtils::WideToNarrow(I18N::Game::Enter);
    updated.closeText = StringUtils::WideToNarrow(I18N::Game::Close388);

    // The original's RenderText(): with a level band, the temple, the six bands (the hero's on a
    // red text box) and the members; else the minimum-level notice. The window's box is 220 units
    // wide, 230 for the notice, which is what each line was shrunk to fit.
    auto makeLine = [&](const wchar_t* text, float boxWidth) -> CursedTempleEnterLine
    {
        return {StringUtils::WideToNarrow(text),
                TextPxInBox(UI::Scaling::FontRole::Normal, text, boxWidth)};
    };
    const float lineBox = CURSEDTEMPLE_ENTER_WINDOW_WIDTH - 10;
    updated.bands.clear();
    wchar_t Text[100] = {};
    int enterlevel = -1;
    updated.eligible = CheckEnterLevel(enterlevel);
    if (updated.eligible)
    {
        mu_swprintf(Text, I18N::Game::TheDIllusionTemple, enterlevel);
        updated.templeLine = makeLine(Text, lineBox);
        for (int i = 0; i < EnterLevelCount + 1; ++i)
        {
            wchar_t band[100] = {};
            if (i == 5)
                wcscpy(band, I18N::Game::MasterLevel);
            else
                mu_swprintf(band, I18N::Game::LevelDD, EnterMinLevel[i], EnterMaxLevel[i]);
            const bool heroBand = enterlevel == i + 1;
            mu_swprintf(Text, L"%ls %ls", band, heroBand ? I18N::Game::EntranceEnabled : I18N::Game::EntranceDisabled);
            const CursedTempleEnterLine line = makeLine(Text, lineBox);
            updated.bands.push_back({line.text, line.textPx, heroBand});
        }
        mu_swprintf(Text, I18N::Game::CurrentMembersD, m_EnterCount);
        updated.membersLine = makeLine(Text, lineBox);
    }
    else
    {
        updated.notice = makeLine(I18N::Game::YouMustBeOfTheMinimumLevel220ToEnterTheZone,
                                  CURSEDTEMPLE_ENTER_WINDOW_WIDTH);
    }

    CursedTempleEnterRmlModel& model = m_RmlView.GetModel();
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleEnterRmlModel::boldTextPx, "bold_text_px", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleEnterRmlModel::lineHeightPx, "line_height_px", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleEnterRmlModel::labelLinePx, "label_line_px", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleEnterRmlModel::title, "title", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleEnterRmlModel::enterText, "enter_text", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleEnterRmlModel::closeText, "close_text", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleEnterRmlModel::eligible, "eligible", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleEnterRmlModel::templeLine, "temple_line", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleEnterRmlModel::membersLine, "members_line", updated);
    SyncFieldFrom(m_RmlView.Binder(), &CursedTempleEnterRmlModel::notice, "notice", updated);
    if (model.bands != updated.bands)
    {
        model.bands = std::move(updated.bands);
        m_RmlView.MarkDirty("bands");
    }
}
