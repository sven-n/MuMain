#pragma once

#ifdef _EDITOR

class CMuInputBlockerCore
{
public:
    static CMuInputBlockerCore& GetInstance();

    void ProcessInputBlocking();

private:
    CMuInputBlockerCore() = default;
    ~CMuInputBlockerCore() = default;
};

#define g_MuInputBlockerCore CMuInputBlockerCore::GetInstance()

#endif // _EDITOR
