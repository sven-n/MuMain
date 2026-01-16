#include "stdafx.h"

#ifdef _EDITOR

#include "../MuEditor\UI\Console\MuEditorConsoleUI.h"
#include "imgui.h"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <cstdarg>
#include <filesystem>
#include <vector>
#include <algorithm>

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MuEditorUI.h"

// Mutex for thread-safe console access
static std::mutex g_consoleMutex;

// Replacement console output functions
extern "C" {
    int editor_wprintf(const wchar_t* format, ...)
    {
        wchar_t buffer[4096];
        va_list args;
        va_start(args, format);
        int result = vswprintf_s(buffer, _countof(buffer), format, args);
        va_end(args);

        // Send to ImGui console
        char utf8Buffer[8192];
        WideCharToMultiByte(CP_UTF8, 0, buffer, -1, utf8Buffer, sizeof(utf8Buffer), NULL, NULL);

        // Remove trailing newline
        std::string str(utf8Buffer);
        while (!str.empty() && (str.back() == '\n' || str.back() == '\r'))
            str.pop_back();

        if (!str.empty())
            g_MuEditorConsoleUI.LogGame(str);

        return result;
    }

    int editor_fwprintf(FILE* stream, const wchar_t* format, ...)
    {
        wchar_t buffer[4096];
        va_list args;
        va_start(args, format);
        int result = vswprintf_s(buffer, _countof(buffer), format, args);
        va_end(args);

        // Send to ImGui console
        char utf8Buffer[8192];
        WideCharToMultiByte(CP_UTF8, 0, buffer, -1, utf8Buffer, sizeof(utf8Buffer), NULL, NULL);

        // Remove trailing newline
        std::string str(utf8Buffer);
        while (!str.empty() && (str.back() == '\n' || str.back() == '\r'))
            str.pop_back();

        if (!str.empty())
            g_MuEditorConsoleUI.LogGame(str);

        return result;
    }
}

// ConsoleStreamBuf implementation
ConsoleStreamBuf::ConsoleStreamBuf(std::ostream& stream, bool isStdout)
    : m_stream(stream)
    , m_oldBuf(stream.rdbuf(this))
    , m_isStdout(isStdout)
{
}

ConsoleStreamBuf::~ConsoleStreamBuf()
{
    // Restore original buffer
    m_stream.rdbuf(m_oldBuf);
}

std::streambuf::int_type ConsoleStreamBuf::overflow(int_type c)
{
    if (c != EOF)
    {
        m_buffer += static_cast<char>(c);

        // If we hit a newline, flush the buffer
        if (c == '\n')
        {
            std::lock_guard<std::mutex> lock(g_consoleMutex);

            // Remove trailing newline for our buffer
            std::string line = m_buffer;
            if (!line.empty() && line.back() == '\n')
                line.pop_back();

            // Send to ImGui console
            if (!line.empty())
            {
                g_MuEditorConsoleUI.LogGame(line);
            }

            // Also write to original stream
            m_oldBuf->sputn(m_buffer.c_str(), m_buffer.size());

            m_buffer.clear();
        }
    }
    return c;
}

std::streamsize ConsoleStreamBuf::xsputn(const char* s, std::streamsize n)
{
    for (std::streamsize i = 0; i < n; ++i)
    {
        overflow(s[i]);
    }
    return n;
}

CMuEditorConsoleUI::CMuEditorConsoleUI()
    : m_pStdoutBuf(nullptr)
    , m_pStderrBuf(nullptr)
{
    Initialize();
}

CMuEditorConsoleUI::~CMuEditorConsoleUI()
{
    Shutdown();
}

CMuEditorConsoleUI& CMuEditorConsoleUI::GetInstance()
{
    static CMuEditorConsoleUI instance;
    return instance;
}

void CMuEditorConsoleUI::CleanupOldLogs()
{
    try
    {
        namespace fs = std::filesystem;

        // Get current time
        time_t now = time(0);
        const time_t maxAge = 14 * 24 * 60 * 60; // 14 days in seconds

        // Ensure MuEditor directory exists
        fs::path logDir = "MuEditor";
        if (!fs::exists(logDir))
        {
            fs::create_directory(logDir);
        }

        // Find all log files matching pattern in MuEditor folder
        std::vector<fs::path> logFiles;
        for (const auto& entry : fs::directory_iterator(logDir))
        {
            if (entry.is_regular_file())
            {
                std::string filename = entry.path().filename().string();
                if (filename.find("MuEditor_") == 0 && filename.find(".log") != std::string::npos)
                {
                    logFiles.push_back(entry.path());
                }
            }
        }

        // Delete logs older than 14 days
        for (const auto& logPath : logFiles)
        {
            auto ftime = fs::last_write_time(logPath);
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
            time_t fileTime = std::chrono::system_clock::to_time_t(sctp);

            if ((now - fileTime) > maxAge)
            {
                fs::remove(logPath);
            }
        }
    }
    catch (const std::exception&)
    {
        // Silently fail if cleanup encounters any errors
    }
}

void CMuEditorConsoleUI::Initialize()
{
    // Create log file with date only (one log per day)
    time_t now = time(0);
    tm timeinfo;
    localtime_s(&timeinfo, &now);

    std::ostringstream filename;
    filename << "MuEditor\\MuEditor_"
             << std::setfill('0') << std::setw(4) << (timeinfo.tm_year + 1900)
             << std::setfill('0') << std::setw(2) << (timeinfo.tm_mon + 1)
             << std::setfill('0') << std::setw(2) << timeinfo.tm_mday
             << ".log";

    m_strLogFilePath = filename.str();

    // Open in append mode to continue writing to today's log
    m_logFile.open(m_strLogFilePath, std::ios::app);

    if (m_logFile.is_open())
    {
        m_logFile << "=== MU Editor Log Started ===" << std::endl;
        m_logFile << "Log file: " << m_strLogFilePath << std::endl;
        m_logFile << "===========================" << std::endl << std::endl;
        m_logFile.flush();
    }

    // Clean up old log files (keep only last 14 days)
    CleanupOldLogs();

    // Redirect stdout and stderr to capture all console output
    m_pStdoutBuf = new ConsoleStreamBuf(std::cout, true);
    m_pStderrBuf = new ConsoleStreamBuf(std::cerr, false);
}

void CMuEditorConsoleUI::Shutdown()
{
    // Restore original stream buffers
    if (m_pStdoutBuf)
    {
        delete m_pStdoutBuf;
        m_pStdoutBuf = nullptr;
    }
    if (m_pStderrBuf)
    {
        delete m_pStderrBuf;
        m_pStderrBuf = nullptr;
    }

    if (m_logFile.is_open())
    {
        m_logFile << std::endl << "=== MU Editor Log Ended ===" << std::endl;
        m_logFile.close();
    }
}

void CMuEditorConsoleUI::WriteToLogFile(const std::string& message)
{
    if (m_logFile.is_open())
    {
        m_logFile << message << std::endl;
        m_logFile.flush();
    }
}

void CMuEditorConsoleUI::LogEditor(const std::string& message)
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

void CMuEditorConsoleUI::LogGame(const std::string& message)
{
    std::lock_guard<std::mutex> lock(g_consoleMutex);

    m_strGameConsole += message;
    m_strGameConsole += "\n";

    // Write to log file with prefix
    WriteToLogFile("[GAME] " + message);
}


void CMuEditorConsoleUI::Render()
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
            g_MuEditorCore.SetHoveringUI(true);
        }

        // Split horizontally
        float split_width = ImGui::GetContentRegionAvail().x * 0.5f;

        // Editor Console (Left)
        ImGui::BeginChild("EditorConsole", ImVec2(split_width - 5, 0), true);

        // Fixed header - not scrollable
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

        // Scrollable content area
        ImGui::BeginChild("EditorConsoleContent", ImVec2(0, 0), false);
        ImGui::InputTextMultiline("##EditorConsoleText", const_cast<char*>(m_strEditorConsole.c_str()),
            m_strEditorConsole.length() + 1, ImVec2(-1, -1), ImGuiInputTextFlags_ReadOnly);
        ImGui::EndChild();

        ImGui::EndChild();

        ImGui::SameLine();

        // Game Console (Right)
        ImGui::BeginChild("GameConsole", ImVec2(0, 0), true);

        // Fixed header - not scrollable
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

        // Scrollable content area
        ImGui::BeginChild("GameConsoleContent", ImVec2(0, 0), false);
        ImGui::InputTextMultiline("##GameConsoleText", const_cast<char*>(m_strGameConsole.c_str()),
            m_strGameConsole.length() + 1, ImVec2(-1, -1), ImGuiInputTextFlags_ReadOnly);
        ImGui::EndChild();

        ImGui::EndChild();

        // Show log file path at bottom
        ImGui::Separator();
        ImGui::Text("Log file: %s", m_strLogFilePath.c_str());
    }
    ImGui::End();
    ImGui::PopStyleColor();
}

#endif // _EDITOR
