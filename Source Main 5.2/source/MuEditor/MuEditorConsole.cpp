#include "stdafx.h"

#ifdef _EDITOR

#include "MuEditorConsole.h"
#include "MuEditor.h"
#include "imgui.h"
#include <ctime>
#include <sstream>
#include <iomanip>

CMuEditorConsole::CMuEditorConsole()
{
    Initialize();
}

CMuEditorConsole::~CMuEditorConsole()
{
    Shutdown();
}

CMuEditorConsole& CMuEditorConsole::GetInstance()
{
    static CMuEditorConsole instance;
    return instance;
}

void CMuEditorConsole::Initialize()
{
    // Create log file with timestamp
    time_t now = time(0);
    tm timeinfo;
    localtime_s(&timeinfo, &now);

    std::ostringstream filename;
    filename << "MuEditor_"
             << std::setfill('0') << std::setw(4) << (timeinfo.tm_year + 1900)
             << std::setfill('0') << std::setw(2) << (timeinfo.tm_mon + 1)
             << std::setfill('0') << std::setw(2) << timeinfo.tm_mday
             << "_"
             << std::setfill('0') << std::setw(2) << timeinfo.tm_hour
             << std::setfill('0') << std::setw(2) << timeinfo.tm_min
             << std::setfill('0') << std::setw(2) << timeinfo.tm_sec
             << ".log";

    m_strLogFilePath = filename.str();
    m_logFile.open(m_strLogFilePath, std::ios::out);

    if (m_logFile.is_open())
    {
        m_logFile << "=== MU Editor Log Started ===" << std::endl;
        m_logFile << "Log file: " << m_strLogFilePath << std::endl;
        m_logFile << "===========================" << std::endl << std::endl;
        m_logFile.flush();
    }
}

void CMuEditorConsole::Shutdown()
{
    if (m_logFile.is_open())
    {
        m_logFile << std::endl << "=== MU Editor Log Ended ===" << std::endl;
        m_logFile.close();
    }
}

void CMuEditorConsole::WriteToLogFile(const std::string& message)
{
    if (m_logFile.is_open())
    {
        m_logFile << message << std::endl;
        m_logFile.flush();
    }
}

void CMuEditorConsole::LogEditor(const std::string& message)
{
    // Add timestamp
    time_t now = time(0);
    tm timeinfo;
    localtime_s(&timeinfo, &now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "[%H:%M:%S]", &timeinfo);

    std::string fullMessage = std::string(timestamp) + " " + message;

    m_strEditorConsole += fullMessage;
    m_strEditorConsole += "\n";

    // Write to log file
    WriteToLogFile(fullMessage);
}

void CMuEditorConsole::LogGame(const std::string& message)
{
    m_strGameConsole += message;
    m_strGameConsole += "\n";

    // Write to log file with prefix
    WriteToLogFile("[GAME] " + message);
}

void CMuEditorConsole::Render()
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
        // Check if hovering this window or any of its children
        if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
        {
            g_MuEditor.SetHoveringUI(true);
        }

        // Split horizontally
        float split_width = ImGui::GetContentRegionAvail().x * 0.5f;

        // Editor Console (Left)
        ImGui::BeginChild("EditorConsole", ImVec2(split_width - 5, 0), true);

        // Header with copy button
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Editor Console");
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);
        if (ImGui::SmallButton("Copy"))
        {
            ImGui::SetClipboardText(m_strEditorConsole.c_str());
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Clear"))
        {
            ClearEditorLog();
        }

        ImGui::Separator();
        ImGui::TextWrapped("%s", m_strEditorConsole.c_str());
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
        ImGui::EndChild();

        ImGui::SameLine();

        // Game Console (Right)
        ImGui::BeginChild("GameConsole", ImVec2(0, 0), true);

        // Header with copy button
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Game Console");
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20);
        if (ImGui::SmallButton("Copy##Game"))
        {
            ImGui::SetClipboardText(m_strGameConsole.c_str());
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Clear##Game"))
        {
            ClearGameLog();
        }

        ImGui::Separator();
        ImGui::TextWrapped("%s", m_strGameConsole.c_str());
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
        ImGui::EndChild();

        // Show log file path at bottom
        ImGui::Separator();
        ImGui::Text("Log file: %s", m_strLogFilePath.c_str());
    }
    ImGui::End();
    ImGui::PopStyleColor();
}

#endif // _EDITOR
