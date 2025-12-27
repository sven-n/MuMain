#include "stdafx.h"

#ifdef _EDITOR

#include "MuEditorConsole.h"
#include "imgui.h"
#include <ctime>

CMuEditorConsole& CMuEditorConsole::GetInstance()
{
    static CMuEditorConsole instance;
    return instance;
}

void CMuEditorConsole::LogEditor(const std::string& message)
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

void CMuEditorConsole::LogGame(const std::string& message)
{
    m_strGameConsole += message;
    m_strGameConsole += "\n";
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

#endif // _EDITOR
