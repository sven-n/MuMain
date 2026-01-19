#pragma once

enum MSG_TYPE
{
    MCD_SEND = 0x01,
    MCD_RECEIVE,
    MCD_ERROR,
    MCD_NORMAL
};

class CmuConsoleDebug
{
    bool	m_bInit;
public:
    virtual ~CmuConsoleDebug();
    static CmuConsoleDebug* GetInstance();

    void UpdateMainScene();
    bool CheckCommand(const std::wstring& strCommand);
    void Write(int iType, const wchar_t* pStr, ...);

protected:
    CmuConsoleDebug();
};

#define g_ConsoleDebug	CmuConsoleDebug::GetInstance()

