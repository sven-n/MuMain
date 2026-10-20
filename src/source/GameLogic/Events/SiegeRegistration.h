#pragma once

// This guild's castle-siege registration, as the server last reported it.
class CSiegeRegistration
{
public:
    // The hero may declare the guild for the siege.
    bool IsSufficientDeclareLevel() const;

    bool HasRegistered() const { return m_registered; }
    void SetRegistered(bool registered) { m_registered = registered; }

    // Guild marks the guild has handed in.
    DWORD GetRegMarkCount() const { return m_markCount; }
    void SetMarkCount(DWORD markCount) { m_markCount = markCount; }

private:
    static constexpr int DeclareLevel = 200;

    bool m_registered = false;
    DWORD m_markCount = 0;
};

extern CSiegeRegistration g_SiegeRegistration;
