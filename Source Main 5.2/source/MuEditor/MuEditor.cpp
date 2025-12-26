#include "stdafx.h"

#ifdef _EDITOR

#include "MuEditor.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl2.h"
#include <sstream>
#include <ctime>

CMuEditor::CMuEditor()
    : m_bEditorMode(false)
    , m_bInitialized(false)
    , m_bGameRunning(false)
    , m_bFrameStarted(false)
    , m_bShowItemEditor(false)
    , m_hGameWindow(nullptr)
    , m_hGameConsole(nullptr)
{
}

CMuEditor::~CMuEditor()
{
    Shutdown();
}

CMuEditor& CMuEditor::GetInstance()
{
    static CMuEditor instance;
    return instance;
}

void CMuEditor::Initialize(HWND hwnd, HDC hdc)
{
    if (m_bInitialized)
        return;

    std::fwprintf(stderr, L"[MuEditor] Initialize() called\n");
    std::fflush(stderr);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Don't let ImGui change the mouse cursor - we'll use the game's custom cursor
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    io.MouseDrawCursor = false; // Don't let ImGui draw its own cursor

    std::fwprintf(stderr, L"[MuEditor] ImGui context created\n");
    std::fflush(stderr);

    // Dark theme
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplOpenGL2_Init();

    std::fwprintf(stderr, L"[MuEditor] ImGui backends initialized\n");
    std::fflush(stderr);

    m_bInitialized = true;
    LogEditor("MU Editor initialized");

    std::fwprintf(stderr, L"[MuEditor] Initialize() completed\n");
    std::fflush(stderr);
}

void CMuEditor::Shutdown()
{
    if (!m_bInitialized)
        return;

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    m_bInitialized = false;
}

void CMuEditor::Update()
{
    if (!m_bInitialized)
        return;

    // Only start a new frame if we haven't already
    if (!m_bFrameStarted)
    {
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        m_bFrameStarted = true;
    }

    // Block game input when ImGui is capturing mouse/keyboard (always check, even if editor is closed)
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse || io.WantCaptureKeyboard)
    {
        // Clear game input flags
        extern bool MouseLButton, MouseLButtonPop, MouseLButtonPush, MouseLButtonDBClick;
        extern bool MouseRButton, MouseRButtonPop, MouseRButtonPush;
        extern bool MouseMButton;

        MouseLButton = false;
        MouseLButtonPop = false;
        MouseLButtonPush = false;
        MouseLButtonDBClick = false;
        MouseRButton = false;
        MouseRButtonPop = false;
        MouseRButtonPush = false;
        MouseMButton = false;
    }
}

void CMuEditor::Render()
{
    if (!m_bInitialized)
        return;

    // Only render if we have a frame started
    if (!m_bFrameStarted)
        return;

    if (!m_bEditorMode)
    {
        // When editor is disabled, show only "Open Editor" button (fully transparent background)
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 40), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

        // Use same window background and border as Toolbar but fully transparent
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));

        if (ImGui::Begin("ToolbarClosed", nullptr, flags))
        {
            ImGui::Spacing();
            ImGui::Indent(10.0f);

            // Open button on the far right (same position as Close button)
            ImGui::SameLine(ImGui::GetWindowWidth() - 110);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
            if (ImGui::Button("Open Editor"))
            {
                SetEnabled(true);
            }
            ImGui::PopStyleColor(2);

            ImGui::Unindent(10.0f);
        }
        ImGui::End();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);

        // Render ImGui and reset frame state
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        m_bFrameStarted = false;
        return;
    }

    // Render UI panels when editor is enabled
    RenderTopToolbar();
    RenderCenterViewport();
    RenderBottomConsole();

    // Render editor windows
    if (m_bShowItemEditor)
    {
        RenderItemEditor();
    }

    // Render ImGui and reset frame state
    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    // Frame is complete, reset for next frame
    m_bFrameStarted = false;
}

void CMuEditor::RenderTopToolbar()
{
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 40), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 0.95f));

    if (ImGui::Begin("Toolbar", nullptr, flags))
    {
        ImGui::Spacing();
        ImGui::Indent(10.0f);

        ImGui::Text("MU Editor");
        ImGui::SameLine();

        if (ImGui::Button("Item Editor"))
        {
            m_bShowItemEditor = !m_bShowItemEditor;
        }

        // Close button on the far right
        ImGui::SameLine(ImGui::GetWindowWidth() - 110);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        if (ImGui::Button("Close Editor"))
        {
            SetEnabled(false);
        }
        ImGui::PopStyleColor(2);

        ImGui::Unindent(10.0f);
    }
    ImGui::End();
    ImGui::PopStyleColor();
}

void CMuEditor::RenderCenterViewport()
{
    // The game renders automatically in the full window
    // The ImGui toolbar and console are overlays on top
    // No need to render anything here
}

void CMuEditor::RenderBottomConsole()
{
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 bottom_pos = ImVec2(0, io.DisplaySize.y - 200);
    ImVec2 bottom_size = ImVec2(io.DisplaySize.x, 200);

    ImGui::SetNextWindowPos(bottom_pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(bottom_size, ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.9f));

    if (ImGui::Begin("Console", nullptr, flags))
    {
        // Split horizontally
        float split_width = ImGui::GetContentRegionAvail().x * 0.5f;

        // Editor Console (Left)
        ImGui::BeginChild("EditorConsole", ImVec2(split_width - 5, 0), true);
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Editor Console");
        ImGui::Separator();
        ImGui::TextWrapped("%s", m_strEditorConsole.c_str());
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
        ImGui::EndChild();

        ImGui::SameLine();

        // Game Console (Right)
        ImGui::BeginChild("GameConsole", ImVec2(0, 0), true);
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Game Console");
        ImGui::Separator();
        ImGui::TextWrapped("%s", m_strGameConsole.c_str());
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
        ImGui::EndChild();
    }
    ImGui::End();
    ImGui::PopStyleColor();
}

void CMuEditor::RenderItemEditor()
{
    // Access external item data
    extern ITEM_ATTRIBUTE* ItemAttribute;
    if (!ItemAttribute)
        return;

    // Fill the entire center view area (between toolbar and console)
    ImGuiIO& io = ImGui::GetIO();
    float width = io.DisplaySize.x;
    float height = io.DisplaySize.y - 40 - 200; // Full height minus toolbar (40px) and console (200px)
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0, 40), ImGuiCond_Always); // Start below toolbar

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar;
    if (ImGui::Begin("Item Editor", &m_bShowItemEditor, flags))
    {
        ImGui::Text("Edit Item Attributes - Total Items: 1000");
        ImGui::SameLine();

        // Save button on the right
        ImGui::SameLine(ImGui::GetWindowWidth() - 120);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.8f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.9f, 1.0f));
        if (ImGui::Button("Save Items"))
        {
            extern bool SaveItemScript(wchar_t* FileName, std::string* outChangeLog);
            extern std::wstring g_strSelectedML;

            wchar_t fileName[256];
            swprintf(fileName, L"Data\\Local\\%ls\\Item_%ls.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());

            std::string changeLog;
            if (SaveItemScript(fileName, &changeLog))
            {
                // Log to editor console
                LogEditor("=== SAVE COMPLETED ===\n");
                LogEditor(changeLog);
                LogEditor("======================\n");
                ImGui::OpenPopup("Save Success");
            }
            else
            {
                LogEditor("ERROR: Failed to save item attributes!\n");
                ImGui::OpenPopup("Save Failed");
            }
        }
        ImGui::PopStyleColor(2);

        // Success popup
        if (ImGui::BeginPopupModal("Save Success", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Item attributes saved successfully!");
            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        // Failed popup
        if (ImGui::BeginPopupModal("Save Failed", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Failed to save item attributes!");
            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::Separator();

        // Create a table with scrolling
        if (ImGui::BeginTable("ItemTable", 10, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX | ImGuiTableFlags_Resizable))
        {
            // Setup columns
            ImGui::TableSetupScrollFreeze(0, 1); // Freeze header row
            ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed, 50.0f);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 150.0f);
            ImGui::TableSetupColumn("Level", ImGuiTableColumnFlags_WidthFixed, 60.0f);
            ImGui::TableSetupColumn("Req Str", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Req Dex", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Req Ene", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Req Vit", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Damage", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Defense", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Durability", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableHeadersRow();

            // Render items (limit to first 100 for performance, add pagination later)
            for (int i = 0; i < 100; i++)
            {
                ImGui::TableNextRow();

                // Index
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", i);

                // Name
                ImGui::TableSetColumnIndex(1);
                char nameBuffer[128];
                wcstombs(nameBuffer, ItemAttribute[i].Name, sizeof(nameBuffer));
                ImGui::Text("%s", nameBuffer);

                // Level
                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%d", ItemAttribute[i].Level);

                // Required Strength (EDITABLE)
                ImGui::TableSetColumnIndex(3);
                ImGui::PushID(i);
                int reqStr = ItemAttribute[i].RequireStrength;
                if (ImGui::InputInt("##str", &reqStr, 0, 0))
                {
                    if (reqStr >= 0 && reqStr <= 65535)
                    {
                        ItemAttribute[i].RequireStrength = (WORD)reqStr;
                        LogEditor("Changed item " + std::to_string(i) + " RequireStrength to " + std::to_string(reqStr));
                    }
                }
                ImGui::PopID();

                // Required Dexterity
                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%d", ItemAttribute[i].RequireDexterity);

                // Required Energy
                ImGui::TableSetColumnIndex(5);
                ImGui::Text("%d", ItemAttribute[i].RequireEnergy);

                // Required Vitality
                ImGui::TableSetColumnIndex(6);
                ImGui::Text("%d", ItemAttribute[i].RequireVitality);

                // Damage
                ImGui::TableSetColumnIndex(7);
                ImGui::Text("%d-%d", ItemAttribute[i].DamageMin, ItemAttribute[i].DamageMax);

                // Defense
                ImGui::TableSetColumnIndex(8);
                ImGui::Text("%d", ItemAttribute[i].Defense);

                // Durability
                ImGui::TableSetColumnIndex(9);
                ImGui::Text("%d", ItemAttribute[i].Durability);
            }

            ImGui::EndTable();
        }
    }
    ImGui::End();
}

void CMuEditor::StartGame()
{
    if (m_bGameRunning)
        return;

    LogEditor("Starting game...");
    m_bGameRunning = true;
    LogEditor("Game started successfully");
    LogGame("Game initialized");
}

void CMuEditor::StopGame()
{
    if (!m_bGameRunning)
        return;

    LogEditor("Stopping game...");
    m_bGameRunning = false;
    LogEditor("Game stopped");
    m_strGameConsole.clear();
}

void CMuEditor::LogEditor(const std::string& message)
{
    // Add timestamp
    time_t now = time(0);
    tm timeinfo;
    localtime_s(&timeinfo, &now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "[%H:%M:%S]", &timeinfo);

    m_strEditorConsole += timestamp;
    m_strEditorConsole += " ";
    m_strEditorConsole += message;
    m_strEditorConsole += "\n";
}

void CMuEditor::LogGame(const std::string& message)
{
    m_strGameConsole += message;
    m_strGameConsole += "\n";
}

#endif // _EDITOR
