#include "stdafx.h"
#include "Engine/Object/ZzzInfomation.h"
#include "UI/Core/UIManager.h"
#include "GameLogic/Buffs/BuffValues.h"
#include "Data/GameData/BuffData/BuffDatabase.h"
#include "w_BuffTimeControl.h"
#include "I18N/All.h"

#include <utility>

namespace
{
constexpr std::uint64_t MillisecondsPerSecond = 1000;
constexpr DWORD SecondsPerMinute = 60;
} // namespace

BuffTimeControlPtr BuffTimeControl::Make()
{
    BuffTimeControlPtr bufftimecontrol(new BuffTimeControl);
    return bufftimecontrol;
}

BuffTimeControl::BuffTimeControl() : m_clock([] { return static_cast<std::uint64_t>(GetTickCount64()); }) {}

BuffTimeControl::~BuffTimeControl() = default;

void BuffTimeControl::SetClock(std::function<std::uint64_t()> clock)
{
    m_clock = std::move(clock);
}

DWORD BuffTimeControl::GetFallbackSeconds(eBuffState bufftype) const
{
    // Without a time from the server, an item buff lasts its duration in Buffs.json; another one has no time.
    const Data::Buffs::BuffDefinition* buff = g_BuffDatabase.Find(bufftype);
    const int duration = buff != nullptr && buff->item ? GameLogic::Buffs::GetDuration(*buff) : 0;
    return duration > 0 ? static_cast<DWORD>(duration) : 0;
}

void BuffTimeControl::Start(eBuffState bufftype, DWORD seconds, TimeSource source)
{
    BuffTime& time = m_times[static_cast<int>(bufftype)];
    time.fullMilliseconds = static_cast<std::uint64_t>(seconds) * MillisecondsPerSecond;
    time.startTick = m_clock();
    time.source = source;
}

void BuffTimeControl::RegisterBuffTime(eBuffState bufftype, DWORD seconds)
{
    if (seconds > 0)
    {
        Start(bufftype, seconds, TimeSource::Sent);
        return;
    }
    const DWORD fallback = GetFallbackSeconds(bufftype);
    if (fallback > 0)
    {
        Start(bufftype, fallback, TimeSource::Fallback);
    }
    else
    {
        m_times.erase(static_cast<int>(bufftype));
    }
}

void BuffTimeControl::SetSentTime(eBuffState bufftype, std::uint32_t seconds)
{
    if (seconds > 0)
    {
        Start(bufftype, seconds, TimeSource::Sent);
    }
    else
    {
        m_times.erase(static_cast<int>(bufftype));
    }
}

bool BuffTimeControl::UnRegisterBuffTime(eBuffState bufftype)
{
    return m_times.erase(static_cast<int>(bufftype)) > 0;
}

void BuffTimeControl::Clear()
{
    m_times.clear();
}

std::optional<std::uint64_t> BuffTimeControl::GetRemainingMilliseconds(eBuffState bufftype) const
{
    const auto found = m_times.find(static_cast<int>(bufftype));
    if (found == m_times.end())
    {
        return std::nullopt;
    }
    const std::uint64_t passed = m_clock() - found->second.startTick;
    return passed < found->second.fullMilliseconds ? found->second.fullMilliseconds - passed : 0;
}

std::optional<std::uint64_t> BuffTimeControl::GetFullMilliseconds(eBuffState bufftype) const
{
    const auto found = m_times.find(static_cast<int>(bufftype));
    return found != m_times.end() ? std::optional<std::uint64_t>(found->second.fullMilliseconds) : std::nullopt;
}

std::optional<BuffTimeControl::TimeSource> BuffTimeControl::GetSource(eBuffState bufftype) const
{
    const auto found = m_times.find(static_cast<int>(bufftype));
    return found != m_times.end() ? std::optional<TimeSource>(found->second.source) : std::nullopt;
}

void BuffTimeControl::GetBuffStringTime(eBuffState bufftype, std::wstring& timeText)
{
    const std::optional<std::uint64_t> remaining = GetRemainingMilliseconds(bufftype);
    if (!remaining.has_value() || *remaining == 0)
    {
        return;
    }
    // Whole seconds, rounded up: a buff with half a second left shows 1, not 0.
    const auto seconds = static_cast<DWORD>((*remaining + MillisecondsPerSecond - 1) / MillisecondsPerSecond);
    if (seconds < SecondsPerMinute)
    {
        // The buff's last minute counts down in seconds (buffs design B7, P3); item tooltips keep "Less than 1
        // minute" for a duration this short.
        wchar_t buffer[100];
        mu_swprintf(buffer, L"%d %ls", seconds, I18N::Game::Second);
        timeText = buffer;
        return;
    }
    GetStringTime(seconds, timeText, true);
}

void BuffTimeControl::GetStringTime(DWORD time, std::wstring& timeText, bool isSecond)
{
    wchar_t buffer[100];

    if (isSecond)
    {
        DWORD day = time / (1440 * 60);
        DWORD oClock = (time - (day * (1440 * 60))) / 3600;
        DWORD minutes = (time - ((oClock * 3600) + (day * (1440 * 60)))) / 60;
        DWORD second = time % 60;

        if (day != 0)
        {
            mu_swprintf(buffer, L"%d %ls %d %ls %d %ls %d %ls", day, I18N::Game::Day, oClock, I18N::Game::Hour, minutes, I18N::Game::Minute, second, I18N::Game::Second);
            timeText = buffer;
        }
        else if (day == 0 && oClock != 0)
        {
            mu_swprintf(buffer, L"%d %ls %d %ls %d %ls", oClock, I18N::Game::Hour, minutes, I18N::Game::Minute, second, I18N::Game::Second);
            timeText = buffer;
        }
        else if (day == 0 && oClock == 0 && minutes != 0)
        {
            mu_swprintf(buffer, L"%d %ls %d %ls", minutes, I18N::Game::Minute, second, I18N::Game::Second);
            timeText = buffer;
        }
        else if (day == 0 && oClock == 0 && minutes == 0)
        {
            mu_swprintf(buffer, L"%ls", I18N::Game::LessThan1Minutes);
            timeText = buffer;
        }
    }
    else
    {
        DWORD day = time / 1440;
        DWORD oClock = (time - (day * 1440)) / 60;
        DWORD minutes = time % 60;

        if (day != 0)
        {
            mu_swprintf(buffer, L"%d %ls %d %ls %d %ls", day, I18N::Game::Day, oClock, I18N::Game::Hour, minutes, I18N::Game::Minute);
            timeText = buffer;
        }
        else if (day == 0 && oClock != 0)
        {
            mu_swprintf(buffer, L"%d %ls %d %ls", oClock, I18N::Game::Hour, minutes, I18N::Game::Minute);
            timeText = buffer;
        }
        else if (day == 0 && oClock == 0 && minutes != 0)
        {
            mu_swprintf(buffer, L"%d %ls", minutes, I18N::Game::Minute);
            timeText = buffer;
        }
    }
}
