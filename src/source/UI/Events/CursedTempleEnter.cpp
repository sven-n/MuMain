
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
#include "UI/Widgets/UIControls.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>

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
    float TextPxInBox(UI::Scaling::FontRole role, const UI::Scaling::Transform& transform, const wchar_t* text,
                      float boxWidth)
    {
        g_pRenderText->SetFont(role == UI::Scaling::FontRole::Bold ? g_hFontBold : g_hFont);
        const int width = g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text))).cx;
        return UI::Scaling::NativeTextPixelSizeInBox(role, transform, static_cast<float>(width), boxWidth);
    }
}

bool mu::ui::window::CCursedTempleEnter::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_CURSEDTEMPLE_NPC, this);

    SetPos(x, y);

    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

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
    UI::RmlBridge::UnregisterForThemeReload(this);
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
    if (p->Type == ITEM_HELPER + 61)
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

    pos = g_pMyInventory->GetInventoryCtrl()->FindItemIndex(ITEM_HELPER + 61, -1);
    if (pos != -1) {
        itempos = pos;
        return true;
    }
    return false;
}

bool mu::ui::window::CCursedTempleEnter::UpdateMouseEvent()
{
    // The Enter and Close buttons are RmlUi's (see Update()); the window keeps the pointer.
    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, CURSEDTEMPLE_ENTER_WINDOW_WIDTH, CURSEDTEMPLE_ENTER_WINDOW_HEIGHT).Contains(MouseX, MouseY))
    {
        return false;
    }

    return true;
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
void mu::ui::window::CCursedTempleEnter::SetCursedTempleEnterInfo(const BYTE* cursedtempleinfo)
{
    m_EnterTime = static_cast<int>(cursedtempleinfo[0]);
    m_EnterCount = static_cast<int>(cursedtempleinfo[1]);
}

void mu::ui::window::CCursedTempleEnter::ReceiveCursedTempleEnterInfo(const BYTE* ReceiveBuffer)
{
    auto data = (LPPMSG_CURSED_TEMPLE_USER_COUNT)ReceiveBuffer;

    int enterlevel = -1;

    if (CheckEnterLevel(enterlevel))
    {
        if (enterlevel > 0)
        {
            m_EnterCount = data->btUserCount[enterlevel - 1];
        }
    }
}

void mu::ui::window::CCursedTempleEnter::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(
        RmlUiRuntime::Instance().GetContext(), "cursed_temple_enter",
        [this](Rml::DataModelConstructor& c, CursedTempleEnterRmlModel& model)
        {
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);
            c.Bind("root_scale", &model.rootScale);
            c.Bind("text_px", &model.textPx);
            c.Bind("bold_text_px", &model.boldTextPx);
            c.Bind("line_height_px", &model.lineHeightPx);
            c.Bind("title", &model.title);
            auto line = c.RegisterStruct<CursedTempleEnterLineEntry>();
            line.RegisterMember("text", &CursedTempleEnterLineEntry::text);
            line.RegisterMember("top", &CursedTempleEnterLineEntry::top);
            line.RegisterMember("left", &CursedTempleEnterLineEntry::left);
            line.RegisterMember("width", &CursedTempleEnterLineEntry::width);
            line.RegisterMember("text_px", &CursedTempleEnterLineEntry::textPx);
            line.RegisterMember("highlighted", &CursedTempleEnterLineEntry::highlighted);
            line.RegisterMember("red", &CursedTempleEnterLineEntry::red);
            c.RegisterArray<std::vector<CursedTempleEnterLineEntry>>();
            c.Bind("lines", &model.lines);
            c.Bind("enter_text", &model.enterText);
            c.Bind("close_text", &model.closeText);
            c.Bind("label_top", &model.labelTop);
            c.Bind("label_line_px", &model.labelLinePx);
            c.BindEventCallback("temple_enter", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                                { m_PendingEnter = true; });
            c.BindEventCallback("temple_close", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                                { m_PendingClose = true; });
        });

    if (!modelCreated)
        return;

    m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
                                                  "Data/Interface/RmlUi/cursed_temple_enter.rml");
}

void mu::ui::window::CCursedTempleEnter::ReloadRmlTheme()
{
    if (!m_pRmlDoc)
        return;
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
}

void mu::ui::window::CCursedTempleEnter::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_pRmlDoc)
        return;

    // Layer depth 10.3: over the HUD and the panels.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_pRmlDoc, IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncRootTransform(m_RmlBinder, m_Pos);
    UI::RmlBridge::SyncNativeTextSize(m_RmlBinder);
    SyncLines();
}

void mu::ui::window::CCursedTempleEnter::SyncLines()
{
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    CursedTempleEnterRmlModel updated = m_RmlBinder.GetModel();
    updated.boldTextPx = UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Bold, transform);
    const int lineHeight = CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal);
    updated.lineHeightPx = static_cast<float>(lineHeight) * transform.scaleY;
    updated.labelTop = static_cast<float>(23 / 2 - lineHeight / 2);
    updated.labelLinePx = updated.lineHeightPx;
    updated.title = StringUtils::WideToNarrow(I18N::Game::DoYouWishToGoToTheIllusionTemple);
    updated.enterText = StringUtils::WideToNarrow(I18N::Game::Enter);
    updated.closeText = StringUtils::WideToNarrow(I18N::Game::Close388);

    // The original's RenderText(): with a level band, the temple, the six bands (the hero's on a
    // red text box) and the members; else the minimum-level notice.
    auto addLine = [&](const wchar_t* text, float left, float top, float width, bool highlighted, bool red)
    {
        updated.lines.push_back({StringUtils::WideToNarrow(text), top, left, width,
                                 TextPxInBox(UI::Scaling::FontRole::Normal, transform, text, width), highlighted, red});
    };
    updated.lines.clear();
    wchar_t Text[100] = {};
    int enterlevel = -1;
    if (CheckEnterLevel(enterlevel))
    {
        mu_swprintf(Text, I18N::Game::TheDIllusionTemple, enterlevel);
        addLine(Text, 3.f, 42.f, CURSEDTEMPLE_ENTER_WINDOW_WIDTH - 10, false, false);
        for (int i = 0; i < EnterLevelCount + 1; ++i)
        {
            wchar_t band[100] = {};
            if (i == 5)
                wcscpy(band, I18N::Game::MasterLevel);
            else
                mu_swprintf(band, I18N::Game::LevelDD, EnterMinLevel[i], EnterMaxLevel[i]);
            const bool heroBand = enterlevel == i + 1;
            mu_swprintf(Text, L"%ls %ls", band, heroBand ? I18N::Game::EntranceEnabled : I18N::Game::EntranceDisabled);
            addLine(Text, 3.f, 67.f + static_cast<float>(i * 15), CURSEDTEMPLE_ENTER_WINDOW_WIDTH - 10, heroBand,
                    false);
        }
        mu_swprintf(Text, I18N::Game::CurrentMembersD, m_EnterCount);
        addLine(Text, 3.f, 70.f + static_cast<float>((EnterLevelCount + 1) * 15), CURSEDTEMPLE_ENTER_WINDOW_WIDTH - 10,
                false, true);
    }
    else
    {
        addLine(I18N::Game::YouMustBeOfTheMinimumLevel220ToEnterTheZone, 0.f, 52.f, CURSEDTEMPLE_ENTER_WINDOW_WIDTH,
                false, true);
    }

    CursedTempleEnterRmlModel& model = m_RmlBinder.GetModel();
    SyncFieldFrom(m_RmlBinder, &CursedTempleEnterRmlModel::boldTextPx, "bold_text_px", updated);
    SyncFieldFrom(m_RmlBinder, &CursedTempleEnterRmlModel::lineHeightPx, "line_height_px", updated);
    SyncFieldFrom(m_RmlBinder, &CursedTempleEnterRmlModel::labelTop, "label_top", updated);
    SyncFieldFrom(m_RmlBinder, &CursedTempleEnterRmlModel::labelLinePx, "label_line_px", updated);
    SyncFieldFrom(m_RmlBinder, &CursedTempleEnterRmlModel::title, "title", updated);
    SyncFieldFrom(m_RmlBinder, &CursedTempleEnterRmlModel::enterText, "enter_text", updated);
    SyncFieldFrom(m_RmlBinder, &CursedTempleEnterRmlModel::closeText, "close_text", updated);
    const bool sameLines = model.lines.size() == updated.lines.size() &&
                           std::equal(model.lines.begin(), model.lines.end(), updated.lines.begin(),
                                      [](const CursedTempleEnterLineEntry& a, const CursedTempleEnterLineEntry& b)
                                      {
                                          return a.text == b.text && a.top == b.top && a.left == b.left &&
                                                 a.width == b.width && a.textPx == b.textPx &&
                                                 a.highlighted == b.highlighted && a.red == b.red;
                                      });
    if (!sameLines)
    {
        model.lines = std::move(updated.lines);
        m_RmlBinder.MarkDirty("lines");
    }
}
