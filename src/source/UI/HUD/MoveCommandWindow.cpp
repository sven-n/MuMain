
#include "stdafx.h"
#include "UI/HUD/MoveCommandWindow.h"
#include "UI/Core/WindowSystem.h"
#include "Data/GameData/MapData/WarpDatabase.h"
#include "GameLogic/Warps/WarpRules.h"
#include "Core/Utilities/KeyGenerator.h"
#include "Network/Server/ServerListManager.h"
#include "Engine/Object/ZzzOpenData.h"
#include "World/MapInfra/MapManager.h"
#include "Character/CharacterManager.h"
#include "Audio/DSPlaySound.h"
#include "I18N/All.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/RmlBridge/RmlRowWheel.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
// The fill slot's smallest panel: the header, the close bar and three one-line rows. A warp with a master
// level or resets takes two lines, so fewer of those fit; the list scrolls, and a minimum measured from the
// rows would change the slot with the server's list (no warp of the data file sets them).
constexpr float kFillMinimumHeight = 102.f;
};

CMoveCommandWindow::CMoveCommandWindow()
{
    m_pNewUIMng = NULL;
    m_dwMoveCommandKey = 0;

}

CMoveCommandWindow::~CMoveCommandWindow()
{
    Release();
}

bool mu::ui::window::CMoveCommandWindow::Create(CManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_MOVEMAP, this);

    RefreshData();

    BuildRmlUi();

    Show(false);

    return true;
}

void mu::ui::window::CMoveCommandWindow::Release()
{

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    m_RmlView.Release();
}

void mu::ui::window::CMoveCommandWindow::RefreshData()
{
    m_listMoveInfoData = CMoveCommandData::GetInstance()->GetMoveCommandDatalist();
    m_listVersion = CMoveCommandData::GetInstance()->GetListVersion();
}

void mu::ui::window::CMoveCommandWindow::RefreshDataIfChanged()
{
    if (m_listVersion == CMoveCommandData::GetInstance()->GetListVersion())
        return;

    RefreshData();
    SetStrifeMap();
}

void mu::ui::window::CMoveCommandWindow::SetFillPlacementSize(float width, float height)
{
    if (m_FillWidth == width && m_FillHeight == height)
        return;
    m_FillWidth = width;
    m_FillHeight = height;
    ApplyFillSize();
}

bool mu::ui::window::CMoveCommandWindow::GetFillMinimumSize(float& width, float& height) const
{
    // Its content width, and the chrome with three rows: the list takes the height it is given.
    width = static_cast<float>(UI::MoveCommand::kWindowWidth);
    if (Rml::Element* panel = m_RmlView.Document() != nullptr ? m_RmlView.Document()->GetElementById("panel") : nullptr)
    {
        if (panel->GetBox().GetSize(Rml::BoxArea::Border).x > 0.f)
            width = panel->GetBox().GetSize(Rml::BoxArea::Border).x;
    }
    height = kFillMinimumHeight;
    return true;
}

void mu::ui::window::CMoveCommandWindow::ApplyFillSize()
{
    Rml::Element* panel = m_RmlView.Document() != nullptr ? m_RmlView.Document()->GetElementById("panel") : nullptr;
    if (panel == nullptr)
        return;
    const bool fill = m_FillWidth > 0.f && m_FillHeight > 0.f;
    panel->SetClass("fill-placement", fill);
    if (fill)
    {
        panel->SetProperty(Rml::PropertyId::Width, Rml::Property(m_FillWidth, Rml::Unit::PX));
        panel->SetProperty(Rml::PropertyId::Height, Rml::Property(m_FillHeight, Rml::Unit::PX));
    }
    else
    {
        panel->RemoveProperty(Rml::PropertyId::Width);
        panel->RemoveProperty(Rml::PropertyId::Height);
    }
    m_RmlView.Document()->UpdateDocument();
}

void mu::ui::window::CMoveCommandWindow::SetMoveCommandKey(DWORD dwKey)
{
    m_dwMoveCommandKey = dwKey;
}

DWORD mu::ui::window::CMoveCommandWindow::GetMoveCommandKey()
{
    m_dwMoveCommandKey = g_KeyGenerator.GenerateKeyValue(m_dwMoveCommandKey);

    return m_dwMoveCommandKey;
}

void mu::ui::window::CMoveCommandWindow::SetStrifeMap()
{
    // The warps for Gens members only (Data/Maps/Warps.json "gensOnly"), on PvP servers.
    const bool nonPvP = g_ServerListManager->IsNonPvP();
    for (auto* moveInfo : m_listMoveInfoData)
    {
        moveInfo->_bStrife = GameLogic::Warps::IsGensWarp(
            GameLogic::Warps::FindWarpRules(g_WarpDatabase.GetList(), moveInfo->_ReqInfo.index), nonPvP);
    }
}

void mu::ui::window::CMoveCommandWindow::SettingCanMoveMap()
{
    // The move rules of Data/Maps/Warps.json; the level, zen, master level and resets of the list, which the server
    // may have changed.
    const GameLogic::Warps::WarpCharacter character = GameLogic::Warps::MakeHeroWarpCharacter();
    const bool nonPvP = g_ServerListManager->IsNonPvP();
    for (auto* moveInfo : m_listMoveInfoData)
    {
        moveInfo->_bSelected = false;
        moveInfo->_bCanMove =
            GameLogic::Warps::IsWarpAllowed(g_WarpDatabase.GetList(), moveInfo->_ReqInfo, character, nonPvP);
    }
}

void mu::ui::window::CMoveCommandWindow::RmlWheelList(Rml::Event& event)
{
    // One row per notch, from row top to row top: a row with a second line is taller.
    Rml::Element* list = event.GetCurrentElement();
    if (list == nullptr)
        return;
    const std::vector<float> rowTops = UI::RmlBridge::RowTops(*list);
    if (rowTops.empty())
        return;

    // Stopping the event keeps RmlUi from scrolling the list itself.
    event.StopPropagation();
    list->SetScrollTop(
        UI::RmlBridge::RowWheelScrollTop(rowTops, list->GetScrollTop(), event.GetParameter("wheel_delta_y", 0.f)));
}

void mu::ui::window::CMoveCommandWindow::RmlClickWarp(int row)
{
    // The requirements are re-evaluated on the click itself, not trusted from the row model:
    // level/zen/equipment can change between the frame that built the row and this one.
    SettingCanMoveMap();

    auto li = m_listMoveInfoData.begin();
    for (int i = 0; i < row && li != m_listMoveInfoData.end(); ++i)
        ++li;

    if (li == m_listMoveInfoData.end() || (*li)->_bCanMove == false)
        return;

    // A warp can end the session here (a map on another game server): the settings the server keeps go
    // first, before any warp.
    SaveOptionsIfInGame();

    SocketClient->ToGameServer()->SendWarpCommandRequest(GetMoveCommandKey(), (*li)->_ReqInfo.index);

    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MOVEMAP);
}

bool mu::ui::window::CMoveCommandWindow::UpdateMouseEvent()
{
    // Row hover, row clicks, the close bar and the scrollbar are all RmlUi's now. What is left is
    // the two things it has no part in: the picked-item backup, and claiming the panel so a click
    // on it doesn't fall through to the world.
    if (!UI::RmlBridge::IsPointerOver(m_RmlView.Document()))
        return true;

    if (IsPress(VK_LBUTTON))
    {
        mu::ui::window::CInventoryCtrl::BackupPickedItem();
    }

    // RmlUi scrolls the list off its own wheel event; this only stops the same notch reaching the
    // camera underneath, exactly as native did.
    MouseWheel = 0;

    return false;
}

bool mu::ui::window::CMoveCommandWindow::UpdateKeyEvent()
{
    if (IsVisible())
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            Show(false);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    return true;
}

bool mu::ui::window::CMoveCommandWindow::Update()
{
    SyncRmlModel();
    return true;
}

bool mu::ui::window::CMoveCommandWindow::Render()
{
    // Nothing native left: the fill, the title, the column headers, every row, the close bar and
    // the scrollbar are all RmlUi. Kept because CObject requires the override.
    return true;
}

void mu::ui::window::CMoveCommandWindow::BindRmlModel(Rml::DataModelConstructor& c, MoveCommandRmlModel& model)
{
    BindMoveCommandFields(c, model);
    c.BindEventCallback("movecommand_warp",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
        {
            if (arguments.size() == 1)
                RmlClickWarp(arguments[0].Get<int>(-1));
        });
    c.BindEventCallback("movecommand_wheel", [this](Rml::DataModelHandle, Rml::Event& event,
                                                    const Rml::VariantList&) { RmlWheelList(event); });
    c.BindEventCallback("movecommand_close",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MOVEMAP);
        });

    model.titleText = StringUtils::WideToNarrow(I18N::Game::WarpCommandWindow);
    model.headStrife = StringUtils::WideToNarrow(I18N::Game::BattleZone);
    model.headMap = StringUtils::WideToNarrow(I18N::Game::Map);
    model.headLevel = StringUtils::WideToNarrow(I18N::Game::MinLevel);
    model.headZen = StringUtils::WideToNarrow(I18N::Game::Cost);
    model.closeText = StringUtils::WideToNarrow(I18N::Game::Close388);
}

void mu::ui::window::CMoveCommandWindow::OnRmlBuilt()
{
    ApplyFillSize();
    // A new document (a theme change) starts without rows.
    m_rowInputs.reset();
}

void mu::ui::window::CMoveCommandWindow::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void mu::ui::window::CMoveCommandWindow::SyncRmlModel()
{
    RefreshDataIfChanged();
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    UI::RmlBridge::SyncDocumentVisibility(m_RmlView.Document(), IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());

    // The rows follow the lists (the names follow the UI language with the list
    // version), the hero and the server; built every frame they cost a search of
    // the rules per warp.
    RowInputs inputs{m_listVersion, g_WarpDatabase.GetVersion(), GameLogic::Warps::MakeHeroWarpCharacter(),
                     g_ServerListManager->IsNonPvP()};
    if (m_rowInputs != inputs)
    {
        SettingCanMoveMap();
        RebuildRowModel();
        m_rowInputs = std::move(inputs);
    }

    // With every row in view the original drew its thumb disabled.
    if (Rml::Element* list = m_RmlView.Document()->GetElementById("list"))
        list->SetClass("no-scroll", list->GetScrollHeight() <= list->GetClientHeight() + 0.5f);

    if (m_bRewindPending)
    {
        m_bRewindPending = false;
        if (Rml::Element* list = m_RmlView.Document()->GetElementById("list"))
            list->SetScrollTop(0.f);
    }
}

void mu::ui::window::CMoveCommandWindow::RebuildRowModel()
{
    MoveCommandRmlModel& model = m_RmlView.GetModel();

    std::vector<MoveCommandRowEntry> rows;
    rows.reserve(m_listMoveInfoData.size());

    const int iLevel = CharacterAttribute->Level;
    const DWORD iZen = CharacterMachine->Gold;
    const int masterLevel = gCharacterManager.GetHeroMasterLevel();
    const int resets = static_cast<int>(CharacterAttribute->Resets);
    const int baseClass = gCharacterManager.GetBaseClass(CharacterAttribute->Class);
    const Rml::String strifeMark = StringUtils::WideToNarrow(I18N::Game::Battle2987);
    const Data::Maps::WarpList& warpRules = g_WarpDatabase.GetList();

    wchar_t szText[24];
    int index = 0;
    for (const auto* moveInfo : m_listMoveInfoData)
    {
        // The level the character needs (lower for the classes of the class level discount), with the master
        // level and resets the warp needs.
        const Data::Maps::Warp* rules = GameLogic::Warps::FindWarpRules(warpRules, moveInfo->_ReqInfo.index);
        const int iReqLevel =
            GameLogic::Warps::GetRequiredLevel(warpRules, rules, moveInfo->_ReqInfo.iReqLevel, baseClass);

        MoveCommandRowEntry entry;
        entry.strifeText = moveInfo->_bStrife ? strifeMark : Rml::String();
        entry.mapName = StringUtils::WideToNarrow(moveInfo->_ReqInfo.szMainMapName);
        entry.reqLevel = std::to_string(iReqLevel);
        entry.reqMasterLevelAndResets =
            StringUtils::WideToNarrow(GameLogic::Warps::GetMasterLevelAndResetsText(moveInfo->_ReqInfo).c_str());
        _itow(moveInfo->_ReqInfo.iReqZen, szText, 10);
        entry.reqZen = StringUtils::WideToNarrow(szText);
        entry.canMove = moveInfo->_bCanMove;
        entry.levelUnmet = iReqLevel > iLevel;
        entry.masterLevelOrResetsUnmet =
            moveInfo->_ReqInfo.iReqMasterLevel > masterLevel || moveInfo->_ReqInfo.iReqResets > resets;
        entry.zenUnmet = moveInfo->_ReqInfo.iReqZen > (int)iZen;
        entry.index = index++;

        rows.push_back(std::move(entry));
    }

    // Only publish a genuine change. Marking the array dirty re-runs every row's text binding, and
    // this list is static for minutes at a time -- only a level-up, a zen change or an equipment
    // swap moves any of it.
    const bool changed = rows.size() != model.rows.size() ||
                         !std::equal(rows.begin(), rows.end(), model.rows.begin(),
                                     [](const MoveCommandRowEntry& a, const MoveCommandRowEntry& b)
                                     {
                                         return a.canMove == b.canMove && a.levelUnmet == b.levelUnmet &&
                                                a.masterLevelOrResetsUnmet == b.masterLevelOrResetsUnmet &&
                                                a.zenUnmet == b.zenUnmet && a.strifeText == b.strifeText &&
                                                a.reqLevel == b.reqLevel &&
                                                a.reqMasterLevelAndResets == b.reqMasterLevelAndResets &&
                                                a.reqZen == b.reqZen && a.mapName == b.mapName;
                                     });
    if (!changed)
        return;

    model.rows = std::move(rows);
    m_RmlView.MarkDirty("rows");
}

void mu::ui::window::CMoveCommandWindow::OpenningProcess()
{
    RefreshData();
    SetStrifeMap();
    SettingCanMoveMap();

    // Native reset its own scroll offset here; RmlUi owns the scroll position now, so the list
    // element is what has to be rewound -- and only once the document is actually visible, which
    // it is not yet: CSystem::Show() runs this before ShowInterface().
    m_bRewindPending = true;
}

void mu::ui::window::CMoveCommandWindow::ClosingProcess()
{
}

float mu::ui::window::CMoveCommandWindow::GetLayerDepth()
{
    return 8.3f;
}

CMoveCommandData::MOVEINFODATA* CMoveCommandWindow::FindMoveInfo(const wchar_t* pszMapName)
{
    if (pszMapName == NULL)
        return NULL;

    for (auto* moveInfo : m_listMoveInfoData)
    {
        if (wcsicmp(moveInfo->_ReqInfo.szMainMapName, pszMapName) == 0 ||
            wcsicmp(moveInfo->_ReqInfo.szSubMapName, pszMapName) == 0)
        {
            return moveInfo;
        }
    }

    return NULL;
}

bool CMoveCommandWindow::CanMoveToMap(const wchar_t* pszMapName)
{
    // The requirements are recomputed first, exactly as the click path does before reading the
    // flag. This used to save and restore _bSelected around that call, because the same sweep
    // clears it and it was the hovered row's highlight -- the hover is an RCSS :hover rule now,
    // so nothing reads _bSelected any more and there is nothing to preserve.
    // The strife flags are otherwise only set when the window opens, and a query may come first.
    RefreshDataIfChanged();
    SetStrifeMap();
    SettingCanMoveMap();

    const CMoveCommandData::MOVEINFODATA* moveInfo = FindMoveInfo(pszMapName);
    return moveInfo != NULL && moveInfo->_bCanMove;
}

int CMoveCommandWindow::GetMapIndexFromMovereq(const wchar_t* pszMapName)
{
    RefreshDataIfChanged();
    const CMoveCommandData::MOVEINFODATA* moveInfo = FindMoveInfo(pszMapName);
    return moveInfo != NULL ? moveInfo->_ReqInfo.index : -1;
}
