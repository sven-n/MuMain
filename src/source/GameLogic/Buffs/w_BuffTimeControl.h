#pragma once

SmartPointer(BuffTimeControl);

class BuffTimeControl
{
public:
    static BuffTimeControlPtr Make();
    virtual ~BuffTimeControl();

public:
    void RegisterBuffTime(eBuffState bufftype, DWORD curbufftime);
    bool UnRegisterBuffTime(eBuffState bufftype);
    bool IsBuffTime(eBuffTimeType bufftype);

public:
    void GetBuffStringTime(eBuffState bufftype, std::wstring& timeText);
    void GetBuffStringTime(DWORD type, std::wstring& timeText, bool issecond = true);
    const DWORD GetBuffTime(DWORD type);
    // Remaining share of the buff's registered duration, 1.0 at registration and 0.0 at expiry.
    // False when the buff has no tracked duration (most skill buffs), so callers can hide the bar.
    bool GetBuffRemainingFraction(eBuffState bufftype, float& fraction);
    // Client-side duration for buffs the server sends only as an effect id (skill effects). The
    // server's duration formula is reproduced from its effect catalogue; item effects keep using
    // the duration the server sends. Registered when the hero gains the buff.
    void RegisterEstimatedBuffTime(eBuffState bufftype, DWORD seconds);
    bool UnRegisterEstimatedBuffTime(eBuffState bufftype);
    bool GetEstimatedBuffRemainingFraction(eBuffState bufftype, float& fraction);
    // Remaining milliseconds of an estimated buff, 0 when it is not tracked or has run out.
    DWORD GetEstimatedBuffRemainingMilliseconds(eBuffState bufftype);
    void GetStringTime(DWORD time, std::wstring& timeText, bool isSecond = true);

private:
    eBuffTimeType CheckBuffTimeType(eBuffState bufftype);
    DWORD GetBuffEventTime(eBuffTimeType bufftimetype);
    DWORD GetBuffMaxTime(eBuffState bufftype, DWORD curbufftime = 0);
    bool CheckBuffTime(DWORD type);
    BuffTimeControl();

private:
    struct BuffTimeInfo
    {
        eBuffState s_BuffType;
        DWORD s_CurBuffTime;
        DWORD s_MaxBuffTime;
        DWORD s_EventBuffTime;

        BuffTimeInfo() : s_BuffType(eBuffNone), s_CurBuffTime(0), s_MaxBuffTime(0), s_EventBuffTime(0) {}
    };
    typedef std::map<DWORD, BuffTimeInfo> BuffTimeInfoMap;

private:
    BuffTimeInfoMap m_BuffTimeList;
    // Skill buffs are keyed by their own effect id: they share no server timer family.
    std::map<eBuffState, BuffTimeInfo> m_EstimatedBuffTimeList;
};

// Seconds the effect lasts when it is applied to the hero, or 0 when the client has no table
// entry. Mirrors the server's base + TotalEnergy/divisor, cap and 1 s minimum.
DWORD BuffDurationSeconds(eBuffState bufftype);
