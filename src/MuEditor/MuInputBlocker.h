#pragma once

#ifdef _EDITOR

class CMuInputBlocker
{
public:
    static CMuInputBlocker& GetInstance();

    void ProcessInputBlocking();

private:
    CMuInputBlocker() = default;
    ~CMuInputBlocker() = default;
};

#define g_MuInputBlocker CMuInputBlocker::GetInstance()

#endif // _EDITOR
