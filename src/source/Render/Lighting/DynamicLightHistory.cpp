#include "Render/Lighting/DynamicLightHistory.h"

#include <algorithm>
#include <cmath>

namespace Render::Lighting
{
namespace
{
constexpr int ColorChannels = 3;
// A light no longer called is forgotten once no channel is brighter than this.
constexpr float ForgetBelow = 0.004f;

LightSource s_currentSource = LightSource::Effect;

float Strongest(const float color[3])
{
    return std::max({std::fabs(color[0]), std::fabs(color[1]), std::fabs(color[2])});
}

// Share of the way to the target covered after `seconds` with time constant `tau`; all of it
// without a time constant.
float Approach(float seconds, float tau)
{
    return tau > 0.f ? 1.f - std::exp(-std::max(seconds, 0.f) / tau) : 1.f;
}
} // namespace

LightSourceScope::LightSourceScope(LightSource source) : m_previous(s_currentSource)
{
    s_currentSource = source;
}

LightSourceScope::~LightSourceScope()
{
    s_currentSource = m_previous;
}

LightSource CurrentLightSource()
{
    return s_currentSource;
}

void DynamicLightHistory::BeginTick()
{
    m_drawing = false;
    m_classicOverflow = false;
    if (m_tickOpen)
    {
        m_callCount = m_tickStart;
        return;
    }
    m_tickStart = m_callCount;
    m_tickOpen = true;
}

bool DynamicLightHistory::Record(const LightCall& call)
{
    if (call.range <= 0)
    {
        return true;
    }
    if (m_classicOverflow || m_callCount == m_calls.size())
    {
        ++m_overflow;
        return false;
    }
    m_calls[m_callCount++] = call;
    return true;
}

std::span<const LightCall> DynamicLightHistory::DrainToClassic()
{
    if (m_classicOverflow)
        return {};
    std::copy_n(m_calls.begin(), m_callCount, m_out.begin());
    const std::size_t count = m_callCount;
    m_callCount = 0;
    m_tickStart = 0;
    m_followedCount = 0;
    m_calledOut = 0;
    m_classicOverflow = true;
    return {m_out.data(), count};
}

void DynamicLightHistory::SyncWorld(int world)
{
    if (world == m_world)
        return;
    m_world = world;
    m_callCount = 0;
    m_tickStart = 0;
    m_followedCount = 0;
    m_calledOut = 0;
    m_classicOverflow = false;
}

DynamicLightHistory::Followed* DynamicLightHistory::Continued(const LightCall& call)
{
    constexpr float MatchDistanceSquared = kMatchDistance * kMatchDistance;
    // A light of another range only continues when no light of the call's range is in reach.
    constexpr float OtherRangePenalty = MatchDistanceSquared;
    Followed* nearest = nullptr;
    float nearestScore = 2.f * MatchDistanceSquared;
    for (std::size_t i = 0; i < m_followedCount; ++i)
    {
        Followed& light = m_followed[i];
        if (light.called || light.source != call.source)
        {
            continue;
        }
        const float dx = light.x - call.x;
        const float dy = light.y - call.y;
        const float distance = dx * dx + dy * dy;
        if (distance > MatchDistanceSquared)
        {
            continue;
        }
        const float score = distance + (light.range == call.range ? 0.f : OtherRangePenalty);
        if (score < nearestScore)
        {
            nearest = &light;
            nearestScore = score;
        }
    }
    return nearest;
}

DynamicLightHistory::Followed* DynamicLightHistory::Slot()
{
    if (m_followedCount < m_followed.size())
    {
        return &m_followed[m_followedCount++];
    }
    Followed* weakest = nullptr;
    float weakestStrength = 0.f;
    for (std::size_t i = 0; i < m_followedCount; ++i)
    {
        Followed& light = m_followed[i];
        const float strength = Strongest(light.value);
        if (!light.called && (weakest == nullptr || strength < weakestStrength))
        {
            weakest = &light;
            weakestStrength = strength;
        }
    }
    return weakest;
}

std::size_t DynamicLightHistory::CapRoot(std::size_t index)
{
    while (m_capParent[index] != index)
    {
        m_capParent[index] = m_capParent[m_capParent[index]];
        index = m_capParent[index];
    }
    return index;
}

std::size_t DynamicLightHistory::PrepareEffectCap(const LightHistorySettings& settings)
{
    std::size_t count = 0;
    for (std::size_t i = 0; i < m_followedCount; ++i)
    {
        if (m_followed[i].source != LightSource::Effect || Strongest(m_followed[i].value) == 0.f)
            continue;
        m_order[count] = i;
        m_capParent[count] = count;
        const LightCall shown = Shown(m_followed[i], settings);
        m_capStrength[count] = Strongest(shown.color);
        m_capRadius[count] = static_cast<float>(shown.range) * kCellSize;
        m_capTotal[count] = m_capStrength[count];
        m_capPeak[count++] = 0.;
    }
    return count;
}

void DynamicLightHistory::MeasureEffectCap(std::size_t count)
{
    // Connect lights whose centres receive each other's contribution. Each connected component
    // gets one scale, chosen from its maximum FINAL shown sum at every participating centre.
    // Uniform component scaling enforces that sampled bound; independent target scales do not.
    for (std::size_t a = 0; a < count; ++a)
    {
        const Followed& first = m_followed[m_order[a]];
        const float firstRadius = m_capRadius[a];
        const float firstStrength = m_capStrength[a];
        for (std::size_t b = a + 1; b < count; ++b)
        {
            const Followed& second = m_followed[m_order[b]];
            const float secondRadius = m_capRadius[b];
            const float dx = second.x - first.x;
            const float dy = second.y - first.y;
            const float distanceSquared = dx * dx + dy * dy;
            const float maxRadius = std::max(firstRadius, secondRadius);
            if (distanceSquared >= maxRadius * maxRadius)
                continue;
            const float distance = std::sqrt(distanceSquared);
            m_capTotal[a] += m_capStrength[b] * std::max(0.f, 1.f - distance / secondRadius);
            m_capTotal[b] += firstStrength * std::max(0.f, 1.f - distance / firstRadius);
            const std::size_t rootA = CapRoot(a);
            const std::size_t rootB = CapRoot(b);
            m_capParent[rootB] = rootA;
        }
    }
}

void DynamicLightHistory::LimitEffectLights(const LightHistorySettings& settings)
{
    if (settings.effectCap <= 0.f)
        return;
    const std::size_t count = PrepareEffectCap(settings);
    MeasureEffectCap(count);
    for (std::size_t a = 0; a < count; ++a)
    {
        const std::size_t root = CapRoot(a);
        m_capPeak[root] = std::max(m_capPeak[root], m_capTotal[a]);
    }
    for (std::size_t a = 0; a < count; ++a)
    {
        const double peak = m_capPeak[CapRoot(a)];
        if (peak <= settings.effectCap)
            continue;
        const float scale = std::nextafter(static_cast<float>(settings.effectCap / peak), 0.f);
        for (float& channel : m_followed[m_order[a]].value)
            channel *= scale; // retain the displayed envelope so its next fade is visible at once
    }
}

std::size_t DynamicLightHistory::Continue(Followed& light, const LightCall& call)
{
    light.x = call.x;
    light.y = call.y;
    light.range = call.range;
    light.called = true;
    std::copy(call.color, call.color + ColorChannels, light.target);
    return static_cast<std::size_t>(&light - m_followed.data());
}

int DynamicLightHistory::EffectRange(int range, const LightHistorySettings& settings)
{
    const int scaled =
        std::max(1, static_cast<int>(std::lround(static_cast<float>(range) * settings.effectRangeScale)));
    const int ceiling = static_cast<int>(std::lround(settings.effectRangeMax));
    if (ceiling <= 0 || scaled <= ceiling)
    {
        return scaled;
    }
    // The ceiling only limits widening: a light already wider keeps its own range.
    return std::max(ceiling, std::min(range, scaled));
}

LightCall DynamicLightHistory::Shown(const Followed& light, const LightHistorySettings& settings)
{
    // Followed lights never darken (darkening calls bypass the history).
    const bool boosted = light.source == LightSource::Effect;
    LightCall result;
    result.x = light.x;
    result.y = light.y;
    result.source = light.source;
    for (int k = 0; k < ColorChannels; ++k)
    {
        result.color[k] = boosted ? light.value[k] * settings.effectGain : light.value[k];
    }
    result.range = boosted ? EffectRange(light.range, settings) : light.range;
    return result;
}

std::span<const LightCall> DynamicLightHistory::Take(float seconds, const LightHistorySettings& settings)
{
    if (m_classicOverflow)
    {
        m_tickOpen = false;
        m_drawing = true;
        return {}; // the prefix and all later calls were already applied in Classic order
    }
    for (std::size_t i = 0; i < m_followedCount; ++i)
    {
        m_followed[i].called = false;
        std::fill(m_followed[i].target, m_followed[i].target + ColorChannels, 0.f);
    }
    // Each call's light, in call order: a followed light's index, or kBypass for a darkening call
    // (it goes out as it is, for this frame only). Continuations are claimed before any new light
    // may replace a followed one, so a new call never takes the place of a light continued later.
    for (std::size_t c = 0; c < m_callCount; ++c)
    {
        const LightCall& call = m_calls[c];
        if (std::min({call.color[0], call.color[1], call.color[2]}) < 0.f)
        {
            m_callLight[c] = kBypass;
            continue;
        }
        Followed* light = Continued(call);
        m_callLight[c] = light == nullptr ? kUnplaced : Continue(*light, call);
    }
    for (std::size_t c = 0; c < m_callCount; ++c)
    {
        if (m_callLight[c] != kUnplaced)
        {
            continue;
        }
        Followed* light = Slot();
        if (light == nullptr)
        {
            ++m_overflow;
            m_callLight[c] = kDropped;
            continue;
        }
        *light = Followed{};
        light->source = m_calls[c].source;
        m_callLight[c] = Continue(*light, m_calls[c]);
    }

    const float mapStep = Approach(seconds, settings.mapSmoothingSeconds);
    const float effectKeep =
        settings.effectFadeSeconds > 0.f ? std::exp(-std::max(seconds, 0.f) / settings.effectFadeSeconds) : 0.f;
    for (std::size_t i = 0; i < m_followedCount; ++i)
    {
        Followed& light = m_followed[i];
        for (int k = 0; k < ColorChannels; ++k)
        {
            if (light.fresh)
            {
                light.value[k] = light.target[k];
            }
            else if (light.source != LightSource::Effect)
            {
                light.value[k] += mapStep * (light.target[k] - light.value[k]);
            }
            else
            {
                const float faded = light.value[k] * effectKeep;
                light.value[k] = light.called && light.target[k] >= faded ? light.target[k] : faded;
            }
        }
        light.fresh = false;
    }

    LimitEffectLights(settings);

    // The frame's lights: the calls in their order (terrain light is clamped after every addition,
    // so the order matters), then the lights no call continued, always strongest first. The total never exceeds
    // what the frame's light list holds.
    std::size_t out = 0;
    for (std::size_t c = 0; c < m_callCount; ++c)
    {
        const std::size_t index = m_callLight[c];
        if (index == kBypass)
        {
            m_out[out++] = m_calls[c];
        }
        else if (index != kDropped)
        {
            m_out[out++] = Shown(m_followed[index], settings);
        }
    }
    m_calledOut = out;
    std::size_t uncalled = 0;
    for (std::size_t i = 0; i < m_followedCount; ++i)
    {
        if (!m_followed[i].called && Strongest(m_followed[i].value) > 0.f)
        {
            m_order[uncalled++] = i;
        }
    }
    const std::size_t room = out < kListBudget ? kListBudget - out : 0;
    std::sort(m_order.begin(), m_order.begin() + uncalled,
              [this, &settings](std::size_t a, std::size_t b)
              {
                  const float first = Strongest(Shown(m_followed[a], settings).color);
                  const float second = Strongest(Shown(m_followed[b], settings).color);
                  return first != second ? first > second : a < b;
              });
    uncalled = std::min(uncalled, room);
    for (std::size_t i = 0; i < uncalled; ++i)
    {
        m_out[out++] = Shown(m_followed[m_order[i]], settings);
    }

    std::size_t kept = 0;
    for (std::size_t i = 0; i < m_followedCount; ++i)
    {
        if (m_followed[i].called || Strongest(m_followed[i].value) >= ForgetBelow)
        {
            m_followed[kept++] = m_followed[i];
        }
    }
    m_followedCount = kept;
    m_callCount = 0;
    m_tickStart = 0;
    m_tickOpen = false;
    m_drawing = true;
    return {m_out.data(), out};
}

void DynamicLightHistory::Clear()
{
    ++m_generation;
    m_drawing = false;
    m_classicOverflow = false;
    m_calledOut = 0;
    m_callCount = 0;
    m_tickStart = 0;
    m_tickOpen = false;
    m_followedCount = 0;
    m_overflow = 0;
}

std::size_t DynamicLightHistory::FollowedCount(LightSource source) const
{
    return static_cast<std::size_t>(std::count_if(m_followed.begin(), m_followed.begin() + m_followedCount,
                                                  [source](const Followed& light) { return light.source == source; }));
}

std::size_t DropReporter::Due(DropCount count, float nowSeconds)
{
    const std::size_t total = count.total;
    if (count.generation != m_generation)
    {
        m_generation = count.generation;
        m_reported = 0; // the count was restarted (history cleared)
    }
    if (total == m_reported || (m_hasReported && nowSeconds - m_lastSeconds < IntervalSeconds))
    {
        return 0;
    }
    const std::size_t due = total - m_reported;
    m_reported = total;
    m_lastSeconds = nowSeconds;
    m_hasReported = true;
    return due;
}

DynamicLightHistory& FrameLightHistory()
{
    static DynamicLightHistory history;
    return history;
}

} // namespace Render::Lighting
