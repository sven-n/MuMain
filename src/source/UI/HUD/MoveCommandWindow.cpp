
#include "stdafx.h"
#include "UI/HUD/MoveCommandWindow.h"
#include "UI/Core/WindowSystem.h"
#include "GameLogic/Items/ChangeRingManager.h"
#include "GameLogic/Items/ItemCategories.h"
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
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>
#include <cmath>

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
    constexpr int MapNameCount = 6;
    // The fill slot's smallest panel: the header, the close bar and three rows.
    constexpr float kFillMinimumHeight = 102.f;

    const std::wstring MapName[MapNameCount] =
    {
        L"Lorencia",
        L"Noria",
        L"Elbeland",
        L"Dungeon",
        L"Devias",
        L"LostTower",
    };

    bool IsLuckySeal(const std::wstring& name)
    {
        if (name.size() != 0) {
            for (int i = 0; i < MapNameCount; ++i) {
                if (name == MapName[i])
                {
                    return true;
                }
            }
        }
        return false;
    }
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

bool mu::ui::window::CMoveCommandWindow::IsLuckySealBuff()
{
    if (g_isCharacterBuff((&Hero->Object), eBuff_Seal1)
        || g_isCharacterBuff((&Hero->Object), eBuff_Seal2)
        || g_isCharacterBuff((&Hero->Object), eBuff_Seal3)
        || g_isCharacterBuff((&Hero->Object), eBuff_Seal4)
        || g_isCharacterBuff((&Hero->Object), eBuff_Seal_HpRecovery)
        || g_isCharacterBuff((&Hero->Object), eBuff_Seal_MpRecovery)
        || g_isCharacterBuff((&Hero->Object), eBuff_AscensionSealMaster)
        || g_isCharacterBuff((&Hero->Object), eBuff_WealthSealMaster)
        || g_isCharacterBuff((&Hero->Object), eBuff_NewWealthSeal)
        || g_isCharacterBuff((&Hero->Object), eBuff_PartyExpBonus)
        )
    {
        return true;
    }
    return false;
}

bool mu::ui::window::CMoveCommandWindow::IsMapMove(const std::wstring& src)
{
    if (Hero->Object.Kind == KIND_PLAYER
        && Hero->Object.Type == MODEL_PLAYER
        && Hero->Object.SubType == MODEL_GM_CHARACTER)
    {
        return true;
    }

    if (g_isCharacterBuff((&Hero->Object), eBuff_GMEffect))
    {
        return true;
    }

    if (IsLuckySealBuff() == false) {
        wchar_t lpszStr1[1024]; wchar_t* lpszStr2 = NULL;
        if (src.find(I18N::Game::Warp) != std::wstring::npos) {
            std::wstring temp = I18N::Game::Warp;
            temp += ' ';
            mu_swprintf(lpszStr1, src.c_str());
            wchar_t* context = nullptr;
            lpszStr2 = wcstok_s(lpszStr1, temp.c_str(), &context);
            if (lpszStr2 == NULL) return false;

            SettingCanMoveMap();
            for (const auto* moveInfo : m_listMoveInfoData) {
                if (!wcscmp(lpszStr2, moveInfo->_ReqInfo.szMainMapName)) {
                    if (moveInfo->_bCanMove == true) {
                        return IsLuckySeal(moveInfo->_ReqInfo.szSubMapName);
                    }
                }
            }
            return false;
        }
        else if (src.find(L"/move") != std::wstring::npos) {
            std::wstring temp = L"/move";
            temp += ' ';
            mu_swprintf(lpszStr1, src.c_str());
            wchar_t* context = nullptr;
            lpszStr2 = wcstok_s(lpszStr1, temp.c_str(), &context);
            if (lpszStr2 == NULL) return false;

            SettingCanMoveMap();
            for (const auto* moveInfo : m_listMoveInfoData) {
                if (!wcsicmp(lpszStr2, moveInfo->_ReqInfo.szMainMapName)) {
                    if (moveInfo->_bCanMove == true) {
                        return IsLuckySeal(moveInfo->_ReqInfo.szSubMapName);
                    }
                }
            }
            return false;
        }
        else {
            return IsLuckySeal(src);
        }
    }
    return true;
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
    std::list<CMoveCommandData::MOVEINFODATA*>::iterator li;

    if (!g_ServerListManager->IsNonPvP())
    {
        int anStrifeIndex[1] = { 42 };
        int i;
        for (li = m_listMoveInfoData.begin(); li != m_listMoveInfoData.end(); advance(li, 1))
        {
            (*li)->_bStrife = false;
            for (i = 0; i < 1; ++i)
            {
                if ((*li)->_ReqInfo.index == anStrifeIndex[i])
                {
                    (*li)->_bStrife = true;
                    break;
                }
            }
        }
    }
    else
    {
        for (li = m_listMoveInfoData.begin(); li != m_listMoveInfoData.end(); advance(li, 1))
            (*li)->_bStrife = false;
    }
}

void mu::ui::window::CMoveCommandWindow::SettingCanMoveMap()
{
    int a = gMapManager.WorldActive;

    DWORD iZen;
    int iLevel, iReqLevel, iReqZen;

    for (auto* moveInfo : m_listMoveInfoData)
    {
        moveInfo->_bCanMove = false;
        moveInfo->_bSelected = false;

        iLevel = CharacterAttribute->Level;
        iZen = CharacterMachine->Gold;
        iReqLevel = moveInfo->_ReqInfo.iReqLevel;
        iReqZen = moveInfo->_ReqInfo.iReqZen;

        if ((gCharacterManager.GetBaseClass(CharacterAttribute->Class) == CLASS_DARK || gCharacterManager.GetBaseClass(CharacterAttribute->Class) == CLASS_DARK_LORD
            || gCharacterManager.GetBaseClass(CharacterAttribute->Class) == CLASS_RAGEFIGHTER)
            && (iReqLevel != 400))
        {
            iReqLevel = int(float(iReqLevel) * 2.f / 3.f);
        }

        if (iLevel >= iReqLevel && (int)iZen >= iReqZen && (int)Hero->PK < PVP_MURDERER1)
        {
            ITEM* pEquipedRightRing = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];
            ITEM* pEquipedLeftRing = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
            ITEM* pEquipedHelper = &CharacterMachine->Equipment[EQUIPMENT_HELPER];
            ITEM* pEquipedWing = &CharacterMachine->Equipment[EQUIPMENT_WING];

            if (wcscmp(moveInfo->_ReqInfo.szMainMapName, I18N::Game::Icarus) == 0)
            {
                if (
                    GameLogic::Items::HasFlightEquipment(pEquipedHelper, pEquipedWing)
                    && !(pEquipedHelper->Type == ITEM_HORN_OF_UNIRIA)
                    && (g_ChangeRingMgr->CheckBanMoveIcarusMap(pEquipedRightRing->Type, pEquipedLeftRing->Type) == false)
                    )
                {
                    moveInfo->_bCanMove = true;
                }
                else
                {
                    moveInfo->_bCanMove = false;
                }
            }
            else if (wcsncmp(moveInfo->_ReqInfo.szMainMapName, I18N::Game::Atlans, wcslen(I18N::Game::Atlans)) == 0)
            {
                if (pEquipedHelper->Type == ITEM_HORN_OF_UNIRIA || pEquipedHelper->Type == ITEM_HORN_OF_DINORANT)
                {
                    moveInfo->_bCanMove = false;
                }
                else
                {
                    moveInfo->_bCanMove = true;
                }
            }
            else if ((g_ServerListManager->IsNonPvP() == true) && (wcscmp(moveInfo->_ReqInfo.szMainMapName, I18N::Game::Vulcanus) == 0))
            {
                moveInfo->_bCanMove = false;
            }
            else
            {
                moveInfo->_bCanMove = true;
            }
        }

        if (moveInfo->_bCanMove && moveInfo->_bStrife && 0 == Hero->m_byGensInfluence)
            moveInfo->_bCanMove = false;
    }
}

void mu::ui::window::CMoveCommandWindow::RmlWheelList(Rml::Event& event)
{
    // One row per notch: a row's height in the list's own (text-sized) layout.
    Rml::Element* list = event.GetCurrentElement();
    Rml::Element* row = list != nullptr && list->GetNumChildren() > 0 ? list->GetChild(0) : nullptr;
    const float rowStep = row != nullptr ? row->GetBox().GetSize(Rml::BoxArea::Border).y : 0.f;
    if (rowStep <= 0.f)
        return;

    // Stopping the event keeps RmlUi from scrolling the list itself.
    event.StopPropagation();
    const float notches = event.GetParameter("wheel_delta_y", 0.f);
    const float rowIndex = std::round(list->GetScrollTop() / rowStep) + notches;
    list->SetScrollTop(rowIndex * rowStep);
}

void mu::ui::window::CMoveCommandWindow::RmlClickWarp(int row)
{
    // The requirements are re-evaluated on the click itself, not trusted from the row model: the
    // list is rebuilt every frame, but level/zen/equipment can still change between the frame that
    // built the row and this one.
    SettingCanMoveMap();

    auto li = m_listMoveInfoData.begin();
    for (int i = 0; i < row && li != m_listMoveInfoData.end(); ++i)
        ++li;

    if (li == m_listMoveInfoData.end() || (*li)->_bCanMove == false)
        return;

    if (IsTheMapInDifferentServer(gMapManager.WorldActive, (*li)->_ReqInfo.index))
    {
        SaveOptions();
    }

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
    c.Bind("text_px", &model.textPx);

    auto row = c.RegisterStruct<MoveCommandRowEntry>();
    row.RegisterMember("strife_text", &MoveCommandRowEntry::strifeText);
    row.RegisterMember("map_name", &MoveCommandRowEntry::mapName);
    row.RegisterMember("req_level", &MoveCommandRowEntry::reqLevel);
    row.RegisterMember("req_zen", &MoveCommandRowEntry::reqZen);
    row.RegisterMember("can_move", &MoveCommandRowEntry::canMove);
    row.RegisterMember("level_unmet", &MoveCommandRowEntry::levelUnmet);
    row.RegisterMember("zen_unmet", &MoveCommandRowEntry::zenUnmet);
    row.RegisterMember("index", &MoveCommandRowEntry::index);
    c.RegisterArray<std::vector<MoveCommandRowEntry>>();
    c.Bind("rows", &model.rows);

    c.Bind("title_text", &model.titleText);
    c.Bind("head_strife", &model.headStrife);
    c.Bind("head_map", &model.headMap);
    c.Bind("head_level", &model.headLevel);
    c.Bind("head_zen", &model.headZen);
    c.Bind("close_text", &model.closeText);

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
}

void mu::ui::window::CMoveCommandWindow::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void mu::ui::window::CMoveCommandWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    UI::RmlBridge::SyncDocumentVisibility(m_RmlView.Document(), IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());

    SettingCanMoveMap();
    RebuildRowModel();

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
    const Rml::String strifeMark = StringUtils::WideToNarrow(I18N::Game::Battle2987);

    wchar_t szText[24];
    int index = 0;
    for (const auto* moveInfo : m_listMoveInfoData)
    {
        // The three-quarter requirement for the Dark classes, exactly as Render() computed it --
        // the displayed number is the adjusted one, not _ReqInfo.iReqLevel.
        int iReqLevel = moveInfo->_ReqInfo.iReqLevel;
        if ((gCharacterManager.GetBaseClass(CharacterAttribute->Class) == CLASS_DARK
             || gCharacterManager.GetBaseClass(CharacterAttribute->Class) == CLASS_DARK_LORD
             || gCharacterManager.GetBaseClass(CharacterAttribute->Class) == CLASS_RAGEFIGHTER)
            && (iReqLevel != 400))
        {
            iReqLevel = int(float(iReqLevel) * 2.f / 3.f);
        }

        MoveCommandRowEntry entry;
        entry.strifeText = moveInfo->_bStrife ? strifeMark : Rml::String();
        entry.mapName = StringUtils::WideToNarrow(moveInfo->_ReqInfo.szMainMapName);
        _itow(iReqLevel, szText, 10);
        entry.reqLevel = StringUtils::WideToNarrow(szText);
        _itow(moveInfo->_ReqInfo.iReqZen, szText, 10);
        entry.reqZen = StringUtils::WideToNarrow(szText);
        entry.canMove = moveInfo->_bCanMove;
        entry.levelUnmet = iReqLevel > iLevel;
        entry.zenUnmet = moveInfo->_ReqInfo.iReqZen > (int)iZen;
        entry.index = index++;

        rows.push_back(std::move(entry));
    }

    // Only publish a genuine change. Marking the array dirty re-runs every row's text binding, and
    // this list is static for minutes at a time -- only a level-up, a zen change or an equipment
    // swap moves any of it.
    const bool changed = rows.size() != model.rows.size()
        || !std::equal(rows.begin(), rows.end(), model.rows.begin(),
                       [](const MoveCommandRowEntry& a, const MoveCommandRowEntry& b)
                       {
                           return a.canMove == b.canMove && a.levelUnmet == b.levelUnmet
                               && a.zenUnmet == b.zenUnmet && a.strifeText == b.strifeText
                               && a.reqLevel == b.reqLevel && a.reqZen == b.reqZen
                               && a.mapName == b.mapName;
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

BOOL CMoveCommandWindow::IsTheMapInDifferentServer(const int iFromMapIndex, const int iToMapIndex) const
{
    BOOL bInOtherServer = FALSE;

    switch (iFromMapIndex)
    {
    case WD_30BATTLECASTLE:
    case WD_79UNITEDMARKETPLACE:
        bInOtherServer = TRUE;
        break;
    default:
        break;
    }

    switch (iToMapIndex)
    {
    case 24:
    case 44:
        bInOtherServer = TRUE;
        break;
    default:
        break;
    }

    return bInOtherServer;
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
    SetStrifeMap();
    SettingCanMoveMap();

    const CMoveCommandData::MOVEINFODATA* moveInfo = FindMoveInfo(pszMapName);
    return moveInfo != NULL && moveInfo->_bCanMove;
}

int CMoveCommandWindow::GetMapIndexFromMovereq(const wchar_t* pszMapName)
{
    const CMoveCommandData::MOVEINFODATA* moveInfo = FindMoveInfo(pszMapName);
    return moveInfo != NULL ? moveInfo->_ReqInfo.index : -1;
}
