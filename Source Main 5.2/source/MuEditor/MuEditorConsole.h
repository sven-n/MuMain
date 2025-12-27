#pragma once

#ifdef _EDITOR

#include <string>

class CMuEditorConsole
{
public:
    static CMuEditorConsole& GetInstance();

    void LogEditor(const std::string& message);
    void LogGame(const std::string& message);

    const std::string& GetEditorLog() const { return m_strEditorConsole; }
    const std::string& GetGameLog() const { return m_strGameConsole; }

    void ClearEditorLog() { m_strEditorConsole.clear(); }
    void ClearGameLog() { m_strGameConsole.clear(); }

    void Render();

private:
    CMuEditorConsole() = default;
    ~CMuEditorConsole() = default;

    std::string m_strEditorConsole;
    std::string m_strGameConsole;
};

#define g_MuEditorConsole CMuEditorConsole::GetInstance()

#endif // _EDITOR
