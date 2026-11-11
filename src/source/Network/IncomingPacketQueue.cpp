#include "stdafx.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Network/IncomingPacketQueue.h"
#include "Network/Server/WSclient.h" // PacketInfo (complete type for the queued unique_ptr)

#include <array>
#include <cstring>
#include <cstdlib>

namespace
{
    constexpr double CombatBacklogAgeMs = 250.0;
    constexpr std::size_t CombatBacklogDepth = 512;
    constexpr std::size_t EntityKeyCount = 1U << 16;
    constexpr BYTE OpenMuBasicMonsterAttackAnimation = 120;

    int GetActionEntityKey(const PacketInfo& packet)
    {
        if (packet.Size < static_cast<int32_t>(sizeof(PRECEIVE_ACTION)))
        {
            return -1;
        }

        const BYTE* receiveBuffer = packet.ReceiveBuffer.get();
        const bool shortHeader = receiveBuffer[0] % 2 == 1;
        if (!shortHeader || receiveBuffer[2] != 0x18)
        {
            return -1;
        }

        const auto action = reinterpret_cast<const PRECEIVE_ACTION*>(receiveBuffer);
        if (action->Action != OpenMuBasicMonsterAttackAnimation)
        {
            return -1;
        }

        return (static_cast<int>(action->KeyH) << 8) + action->KeyL;
    }

    std::size_t MarkSupersededActions(std::deque<std::unique_ptr<PacketInfo>>& packets)
    {
        static std::array<std::uint32_t, EntityKeyCount> seenGenerations{};
        static std::uint32_t generation = 0;
        if (++generation == 0)
        {
            seenGenerations.fill(0);
            ++generation;
        }

        std::size_t coalescedCount = 0;
        for (auto packet = packets.rbegin(); packet != packets.rend(); ++packet)
        {
            const int entityKey = GetActionEntityKey(**packet);
            if (entityKey < 0)
            {
                continue;
            }

            if (seenGenerations[entityKey] == generation)
            {
                (*packet)->Superseded = true;
                ++coalescedCount;
                continue;
            }

            seenGenerations[entityKey] = generation;
        }
        return coalescedCount;
    }

    void LogQueueStats(const Network::IncomingPacketQueue::Stats& stats)
    {
        static const bool enabled = [] {
            const char* value = std::getenv("MU_NETWORK_DIAGNOSTICS");
            return value != nullptr && std::strcmp(value, "1") == 0;
        }();
        static auto nextLogTime = std::chrono::steady_clock::now();
        const auto now = std::chrono::steady_clock::now();
        if (!enabled || now < nextLogTime)
        {
            return;
        }

        nextLogTime = now + std::chrono::seconds(1);
        mu::log::Get("network")->debug(
            "[PacketQueue] depth={} high={} drained={} coalesced={} oldest={:.1f}ms drain={:.1f}ms",
            stats.depth, stats.highWaterMark, stats.lastDrainedCount,
            stats.coalescedActionCount,
            stats.oldestPacketAgeMs, stats.lastDrainDurationMs);
    }
}

namespace Network
{
    IncomingPacketQueue::~IncomingPacketQueue() = default;

    IncomingPacketQueue& IncomingPacketQueue::Instance()
    {
        // Intentionally leaked: the instance is never destroyed at process exit,
        // so a network thread still winding down during shutdown can call Push()
        // without racing a destroyed static. The OS reclaims the memory on exit.
        static IncomingPacketQueue* instance = new IncomingPacketQueue();
        return *instance;
    }

    void IncomingPacketQueue::Push(std::unique_ptr<PacketInfo> packet)
    {
        if (!packet)
        {
            return;
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        packet->EnqueuedAt = std::chrono::steady_clock::now();
        m_queue.push_back(std::move(packet));
        m_stats.depth = m_queue.size();
        m_stats.highWaterMark = std::max(m_stats.highWaterMark, m_stats.depth);
    }

    void IncomingPacketQueue::DrainTo(Processor process)
    {
        if (!process)
        {
            return;
        }

        // Push rejects null packets, so every entry here is non-null; no
        // per-packet guard is needed.
        std::deque<std::unique_ptr<PacketInfo>> pending;
        double oldestPacketAgeMs = 0.0;
        std::size_t capturedDepth = 0;
        const auto captureTime = std::chrono::steady_clock::now();
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::swap(pending, m_queue);
            capturedDepth = pending.size();
            m_stats.depth = 0;
            if (!pending.empty())
            {
                oldestPacketAgeMs = std::chrono::duration<double, std::milli>(
                    captureTime - pending.front()->EnqueuedAt).count();
            }
        }

        const std::size_t coalescedCount = MarkSupersededActions(pending);
        const bool suppressOptionalPresentation = oldestPacketAgeMs > CombatBacklogAgeMs
            || capturedDepth > CombatBacklogDepth;
        for (auto& packet : pending)
        {
            packet->SuppressOptionalPresentation = suppressOptionalPresentation;
        }

        const auto drainStart = std::chrono::steady_clock::now();
        std::size_t drainedCount = 0;
        while (!pending.empty())
        {
            if (!pending.front()->Superseded)
            {
                process(pending.front().get());
                ++drainedCount;
            }
            pending.pop_front();
        }

        const double drainDurationMs = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - drainStart).count();
        Stats stats;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_stats.lastDrainedCount = drainedCount;
            m_stats.oldestPacketAgeMs = oldestPacketAgeMs;
            m_stats.lastDrainDurationMs = drainDurationMs;
            m_stats.coalescedActionCount += coalescedCount;
            m_stats.depth = m_queue.size();
            stats = m_stats;
        }
        LogQueueStats(stats);
    }

    void IncomingPacketQueue::Clear()
    {
        // Discard packets queued before a session teardown so they are not
        // processed against freed world data on the next frame. Swap under the
        // lock and let the packets destruct after releasing it.
        std::deque<std::unique_ptr<PacketInfo>> discarded;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::swap(discarded, m_queue);
            m_stats.depth = 0;
        }
    }

    IncomingPacketQueue::Stats IncomingPacketQueue::GetStats() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        Stats stats = m_stats;
        stats.depth = m_queue.size();
        if (!m_queue.empty())
        {
            stats.oldestPacketAgeMs = std::chrono::duration<double, std::milli>(
                std::chrono::steady_clock::now() - m_queue.front()->EnqueuedAt).count();
        }
        return stats;
    }
}
