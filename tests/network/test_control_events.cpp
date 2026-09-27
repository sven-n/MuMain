// doctest unit tests for the control socket's event ring.
//
// The ring is plain bookkeeping: no window, no packets. Object names resolve
// against the client's (empty) character table here, so the shape of the JSON
// is what is asserted, not the names of live objects.
//
// Run: ctest --test-dir <build directory> --build-config Release -R "Control events"

#include "doctest.h"

#include "App/Control/ControlEvents.h"

#include "json.hpp"

#include <string>
#include <vector>

namespace Events = App::Control::Events;

namespace
{
// Every case starts from an empty, enabled ring with no followers.
struct RingFixture
{
    RingFixture()
    {
        Events::Reset();
        Events::SetEnabled(true);
    }

    ~RingFixture()
    {
        Events::SetEnabled(false);
        Events::Reset();
    }
};

nlohmann::json FieldsOf(const Events::Record& record)
{
    return nlohmann::json::parse(record.fields, nullptr, false);
}
} // namespace

TEST_CASE("Control events number every record and keep order [network][control-events]")
{
    const RingFixture fixture;

    CHECK(Events::LastSequence() == 0);
    CHECK(Events::Since(0).empty());

    Events::RecordScene("login");
    Events::RecordScene("character_list");
    Events::RecordScene("world");

    const std::vector<Events::Record> all = Events::Since(0);
    REQUIRE(all.size() == 3);
    CHECK(all[0].seq == 1);
    CHECK(all[1].seq == 2);
    CHECK(all[2].seq == 3);
    CHECK(Events::LastSequence() == 3);

    CHECK(FieldsOf(all[0])["scene"] == "login");
    CHECK(FieldsOf(all[2])["scene"] == "world");

    // A UTC timestamp with milliseconds, e.g. 2026-09-16T11:22:33.456Z.
    CHECK(all[0].utc.size() == 24);
    CHECK(all[0].utc.back() == 'Z');
    CHECK(all[0].utc[10] == 'T');
}

TEST_CASE("Control events return only what is newer than a sequence [network][control-events]")
{
    const RingFixture fixture;

    Events::RecordScene("login");
    Events::RecordScene("character_list");
    Events::RecordScene("world");

    const std::vector<Events::Record> afterFirst = Events::Since(1);
    REQUIRE(afterFirst.size() == 2);
    CHECK(afterFirst.front().seq == 2);

    CHECK(Events::Since(3).empty());
    CHECK(Events::Since(99).empty());
}

TEST_CASE("Control events keep the newest records across wrap-around [network][control-events]")
{
    const RingFixture fixture;

    // Two and a half rings of events: the sequence must stay strictly
    // increasing while the oldest entries are overwritten.
    const std::size_t written = Events::RingCapacity * 2 + Events::RingCapacity / 2;
    for (std::size_t index = 0; index < written; ++index)
    {
        Events::RecordStat("life", static_cast<long long>(index), 1000);
    }

    CHECK(Events::LastSequence() == written);

    const std::vector<Events::Record> retained = Events::Since(0);
    REQUIRE(retained.size() == Events::RingCapacity);
    // The spec's floor.
    CHECK(retained.size() >= 1000);

    CHECK(retained.front().seq == written - Events::RingCapacity + 1);
    CHECK(retained.back().seq == written);
    for (std::size_t index = 1; index < retained.size(); ++index)
    {
        REQUIRE(retained[index].seq == retained[index - 1].seq + 1);
    }

    // Asking from inside the retained window still cuts at the right place.
    const std::vector<Events::Record> tail = Events::Since(written - 5);
    REQUIRE(tail.size() == 5);
    CHECK(tail.front().seq == written - 4);
    CHECK(FieldsOf(tail.back())["value"] == static_cast<long long>(written - 1));
}

TEST_CASE("Control events record nothing while disabled [network][control-events]")
{
    Events::Reset();
    Events::SetEnabled(false);

    Events::RecordScene("world");
    Events::RecordHit(Events::Direction::Dealt, 1, 2, 10, 0, false, false);
    Events::RecordChat(L"someone", L"hello", "public");
    Events::RecordStat("life", 100, 1000);
    Events::RecordDrop(5, L"Jewel of Bless", 140, 130);
    Events::RecordDropGone(5, "picked");
    Events::RecordMap(0, L"Lorencia", 140, 130);
    Events::RecordViewEnter(7);
    Events::RecordViewLeave(7);
    Events::RecordParty("joined", L"test2");
    Events::RecordDeath(1, 2);
    Events::RecordDisconnect("server closed the connection");
    Events::RecordError("move", "no_path", "unreachable");

    CHECK(Events::LastSequence() == 0);
    CHECK(Events::Since(0).empty());
}

TEST_CASE("Control events describe a hit on both sides [network][control-events]")
{
    const RingFixture fixture;

    Events::RecordHit(Events::Direction::Received, 4321, 1234, 57, 3, true, false);

    const std::vector<Events::Record> recorded = Events::Since(0);
    REQUIRE(recorded.size() == 1);
    CHECK(recorded.front().name == "hit");

    const nlohmann::json fields = FieldsOf(recorded.front());
    REQUIRE_FALSE(fields.is_discarded());
    CHECK(fields["direction"] == "received");
    CHECK(fields["damage"] == 57);
    CHECK(fields["shield_damage"] == 3);
    CHECK(fields["critical"] == true);
    CHECK(fields["missed"] == false);

    REQUIRE(fields.contains("attacker"));
    REQUIRE(fields.contains("target"));
    CHECK(fields["attacker"]["id"] == 4321);
    CHECK(fields["target"]["id"] == 1234);
    for (const char* side : {"attacker", "target"})
    {
        CAPTURE(side);
        CHECK(fields[side].contains("id"));
        CHECK(fields[side].contains("name"));
        CHECK(fields[side].contains("kind"));
        // Out of view here, so the kind is honest about not knowing.
        CHECK(fields[side]["kind"] == "unknown");
    }

    Events::RecordHit(Events::Direction::Dealt, 1234, 4321, 12, 0, false, true);
    const std::vector<Events::Record> dealt = Events::Since(1);
    REQUIRE(dealt.size() == 1);
    CHECK(FieldsOf(dealt.front())["direction"] == "dealt");
    CHECK(FieldsOf(dealt.front())["missed"] == true);
}

TEST_CASE("Control events name each recorder's fields [network][control-events]")
{
    const RingFixture fixture;

    Events::RecordChat(L"test2", L"hello", "public");
    Events::RecordStat("mana", 40, 120);
    Events::RecordStat("level", 7, -1);
    Events::RecordDrop(9, L"Jewel of Bless", 140, 130);
    Events::RecordDropGone(9, "picked");
    Events::RecordMap(2, L"Devias", 10, 20);
    Events::RecordParty("joined", L"test2");
    Events::RecordDisconnect("server closed the connection");
    Events::RecordError("move", "no_path", "unreachable tile");

    const std::vector<Events::Record> recorded = Events::Since(0);
    REQUIRE(recorded.size() == 9);

    CHECK(recorded[0].name == "chat");
    CHECK(FieldsOf(recorded[0])["sender"] == "test2");
    CHECK(FieldsOf(recorded[0])["text"] == "hello");
    CHECK(FieldsOf(recorded[0])["kind"] == "public");

    CHECK(recorded[1].name == "stat");
    CHECK(FieldsOf(recorded[1])["stat"] == "mana");
    CHECK(FieldsOf(recorded[1])["value"] == 40);
    CHECK(FieldsOf(recorded[1])["max"] == 120);
    // A stat without a maximum carries no `max` rather than a fake one.
    CHECK_FALSE(FieldsOf(recorded[2]).contains("max"));

    CHECK(recorded[3].name == "drop");
    CHECK(FieldsOf(recorded[3])["id"] == 9);
    CHECK(FieldsOf(recorded[3])["item"] == "Jewel of Bless");
    CHECK(FieldsOf(recorded[3])["position"] == nlohmann::json::array({140, 130}));

    CHECK(recorded[4].name == "drop_gone");
    CHECK(FieldsOf(recorded[4])["reason"] == "picked");

    CHECK(recorded[5].name == "map");
    CHECK(FieldsOf(recorded[5])["map"] == 2);
    CHECK(FieldsOf(recorded[5])["map_name"] == "Devias");

    CHECK(recorded[6].name == "party");
    CHECK(FieldsOf(recorded[6])["change"] == "joined");

    CHECK(recorded[7].name == "disconnect");
    CHECK(FieldsOf(recorded[7])["reason"] == "server closed the connection");

    CHECK(recorded[8].name == "error");
    CHECK(FieldsOf(recorded[8])["command"] == "move");
    CHECK(FieldsOf(recorded[8])["error"] == "no_path");
}

TEST_CASE("Control events describe the steps of a trade [network][control-events]")
{
    const RingFixture fixture;

    Events::RecordTrade("requested", "testgm2Sum", "");
    Events::RecordTrade("partner_confirm", "", "checked");
    Events::RecordTrade("closed", "", "completed");

    const std::vector<Events::Record> recorded = Events::Since(0);
    REQUIRE(recorded.size() == 3);

    CHECK(recorded[0].name == "trade");
    CHECK(FieldsOf(recorded[0])["change"] == "requested");
    CHECK(FieldsOf(recorded[0])["name"] == "testgm2Sum");
    CHECK_FALSE(FieldsOf(recorded[0]).contains("state"));

    // The confirm button reports its state; the end of a trade its result.
    CHECK(FieldsOf(recorded[1])["state"] == "checked");
    CHECK_FALSE(FieldsOf(recorded[1]).contains("name"));
    CHECK(FieldsOf(recorded[2])["change"] == "closed");
    CHECK(FieldsOf(recorded[2])["result"] == "completed");
}
