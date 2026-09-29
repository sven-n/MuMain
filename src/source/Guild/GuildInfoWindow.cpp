
#include "stdafx.h"
#include "GuildInfoWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "Audio/DSPlaySound.h"
#include "UIGuildInfo.h"
#include "UI/Widgets/UIControls.h"
#include "UI/Dialogs/UIPopup.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Scenes/SceneCore.h" // g_iLengthAuthorityCode -- CGuildBreakPasswordMsgBoxLayout's own maxLength
#include "I18N/All.h"

#include "Character/CharacterManager.h"

#include "Core/Utilities/StringUtils.h"
#include "Guild/GuildMarkPalette.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlColor.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>

int	DeleteIndex = 0;
int AppointStatus = 0;
wchar_t DeleteID[100];

extern CUIPopup* g_pUIPopup;
extern MARK_t GuildMark[MAX_MARKS];

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
    // Was CGuildBreakPasswordMsgBoxLayout (CustomMessageBox.h) -- a masked (bIsPassword=true),
    // non-numeric-restricted Mode::Text WEBZEN.COM password entry, ported 2026-09-14. Shared across
    // this file's 3 call sites (guild-disband, leave-guild, kick-member), all of which already set
    // the global `DeleteIndex` right before showing this. Unlike the other masked-password dialogs
    // in this batch, native's own ProcessOk does NOT return CALLBACK_CONTINUE on empty input -- it
    // always closes, just logs an error message -- so this one deliberately never calls KeepOpen().
    void ShowGuildBreakPasswordDialog()
    {
        GenericDialogConfig cfg;
        cfg.showCancel = true;
        cfg.lines = {
            { I18N::Game::IfYouWantToLeaveYourGuild, false },
            { I18N::Game::PleaseEnterYourWEBZENCOMPassword, false },
        };
        cfg.input = GenericDialogConfig::InputField{};
        cfg.input->mode = GenericDialogConfig::InputField::Mode::Text;
        cfg.input->maxLength = g_iLengthAuthorityCode;
        cfg.input->masked = true;
        cfg.onPrimary = []
        {
            const std::wstring strText = g_pGenericConfirmDialog->GetInputText();
            if (!strText.empty())
            {
                SocketClient->ToGameServer()->SendGuildKickPlayerRequest(MU_C16(GuildList[DeleteIndex].Name), MU_C16(strText.c_str()));
            }
            else
            {
                g_pSystemLogBox->AddText(I18N::Game::ThePasswordYouHaveEnteredIsIncorrect, mu::ui::window::TYPE_ERROR_MESSAGE);
            }
        };
        g_pGenericConfirmDialog->Show(std::move(cfg));
    }
}

namespace
{
// The mark cells of a 64-byte guild mark, as CreateGuildMark() builds the texture.
std::vector<Rml::String> MarkCells(const BYTE* mark)
{
    std::vector<Rml::String> cells;
    cells.reserve(Guild::MarkPalette::CellCount);
    for (int i = 0; i < Guild::MarkPalette::CellCount; ++i)
        cells.push_back(Guild::MarkPalette::CellColor(mark[i]));
    return cells;
}

bool SameText(const GuildInfoTextEntry& a, const GuildInfoTextEntry& b)
{
    return a.text == b.text && a.left == b.left && a.top == b.top && a.width == b.width && a.textPx == b.textPx &&
           a.align == b.align && a.bold == b.bold && a.color == b.color;
}

bool SameBox(const GuildInfoBoxEntry& a, const GuildInfoBoxEntry& b)
{
    return a.left == b.left && a.top == b.top && a.width == b.width && a.height == b.height;
}

bool SameMark(const GuildInfoMarkEntry& a, const GuildInfoMarkEntry& b)
{
    return a.left == b.left && a.top == b.top && a.cells == b.cells;
}

bool SameButton(const GuildInfoButtonEntry& a, const GuildInfoButtonEntry& b)
{
    return a.label == b.label && a.index == b.index && a.left == b.left && a.top == b.top;
}

template <typename T, typename Same>
void SyncList(std::vector<T>& current, std::vector<T>&& updated, Same same, RmlModelBinder<GuildInfoRmlModel>& binder,
              const char* name)
{
    if (current.size() == updated.size() && std::equal(current.begin(), current.end(), updated.begin(), same))
        return;
    current = std::move(updated);
    binder.MarkDirty(name);
}

template <typename T>
void SyncField(RmlModelBinder<GuildInfoRmlModel>& binder, T GuildInfoRmlModel::* field, const char* name, T value)
{
    GuildInfoRmlModel& model = binder.GetModel();
    if (model.*field == value)
        return;
    model.*field = std::move(value);
    binder.MarkDirty(name);
}
} // namespace

int mu::ui::window::CGuildInfoWindow::GetGuildMemberIndex(wchar_t* szName)
{
    for (int i = 0; i < g_nGuildMemberCount; ++i)
    {
        if (GuildList[i].Name && !wcscmp(GuildList[i].Name, szName))
            return i;
    }

    return -1;
}

mu::ui::window::CGuildInfoWindow::CGuildInfoWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_nCurrentTab = static_cast<int>(GuildConstants::GuildTab::MEMBERS);
    m_EventState = EVENT_NONE;
    m_Loc = 0;
    m_BackUp = 0;
    m_CurrentListPos = 0;
    m_Loc_Bk = 0;
    m_Tot_Notice = 0;
    m_dwPopupID = 0;

    m_bRequestUnionList = false;
}

mu::ui::window::CGuildInfoWindow::~CGuildInfoWindow()
{
    Release();
}

bool mu::ui::window::CGuildInfoWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == g_pNewUI3DRenderMng || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_GUILDINFO, this);

    SetPos(x, y);

    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}

void mu::ui::window::CGuildInfoWindow::OpenningProcess()
{
    m_nCurrentTab = static_cast<int>(GuildConstants::GuildTab::MEMBERS);

    SocketClient->ToGameServer()->SendGuildListRequest();
}

void mu::ui::window::CGuildInfoWindow::ClosingProcess()
{
    m_bRequestUnionList = false;
}

void mu::ui::window::CGuildInfoWindow::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CGuildInfoWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool mu::ui::window::CGuildInfoWindow::UpdateMouseEvent()
{
    bool ret = true;

    if (mu::ui::window::IsPress(VK_LBUTTON))
    {
        ret = Check_Mouse(MouseX, MouseY);
        if (ret == false)
        {
            PlayBuffer(SOUND_CLICK01);
        }
    }

    if (m_EventState == EVENT_SCROLL_BTN_DOWN)
    {
        if (mu::ui::window::IsRepeat(VK_LBUTTON))
        {
            return false;
        }
        if (mu::ui::window::IsRelease(VK_LBUTTON))
        {
            m_EventState = EVENT_NONE;
            return true;
        }
    }

    // Top-right corner close "X" (shared frame): hides + swallows the click. The buttons are
    // RmlUi's (see Update()).
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_GUILDINFO))
    {
        m_EventState = EVENT_NONE;
        return false;
    }

    if (m_nCurrentTab == static_cast<int>(GuildConstants::GuildTab::INFO))
    {
        m_GuildNotice.DoAction();
    }
    else if (m_nCurrentTab == static_cast<int>(GuildConstants::GuildTab::MEMBERS))
    {
        m_GuildMember.DoAction();
    }
    else if (m_nCurrentTab == static_cast<int>(GuildConstants::GuildTab::UNION))
    {
        m_UnionListBox.DoAction();
    }

    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, GUILDINFO_WIDTH, GUILDINFO_HEIGHT).Contains(MouseX, MouseY))
    {
        return false;
    }

    return ret;
}

bool mu::ui::window::CGuildInfoWindow::Check_Btn(int button)
{
    if (button == BUTTON_EXIT)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GUILDINFO);
        m_EventState = EVENT_NONE;
        return false;
    }

    if (m_nCurrentTab == static_cast<int>(GuildConstants::GuildTab::INFO))
    {
        if (button == BUTTON_GUILD_OUT)
        {
            if (Hero->GuildStatus == G_MASTER)
            {
                if (!wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[Hero->GuildMarkIndex].UnionName))
                {
                    // First proof case for CGenericConfirmDialog (see UI/Dialogs/GenericConfirmDialog.h) --
                    // was CreateMessageBox(MSGBOX_LAYOUT_CLASS(CGuildOutPerson)), an OK-only informational box.
                    mu::ui::window::GenericDialogConfig cfg;
                    cfg.lines.push_back({ I18N::Game::AllianceMasterCanTDisbandTheGuild, true });
                    mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
                }
                else
                {
                    DeleteIndex = GetGuildMemberIndex(Hero->ID);
                    mu::ui::window::GenericDialogConfig cfg;
                    cfg.showCancel = true;
                    cfg.lines = {
                        { I18N::Game::OnceYouDisbandTheGuild, true },
                        { I18N::Game::AllTheItemsAndZenInTheGuildVaultWillDisappear, true },
                        { I18N::Game::AlsoTheGuildRankingInformationWillDisappear, true },
                        { I18N::Game::WouldYouLikeToDisbandTheGuild, true },
                    };
                    cfg.onPrimary = []
                    {
                        ShowGuildBreakPasswordDialog();
                    };
                    mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
                }
            }
            else
            {
                DeleteIndex = GetGuildMemberIndex(Hero->ID);
                ShowGuildBreakPasswordDialog();
            }
        }
    }
    else if (m_nCurrentTab == static_cast<int>(GuildConstants::GuildTab::MEMBERS))
    {
        if (button == BUTTON_GET_OUT)
        {
            if (Hero->GuildStatus == G_MASTER)
            {
                if (GUILDLIST_TEXT* pText = m_GuildMember.GetSelectedText())
                {
                    if (pText->m_GuildStatus != G_MASTER)
                    {
                        if (GUILDLIST_TEXT* pText = m_GuildMember.GetSelectedText())
                        {
                            DeleteIndex = GetGuildMemberIndex(pText->m_szID);
                            wchar_t szNameText[300];
                            mu_swprintf(szNameText, I18N::Game::CharacterS, GuildList[DeleteIndex].Name);
                            mu::ui::window::GenericDialogConfig cfg;
                            cfg.showCancel = true;
                            cfg.lines = {
                                { szNameText, true },
                                { I18N::Game::WouldYouLikeToRelease, true },
                            };
                            cfg.onPrimary = []
                            {
                                ShowGuildBreakPasswordDialog();
                            };
                            mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
                        }
                    }
                }
            }
        }
        else if (button == BUTTON_GET_POSITION)
        {
            if (GUILDLIST_TEXT* pText = m_GuildMember.GetSelectedText())
            {
                if (Hero->GuildStatus == G_MASTER)
                {
                    if (pText->m_GuildStatus != G_MASTER)
                    {
                        if (GUILDLIST_TEXT* pText = m_GuildMember.GetSelectedText())
                        {
                            AppointStatus = (GUILD_STATUS)pText->m_GuildStatus;
                            DeleteIndex = GetGuildMemberIndex(pText->m_szID);
                            mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(CGuild_ToPerson_PositionLayout));
                        }
                    }
                }
            }
        }
        else if (button == BUTTON_FREE_POSITION)
        {
            if (GUILDLIST_TEXT* pText = m_GuildMember.GetSelectedText())
            {
                if (pText->m_GuildStatus == G_SUB_MASTER || pText->m_GuildStatus == G_BATTLE_MASTER)
                {
                    if (GUILDLIST_TEXT* pText = m_GuildMember.GetSelectedText())
                    {
                        AppointStatus = (GUILD_STATUS)pText->m_GuildStatus;
                        DeleteIndex = GetGuildMemberIndex(pText->m_szID);
                        wchar_t strText[256];
                        mu_swprintf(strText, I18N::Game::CharacterS, pText->m_szID);
                        mu::ui::window::GenericDialogConfig cfg;
                        cfg.showCancel = true;
                        cfg.lines = {
                            { strText, false },
                            { I18N::Game::WouldYouLikeToCancelTheRanking, false },
                        };
                        cfg.onPrimary = []
                        {
                            SocketClient->ToGameServer()->SendGuildRoleAssignRequest(G_PERSON, MU_C16(GuildList[DeleteIndex].Name), 0x03);
                        };
                        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
                    }
                }
            }
        }
    }
    else if (m_nCurrentTab == static_cast<int>(GuildConstants::GuildTab::UNION))
    {
        if (button == BUTTON_UNION_CREATE)
        {
            if (Hero->GuildStatus == G_MASTER)
            {
                if (UNIONGUILD_TEXT* pText = m_UnionListBox.GetSelectedText())
                {
                    if (wcscmp(pText->szName, GuildMark[Hero->GuildMarkIndex].GuildName))
                    {
                        wcscpy(DeleteID, pText->szName);
                        wchar_t szAllianceText[256];
                        mu_swprintf(szAllianceText, I18N::Game::SGuildFromTheAlliance, DeleteID);
                        mu::ui::window::GenericDialogConfig cfg;
                        cfg.showCancel = true;
                        cfg.lines = {
                            { szAllianceText, false },
                            { I18N::Game::WouldYouLikeToRelease, false },
                        };
                        cfg.onPrimary = [] { SocketClient->ToGameServer()->SendRemoveAllianceGuildRequest(MU_C16(DeleteID)); };
                        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
                    }
                }
            }
        }
        else if (button == BUTTON_UNION_OUT)
        {
            if (Hero->GuildStatus == G_MASTER)
            {
                const bool isUnionMaster = wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[Hero->GuildMarkIndex].UnionName) == 0;
                if (isUnionMaster)
                {
                    mu::ui::window::GenericDialogConfig cfg;
                    cfg.lines.push_back({ I18N::Game::AllianceMasterCanTWithdrawTheGuild, true });
                    mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
                }
                else
                {
                    SocketClient->ToGameServer()->SendGuildRelationshipChangeRequest(GuildRelationshipType::Alliance, GuildRequestType::Leave, Hero->Key);
                }
            }
        }
    }
    return true;
}

bool mu::ui::window::CGuildInfoWindow::Check_Mouse(int mx, int my)
{
    if (mx > m_Pos.x && mx < (m_Pos.x + GUILDINFO_WIDTH) && my > m_Pos.y && my < (m_Pos.y + GUILDINFO_HEIGHT))
    {
        for (int i = 0; i < 3; i++)
        {
            int Tab_Pos = i * 56;
            if (mx > (m_Pos.x + 12 + Tab_Pos) && mx < (m_Pos.x + 12 + Tab_Pos + 56) && my > m_Pos.y && my < (m_Pos.y + 90))
            {
                if (m_nCurrentTab != i)
                {
                    m_BackUp = 0;
                    m_Loc = 0;
                    m_CurrentListPos = 0;
                }
                m_nCurrentTab = i;
                switch (m_nCurrentTab)
                {
                case static_cast<int>(GuildConstants::GuildTab::INFO):
                    break;
                case static_cast<int>(GuildConstants::GuildTab::MEMBERS):
                {
                    SocketClient->ToGameServer()->SendGuildListRequest();
                }
                break;
                case static_cast<int>(GuildConstants::GuildTab::UNION):
                {
                    if (m_bRequestUnionList == false
                        && GuildMark[Hero->GuildMarkIndex].UnionName[0] != 0)
                    {
                        SocketClient->ToGameServer()->SendRequestAllianceList();
                        m_bRequestUnionList = true;
                    }
                }
                break;
                }
                return false;
            }
        }
    }
    if (m_nCurrentTab == 0)
    {
        if (mu::ui::window::CheckMouseIn(m_Pos.x + 163, m_Pos.y + 262 + m_Loc, 18, 33 + m_Loc) == true && m_EventState == EVENT_NONE)
        {
            m_EventState = EVENT_SCROLL_BTN_DOWN;
            if (m_BackUp == 0)
            {
                m_BackUp = (262 + (MouseY - 262));
            }
            return false;
        }
    }
    else if (m_nCurrentTab == 1)
    {
        if ((MouseX > m_Pos.x + 166 && MouseX < (m_Pos.x + 181) && MouseY > m_Pos.y + 125 + m_Loc && MouseY < (m_Pos.y + 175 + m_Loc)) && m_EventState == EVENT_NONE)
        {
            m_EventState = EVENT_SCROLL_BTN_DOWN;
            if (m_BackUp == 0)
                m_BackUp = (125 + (MouseY - 125));
            return false;
        }
    }
    return true;
}

bool mu::ui::window::CGuildInfoWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_GUILDINFO) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_GUILDINFO);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    return true;
}
bool mu::ui::window::CGuildInfoWindow::Update()
{
    // A button RmlUi reported (the original's CButton handling in Check_Btn()).
    const int button = m_PendingButton;
    m_PendingButton = -1;
    if (IsVisible() && button >= 0)
    {
        PlayBuffer(SOUND_CLICK01);
        Check_Btn(button);
    }

    if (IsVisible() && Hero->GuildStatus != G_NONE)
        UpdateScrollThumb();

    SyncRmlModel();
    return true;
}

// The original did this in RenderScrollBar() / Render_Guild_History(): while the thumb is held,
// it follows the pointer and scrolls the list by the same fraction.
void mu::ui::window::CGuildInfoWindow::UpdateScrollThumb()
{
    const bool notice = m_nCurrentTab == static_cast<int>(GuildConstants::GuildTab::INFO);
    const bool members = m_nCurrentTab == static_cast<int>(GuildConstants::GuildTab::MEMBERS);
    if (!notice && !members)
        return;

    const int range =
        notice ? GuildConstants::UILayout::SCROLL_RANGE_NOTICE : GuildConstants::UILayout::SCROLL_RANGE_MEMBERS;
    int Line = 0;
    if (m_EventState == EVENT_SCROLL_BTN_DOWN && m_BackUp > 0)
    {
        m_Loc = (MouseY - m_BackUp);
        if (m_Loc < 0)
            m_Loc = 0;
        else if (m_Loc > range)
            m_Loc = range;

        if (m_Loc != m_Loc_Bk)
        {
            const int lines =
                notice ? m_Tot_Notice - m_GuildNotice.GetBoxSize() : g_nGuildMemberCount - m_GuildMember.GetBoxSize();
            const int Loc_Scroll =
                static_cast<int>(static_cast<float>(lines) / static_cast<float>(range) * static_cast<float>(m_Loc));
            Line = Loc_Scroll - m_CurrentListPos;
            m_CurrentListPos += Line;
            m_Loc_Bk = m_Loc;
        }
    }

    if (notice)
        m_GuildNotice.Scrolling(Line);
    else
        m_GuildMember.Scrolling(Line);
}

bool mu::ui::window::CGuildInfoWindow::Render()
{
    // Nothing native left: the frame, the tabs, the lists' lines, the marks and the buttons are
    // RmlUi. Kept because CObject requires the override.
    return true;
}

void mu::ui::window::CGuildInfoWindow::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(
        RmlUiRuntime::Instance().GetContext(), "guild_info",
        [this](Rml::DataModelConstructor& c, GuildInfoRmlModel& model)
        {
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);
            c.Bind("root_scale", &model.rootScale);
            c.Bind("text_px", &model.textPx);
            c.Bind("no_guild", &model.noGuild);
            c.Bind("tab", &model.tab);
            c.Bind("union_shown", &model.unionShown);
            c.Bind("scroll_top", &model.scrollTop);
            c.Bind("thumb_top", &model.thumbTop);
            c.RegisterArray<std::vector<Rml::String>>();
            c.Bind("mark_cells", &model.markCells);

            auto text = c.RegisterStruct<GuildInfoTextEntry>();
            text.RegisterMember("text", &GuildInfoTextEntry::text);
            text.RegisterMember("left", &GuildInfoTextEntry::left);
            text.RegisterMember("top", &GuildInfoTextEntry::top);
            text.RegisterMember("width", &GuildInfoTextEntry::width);
            text.RegisterMember("text_px", &GuildInfoTextEntry::textPx);
            text.RegisterMember("align", &GuildInfoTextEntry::align);
            text.RegisterMember("bold", &GuildInfoTextEntry::bold);
            text.RegisterMember("color", &GuildInfoTextEntry::color);
            c.RegisterArray<std::vector<GuildInfoTextEntry>>();
            c.Bind("texts", &model.texts);

            auto box = c.RegisterStruct<GuildInfoBoxEntry>();
            box.RegisterMember("left", &GuildInfoBoxEntry::left);
            box.RegisterMember("top", &GuildInfoBoxEntry::top);
            box.RegisterMember("width", &GuildInfoBoxEntry::width);
            box.RegisterMember("height", &GuildInfoBoxEntry::height);
            c.RegisterArray<std::vector<GuildInfoBoxEntry>>();
            c.Bind("boxes", &model.boxes);

            auto mark = c.RegisterStruct<GuildInfoMarkEntry>();
            mark.RegisterMember("left", &GuildInfoMarkEntry::left);
            mark.RegisterMember("top", &GuildInfoMarkEntry::top);
            mark.RegisterMember("cells", &GuildInfoMarkEntry::cells);
            c.RegisterArray<std::vector<GuildInfoMarkEntry>>();
            c.Bind("marks", &model.marks);

            auto button = c.RegisterStruct<GuildInfoButtonEntry>();
            button.RegisterMember("label", &GuildInfoButtonEntry::label);
            button.RegisterMember("index", &GuildInfoButtonEntry::index);
            button.RegisterMember("left", &GuildInfoButtonEntry::left);
            button.RegisterMember("top", &GuildInfoButtonEntry::top);
            c.RegisterArray<std::vector<GuildInfoButtonEntry>>();
            c.Bind("buttons", &model.buttons);

            c.Bind("exit_tooltip", &model.exitTooltip);
            c.Bind("label_top", &model.labelTop);
            c.Bind("label_line_px", &model.labelLinePx);
            c.BindEventCallback("guild_info_button",
                                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                                {
                                    if (arguments.size() == 1)
                                        m_PendingButton = arguments[0].Get<int>(-1);
                                });
        });
    if (!modelCreated)
        return;

    m_pRmlDoc =
        UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/guild_info.rml");
}

void mu::ui::window::CGuildInfoWindow::ReloadRmlTheme()
{
    if (!m_pRmlDoc)
        return;
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
}

void mu::ui::window::CGuildInfoWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_pRmlDoc)
        return;

    // Layer depth 4.5: over the HUD like every panel the original opened.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_pRmlDoc, IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncRootTransform(m_RmlBinder, m_Pos);
    UI::RmlBridge::SyncNativeTextSize(m_RmlBinder);
    SyncContent();
}

void mu::ui::window::CGuildInfoWindow::SyncContent()
{
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    const DWORD white = 0xFFFFFFFF;
    const float x0 = static_cast<float>(m_Pos.x);
    const float y0 = static_cast<float>(m_Pos.y);

    std::vector<GuildInfoTextEntry> texts;
    std::vector<GuildInfoBoxEntry> boxes;
    std::vector<GuildInfoMarkEntry> marks;
    std::vector<GuildInfoButtonEntry> buttons;

    // RenderText(x, y, text, width, 0, sort): positions in window coordinates; `width` 0 = no box.
    auto addText = [&](const wchar_t* text, float x, float y, float width, int align, DWORD color, bool bold = false)
    {
        if (text == nullptr || text[0] == L'\0')
            return;
        g_pRenderText->SetFont(bold ? g_hFontBold : g_hFont);
        const int measured = g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text))).cx;
        const auto role = bold ? UI::Scaling::FontRole::Bold : UI::Scaling::FontRole::Normal;
        const float px =
            width > 0.f ? UI::Scaling::NativeTextPixelSizeInBox(role, transform, static_cast<float>(measured), width)
                        : UI::Scaling::NativeTextPixelSize(role, transform);
        texts.push_back(
            {StringUtils::WideToNarrow(text), x - x0, y - y0, width, px, align, bold, UI::RmlBridge::RgbaToCss(color)});
    };
    auto addButton = [&](int index, const wchar_t* label, float left, float top)
    { buttons.push_back({StringUtils::WideToNarrow(label), index, left, top}); };

    const bool noGuild = Hero->GuildStatus == G_NONE;
    std::vector<Rml::String> markCells;
    bool unionShown = false;
    float scrollTop = 0.f;
    wchar_t Text[300] = {};

    if (noGuild)
    {
        // The original's RenderNoneGuild().
        addText(I18N::Game::Guild, x0, y0 + 15, 190, 1, white, true);
        addText(I18N::Game::TypeGuildInFrontOf, x0 + 25, y0 + 46, 0, 0, white);
        addText(I18N::Game::TheGuildMasterYouWantToJoin, x0 + 25, y0 + 61, 0, 0, white);
        addText(I18N::Game::AndYouCanJoinTheGuild, x0 + 25, y0 + 76, 0, 0, white);
    }
    else
    {
        // The original's Render_Text() and each tab's Render_*().
        addText(I18N::Game::Guild, x0, y0 + 12, 190, 1, white);
        mu_swprintf(Text, L"%ls ( Score:%d )", GuildMark[Hero->GuildMarkIndex].GuildName, GuildTotalScore);
        addText(Text, x0 + 35, y0 + 48, 120, 1, RGBA(200, 255, 100, 255));
        const int tabWidth = GuildConstants::UILayout::TAB_WIDTH;
        addText(I18N::Game::Guild, x0 + 13 + static_cast<int>(GuildConstants::GuildTab::INFO) * tabWidth, y0 + 76,
                tabWidth, 1, white);
        addText(I18N::Game::Members, x0 + 13 + static_cast<int>(GuildConstants::GuildTab::MEMBERS) * tabWidth, y0 + 76,
                tabWidth, 1, white);
        addText(I18N::Game::Alliance, x0 + 13 + static_cast<int>(GuildConstants::GuildTab::UNION) * tabWidth, y0 + 76,
                tabWidth, 1, white);

        if (m_nCurrentTab == static_cast<int>(GuildConstants::GuildTab::INFO))
        {
            markCells = MarkCells(GuildMark[Hero->GuildMarkIndex].Mark);
            scrollTop = 262.f - y0;
            addButton(BUTTON_GUILD_OUT, Hero->GuildStatus == G_MASTER ? I18N::Game::Disband : I18N::Game::Leave, 100,
                      350);

            addText(I18N::Game::GuildAnnouncement, x0 + 22, y0 + 249, 40, 1, _ARGB(255, 255, 185, 1));

            m_GuildNotice.SetSize(GuildConstants::UILayout::NOTICE_BOX_WIDTH,
                                  GuildConstants::UILayout::NOTICE_BOX_HEIGHT);
            m_GuildNotice.SetPosition(m_Pos.x + 15, m_Pos.y + 264 + m_GuildNotice.GetHeight());
            const float listX = static_cast<float>(m_GuildNotice.GetPosition_x());
            const float lineWidth = static_cast<float>(m_GuildNotice.GetWidth() - 13 + 1);
            m_GuildNotice.ForEachRenderLine(
                [&](int line, const GUILDLOG_TEXT& item, bool selected)
                {
                    const float y = static_cast<float>(m_GuildNotice.GetRenderLinePos_y(line));
                    if (selected)
                        boxes.push_back({listX - x0, y - 3 - y0, lineWidth, 13});
                    addText(item.m_szContent, listX + 4, y, 0, 0,
                            selected ? RGBA(0, 0, 0, 255) : RGBA(230, 220, 200, 255));
                });

            int Nm_Loc = m_Pos.y + 169;
            mu_swprintf(Text, L"%ls :", I18N::Game::GuildCreationDate);
            addText(Text, x0 + 22, static_cast<float>(Nm_Loc), 40, 0, white);
            Nm_Loc += 13;
            mu_swprintf(Text, I18N::Game::GuildScoreD, GuildTotalScore);
            addText(Text, x0 + 22, static_cast<float>(Nm_Loc), 80, 0, white);
            Nm_Loc += 13;
            if (Hero->GuildStatus == G_MASTER)
            {
                const int Class = gCharacterManager.GetBaseClass(CharacterAttribute->Class);
                if (Class == CLASS_DARK_LORD)
                {
                    int nCount = CharacterAttribute->Level / 10 + CharacterAttribute->Charisma / 10;
                    if (nCount > 80)
                        nCount = 80;
                    mu_swprintf(Text, I18N::Game::GuildMembersDD, g_nGuildMemberCount, nCount);
                }
                else
                {
                    mu_swprintf(Text, I18N::Game::GuildMembersDD, g_nGuildMemberCount, CharacterAttribute->Level / 10);
                }
            }
            else
            {
                mu_swprintf(Text, I18N::Game::GuildMemberD, g_nGuildMemberCount);
            }
            addText(Text, x0 + 22, static_cast<float>(Nm_Loc), 80, 0, white);
            Nm_Loc += 13;
            mu_swprintf(Text, L"%ls : %ls", I18N::Game::HostilityGuild,
                        m_RivalGuildName[0] ? m_RivalGuildName : I18N::Game::None);
            addText(Text, x0 + 22, static_cast<float>(Nm_Loc), 0, 0, white);
        }
        else if (m_nCurrentTab == static_cast<int>(GuildConstants::GuildTab::MEMBERS))
        {
            scrollTop = 125.f - y0;
            addText(I18N::Game::Name, x0 + 24, y0 + 112, 40, 0, white);
            addText(I18N::Game::Position, x0 + 89, y0 + 112, 40, 0, white);
            addText(I18N::Game::Server, x0 + 126, y0 + 112, 40, 0, white);

            m_GuildMember.SetSize(GuildConstants::UILayout::MEMBER_BOX_WIDTH,
                                  GuildConstants::UILayout::MEMBER_BOX_HEIGHT);
            m_GuildMember.SetPosition(m_Pos.x + 13, m_Pos.y + 123 + m_GuildMember.GetHeight());
            const float listX = static_cast<float>(m_GuildMember.GetPosition_x());
            const float listWidth = static_cast<float>(m_GuildMember.GetWidth());
            const float lineWidth = listWidth - 13 + 1;
            // CUINewGuildMemberListBox::RenderDataLine(): the master, assistant and battle master
            // lines and the selected line on a box (its colour: guild_info.rcss .line-box), the
            // selected line's text black; the role centred on 70 units, the server number in (255, 196, 0).
            m_GuildMember.ForEachRenderLine(
                [&](int line, const GUILDLIST_TEXT& item, bool selected)
                {
                    const float y = static_cast<float>(m_GuildMember.GetRenderLinePos_y(line));
                    const wchar_t* role = nullptr;
                    if (item.m_GuildStatus == G_MASTER)
                        role = I18N::Game::Master;
                    else if (item.m_GuildStatus == G_SUB_MASTER)
                        role = I18N::Game::AssistM;
                    else if (item.m_GuildStatus == G_BATTLE_MASTER)
                        role = I18N::Game::BattleM;
                    if (role != nullptr || selected)
                        boxes.push_back({listX - x0, y - 3 - y0, lineWidth, 13});
                    const DWORD color = selected ? RGBA(0, 0, 0, 255) : RGBA(230, 220, 200, 255);
                    addText(item.m_szID, listX + 8, y, 0, 0, color);
                    if (role != nullptr)
                        addText(role, listX + 8 + 45, y, 70, 1, color);
                    if (item.m_Server != 255)
                    {
                        wchar_t server[16] = {};
                        mu_swprintf(server, L"%d", item.m_Server + 1);
                        addText(server, listX + listWidth - 30, y, 0, 0, RGBA(255, 196, 0, 255));
                    }
                });

            if (Hero->GuildStatus == G_MASTER)
            {
                addButton(BUTTON_GET_POSITION, I18N::Game::Position, 3, 360);
                addButton(BUTTON_FREE_POSITION, I18N::Game::Dissolve, 64, 360);
                addButton(BUTTON_GET_OUT, I18N::Game::Release, 125, 360);
            }
        }
        else if (m_nCurrentTab == static_cast<int>(GuildConstants::GuildTab::UNION))
        {
            unionShown = GuildMark[Hero->GuildMarkIndex].UnionName[0] != 0;
            if (unionShown)
            {
                addText(I18N::Game::NAME, x0 + 34, y0 + 115, 40, 0, white);
                addText(I18N::Game::Members, x0 + 140, y0 + 115, 40, 0, white);

                m_UnionListBox.SetSize(GuildConstants::UILayout::UNION_BOX_WIDTH,
                                       GuildConstants::UILayout::UNION_BOX_HEIGHT);
                m_UnionListBox.SetPosition(m_Pos.x + 15, m_Pos.y + 210);
                const float listX = static_cast<float>(m_UnionListBox.GetPosition_x());
                const float lineWidth = static_cast<float>(m_UnionListBox.GetWidth() - 13 + 1);
                m_UnionListBox.ForEachRenderLine(
                    [&](int line, const UNIONGUILD_TEXT& item, bool selected)
                    {
                        const float y = static_cast<float>(m_UnionListBox.GetRenderLinePos_y(line));
                        if (selected)
                            boxes.push_back({listX - x0, y - 3 - y0, lineWidth, 13});
                        marks.push_back({listX + 4 - x0, y - y0, MarkCells(item.GuildMark)});
                        const DWORD color = selected ? RGBA(0, 0, 0, 255) : RGBA(230, 220, 220, 255);
                        addText(item.szName, listX + 4 + 12, y, 0, 0, color);
                        wchar_t count[16] = {};
                        mu_swprintf(count, L"%d", item.nMemberCount);
                        // RT3_WRITE_RIGHT_TO_LEFT: the text ends at x.
                        addText(count, listX + 4 + 138 - 60, y, 60, 2, color);
                    });

                addButton(BUTTON_UNION_CREATE, I18N::Game::DisbandAlliance, 30, 230);
                addButton(BUTTON_UNION_OUT, I18N::Game::DisbandGuildAlliance, 100, 230);
            }
            else
            {
                // Render_Guild_Info()'s explanation.
                float y = y0 + 106;
                const float x = x0 + 25;
                const std::pair<const wchar_t*, int> lines[] = {
                    {I18N::Game::ToMakeTheAlliance, 15},
                    {I18N::Game::FaceTheGuildMaster, 15},
                    {I18N::Game::OfDesiredGuildForGuildAlliance, 15},
                    {I18N::Game::EnterAllianceOrGuildAlliance, 15},
                    {I18N::Game::ButtonInCommandWindow, 25},
                    {I18N::Game::IfTheOppositeIsNotAGuild, 15},
                    {I18N::Game::AllianceOppositeAllianceShould, 15},
                    {I18N::Game::BeTheMainAllianceForCreating, 20},
                    {I18N::Game::GuildAllianceRequestThe, 15},
                    {I18N::Game::RegistrationToOppositeAlliance, 15},
                    {I18N::Game::IfTheOppositeIsGuildAlliance, 0},
                };
                for (const auto& [text, advance] : lines)
                {
                    addText(text, x, y, 0, 0, white);
                    y += static_cast<float>(advance);
                }
            }
        }
    }

    GuildInfoRmlModel& model = m_RmlBinder.GetModel();
    SyncField(m_RmlBinder, &GuildInfoRmlModel::noGuild, "no_guild", noGuild);
    SyncField(m_RmlBinder, &GuildInfoRmlModel::tab, "tab", m_nCurrentTab);
    SyncField(m_RmlBinder, &GuildInfoRmlModel::unionShown, "union_shown", unionShown);
    SyncField(m_RmlBinder, &GuildInfoRmlModel::scrollTop, "scroll_top", scrollTop);
    SyncField(m_RmlBinder, &GuildInfoRmlModel::thumbTop, "thumb_top", scrollTop + static_cast<float>(m_Loc));
    SyncField(m_RmlBinder, &GuildInfoRmlModel::markCells, "mark_cells", std::move(markCells));
    SyncField(m_RmlBinder, &GuildInfoRmlModel::exitTooltip, "exit_tooltip",
              StringUtils::WideToNarrow(I18N::Game::Close388));
    SyncList(model.texts, std::move(texts), SameText, m_RmlBinder, "texts");
    SyncList(model.boxes, std::move(boxes), SameBox, m_RmlBinder, "boxes");
    SyncList(model.marks, std::move(marks), SameMark, m_RmlBinder, "marks");
    SyncList(model.buttons, std::move(buttons), SameButton, m_RmlBinder, "buttons");

    const int lineHeight = CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal);
    const int labelTop = GuildConstants::UILayout::BUTTON_HEIGHT / 2 - lineHeight / 2;
    SyncField(m_RmlBinder, &GuildInfoRmlModel::labelTop, "label_top", static_cast<float>(labelTop));
    SyncField(m_RmlBinder, &GuildInfoRmlModel::labelLinePx, "label_line_px",
              static_cast<float>(lineHeight) * transform.scaleY);
}

float mu::ui::window::CGuildInfoWindow::GetLayerDepth()
{
    return 4.5f;
}

void mu::ui::window::CGuildInfoWindow::AddGuildNotice(wchar_t* szText)
{
    wchar_t szTemp[GuildConstants::UILayout::TEXT_MAX_LINES][MAX_TEXT_LENGTH + 1] = { {0}, {0}, {0}, {0}, {0} };
    CutText3(szText, szTemp[0], GuildConstants::UILayout::TEXT_SPLIT_WIDTH, GuildConstants::UILayout::TEXT_MAX_LINES, MAX_TEXT_LENGTH + 1);

    for (int i = 0; i < GuildConstants::UILayout::TEXT_MAX_LINES; ++i)
    {
        if (szTemp[i][0])
        {
            m_GuildNotice.AddText(szTemp[i]);
            m_Tot_Notice++;
        }
    }
    m_GuildNotice.Scrolling(m_GuildNotice.GetLineNum() - m_GuildNotice.GetBoxSize());
}

void mu::ui::window::CGuildInfoWindow::AddGuildMember(GUILD_LIST_t* pInfo)
{
    m_GuildMember.AddText(pInfo->Name, pInfo->Number, pInfo->Server, pInfo->GuildStatus);
    m_GuildMember.Scrolling(-m_GuildMember.GetBoxSize());
}

void mu::ui::window::CGuildInfoWindow::GuildClear()
{
    m_GuildMember.Clear();
}

void mu::ui::window::CGuildInfoWindow::UnionGuildClear()
{
    m_UnionListBox.Clear();
}

void mu::ui::window::CGuildInfoWindow::NoticeClear()
{
    m_GuildNotice.Clear();
}

void mu::ui::window::CGuildInfoWindow::SetRivalGuildName(wchar_t* szName)
{
    wcsncpy(m_RivalGuildName, szName, MAX_GUILDNAME);
    m_RivalGuildName[MAX_GUILDNAME] = 0;
}

void mu::ui::window::CGuildInfoWindow::AddUnionList(BYTE* pGuildMark, wchar_t* szGuildName, int nMemberCount)
{
    m_UnionListBox.AddText(pGuildMark, szGuildName, nMemberCount);
    m_bRequestUnionList = false;
}

int mu::ui::window::CGuildInfoWindow::GetUnionCount()
{
    return m_UnionListBox.GetTextCount();
}

void mu::ui::window::CGuildInfoWindow::ReceiveGuildRelationShip(GuildRelationshipType byRelationShipType, GuildRequestType byRequestType,
    BYTE  byTargetUserIndexH, BYTE byTargetUserIndexL)
{
    if (mu::ui::window::g_pGenericConfirmDialog->IsVisible())
    {
        SocketClient->ToGameServer()->SendGuildRelationshipChangeResponse(
            byRelationShipType,
            byRequestType,
            0x00,
            MAKEWORD(byTargetUserIndexH, byTargetUserIndexL));
    }
    else
    {
        m_MessageInfo.s_byRelationShipType = byRelationShipType;
        m_MessageInfo.s_byRelationShipRequestType = byRequestType;
        m_MessageInfo.s_byTargetUserIndexH = byTargetUserIndexH;
        m_MessageInfo.s_byTargetUserIndexL = byTargetUserIndexL;

        int nCharKey = MAKEWORD(m_MessageInfo.s_byTargetUserIndexL, m_MessageInfo.s_byTargetUserIndexH);
        int nIndex = FindCharacterIndex(nCharKey);
        if (nIndex < 0 || nIndex >= MAX_CHARACTERS_CLIENT)
            return;
        CHARACTER* pPlayer = &CharactersClient[nIndex];

        wchar_t szText[3][64];
        ZeroMemory(szText, sizeof(szText));

        if (m_MessageInfo.s_byRelationShipType == GuildRelationshipType::Alliance)
        {
            if (m_MessageInfo.s_byRelationShipRequestType == GuildRequestType::Join)
            {
                mu_swprintf(szText[0], I18N::Game::FromSForAGuildAlliance, pPlayer->ID);
                mu_swprintf(szText[1], I18N::Game::ReceivedARegistrationRequest);
                mu_swprintf(szText[2], I18N::Game::Approve);
            }
            else										// Break Off
            {
                mu_swprintf(szText[0], I18N::Game::FromSForAGuildAlliance, pPlayer->ID);
                mu_swprintf(szText[1], I18N::Game::ReceivedAWithdrawalRequest);
                mu_swprintf(szText[2], I18N::Game::Approve);
            }
        }
        else if (m_MessageInfo.s_byRelationShipType == GuildRelationshipType::Hostility)
        {
            if (m_MessageInfo.s_byRelationShipRequestType == GuildRequestType::Join)
            {
                mu_swprintf(szText[0], I18N::Game::FromSForAHostileGuild, pPlayer->ID);
                mu_swprintf(szText[1], I18N::Game::ReceivedApprovalRequest);
                mu_swprintf(szText[2], I18N::Game::Approve);
            }
            else
            {
                mu_swprintf(szText[0], I18N::Game::FromSForAHostileGuild, pPlayer->ID);
                mu_swprintf(szText[1], I18N::Game::ReceivedCancellationRequest);
                mu_swprintf(szText[2], I18N::Game::Approve);
            }
        }

        mu::ui::window::GenericDialogConfig cfg;
        cfg.showCancel = true;
        cfg.lines = {
            { szText[0], false },
            { szText[1], false },
            { szText[2], false },
        };
        cfg.onPrimary = [byRelationShipType, byRequestType, byTargetUserIndexH, byTargetUserIndexL]
        {
            SocketClient->ToGameServer()->SendGuildRelationshipChangeResponse(
                byRelationShipType, byRequestType, 0x01,
                MAKEWORD(byTargetUserIndexH, byTargetUserIndexL));
        };
        cfg.onSecondary = [byRelationShipType, byRequestType, byTargetUserIndexH, byTargetUserIndexL]
        {
            SocketClient->ToGameServer()->SendGuildRelationshipChangeResponse(
                byRelationShipType, byRequestType, 0x00,
                MAKEWORD(byTargetUserIndexH, byTargetUserIndexL));
        };
        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
    }
}
