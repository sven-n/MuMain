#include "stdafx.h"
#include "Network/IncomingPacketQueue.h"
#include "Network/Server/WSclient.h" // PacketInfo (complete type for the queued unique_ptr)

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
        m_queue.push(std::move(packet));
    }

    void IncomingPacketQueue::DrainTo(Processor process)
    {
        if (!process)
        {
            return;
        }

        // Push rejects null packets, so every entry here is non-null; no
        // per-packet guard is needed.
        std::queue<std::unique_ptr<PacketInfo>> pending;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::swap(pending, m_queue);
        }

        while (!pending.empty())
        {
            process(pending.front().get());
            pending.pop();
        }
    }

    void IncomingPacketQueue::Clear()
    {
        // Discard packets queued before a session teardown so they are not
        // processed against freed world data on the next frame. Swap under the
        // lock and let the packets destruct after releasing it.
        std::queue<std::unique_ptr<PacketInfo>> discarded;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            std::swap(discarded, m_queue);
        }
    }
}
