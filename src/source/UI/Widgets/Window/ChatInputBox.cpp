#include "stdafx.h"
#include "UI/Chat/ChatInput.h"
#include "UI/Widgets/Window/ChatInputBox.h"
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"
#include "UI/HUD/ChatLogWindow.h"
#include "UI/Widgets/UIControls.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "Engine/Object/ZzzOpenData.h"
#include "World/MapInfra/MapManager.h"
#include "Engine/Object/ZzzInterface.h"

#ifdef _EDITOR
#include "imgui.h"
#include "../MuEditor/Core/MuEditorCore.h"
#endif

// RmlUi migration -- see ChatInputRmlModel (ChatInputBox.h).
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "Core/Utilities/StringUtils.h"
#include "Core/Input/ImeInput.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Elements/ElementFormControlInput.h>

using namespace SEASON3B;
using namespace mu::ui::window;

mu::ui::window::CChatInputBox::CChatInputBox()
{
    Init();
}

mu::ui::window::CChatInputBox::~CChatInputBox()
{
    Release();
}

void mu::ui::window::CChatInputBox::Init()
{
    m_pNewUIMng = nullptr;
    m_pNewUIChatLogWnd = nullptr;
    m_pNewUISystemLogWnd = nullptr;
    m_WndPos = {};
    m_WndSize = {};
    m_WndPos.x = m_WndPos.y = 0;
    m_WndSize.cx = m_WndSize.cy = 0;
    m_iCurChatHistory = 0;
    m_iCurWhisperIDHistory = 0;

    m_iTooltipType = INPUT_TOOLTIP_NOTHING;
    m_iInputMsgType = INPUT_CHAT_MESSAGE;
    m_bBlockWhisper = false;
    m_bShowSystemMessages = true;
    m_bShowChatLog = true;
    m_bWhisperSend = true;

    m_bShowMessageElseNormal = false;
}

bool mu::ui::window::CChatInputBox::Create(
    CManager* pNewUIMng,
    CChatLogWindow* pNewUIChatLogWnd,
    CSystemLogWindow* pNewUISystemLogWnd,
    int x,
    int y)
{
    Release();

    if (nullptr == pNewUIChatLogWnd || nullptr == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_CHATINPUTBOX, this);

    m_pNewUIChatLogWnd = pNewUIChatLogWnd;
    m_pNewUISystemLogWnd = pNewUISystemLogWnd;
    SetWndPos(x, y);

    // Both text fields, their tab pairing, their colours/limits and every button's hit box now
    // come from chat_input.rml/.rcss -- nothing to construct here.
    SetInputMsgType(INPUT_CHAT_MESSAGE);

    Show(false);

    return true;
}

void mu::ui::window::CChatInputBox::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
    }

    Init();
}

void mu::ui::window::CChatInputBox::SetWndPos(int x, int y)
{
    // Still tracked for the native hit test in UpdateMouseEvent(); the fields position themselves
    // from RCSS now.
    m_WndPos.x = x; m_WndPos.y = y;
    m_WndSize.cx = CHATBOX_WIDTH; m_WndSize.cy = CHATBOX_HEIGHT;
}

void mu::ui::window::CChatInputBox::SetInputMsgType(int iInputMsgType)
{
    m_iInputMsgType = iInputMsgType;
}

int mu::ui::window::CChatInputBox::GetInputMsgType() const
{
    return m_iInputMsgType;
}

bool mu::ui::window::CChatInputBox::HaveFocus()
{
    return IsFieldFocused("chat_field") || IsFieldFocused("whisper_field");
}

void mu::ui::window::CChatInputBox::AddChatHistory(const type_string& strText)
{
    auto vi = std::find(m_vecChatHistory.begin(), m_vecChatHistory.end(), strText);
    if (vi != m_vecChatHistory.end())
        m_vecChatHistory.erase(vi);
    else if (m_vecChatHistory.size() > 12)
        m_vecChatHistory.erase(m_vecChatHistory.begin());
    m_vecChatHistory.push_back(strText);
}

void mu::ui::window::CChatInputBox::RemoveChatHistory(int index)
{
    if (index >= 0 && index < (int)m_vecChatHistory.size())
        m_vecChatHistory.erase(m_vecChatHistory.begin() + index);
}

void mu::ui::window::CChatInputBox::RemoveAllChatHIstory()
{
    m_vecChatHistory.clear();
}

void mu::ui::window::CChatInputBox::AddWhsprIDHistory(const type_string& strWhsprID)
{
    auto vi = std::find(m_vecWhsprIDHistory.begin(), m_vecWhsprIDHistory.end(), strWhsprID);
    if (vi != m_vecWhsprIDHistory.end())
        m_vecWhsprIDHistory.erase(vi);
    else if (m_vecWhsprIDHistory.size() > 5)
        m_vecWhsprIDHistory.erase(m_vecWhsprIDHistory.begin());
    m_vecWhsprIDHistory.push_back(strWhsprID);
}

void mu::ui::window::CChatInputBox::RemoveWhsprIDHistory(int index)
{
    if (index >= 0 && index < (int)m_vecWhsprIDHistory.size())
    {
        m_vecWhsprIDHistory.erase(m_vecWhsprIDHistory.begin() + index);
    }
}

void mu::ui::window::CChatInputBox::RemoveAllWhsprIDHIstory()
{
    m_vecWhsprIDHistory.clear();
}

bool mu::ui::window::CChatInputBox::IsBlockWhisper()
{
    return m_bBlockWhisper;
}

void mu::ui::window::CChatInputBox::SetBlockWhisper(bool bBlockWhisper)
{
    m_bBlockWhisper = bBlockWhisper;
}

bool mu::ui::window::CChatInputBox::UpdateMouseEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_CHATINPUTBOX) == false)
    {
        return true;
    }

    // Every button hit test, the hover tooltip and the whole button row moved to chat_input.rml's
    // own elements (data-event-click / data-event-mouseover). What is left is the two things RmlUi
    // has no part in: retargeting a whisper from a right-click elsewhere on screen, and claiming
    // the bar's own rectangle so a click on it does not fall through to the world.
    UpdateWhisperTargetFromRightClick();

    return !mu::ui::window::WindowGeometry(m_WndPos.x, m_WndPos.y, m_WndSize.cx, m_WndSize.cy).Contains(MouseX, MouseY);
}

bool mu::ui::window::CChatInputBox::UpdateKeyEvent()
{
    if (mu::ui::window::IsPress(VK_F2))
    {
        m_bShowMessageElseNormal = !m_bShowMessageElseNormal;

        if (m_bShowMessageElseNormal)
        {
            m_pNewUIChatLogWnd->ChangeMessage(mu::ui::window::TYPE_WHISPER_MESSAGE);
        }
        else
        {
            m_pNewUIChatLogWnd->ChangeMessage(mu::ui::window::TYPE_ALL_MESSAGE);
        }

        PlayBuffer(SOUND_CLICK01);
        return false;
    }

    if (mu::ui::window::IsPress(VK_F3))
    {
        if (m_bWhisperSend == false)
        {
            m_bWhisperSend = true;
        }
        else
        {
            m_bWhisperSend = false;
            // RmlUi disables the recipient field through its binding. Keep typing in the
            // message field when whispering is switched off.
            if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_CHATINPUTBOX))
            {
                FocusField("chat_field");
            }
        }

        return false;
    }

    if (m_pNewUIChatLogWnd->IsShowFrame())
    {
        if (mu::ui::window::IsPress(VK_F4))
        {
            m_pNewUIChatLogWnd->SetSizeAuto();
            m_pNewUIChatLogWnd->UpdateWndSize();
            m_pNewUIChatLogWnd->UpdateScrollPos();
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    if (mu::ui::window::IsPress(VK_F5))
    {
        if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_CHATINPUTBOX))
        {
            if (m_pNewUIChatLogWnd->IsShowFrame())
            {
                m_pNewUIChatLogWnd->HideFrame();
            }
            else
            {
                m_pNewUIChatLogWnd->ShowFrame();
            }
        }

        PlayBuffer(SOUND_CLICK01);
        return false;
    }

    if (m_pNewUIChatLogWnd->IsShowFrame())
    {
        if (IsPress(VK_PRIOR))
        {
            m_pNewUIChatLogWnd->Scrolling(m_pNewUIChatLogWnd->GetCurrentRenderEndLine() - m_pNewUIChatLogWnd->GetNumberOfShowingLines());
            return false;
        }
        if (IsPress(VK_NEXT))
        {
            m_pNewUIChatLogWnd->Scrolling(m_pNewUIChatLogWnd->GetCurrentRenderEndLine() + m_pNewUIChatLogWnd->GetNumberOfShowingLines());
            return false;
        }
    }

    if (false == IsVisible() && mu::ui::window::IsPress(VK_RETURN))
    {
#ifdef _EDITOR
        // Don't open chat if editor has keyboard focus
        if (g_MuEditorCore.IsEnabled())
        {
            ImGuiIO& io = ImGui::GetIO();
            if (io.WantCaptureKeyboard || io.WantCaptureMouse)
            {
                return false;
            }
        }
#endif // _EDITOR

        if (gMapManager.InChaosCastle() == true && g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_CHAOSCASTLE_TIME) == false)
        {
            g_pNewUISystem->Show(mu::ui::window::INTERFACE_CHATINPUTBOX);
        }
        else if (gMapManager.InChaosCastle() == false)
        {
            g_pNewUISystem->Show(mu::ui::window::INTERFACE_CHATINPUTBOX);
        }

        RestoreIMEStatus();
        return false;
    }

    const uint64_t currentTickCount = GetTickCount64();
    if (IsVisible() && HaveFocus() && mu::ui::window::IsPress(VK_RETURN)
        && m_lastChatTime < currentTickCount - ChatCooldownMs)
    {
        m_lastChatTime = currentTickCount;
        wchar_t	szChatText[MAX_CHAT_SIZE + 1] = { '\0' };
        wchar_t	szWhisperID[MAX_USERNAME_SIZE + 1] = { '\0' };

        {
            type_string strChat, strWhisper;
            GetChatText(strChat);
            GetWhsprID(strWhisper);
            wcsncpy(szChatText, strChat.c_str(), MAX_CHAT_SIZE);
            szChatText[MAX_CHAT_SIZE] = L'\0';
            wcsncpy(szWhisperID, strWhisper.c_str(), MAX_USERNAME_SIZE);
            szWhisperID[MAX_USERNAME_SIZE] = L'\0';
        }

        //for (int i = 0; i < MAX_CHAT_SIZE; i++)
        //    szReceivedChat[i] = g_pMultiLanguage->ConvertFulltoHalfWidthChar(szReceivedChat[i]);

        std::wstring wstrText = L"";

        if (szChatText[0] != 0x002F)
        {
            switch (m_iInputMsgType) {
            case INPUT_PARTY_MESSAGE:
                wstrText = L"~";
                break;
            case INPUT_GUILD_MESSAGE:
                wstrText = L"@";
                break;
            case INPUT_GENS_MESSAGE:
                wstrText = L"$";
                break;
            default:
                break;
            }
        }
        wstrText.append(szChatText);

        if (wcslen(szChatText) != 0)
        {
            if (!CheckCommand(szChatText))
            {
                {
                    //if (CheckAbuseFilter(szChatText))
                    //{
                    //    wstrText = I18N::Game::PwnedByTheFilter;
                    //}

                    if (m_bWhisperSend && wcslen(szChatText) && wcslen(szWhisperID) > 0)
                    {
                        SocketClient->ToGameServer()->SendWhisperMessage(MU_C16(szWhisperID), MU_C16(wstrText.c_str()));
                        g_pChatListBox->AddText(Hero->ID, szChatText, mu::ui::window::TYPE_WHISPER_MESSAGE);
                        AddWhsprIDHistory(szWhisperID);
                    }
                    else if (wcsncmp(szChatText, I18N::Game::Warp, wcslen(I18N::Game::Warp)) == 0)
                    {
                        wchar_t* pszMapName = szChatText + wcslen(I18N::Game::Warp) + 1;
                        int iMapIndex = g_pMoveCommandWindow->GetMapIndexFromMovereq(pszMapName);

                        if (g_pMoveCommandWindow->IsTheMapInDifferentServer(gMapManager.WorldActive, iMapIndex))
                        {
                            SaveOptions();
                        }

                        SocketClient->ToGameServer()->SendWarpCommandRequest(g_pMoveCommandWindow->GetMoveCommandKey(), iMapIndex);
                    }
                    else
                    {
                        if (Hero->SafeZone || (Hero->Helper.Type != MODEL_HORN_OF_UNIRIA && Hero->Helper.Type != MODEL_HORN_OF_DINORANT && Hero->Helper.Type != MODEL_DARK_HORSE_ITEM && Hero->Helper.Type != MODEL_HORN_OF_FENRIR))
                        {
                            UI::Chat::CheckChatText(szChatText);
                        }

                        SocketClient->ToGameServer()->SendPublicChatMessage(MU_C16(Hero->ID), MU_C16(wstrText.c_str()));
                        AddChatHistory(wstrText);
                    }
                }
            }
        }
        SetFieldText("chat_field", L"");
        m_iCurChatHistory = m_iCurWhisperIDHistory = 0;

        SaveIMEStatus();

        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_CHATINPUTBOX);
        return false;
    }
    if (IsVisible() && IsFieldFocused("chat_field"))
    {
        if (mu::ui::window::IsPress(VK_UP) && false == m_vecChatHistory.empty())
        {
            m_iCurChatHistory--;
            if (m_iCurChatHistory < 0)
                m_iCurChatHistory = m_vecChatHistory.size() - 1;
            SetFieldText("chat_field", m_vecChatHistory[m_iCurChatHistory]);

            return false;
        }
        else if (mu::ui::window::IsPress(VK_DOWN) && false == m_vecChatHistory.empty())
        {
            m_iCurChatHistory++;

            if (m_iCurChatHistory >= (int)m_vecChatHistory.size())
                m_iCurChatHistory = 0;

            SetFieldText("chat_field", m_vecChatHistory[m_iCurChatHistory]);

            return false;
        }
    }

    if (IsVisible() && IsFieldFocused("whisper_field") && m_bWhisperSend)
    {
        if (mu::ui::window::IsPress(VK_UP) && false == m_vecWhsprIDHistory.empty())
        {
            m_iCurWhisperIDHistory--;
            if (m_iCurWhisperIDHistory < 0)
                m_iCurWhisperIDHistory = m_vecWhsprIDHistory.size() - 1;
            SetFieldText("whisper_field", m_vecWhsprIDHistory[m_iCurWhisperIDHistory]);

            return false;
        }
        else if (mu::ui::window::IsPress(VK_DOWN) && false == m_vecWhsprIDHistory.empty())
        {
            m_iCurWhisperIDHistory++;

            if (m_iCurWhisperIDHistory >= (int)m_vecWhsprIDHistory.size())
                m_iCurWhisperIDHistory = 0;

            SetFieldText("whisper_field", m_vecWhsprIDHistory[m_iCurWhisperIDHistory]);

            return false;
        }
    }
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_CHATINPUTBOX) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_CHATINPUTBOX);

            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }

    return true;
}

bool mu::ui::window::CChatInputBox::Update()
{
    BuildRmlUi();
    SyncRmlModel();

    // CManager::UpdateKeyEvent() only dispatches to a window whose GetRelatedWnd() matches the
    // currently-focused handle, and it reports a focused RmlUi <input> as RmlUiRuntime's own
    // address. Claiming that address here is what keeps THIS window receiving keys while the
    // player is typing -- exactly the role the focused CUITextInputBox's HWND used to play. Get it
    // wrong and Enter/Escape/history simply never arrive.
    HWND hRmlFocus = reinterpret_cast<HWND>(&RmlUiRuntime::Instance());
    if (HaveFocus())
    {
        if (GetRelatedWnd() != hRmlFocus)
            SetRelatedWnd(hRmlFocus);
    }
    else if (GetRelatedWnd() != g_hWnd)
    {
        SetRelatedWnd(g_hWnd);
    }

    return true;
}

bool mu::ui::window::CChatInputBox::Render()
{
    // Nothing native left: the bar art, the buttons, the tooltip and both fields are all RmlUi.
    // Kept because CObject requires the override.
    return true;
}

void mu::ui::window::CChatInputBox::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "chat_input",
        [this](Rml::DataModelConstructor& c, ChatInputRmlModel& model)
        {
            c.Bind("chat_text", &model.chatText);
            c.Bind("whisper_id", &model.whisperId);
            c.Bind("input_msg_type", &model.inputMsgType);
            c.Bind("block_whisper", &model.blockWhisper);
            c.Bind("show_system", &model.showSystem);
            c.Bind("show_chat_log", &model.showChatLog);
            c.Bind("show_frame", &model.showFrame);
            c.Bind("whisper_send", &model.whisperSend);
            c.Bind("tooltip_index", &model.tooltipIndex);
            c.Bind("tooltip_left", &model.tooltipLeft);
            c.Bind("tooltip_text", &model.tooltipText);

            c.BindEventCallback("chat_set_type",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                {
                    if (args.empty()) return;
                    SetInputMsgType(INPUT_CHAT_MESSAGE + args[0].Get<int>(0));
                    PlayBuffer(SOUND_CLICK01);
                });

            c.BindEventCallback("chat_toggle_whisper_block",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                {
                    m_bBlockWhisper = !m_bBlockWhisper;
                    PlayBuffer(SOUND_CLICK01);
                });

            c.BindEventCallback("chat_toggle_system",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                {
                    m_bShowSystemMessages = !m_bShowSystemMessages;
                    if (m_bShowSystemMessages)
                        m_pNewUISystemLogWnd->ShowMessages();
                    else
                        m_pNewUISystemLogWnd->HideMessages();
                    PlayBuffer(SOUND_CLICK01);
                });

            c.BindEventCallback("chat_toggle_chatlog",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                {
                    m_bShowChatLog = !m_bShowChatLog;
                    if (m_bShowChatLog)
                        m_pNewUIChatLogWnd->ShowChatLog();
                    else
                        m_pNewUIChatLogWnd->HideChatLog();
                    PlayBuffer(SOUND_CLICK01);
                });

            c.BindEventCallback("chat_toggle_frame",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                {
                    if (m_pNewUIChatLogWnd->IsShowFrame())
                        m_pNewUIChatLogWnd->HideFrame();
                    else
                        m_pNewUIChatLogWnd->ShowFrame();
                    PlayBuffer(SOUND_CLICK01);
                });

            c.BindEventCallback("chat_size_step",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                {
                    m_pNewUIChatLogWnd->SetSizeAuto();
                    m_pNewUIChatLogWnd->UpdateWndSize();
                    m_pNewUIChatLogWnd->UpdateScrollPos();
                    PlayBuffer(SOUND_CLICK01);
                });

            c.BindEventCallback("chat_alpha_step",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                {
                    m_pNewUIChatLogWnd->SetBackAlphaAuto();
                    PlayBuffer(SOUND_CLICK01);
                });

            c.BindEventCallback("chat_tooltip",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                {
                    m_iTooltipType = args.empty() ? INPUT_TOOLTIP_NOTHING : args[0].Get<int>(-1);
                });
        });

    if (modelCreated)
    {
        m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
                                                     "Data/Interface/RmlUi/chat_input.rml");
        UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });
    }
}

void mu::ui::window::CChatInputBox::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return;

    // Destroy() resets the model, so carry the unsent text across the rebuild.
    const Rml::String chatText = m_RmlBinder.GetModel().chatText;
    const Rml::String whisperId = m_RmlBinder.GetModel().whisperId;

    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
    m_RmlBinder.GetModel().chatText = chatText;
    m_RmlBinder.GetModel().whisperId = whisperId;
    m_RmlBinder.MarkDirty("chat_text");
    m_RmlBinder.MarkDirty("whisper_id");
}

void mu::ui::window::CChatInputBox::SyncRmlModel()
{
    if (!m_pRmlDoc) return;

    UI::RmlBridge::SyncDocumentVisibility(m_pRmlDoc, IsVisible());

    // Deliberately after the line above: the field can only take focus once its document is
    // showing, and SyncDocumentVisibility()'s Show() would blur it if the order were reversed.
    if (m_bFocusPending && m_pRmlDoc->IsVisible())
    {
        m_bFocusPending = false;
        FocusField("chat_field");
    }

    ChatInputRmlModel& model = m_RmlBinder.GetModel();

    auto syncBool = [&](bool ChatInputRmlModel::* field, const char* name, bool value)
    {
        if (model.*field != value) { model.*field = value; m_RmlBinder.MarkDirty(name); }
    };

    syncBool(&ChatInputRmlModel::blockWhisper, "block_whisper", m_bBlockWhisper);
    syncBool(&ChatInputRmlModel::showSystem, "show_system", m_bShowSystemMessages);
    syncBool(&ChatInputRmlModel::showChatLog, "show_chat_log", m_bShowChatLog);
    syncBool(&ChatInputRmlModel::whisperSend, "whisper_send", m_bWhisperSend);
    syncBool(&ChatInputRmlModel::showFrame, "show_frame",
             m_pNewUIChatLogWnd && m_pNewUIChatLogWnd->IsShowFrame());

    const int typeIndex = GetInputMsgType() - INPUT_CHAT_MESSAGE;
    if (model.inputMsgType != typeIndex)
    {
        model.inputMsgType = typeIndex;
        m_RmlBinder.MarkDirty("input_msg_type");
    }

    if (model.tooltipIndex != m_iTooltipType)
    {
        model.tooltipIndex = m_iTooltipType;
        m_RmlBinder.MarkDirty("tooltip_index");

        if (m_iTooltipType != INPUT_TOOLTIP_NOTHING)
        {
            // Native's own string table and x formula (RenderTooltip()), kept verbatim -- the
            // formula's own group stepping is /3, which does not match the button row's /4
            // grouping, but it is what ships, so it is reproduced rather than "corrected".
            static const int iTextIndex[10] = {
                1681, 1682, 1683, 3321,
                1684, 1685, 750, 1686, 751, 752 };
            if (m_iTooltipType >= 0 && m_iTooltipType < 10)
            {
                model.tooltipText = StringUtils::WideToNarrow(I18N::Game::Lookup(iTextIndex[m_iTooltipType]));
                m_RmlBinder.MarkDirty("tooltip_text");

                // Native then subtracted half the measured text width; the RCSS centres on this
                // point instead, so no measurement is needed here.
                model.tooltipLeft = (float)m_iTooltipType * BUTTON_WIDTH
                                  + (float)(m_iTooltipType / 3) * GROUP_SEPARATING_WIDTH + 10.0f;
                m_RmlBinder.MarkDirty("tooltip_left");
            }
        }
    }
}

Rml::Element* mu::ui::window::CChatInputBox::GetField(const char* id) const
{
    return m_pRmlDoc ? m_pRmlDoc->GetElementById(id) : nullptr;
}

bool mu::ui::window::CChatInputBox::IsFieldFocused(const char* id) const
{
    Rml::Element* field = GetField(id);
    return field != nullptr && field->IsPseudoClassSet("focus");
}

void mu::ui::window::CChatInputBox::SetFieldText(const char* id, const type_string& text)
{
    ChatInputRmlModel& model = m_RmlBinder.GetModel();
    const Rml::String narrow = StringUtils::WideToNarrow(text.c_str());

    if (strcmp(id, "chat_field") == 0)
    {
        if (model.chatText == narrow) return;
        model.chatText = narrow;
        m_RmlBinder.MarkDirty("chat_text");
    }
    else
    {
        if (model.whisperId == narrow) return;
        model.whisperId = narrow;
        m_RmlBinder.MarkDirty("whisper_id");
    }
}

void mu::ui::window::CChatInputBox::FocusField(const char* id)
{
    if (Rml::Element* field = GetField(id))
        field->Focus();
}

float mu::ui::window::CChatInputBox::GetLayerDepth()
{
    return 6.2f;
}

float mu::ui::window::CChatInputBox::GetKeyEventOrder()
{
    return 9.0f;
}

void mu::ui::window::CChatInputBox::OpenningProcess()
{
    // Only clears the field and arms the focus latch -- see m_bFocusPending. The document is not
    // visible yet at this point, so focusing here would be silently dropped.
    BuildRmlUi();
    SetFieldText("chat_field", L"");
    m_bFocusPending = true;
}

void mu::ui::window::CChatInputBox::ClosingProcess()
{
    m_pNewUIChatLogWnd->HideFrame();

    m_bFocusPending = false;
    if (Rml::Element* field = GetField("chat_field")) field->Blur();
    if (Rml::Element* field = GetField("whisper_field")) field->Blur();

    SetFocus(g_hWnd);
}

void mu::ui::window::CChatInputBox::GetChatText(type_string& strText)
{
    strText = StringUtils::NarrowToWide(m_RmlBinder.GetModel().chatText);
}

void mu::ui::window::CChatInputBox::GetWhsprID(type_string& strWhsprID)
{
    strWhsprID = StringUtils::NarrowToWide(m_RmlBinder.GetModel().whisperId);
}

void mu::ui::window::CChatInputBox::SetWhsprID(const wchar_t* strWhsprID)
{
    SetFieldText("whisper_field", strWhsprID ? strWhsprID : L"");
}

void mu::ui::window::CChatInputBox::UpdateWhisperTargetFromRightClick()
{
    if (SelectedCharacter < 0 || !mu::ui::window::IsRelease(VK_RBUTTON))
    {
        return;
    }

    auto const character = &CharactersClient[SelectedCharacter];
    if (character->Object.Kind != KIND_PLAYER)
    {
        return;
    }

    if (gMapManager.InChaosCastle())
    {
        return;
    }

    auto const blockedByGensRivalry = ::IsStrifeMap(gMapManager.WorldActive)
        && Hero->m_byGensInfluence != character->m_byGensInfluence;
    if (blockedByGensRivalry)
    {
        return;
    }

    SetWhsprID(character->ID);
}
