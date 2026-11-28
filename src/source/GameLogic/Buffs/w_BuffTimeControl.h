#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>

SmartPointer(BuffTimeControl);

// The time of each buff on the hero (buffs design section 4): its full duration, the time left and where it came
// from, by buff number. Every new time restarts it (P2); any buff can have one (P1).
class BuffTimeControl
{
public:
    enum class TimeSource : std::uint8_t
    {
        Fallback, // the item buff's duration in Buffs.json, from a server that sends no time
        Sent,     // the server's
    };

    static BuffTimeControlPtr Make();
    virtual ~BuffTimeControl();

public:
    // A buff came on or was given again (0x2D, 0x07): `seconds` from the server, or 0 for none, which gives an item
    // buff its Buffs.json duration and another buff no time.
    void RegisterBuffTime(eBuffState bufftype, DWORD seconds);
    // The extended C1 07's time (B25): it wins, and 0 means the server sends no time for the buff, so it has none.
    void SetSentTime(eBuffState bufftype, std::uint32_t seconds);
    bool UnRegisterBuffTime(eBuffState bufftype);
    void Clear();

    std::optional<std::uint64_t> GetRemainingMilliseconds(eBuffState bufftype) const;
    std::optional<std::uint64_t> GetFullMilliseconds(eBuffState bufftype) const;
    std::optional<TimeSource> GetSource(eBuffState bufftype) const;

    // The clock in milliseconds; GetTickCount64 unless a test sets another. 64 bits: a buff can last longer than a
    // 32-bit count of milliseconds (49.7 days), and OpenMU sends up to a uint32 of seconds.
    void SetClock(std::function<std::uint64_t()> clock);

public:
    void GetBuffStringTime(eBuffState bufftype, std::wstring& timeText);
    void GetStringTime(DWORD time, std::wstring& timeText, bool isSecond = true);

private:
    BuffTimeControl();
    DWORD GetFallbackSeconds(eBuffState bufftype) const;
    void Start(eBuffState bufftype, DWORD seconds, TimeSource source);

private:
    struct BuffTime
    {
        std::uint64_t fullMilliseconds = 0;
        std::uint64_t startTick = 0;
        TimeSource source = TimeSource::Sent;
    };

    std::map<int, BuffTime> m_times;
    std::function<std::uint64_t()> m_clock;
};
