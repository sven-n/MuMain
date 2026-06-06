#include "stdafx.h"
#include "Network/IncomingPacketQueue.h"
#include "Network/Server/WSclient.h" // PacketInfo (complete type for the queued unique_ptr)

namespace Network
{
    IncomingPacketQueue::~IncomingPacketQueue() = default;

    IncomingPacketQueue& IncomingPacketQueue::Instance()
    {
        static IncomingPacketQueue instance;
        return instance;
    }

    void IncomingPacketQueue::Push(std::unique_ptr<PacketInfo> packet)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(std::move(packet));
    }

    void IncomingPacketQueue::DrainTo(Processor process)
    {
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
}
