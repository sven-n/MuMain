// NewUIChatCommandWindow.cpp: implementation of the CNewUIChatCommandWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/NewUI/Dialogs/NewUIChatCommandWindow.h"
#include "UI/NewUI/NewUISystem.h"
#include "Audio/DSPlaySound.h"
#include "I18N/All.h"
#include <algorithm>

using namespace SEASON3B;
using GameLogic::Commands::ChatCommand;
using GameLogic::Commands::ChatCommandParameterType;
using GameLogic::Commands::Catalog;

SEASON3B::CNewUIChatCommandWindow::CNewUIChatCommandWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = 0;
    m_Pos.y = 0;
    m_selectedIndex = -1;
    m_scrollOffset = 0;
    m_pValueInput = NULL;
    m_editedParameter = -1;
}

SEASON3B::CNewUIChatCommandWindow::~CNewUIChatCommandWindow()
{
    Release();
}

bool SEASON3B::CNewUIChatCommandWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_COMMAND_LIST, this);

    LoadImages();

    m_pValueInput = new CUITextInputBox;
    m_pValueInput->Init(g_hWnd, 150, 14, 60);
    m_pValueInput->SetTextColor(255, 255, 230, 210);
    m_pValueInput->SetBackColor(128, 0, 0, 25);
    m_pValueInput->SetFont(g_hFont);
    m_pValueInput->SetState(UISTATE_HIDE);

    SetPos(x, y);
    Show(false);

    return true;
}

void SEASON3B::CNewUIChatCommandWindow::Release()
{
    UnloadImages();
    SAFE_DELETE(m_pValueInput);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUIChatCommandWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

float SEASON3B::CNewUIChatCommandWindow::GetLayerDepth()
{
    return 3.f;
}

float SEASON3B::CNewUIChatCommandWindow::GetKeyEventOrder()
{
    return 10.f;
}

void SEASON3B::CNewUIChatCommandWindow::OpenningProcess()
{
    // A player may have gained or lost commands, so start from the top.
    m_scrollOffset = 0;
    SelectCommand(Catalog().GetCommands().empty() ? -1 : 0);
}

void SEASON3B::CNewUIChatCommandWindow::ClosingProcess()
{
    StopEditing();
}

const ChatCommand* SEASON3B::CNewUIChatCommandWindow::GetSelectedCommand() const
{
    const auto& commands = Catalog().GetCommands();
    if (m_selectedIndex < 0 || static_cast<size_t>(m_selectedIndex) >= commands.size())
    {
        return NULL;
    }

    return &commands[m_selectedIndex];
}

void SEASON3B::CNewUIChatCommandWindow::SelectCommand(int index)
{
    StopEditing();
    m_selectedIndex = index;
    m_parameterValues.clear();

    if (const auto* command = GetSelectedCommand())
    {
        m_parameterValues.resize(command->Parameters.size());
    }
}

std::vector<std::wstring> SEASON3B::CNewUIChatCommandWindow::SplitValidValues(const std::wstring& validValues)
{
    std::vector<std::wstring> result;
    size_t start = 0;
    while (start <= validValues.size())
    {
        const auto separator = validValues.find(L'|', start);
        const auto end = (separator == std::wstring::npos) ? validValues.size() : separator;
        result.push_back(validValues.substr(start, end - start));
        if (separator == std::wstring::npos)
        {
            break;
        }

        start = separator + 1;
    }

    return result;
}

bool SEASON3B::CNewUIChatCommandWindow::HasFixedValues(const GameLogic::Commands::ChatCommandParameter& parameter)
{
    return !parameter.ValidValues.empty();
}

void SEASON3B::CNewUIChatCommandWindow::BeginEditingParameter(size_t parameterIndex)
{
    CommitEditedValue();

    const auto* command = GetSelectedCommand();
    if (command == NULL || parameterIndex >= command->Parameters.size() || m_pValueInput == NULL)
    {
        return;
    }

    m_editedParameter = static_cast<int>(parameterIndex);
    const auto rowY = GetDetailTop() + 52 + static_cast<int>(parameterIndex) * static_cast<int>(ROW_HEIGHT);
    m_pValueInput->SetPosition(m_Pos.x + 160, rowY);
    m_pValueInput->SetText(m_parameterValues[parameterIndex].c_str());
    m_pValueInput->SetState(UISTATE_NORMAL);
    m_pValueInput->GiveFocus();
}

void SEASON3B::CNewUIChatCommandWindow::CommitEditedValue()
{
    if (m_editedParameter < 0 || m_pValueInput == NULL
        || static_cast<size_t>(m_editedParameter) >= m_parameterValues.size())
    {
        return;
    }

    wchar_t text[64] = { 0 };
    m_pValueInput->GetText(text, 64);
    m_parameterValues[m_editedParameter] = text;
}

void SEASON3B::CNewUIChatCommandWindow::StopEditing()
{
    CommitEditedValue();
    m_editedParameter = -1;
    if (m_pValueInput != NULL)
    {
        m_pValueInput->SetState(UISTATE_HIDE);
    }
}

void SEASON3B::CNewUIChatCommandWindow::CycleParameterValue(size_t parameterIndex)
{
    const auto* command = GetSelectedCommand();
    if (command == NULL || parameterIndex >= command->Parameters.size())
    {
        return;
    }

    const auto& parameter = command->Parameters[parameterIndex];
    auto values = SplitValidValues(parameter.ValidValues);
    if (values.empty())
    {
        return;
    }

    const auto& current = m_parameterValues[parameterIndex];
    size_t next = 0;
    for (size_t i = 0; i < values.size(); ++i)
    {
        if (values[i] == current)
        {
            next = i + 1;
            break;
        }
    }

    // One step past the last value clears it again, which is how an optional
    // parameter is left out.
    m_parameterValues[parameterIndex] = (next >= values.size()) ? std::wstring() : values[next];
}

void SEASON3B::CNewUIChatCommandWindow::ExecuteSelectedCommand()
{
    CommitEditedValue();

    const auto* command = GetSelectedCommand();
    if (command == NULL)
    {
        return;
    }

    // A required parameter without a value would only produce an error message
    // from the server, so don't send anything.
    for (size_t i = 0; i < command->Parameters.size(); ++i)
    {
        if (command->Parameters[i].IsRequired && m_parameterValues[i].empty())
        {
            return;
        }
    }

    GameLogic::Commands::ChatCommandCatalog::Execute(
        GameLogic::Commands::ChatCommandCatalog::BuildCommandLine(*command, m_parameterValues));
    PlayBuffer(SOUND_CLICK01);
}

bool SEASON3B::CNewUIChatCommandWindow::UpdateMouseEvent()
{
    const auto& commands = Catalog().GetCommands();
    const auto listTop = GetListTop();

    for (int row = 0; row < VISIBLE_ROWS; ++row)
    {
        const auto index = m_scrollOffset + row;
        if (static_cast<size_t>(index) >= commands.size())
        {
            break;
        }

        if (CheckMouseIn(m_Pos.x + 25, listTop + row * static_cast<int>(ROW_HEIGHT), static_cast<int>(WINDOW_WIDTH - 2 * FRAME_SIDE - SCROLLBAR_WIDTH), static_cast<int>(ROW_HEIGHT))
            && IsRelease(VK_LBUTTON))
        {
            SelectCommand(index);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    if (const auto* command = GetSelectedCommand())
    {
        const auto detailTop = GetDetailTop() + 52;
        for (size_t i = 0; i < command->Parameters.size(); ++i)
        {
            const auto rowY = detailTop + static_cast<int>(i) * static_cast<int>(ROW_HEIGHT);
            if (CheckMouseIn(m_Pos.x + 10, rowY, static_cast<int>(WINDOW_WIDTH) - 20, static_cast<int>(ROW_HEIGHT))
                && IsRelease(VK_LBUTTON))
            {
                if (HasFixedValues(command->Parameters[i]))
                {
                    StopEditing();
                    CycleParameterValue(i);
                }
                else
                {
                    BeginEditingParameter(i);
                }

                PlayBuffer(SOUND_CLICK01);
                return false;
            }
        }

        const auto buttonY = m_Pos.y + static_cast<int>(WINDOW_HEIGHT - FRAME_BOTTOM) + 12;
        if (CheckMouseIn(m_Pos.x + static_cast<int>(WINDOW_WIDTH - FRAME_SIDE) - 85, buttonY, 80, 20) && IsRelease(VK_LBUTTON))
        {
            ExecuteSelectedCommand();
            return false;
        }
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y, static_cast<int>(WINDOW_WIDTH), static_cast<int>(WINDOW_HEIGHT)))
    {
        if (MouseWheel != 0)
        {
            const auto maxOffset = static_cast<int>(commands.size()) - VISIBLE_ROWS;
            m_scrollOffset -= MouseWheel / 120;
            m_scrollOffset = std::max(0, std::min(m_scrollOffset, std::max(0, maxOffset)));
            MouseWheel = 0;
        }

        return false;
    }

    return true;
}

bool SEASON3B::CNewUIChatCommandWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_COMMAND_LIST) == false)
    {
        return true;
    }

    if (IsPress(VK_ESCAPE) == true)
    {
        // The first escape leaves the field, the next one closes the window.
        if (m_editedParameter >= 0)
        {
            StopEditing();
        }
        else
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_COMMAND_LIST);
        }

        PlayBuffer(SOUND_CLICK01);
        return false;
    }

    if (m_pValueInput != NULL && m_pValueInput->HaveFocus())
    {
        // Everything else is typed into the field.
        return true;
    }

    const auto commandCount = static_cast<int>(Catalog().GetCommands().size());
    if (IsPress(VK_DOWN) && m_selectedIndex + 1 < commandCount)
    {
        SelectCommand(m_selectedIndex + 1);
        if (m_selectedIndex >= m_scrollOffset + VISIBLE_ROWS)
        {
            ++m_scrollOffset;
        }

        return false;
    }

    if (IsPress(VK_UP) && m_selectedIndex > 0)
    {
        SelectCommand(m_selectedIndex - 1);
        if (m_selectedIndex < m_scrollOffset)
        {
            --m_scrollOffset;
        }

        return false;
    }

    return true;
}

bool SEASON3B::CNewUIChatCommandWindow::Update()
{
    return true;
}

bool SEASON3B::CNewUIChatCommandWindow::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    RenderFrame();
    RenderCommandList();
    RenderSelectedCommand();

    if (m_editedParameter >= 0 && m_pValueInput != NULL)
    {
        m_pValueInput->Render();
    }

    DisableAlphaBlend();
    return true;
}

void SEASON3B::CNewUIChatCommandWindow::LoadImages()
{
    // The ids are shared with the other windows, but every window loads what it
    // draws - relying on another one having done it means an empty frame when
    // that window wasn't opened yet.
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_COMMAND_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_btn_empty_small.tga", IMAGE_COMMAND_BUTTON, GL_LINEAR);

    LoadBitmap(L"Interface\\newui_item_table01(L).tga", IMAGE_COMMAND_TOP_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_table01(R).tga", IMAGE_COMMAND_TOP_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_table02(L).tga", IMAGE_COMMAND_BOTTOM_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_table02(R).tga", IMAGE_COMMAND_BOTTOM_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_table03(Up).tga", IMAGE_COMMAND_TOP_PIXEL, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_table03(Dw).tga", IMAGE_COMMAND_BOTTOM_PIXEL, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_table03(L).tga", IMAGE_COMMAND_LEFT_PIXEL, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_table03(R).tga", IMAGE_COMMAND_RIGHT_PIXEL, GL_LINEAR);
}

void SEASON3B::CNewUIChatCommandWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_COMMAND_BACK);
    DeleteBitmap(IMAGE_COMMAND_BUTTON);
    DeleteBitmap(IMAGE_COMMAND_TOP_LEFT);
    DeleteBitmap(IMAGE_COMMAND_TOP_RIGHT);
    DeleteBitmap(IMAGE_COMMAND_BOTTOM_LEFT);
    DeleteBitmap(IMAGE_COMMAND_BOTTOM_RIGHT);
    DeleteBitmap(IMAGE_COMMAND_TOP_PIXEL);
    DeleteBitmap(IMAGE_COMMAND_BOTTOM_PIXEL);
    DeleteBitmap(IMAGE_COMMAND_LEFT_PIXEL);
    DeleteBitmap(IMAGE_COMMAND_RIGHT_PIXEL);
}

int SEASON3B::CNewUIChatCommandWindow::GetListTop() const
{
    return m_Pos.y + static_cast<int>(FRAME_TOP) - 20;
}

int SEASON3B::CNewUIChatCommandWindow::GetDetailTop() const
{
    return GetListTop() + VISIBLE_ROWS * static_cast<int>(ROW_HEIGHT) + 8;
}

void SEASON3B::CNewUIChatCommandWindow::RenderFrame()
{
    const auto x = static_cast<float>(m_Pos.x);
    const auto y = static_cast<float>(m_Pos.y);

    RenderImage(IMAGE_COMMAND_BACK, x, y, WINDOW_WIDTH, WINDOW_HEIGHT);

    // Corners first, then the edges repeated between them.
    RenderImage(IMAGE_COMMAND_TOP_LEFT, x, y, BORDER, BORDER);
    RenderImage(IMAGE_COMMAND_TOP_RIGHT, x + WINDOW_WIDTH - BORDER, y, BORDER, BORDER);
    RenderImage(IMAGE_COMMAND_BOTTOM_LEFT, x, y + WINDOW_HEIGHT - BORDER, BORDER, BORDER);
    RenderImage(IMAGE_COMMAND_BOTTOM_RIGHT, x + WINDOW_WIDTH - BORDER, y + WINDOW_HEIGHT - BORDER, BORDER, BORDER);

    RenderImage(IMAGE_COMMAND_TOP_PIXEL, x + BORDER, y, WINDOW_WIDTH - 2 * BORDER, BORDER);
    RenderImage(IMAGE_COMMAND_BOTTOM_PIXEL, x + BORDER, y + WINDOW_HEIGHT - BORDER, WINDOW_WIDTH - 2 * BORDER, BORDER);
    RenderImage(IMAGE_COMMAND_LEFT_PIXEL, x, y + BORDER, BORDER, WINDOW_HEIGHT - 2 * BORDER);
    RenderImage(IMAGE_COMMAND_RIGHT_PIXEL, x + WINDOW_WIDTH - BORDER, y + BORDER, BORDER, WINDOW_HEIGHT - 2 * BORDER);

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(255, 255, 220, 120);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 10, I18N::Game::ChatCommandsTitle, static_cast<int>(WINDOW_WIDTH), 0, RT3_SORT_CENTER);
}

void SEASON3B::CNewUIChatCommandWindow::RenderCommandList()
{
    const auto& commands = Catalog().GetCommands();
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetBgColor(0);

    if (commands.empty())
    {
        g_pRenderText->SetTextColor(200, 200, 200, 255);
        g_pRenderText->RenderText(m_Pos.x + 25, GetListTop(), I18N::Game::ChatCommandsNotSupported, static_cast<int>(WINDOW_WIDTH) - 20, 0);
        return;
    }

    auto y = GetListTop();
    for (int row = 0; row < VISIBLE_ROWS; ++row)
    {
        const auto index = m_scrollOffset + row;
        if (static_cast<size_t>(index) >= commands.size())
        {
            break;
        }

        if (index == m_selectedIndex)
        {
            g_pRenderText->SetTextColor(255, 255, 100, 255);
        }
        else
        {
            g_pRenderText->SetTextColor(220, 220, 220, 255);
        }

        g_pRenderText->RenderText(m_Pos.x + 25, y, commands[index].Command.c_str(), 110, 0);
        g_pRenderText->RenderText(m_Pos.x + 140, y, commands[index].Name.c_str(), static_cast<int>(WINDOW_WIDTH - 140 - FRAME_SIDE - SCROLLBAR_WIDTH), 0);
        y += static_cast<int>(ROW_HEIGHT);
    }
}

void SEASON3B::CNewUIChatCommandWindow::RenderSelectedCommand()
{
    const auto* command = GetSelectedCommand();
    if (command == NULL)
    {
        return;
    }

    auto y = GetDetailTop();

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(200, 220, 255, 255);
    g_pRenderText->RenderText(m_Pos.x + 25, y, command->Description.c_str(), static_cast<int>(WINDOW_WIDTH - 2 * FRAME_SIDE), 48);
    y += 50;

    for (size_t i = 0; i < command->Parameters.size(); ++i)
    {
        const auto& parameter = command->Parameters[i];
        const auto& value = m_parameterValues[i];

        // A required parameter without a value is what keeps the command from
        // being sent, so it's the one to point at.
        if (parameter.IsRequired && value.empty())
        {
            g_pRenderText->SetTextColor(255, 150, 150, 255);
        }
        else
        {
            g_pRenderText->SetTextColor(220, 220, 220, 255);
        }

        g_pRenderText->RenderText(m_Pos.x + 25, y, parameter.Name.c_str(), 130, 0);
        if (m_editedParameter != static_cast<int>(i))
        {
            g_pRenderText->RenderText(m_Pos.x + 160, y, value.empty() ? parameter.ValidValues.c_str() : value.c_str(), static_cast<int>(WINDOW_WIDTH) - 165, 0);
        }
        y += static_cast<int>(ROW_HEIGHT);
    }

    const auto buttonY = m_Pos.y + static_cast<int>(WINDOW_HEIGHT - FRAME_BOTTOM) + 12;
    RenderImage(IMAGE_COMMAND_BUTTON, static_cast<float>(m_Pos.x + static_cast<int>(WINDOW_WIDTH - FRAME_SIDE) - 85), static_cast<float>(buttonY), 80.f, 20.f);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->RenderText(m_Pos.x + static_cast<int>(WINDOW_WIDTH - FRAME_SIDE) - 85, buttonY + 4, I18N::Game::ChatCommandsExecute, 80, 0, RT3_SORT_CENTER);
}
