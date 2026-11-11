#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>

struct PacketInfo;

namespace Network
{
    // Thread-safe hand-off of received server packets from the network thread
    // to the main thread. The socket read callback enqueues packets; the main
    // loop drains and processes them once per frame.
    //
    // This preserves the "packets are processed on the main thread" guarantee
    // that the previous PostMessage(WM_RECEIVE_BUFFER) round-trip provided,
    // without routing game data through the Win32 message queue (which does not
    // exist off Windows).
    class IncomingPacketQueue
    {
    public:
        using Processor = void (*)(const PacketInfo*);

        struct Stats
        {
            std::size_t depth = 0;
            std::size_t highWaterMark = 0;
            std::size_t lastDrainedCount = 0;
            std::uint64_t coalescedActionCount = 0;
            double oldestPacketAgeMs = 0.0;
            double lastDrainDurationMs = 0.0;
        };

        static IncomingPacketQueue& Instance();

        // Enqueue a received packet. Called from the network thread.
        void Push(std::unique_ptr<PacketInfo> packet);

        // Process every queued packet in arrival order. Called from the main
        // thread once per frame. The queue is moved out under the lock and
        // drained after releasing it, so packet handling never blocks the
        // network thread.
        void DrainTo(Processor process);

        // Discard all queued packets. Called on session teardown (disconnect /
        // reconnect) so packets for the old session are not processed against
        // freed world data.
        void Clear();
        Stats GetStats() const;

    private:
        IncomingPacketQueue() = default;
        ~IncomingPacketQueue();

        mutable std::mutex m_mutex;
        std::deque<std::unique_ptr<PacketInfo>> m_queue;
        Stats m_stats;
    };
}
