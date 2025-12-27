#pragma once

#ifdef _EDITOR

#include <string>
#include <fstream>

class CMuEditorConsole
{
public:
    static CMuEditorConsole& GetInstance();

    void Initialize();
    void Shutdown();

    void LogEditor(const std::string& message);
    void LogGame(const std::string& message);

    const std::string& GetEditorLog() const { return m_strEditorConsole; }
    const std::string& GetGameLog() const { return m_strGameConsole; }

    void ClearEditorLog() { m_strEditorConsole.clear(); }
    void ClearGameLog() { m_strGameConsole.clear(); }

    void Render();

private:
    CMuEditorConsole();
    ~CMuEditorConsole();

    void WriteToLogFile(const std::string& message);

    std::string m_strEditorConsole;
    std::string m_strGameConsole;
    std::ofstream m_logFile;
    std::string m_strLogFilePath;
};

#define g_MuEditorConsole CMuEditorConsole::GetInstance()

#endif // _EDITOR
