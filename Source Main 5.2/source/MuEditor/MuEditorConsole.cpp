#include "stdafx.h"

#ifdef _EDITOR

#include "MuEditorConsole.h"
#include "MuEditor.h"
#include "imgui.h"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <cstdarg>

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
            g_MuEditorConsole.LogGame(str);

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
            g_MuEditorConsole.LogGame(str);

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
                g_MuEditorConsole.LogGame(line);
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

CMuEditorConsole::CMuEditorConsole()
    : m_pStdoutBuf(nullptr)
    , m_pStderrBuf(nullptr)
    , m_lastConsoleY(0)
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

    // Redirect stdout and stderr to capture all console output
    m_pStdoutBuf = new ConsoleStreamBuf(std::cout, true);
    m_pStderrBuf = new ConsoleStreamBuf(std::cerr, false);

    // Capture all existing console output from the start
    CaptureConsoleOutput();
}

void CMuEditorConsole::Shutdown()
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
    std::lock_guard<std::mutex> lock(g_consoleMutex);

    m_strGameConsole += message;
    m_strGameConsole += "\n";

    // Write to log file with prefix
    WriteToLogFile("[GAME] " + message);
}

void CMuEditorConsole::CaptureConsoleOutput()
{
    // Get console screen buffer info
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE)
        return;

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (!GetConsoleScreenBufferInfo(hConsole, &csbi))
        return;

    // Read entire visible console buffer
    COORD bufferSize;
    bufferSize.X = csbi.dwSize.X;
    bufferSize.Y = csbi.dwCursorPosition.Y + 1;

    if (bufferSize.Y <= 0)
        return;

    CHAR_INFO* pCharBuffer = new CHAR_INFO[bufferSize.X * bufferSize.Y];

    COORD startPoint = { 0, 0 };
    SMALL_RECT readRegion;
    readRegion.Left = 0;
    readRegion.Top = 0;
    readRegion.Right = bufferSize.X - 1;
    readRegion.Bottom = bufferSize.Y - 1;

    if (ReadConsoleOutput(hConsole, pCharBuffer, bufferSize, startPoint, &readRegion))
    {
        // Convert entire console buffer to text
        std::string currentContent;
        for (short y = 0; y < bufferSize.Y; ++y)
        {
            std::wstring wline;
            for (short x = 0; x < bufferSize.X; ++x)
            {
                CHAR_INFO& charInfo = pCharBuffer[y * bufferSize.X + x];
                wchar_t wch = charInfo.Char.UnicodeChar;
                if (wch != L'\0')
                    wline += wch;
            }

            // Trim trailing spaces
            while (!wline.empty() && (wline.back() == L' ' || wline.back() == L'\0'))
                wline.pop_back();

            // Convert to UTF-8
            if (!wline.empty())
            {
                char utf8Buffer[4096];
                WideCharToMultiByte(CP_UTF8, 0, wline.c_str(), -1, utf8Buffer, sizeof(utf8Buffer), NULL, NULL);
                std::string line(utf8Buffer);

                // Trim trailing null/spaces again
                while (!line.empty() && (line.back() == ' ' || line.back() == '\0'))
                    line.pop_back();

                if (!line.empty())
                {
                    currentContent += line;
                    currentContent += '\n';
                }
            }
        }

        // Determine what's new content
        std::string newContent;
        if (m_lastConsoleContent.empty())
        {
            // First time - capture everything
            newContent = currentContent;
        }
        else if (currentContent.size() > m_lastConsoleContent.size())
        {
            // Subsequent times - only capture new content
            newContent = currentContent.substr(m_lastConsoleContent.size());
        }

        if (!newContent.empty())
        {
            // Split by newlines and add each line
            size_t pos = 0;
            while (pos < newContent.size())
            {
                size_t newlinePos = newContent.find('\n', pos);
                if (newlinePos == std::string::npos)
                {
                    // Last line without newline
                    std::string line = newContent.substr(pos);
                    if (!line.empty())
                    {
                        std::lock_guard<std::mutex> lock(g_consoleMutex);
                        m_strGameConsole += line;
                        m_strGameConsole += "\n";
                        WriteToLogFile("[GAME] " + line);
                    }
                    break;
                }
                else
                {
                    std::string line = newContent.substr(pos, newlinePos - pos);
                    if (!line.empty())
                    {
                        std::lock_guard<std::mutex> lock(g_consoleMutex);
                        m_strGameConsole += line;
                        m_strGameConsole += "\n";
                        WriteToLogFile("[GAME] " + line);
                    }
                    pos = newlinePos + 1;
                }
            }

            m_lastConsoleContent = currentContent;
        }
    }

    delete[] pCharBuffer;
}

void CMuEditorConsole::Update()
{
    CaptureConsoleOutput();
}

void CMuEditorConsole::Render()
{
    // Capture new console output each frame
    CaptureConsoleOutput();

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
        ImGui::TextWrapped("%s", m_strEditorConsole.c_str());
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
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
        ImGui::TextWrapped("%s", m_strGameConsole.c_str());
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
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
