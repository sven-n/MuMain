#include "doctest.h"

#include <cstdint>
#include <memory>
#include <vector>

#include "Network/IncomingPacketQueue.h"

namespace
{
using Network::ConnectionEvent;
using Network::IncomingPacketQueue;

std::vector<int> s_drained;

void Record(const PacketInfo* entry)
{
    s_drained.push_back(entry->Event == ConnectionEvent::None ? entry->ReceiveBuffer[0]
                                                              : -static_cast<int>(entry->Event));
}

void PushPacket(std::uint8_t marker)
{
    auto packet = std::make_unique<PacketInfo>();
    packet->ReceiveBuffer = std::make_unique<std::uint8_t[]>(1);
    packet->ReceiveBuffer[0] = marker;
    packet->Size = 1;
    IncomingPacketQueue::Instance().Push(std::move(packet));
}

void PushEvent(ConnectionEvent event)
{
    auto entry = std::make_unique<PacketInfo>();
    entry->Event = event;
    IncomingPacketQueue::Instance().Push(std::move(entry));
}

std::vector<int> Drain()
{
    s_drained.clear();
    IncomingPacketQueue::Instance().DrainTo(&Record);
    return s_drained;
}
} // namespace

TEST_CASE("Connection events drain in order with the packets [network]")
{
    Drain();
    PushEvent(ConnectionEvent::Opened);
    PushPacket(7);
    PushPacket(8);
    PushEvent(ConnectionEvent::Closed);
    CHECK(Drain() == std::vector<int>{-1, 7, 8, -2});
}

TEST_CASE("Clearing the queue drops packets but keeps connection events [network]")
{
    Drain();
    PushPacket(1);
    PushEvent(ConnectionEvent::Opened);
    PushPacket(2);
    PushEvent(ConnectionEvent::Closed);
    IncomingPacketQueue::Instance().Clear();
    CHECK(IncomingPacketQueue::Instance().GetStats().depth == 2);
    CHECK(Drain() == std::vector<int>{-1, -2});
}
