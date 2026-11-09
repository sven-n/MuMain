#pragma once

#include <cstdint>
#include <string>

enum MSG_TYPE
{
    MCD_SEND = 0x01,
    MCD_RECEIVE,
    MCD_ERROR,
    MCD_NORMAL
};

// What CheckCommand made of the text: it ran a command, the text is not a
// command of the console, or it is one whose argument is missing or malformed.
// `message` carries the usage line of the last case; the callers report it
// their own way (the chat in the system log, the control socket as an error).
enum class ConsoleCommandStatus : std::uint8_t
{
    NotHandled,
    Handled,
    BadArgument,
};

struct ConsoleCommandResult
{
    ConsoleCommandStatus status{ConsoleCommandStatus::NotHandled};
    std::wstring message;
};

class CmuConsoleDebug
{
    bool	m_bInit;
public:
    virtual ~CmuConsoleDebug();
    static CmuConsoleDebug* GetInstance();

    void UpdateMainScene();
    ConsoleCommandResult CheckCommand(const std::wstring& strCommand);
    void Write(int iType, const wchar_t* pStr, ...);

protected:
    CmuConsoleDebug();
};

#define g_ConsoleDebug	CmuConsoleDebug::GetInstance()

