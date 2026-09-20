
#include "stdafx.h"
#include "UI/Quests/MyQuestInfoWindow.h"
#include "I18N/All.h"

#include "GameLogic/Quests/CSQuest.h"
#include "GameLogic/Quests/QuestMng.h"
#include "Audio/DSPlaySound.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Quests/QuestProgressByEtc.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "Engine/Object/ZzzInventory.h"
#include "Core/Utilities/StringUtils.h"
#include "UI/Scaling/UITransform.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlTooltip.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>

using namespace SEASON3B;
using namespace mu::ui::window;

extern int g_iNumLineMessageBoxCustom;
extern int g_iNumAnswer;
extern wchar_t g_lpszMessageBoxCustom[NUM_LINE_CMB][MAX_LENGTH_CMB];

// Computed fresh per call rather than cached in a member -- SyncRmlModel() builds both the
// empty-quest and job-change messages in the same pass, so a shared buffer would let one stomp the other.
std::vector<mu::ui::window::CMyQuestInfoWindow::TextLine> mu::ui::window::CMyQuestInfoWindow::BuildTextLines(
    int nGlobalTextIndex, int nPixelWidth)
{
    wchar_t aszMsg[2][64] = {};
    const int nLine = ::DivideStringByPixel(&aszMsg[0][0], 2, 64, I18N::Game::Lookup(nGlobalTextIndex), nPixelWidth);

    std::vector<TextLine> lines;
    for (int i = 0; i < nLine; ++i)
        lines.push_back({ StringUtils::WideToNarrow(aszMsg[i]) });
    return lines;
}

mu::ui::window::CMyQuestInfoWindow::CMyQuestInfoWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
}

mu::ui::window::CMyQuestInfoWindow::~CMyQuestInfoWindow()
{
    Release();
}

bool mu::ui::window::CMyQuestInfoWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_MYQUEST, this);

    SetPos(x, y);
    LoadImages();
    m_eTabBtnIndex = TAB_QUEST;

    if (RmlUiRuntime::Instance().IsCreated())
    {
        BuildRmlUi();
    }

    Show(false);

    return true;
}

void mu::ui::window::CMyQuestInfoWindow::BuildRmlUi()
{
    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "my_quest_info",
            [this](Rml::DataModelConstructor& c, MyQuestInfoRmlModel& model)
            {
                c.Bind("root_x", &model.rootX);
                c.Bind("root_y", &model.rootY);
                c.Bind("root_scale", &model.rootScale);

                c.Bind("active_tab", &model.activeTab);
                c.Bind("tab_quest_label", &model.tabQuestLabel);
                c.Bind("tab_jobchange_label", &model.tabJobChangeLabel);
                c.Bind("tab_castletemple_label", &model.tabCastleTempleLabel);

                c.Bind("quest_list_empty", &model.questListEmpty);
                c.Bind("open_enabled", &model.openEnabled);
                c.Bind("giveup_enabled", &model.giveupEnabled);

                c.Bind("open_tooltip", &model.openTooltip);
                c.Bind("giveup_tooltip", &model.giveupTooltip);
                c.Bind("exit_tooltip", &model.exitTooltip);

                auto textLine = c.RegisterStruct<TextLine>();
                textLine.RegisterMember("text", &TextLine::text);
                c.RegisterArray<std::vector<TextLine>>();
                c.Bind("empty_quest_lines", &model.emptyQuestLines);
                c.Bind("jobchange_lines", &model.jobChangeLines);
                c.Bind("jobchange_state_lines", &model.jobChangeStateLines);

                auto quest = c.RegisterStruct<QuestEntry>();
                quest.RegisterMember("text", &QuestEntry::text);
                quest.RegisterMember("index", &QuestEntry::index);
                quest.RegisterMember("selected", &QuestEntry::selected);
                c.RegisterArray<std::vector<QuestEntry>>();
                c.Bind("quests", &model.quests);

                auto content = c.RegisterStruct<ContentEntry>();
                content.RegisterMember("text", &ContentEntry::text);
                content.RegisterMember("color", &ContentEntry::color);
                content.RegisterMember("bold", &ContentEntry::bold);
                content.RegisterMember("index", &ContentEntry::index);
                content.RegisterMember("clickable", &ContentEntry::clickable);
                c.RegisterArray<std::vector<ContentEntry>>();
                c.Bind("contents", &model.contents);

                c.Bind("jobchange_title", &model.jobChangeTitle);
                c.Bind("castle_title", &model.castleTitle);
                c.Bind("castle_line0", &model.castleLine0);
                c.Bind("castle_line1", &model.castleLine1);
                c.Bind("temple_title", &model.templeTitle);
                c.Bind("temple_line0", &model.templeLine0);
                c.Bind("temple_line1", &model.templeLine1);

                c.BindEventCallback("myquest_select_tab",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                    {
                        if (arguments.size() == 1)
                            RmlClickSelectTab(arguments[0].Get<int>(-1));
                    });
                c.BindEventCallback("myquest_select_quest",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                    {
                        if (arguments.size() == 1)
                            RmlClickSelectQuest(arguments[0].Get<int>(-1));
                    });
                c.BindEventCallback("myquest_select_content",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                    {
                        if (arguments.size() == 1)
                            RmlClickSelectContent(arguments[0].Get<int>(-1));
                    });
                c.BindEventCallback("myquest_click_open",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickOpen(); });
                c.BindEventCallback("myquest_click_giveup",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickGiveUp(); });
                c.BindEventCallback("myquest_click_exit",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickExit(); });
            });

        if (modelCreated)
        {
            auto& model = m_RmlBinder.GetModel();
            model.tabQuestLabel = StringUtils::WideToNarrow(I18N::Game::Quest);
            model.tabJobChangeLabel = StringUtils::WideToNarrow(I18N::Game::ChangeClass);
            model.tabCastleTempleLabel = StringUtils::WideToNarrow(I18N::Game::CastleTemple);

            model.openTooltip = StringUtils::WideToNarrow(I18N::Game::StartQuest);
            model.giveupTooltip = StringUtils::WideToNarrow(I18N::Game::GiveUpQuest);
            model.exitTooltip = StringUtils::WideToNarrow(I18N::Game::Exit);
        }

    m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
        "Data/Interface/RmlUi/my_quest_info.rml");
}

void mu::ui::window::CMyQuestInfoWindow::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return; // never opened -- BuildRmlUi() will simply pick up the new theme whenever it first is

    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
    // Next frame's SyncRmlModel() self-corrects visibility/live model state.
}

void mu::ui::window::CMyQuestInfoWindow::Release()
{
    UnloadImages();

    if (m_pRmlDoc)
    {
        m_pRmlDoc->Close();
        m_pRmlDoc = nullptr;
    }

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CMyQuestInfoWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

void mu::ui::window::CMyQuestInfoWindow::Show(bool bShow)
{
    mu::ui::window::CObject::Show(bShow);
    if (m_pRmlDoc)
    {
        if (bShow) m_pRmlDoc->Show();
        else m_pRmlDoc->Hide();
    }
}

bool mu::ui::window::CMyQuestInfoWindow::UpdateMouseEvent()
{
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_MYQUEST))
        return false;

    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, MYQUESTINFO_WINDOW_WIDTH, MYQUESTINFO_WINDOW_HEIGHT).Contains(MouseX, MouseY))
        return false;

    return true;
}

bool mu::ui::window::CMyQuestInfoWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_MYQUEST) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MYQUEST);
            return false;
        }
    }

    return true;
}

bool mu::ui::window::CMyQuestInfoWindow::Update()
{
    SyncRmlModel();

    // Mirrors Render()'s own condition for drawing the reward-item info popup: whenever it
    // wouldn't be (re)shown this frame -- tab switched away from TAB_QUEST, selection cleared,
    // or the window itself hidden -- make sure the persistent tooltip document doesn't linger.
    if (IsVisible() && !(m_eTabBtnIndex == TAB_QUEST && m_pSelectedRewardItem))
    {
        UI::RmlBridge::Tooltip::Hide();
    }

    return true;
}

bool mu::ui::window::CMyQuestInfoWindow::Render()
{
    // RmlUi's #panel owns all chrome/text/list rendering; only the selected reward item's info
    // popup is still a native per-frame call here (see m_pSelectedRewardItem).
    if (m_eTabBtnIndex == TAB_QUEST && m_pSelectedRewardItem)
    {
        // Reference-pixel, not screen pixel -- RenderItemInfo() converts internally via the
        // ambient transform (same convention every other caller uses, see ZzzInventory.cpp).
        // Pre-converting here too used to double-apply the transform.
        ::RenderItemInfo(m_Pos.x + 95, m_Pos.y + 230, m_pSelectedRewardItem, false, 0, true);
    }

    return true;
}

float mu::ui::window::CMyQuestInfoWindow::GetLayerDepth()
{
    return 3.3f;
}

void mu::ui::window::CMyQuestInfoWindow::OpenningProcess()
{
    g_csQuest.ShowQuestPreviewWindow(-1);
}

void mu::ui::window::CMyQuestInfoWindow::ClosingProcess()
{
    UnselectQuestList();
    SocketClient->ToGameServer()->SendCloseNpcRequest();
    ::PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::CMyQuestInfoWindow::UnselectQuestList()
{
    m_dwSelectedQuestIndex = 0;
    m_ContentRows.clear();
    m_pSelectedRewardItem = nullptr;
    QuestOpenBtnEnable(false);
    QuestGiveUpBtnEnable(false);
}

void mu::ui::window::CMyQuestInfoWindow::SetCurQuestList(DWordList* pDWordList)
{
    m_QuestIndices.assign(pDWordList->begin(), pDWordList->end());

    m_dwSelectedQuestIndex = 0;
    m_ContentRows.clear();
    m_pSelectedRewardItem = nullptr;
    QuestOpenBtnEnable(false);
    QuestGiveUpBtnEnable(false);
}

void mu::ui::window::CMyQuestInfoWindow::SetSelQuestSummary()
{
    m_ContentRows.clear();
    m_pSelectedRewardItem = nullptr;

    const DWORD dwSelQuestIndex = GetSelQuestIndex();
    if (0 == dwSelQuestIndex)
        return;

    m_ContentRows.push_back({ StringUtils::WideToNarrow(g_QuestMng.GetSubject(dwSelQuestIndex)), 0xff0ab9ff, 0, nullptr });

    wchar_t aszSummary[8][64];
    const int nLine = ::DivideStringByPixel(&aszSummary[0][0], 8, 64, g_QuestMng.GetSummary(dwSelQuestIndex), 150);
    for (int i = 0; i < nLine; ++i)
        m_ContentRows.push_back({ StringUtils::WideToNarrow(aszSummary[i]), 0xffd2e6ff, 0, nullptr });
}

void mu::ui::window::CMyQuestInfoWindow::SetSelQuestRequestReward()
{
    const DWORD dwSelQuestIndex = GetSelQuestIndex();
    if (0 == dwSelQuestIndex)
        return;

    if (!g_QuestMng.IsRequestRewardQS(dwSelQuestIndex))
        return;

    const SQuestRequestReward* pQuestRequestReward = g_QuestMng.GetRequestReward(dwSelQuestIndex);
    if (NULL == pQuestRequestReward)
        return;

    SRequestRewardText aRequestRewardText[13];
    g_QuestMng.GetRequestRewardText(aRequestRewardText, 13, dwSelQuestIndex);

    int i = 0;
    int j, nLoop;
    for (j = 0; j < 3; ++j)
    {
        if (0 == j)
        {
            m_ContentRows.push_back({ " ", 0xffffffff, 0, nullptr });
            nLoop = 1 + pQuestRequestReward->m_byRequestCount;
        }
        else if (1 == j && pQuestRequestReward->m_byGeneralRewardCount)
        {
            m_ContentRows.push_back({ " ", 0xffffffff, 0, nullptr });
            nLoop = 1 + pQuestRequestReward->m_byGeneralRewardCount + i;
        }
        else if (2 == j && pQuestRequestReward->m_byRandRewardCount)
        {
            m_ContentRows.push_back({ " ", 0xffffffff, 0, nullptr });
            nLoop = 1 + pQuestRequestReward->m_byRandRewardCount + i;
        }
        else
            nLoop = 0;

        for (; i < nLoop; ++i)
        {
            m_ContentRows.push_back({ StringUtils::WideToNarrow(aRequestRewardText[i].m_szText),
                static_cast<DWORD>(aRequestRewardText[i].m_dwColor), aRequestRewardText[i].m_dwType,
                aRequestRewardText[i].m_pItem });
        }
    }
}

void mu::ui::window::CMyQuestInfoWindow::QuestOpenBtnEnable(bool bEnable)
{
    if (m_RmlBinder.GetModel().openEnabled != bEnable)
    {
        m_RmlBinder.GetModel().openEnabled = bEnable;
        m_RmlBinder.MarkDirty("open_enabled");
    }
}

void mu::ui::window::CMyQuestInfoWindow::QuestGiveUpBtnEnable(bool bEnable)
{
    if (m_RmlBinder.GetModel().giveupEnabled != bEnable)
    {
        m_RmlBinder.GetModel().giveupEnabled = bEnable;
        m_RmlBinder.MarkDirty("giveup_enabled");
    }
}

DWORD mu::ui::window::CMyQuestInfoWindow::GetSelQuestIndex()
{
    return m_dwSelectedQuestIndex;
}

void mu::ui::window::CMyQuestInfoWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_MYQUEST_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back01.tga", IMAGE_MYQUEST_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_MYQUEST_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_MYQUEST_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_MYQUEST_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_MYQUEST_BTN_EXIT, GL_LINEAR);

    LoadBitmap(L"Interface\\newui_myquest_Line.tga", IMAGE_MYQUEST_LINE, GL_LINEAR);
    LoadBitmap(L"Interface\\Quest_Bt_open.tga", IMAGE_MYQUEST_BTN_OPEN, GL_LINEAR);
    LoadBitmap(L"Interface\\Quest_Bt_cast.tga", IMAGE_MYQUEST_BTN_GIVE_UP, GL_LINEAR);
    LoadBitmap(L"Interface\\Quest_tab01.tga", IMAGE_MYQUEST_TAB_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\Quest_tab02.tga", IMAGE_MYQUEST_TAB_SMALL, GL_LINEAR);
    LoadBitmap(L"Interface\\Quest_tab03.tga", IMAGE_MYQUEST_TAB_BIG, GL_LINEAR);
}

void mu::ui::window::CMyQuestInfoWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_MYQUEST_TAB_BIG);
    DeleteBitmap(IMAGE_MYQUEST_TAB_SMALL);
    DeleteBitmap(IMAGE_MYQUEST_TAB_BACK);
    DeleteBitmap(IMAGE_MYQUEST_BTN_GIVE_UP);
    DeleteBitmap(IMAGE_MYQUEST_BTN_OPEN);
    DeleteBitmap(IMAGE_MYQUEST_LINE);
    DeleteBitmap(IMAGE_MYQUEST_BTN_EXIT);
    DeleteBitmap(IMAGE_MYQUEST_BOTTOM);
    DeleteBitmap(IMAGE_MYQUEST_RIGHT);
    DeleteBitmap(IMAGE_MYQUEST_LEFT);
    DeleteBitmap(IMAGE_MYQUEST_TOP);
    DeleteBitmap(IMAGE_MYQUEST_BACK);
}

void mu::ui::window::CMyQuestInfoWindow::RmlClickSelectTab(int nTab)
{
    if (nTab < TAB_QUEST || nTab > TAB_CASTLE_TEMPLE)
        return;

    m_eTabBtnIndex = static_cast<TAB_BUTTON_INDEX>(nTab);
    ::PlayBuffer(SOUND_CLICK01);

    if (m_eTabBtnIndex == TAB_CASTLE_TEMPLE)
    {
        SocketClient->ToGameServer()->SendMiniGameEventCountRequest(MiniGameType::BloodCastle);
        SocketClient->ToGameServer()->SendMiniGameEventCountRequest(MiniGameType::CursedTemple);
    }
}

void mu::ui::window::CMyQuestInfoWindow::RmlClickSelectQuest(int nQuestIndex)
{
    const DWORD dwQuestIndex = static_cast<DWORD>(nQuestIndex);
    if (dwQuestIndex == m_dwSelectedQuestIndex)
        return;

    ::PlayBuffer(SOUND_CLICK01);

    m_dwSelectedQuestIndex = dwQuestIndex;

    QuestOpenBtnEnable(g_QuestMng.IsQuestByEtc(dwQuestIndex));
    QuestGiveUpBtnEnable(true);
    SetSelQuestSummary();

    const auto questNumber = static_cast<uint16_t>(LOWORD(dwQuestIndex));
    const auto questGroup = static_cast<uint16_t>(HIWORD(dwQuestIndex));
    SocketClient->ToGameServer()->SendQuestStateRequest(questNumber, questGroup);
}

void mu::ui::window::CMyQuestInfoWindow::RmlClickSelectContent(int nContentIndex)
{
    if (nContentIndex < 0 || static_cast<size_t>(nContentIndex) >= m_ContentRows.size())
        return;

    const ContentRowData& row = m_ContentRows[nContentIndex];
    if ((row.dwType == QUEST_REQUEST_ITEM || row.dwType == QUEST_REWARD_ITEM) && row.pItem)
        m_pSelectedRewardItem = row.pItem;
    else
        m_pSelectedRewardItem = nullptr;
}

void mu::ui::window::CMyQuestInfoWindow::RmlClickOpen()
{
    if (!m_RmlBinder.GetModel().openEnabled)
        return;
    ::PlayBuffer(SOUND_CLICK01);
    g_pQuestProgressByEtc->SetContents(GetSelQuestIndex());
    g_pNewUISystem->Show(mu::ui::window::INTERFACE_QUEST_PROGRESS_ETC);
}

void mu::ui::window::CMyQuestInfoWindow::RmlClickGiveUp()
{
    if (!m_RmlBinder.GetModel().giveupEnabled)
        return;
    ::PlayBuffer(SOUND_CLICK01);

    // Second proof case for CGenericConfirmDialog (see UI/Dialogs/GenericConfirmDialog.h) -- was
    // CreateMessageBox(MSGBOX_LAYOUT_CLASS(CQuestGiveUpMsgBoxLayout)), an OK/Cancel confirm whose
    // OK sent SendQuestCancelRequest for the currently-selected quest.
    mu::ui::window::GenericDialogConfig cfg;
    cfg.showCancel = true;
    cfg.lines.push_back({ I18N::Game::IfYouGiveUpYouWill, false });
    cfg.onPrimary = [this]()
    {
        const DWORD dwSelectedQuest = GetSelQuestIndex();
        const auto questNumber = static_cast<uint16_t>(LOWORD(dwSelectedQuest));
        const auto questGroup = static_cast<uint16_t>(HIWORD(dwSelectedQuest));
        SocketClient->ToGameServer()->SendQuestCancelRequest(questNumber, questGroup);
    };
    mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
}

void mu::ui::window::CMyQuestInfoWindow::RmlClickExit()
{
    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MYQUEST);
}

void mu::ui::window::CMyQuestInfoWindow::SyncRmlModel()
{
    if (!m_pRmlDoc)
        return;

    auto& model = m_RmlBinder.GetModel();

    if (model.activeTab != static_cast<int>(m_eTabBtnIndex))
    {
        model.activeTab = static_cast<int>(m_eTabBtnIndex);
        m_RmlBinder.MarkDirty("active_tab");
    }

    const auto transform = UI::Scaling::GetActiveTransform();
    const float rootX = static_cast<float>(m_Pos.x) * transform.scaleX + transform.offsetX;
    const float rootY = static_cast<float>(m_Pos.y) * transform.scaleY + transform.offsetY;
    if (model.rootX != rootX || model.rootY != rootY || model.rootScale != transform.scaleX)
    {
        model.rootX = rootX;
        model.rootY = rootY;
        model.rootScale = transform.scaleX;
        m_RmlBinder.MarkDirty("root_x");
        m_RmlBinder.MarkDirty("root_y");
        m_RmlBinder.MarkDirty("root_scale");
    }

    const bool bEmpty = m_QuestIndices.empty();
    if (model.questListEmpty != bEmpty)
    {
        model.questListEmpty = bEmpty;
        m_RmlBinder.MarkDirty("quest_list_empty");
    }

    // Small lists, rebuilt and marked dirty unconditionally each sync (same as CreditWin/BuffStrip).
    model.emptyQuestLines = bEmpty ? BuildTextLines(2825, 140) : std::vector<TextLine>{};
    m_RmlBinder.MarkDirty("empty_quest_lines");

    model.quests.clear();
    wchar_t szInput[64];
    wchar_t szOutput[64];
    int i = 1;
    for (DWORD dwQuestIndex : m_QuestIndices)
    {
        ::mu_swprintf(szInput, L"%d.%ls", i, g_QuestMng.GetSubject(dwQuestIndex));
        ::ReduceStringByPixel(szOutput, 64, szInput, 150);
        model.quests.push_back({ StringUtils::WideToNarrow(szOutput), static_cast<int>(dwQuestIndex),
            dwQuestIndex == m_dwSelectedQuestIndex });
        ++i;
    }
    m_RmlBinder.MarkDirty("quests");

    model.contents.clear();
    for (size_t rowIndex = 0; rowIndex < m_ContentRows.size(); ++rowIndex)
    {
        const ContentRowData& row = m_ContentRows[rowIndex];
        wchar_t colorBuf[32];
        mu_swprintf(colorBuf, L"rgba(%d,%d,%d,%d)", (row.dwColor >> 16) & 0xff, (row.dwColor >> 8) & 0xff,
            row.dwColor & 0xff, (row.dwColor >> 24) & 0xff);
        const bool clickable = row.pItem && (row.dwType == QUEST_REQUEST_ITEM || row.dwType == QUEST_REWARD_ITEM);
        model.contents.push_back({ row.text, StringUtils::WideToNarrow(colorBuf), false,
            static_cast<int>(rowIndex), clickable });
    }
    m_RmlBinder.MarkDirty("contents");

    model.jobChangeLines.clear();
    for (int j = 0; j < g_iNumLineMessageBoxCustom; ++j)
        model.jobChangeLines.push_back({ StringUtils::WideToNarrow(g_lpszMessageBoxCustom[j]) });
    m_RmlBinder.MarkDirty("jobchange_lines");

    if (model.jobChangeTitle.empty())
    {
        model.jobChangeTitle = StringUtils::WideToNarrow(g_csQuest.getQuestTitleWindow());
        m_RmlBinder.MarkDirty("jobchange_title");
    }

    if (m_eTabBtnIndex == TAB_JOB_CHANGE)
    {
        const BYTE byState = g_csQuest.getCurrQuestState();
        int nStateTextIndex = 930;
        if (byState == QUEST_ING)
            nStateTextIndex = 931;
        else if (byState == QUEST_END)
            nStateTextIndex = 932;
        // Any other state (QUEST_NONE/QUEST_NO/QUEST_ERROR) falls through to 930, matching the original.

        model.jobChangeStateLines = BuildTextLines(nStateTextIndex, 140);
        m_RmlBinder.MarkDirty("jobchange_state_lines");
    }

    if (m_eTabBtnIndex == TAB_CASTLE_TEMPLE)
    {
        wchar_t strText[256];

        model.castleTitle = StringUtils::WideToNarrow(I18N::Game::BloodCastle);
        mu_swprintf(strText, I18N::Game::EntranceIsAllowedForDTimes, g_csQuest.GetEventCount(2));
        model.castleLine0 = StringUtils::WideToNarrow(strText);
        mu_swprintf(strText, I18N::Game::YouMayEnterOnlyDTimesPerDay, 6);
        model.castleLine1 = StringUtils::WideToNarrow(strText);
        m_RmlBinder.MarkDirty("castle_title");
        m_RmlBinder.MarkDirty("castle_line0");
        m_RmlBinder.MarkDirty("castle_line1");

        model.templeTitle = StringUtils::WideToNarrow(I18N::Game::IllusionTemple);
        mu_swprintf(strText, I18N::Game::EntranceIsAllowedForDTimes, g_csQuest.GetEventCount(3));
        model.templeLine0 = StringUtils::WideToNarrow(strText);
        mu_swprintf(strText, I18N::Game::YouMayEnterOnlyDTimesPerDay, 6);
        model.templeLine1 = StringUtils::WideToNarrow(strText);
        m_RmlBinder.MarkDirty("temple_title");
        m_RmlBinder.MarkDirty("temple_line0");
        m_RmlBinder.MarkDirty("temple_line1");
    }
}
