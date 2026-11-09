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
    // Remaining share of the buff's server-sent duration, 1.0 at registration and 0.0 at expiry.
    // False when the buff has no server-sent duration, so callers can hide the bar.
    bool GetBuffRemainingFraction(eBuffState bufftype, float& fraction);
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
};
