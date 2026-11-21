#include <doctest.h>

#include "Core/Content/ContentStore.h"
#include "Core/Utilities/TeardownStack.h"
#include "World/MapInfra/MapLoad.h"

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace
{
// Stands in for what a map load's steps hold, like its texture batch: says when it ends.
struct Held
{
    explicit Held(std::string& log) : log(log) {}
    ~Held()
    {
        log += "batch ended;";
    }
    std::string& log;
};

std::vector<Core::StagedLoad::Step> TwoSteps(std::string& log)
{
    auto held = std::make_shared<Held>(log);
    return {[held, &log] { log += "step 1;"; }, [held, &log] { log += "step 2;"; }};
}

bool AfterEachStep()
{
    return true;
}

// A target's pack source for the test: answers `state` for every pack and counts the asks.
struct FakePacks
{
    explicit FakePacks(Content::PackStatus::State state) : state(state)
    {
        Content::SetPackSource(
            [this](std::string_view pack)
            {
                asked = std::string(pack);
                ++asks;
                return Content::PackStatus{.state = this->state, .progress = 0.5f, .error = "HTTP 404"};
            });
    }
    ~FakePacks()
    {
        Content::SetPackSource({});
    }
    FakePacks(const FakePacks&) = delete;
    FakePacks& operator=(const FakePacks&) = delete;

    Content::PackStatus::State state;
    std::string asked;
    int asks = 0;
};
} // namespace

TEST_CASE("Without a pack source, every content pack is ready")
{
    CHECK(Content::RequestPack("map0").state == Content::PackStatus::State::Ready);
}

TEST_CASE("A map's pack is named after the map, as its file list is")
{
    CHECK(World::MapLoad::PackOf(0) == "map0");
    CHECK(World::MapLoad::PackOf(33) == "map33");
}

TEST_CASE("A map load whose pack is ready runs its first step in the first pump")
{
    FakePacks packs(Content::PackStatus::State::Ready);
    std::string log;
    World::MapLoad::Begin("map2", TwoSteps(log), [&log] { log += "after load;"; });
    CHECK(packs.asked == "map2");

    World::MapLoad::Pump(AfterEachStep);
    CHECK(log == "step 1;");
    World::MapLoad::Pump(AfterEachStep);
    CHECK(log == "step 1;step 2;batch ended;after load;");
}

TEST_CASE("A map load waits for a pack still coming, then runs its steps")
{
    FakePacks packs(Content::PackStatus::State::Pending);
    std::string log;
    World::MapLoad::Begin("map3", TwoSteps(log), [&log] { log += "after load;"; });
    CHECK(packs.asks == 1); // asked at the packet, so the download starts then

    World::MapLoad::Pump(AfterEachStep);
    World::MapLoad::Pump(AfterEachStep);
    CHECK(log.empty());
    CHECK(World::MapLoad::Active());

    packs.state = Content::PackStatus::State::Ready;
    World::MapLoad::Pump(AfterEachStep);
    World::MapLoad::Pump(AfterEachStep);
    CHECK(log == "step 1;step 2;batch ended;after load;");
    const int asksWhenReady = packs.asks;
    World::MapLoad::Pump(AfterEachStep);
    CHECK(packs.asks == asksWhenReady);
    CHECK_FALSE(World::MapLoad::TakeFailure());
}

TEST_CASE("A map load whose pack fails runs no step and reports the failure once")
{
    FakePacks packs(Content::PackStatus::State::Failed);
    std::string log;
    World::MapLoad::Begin("map4", TwoSteps(log), [&log] { log += "after load;"; });

    World::MapLoad::Pump(AfterEachStep);
    CHECK(log == "batch ended;");
    CHECK_FALSE(World::MapLoad::Active());
    const std::optional<std::string> failure = World::MapLoad::TakeFailure();
    REQUIRE(failure);
    CHECK(*failure == "map4: HTTP 404");
    CHECK_FALSE(World::MapLoad::TakeFailure());
}

TEST_CASE("A map load cancelled while its pack is coming never runs a step")
{
    FakePacks packs(Content::PackStatus::State::Pending);
    std::string log;
    World::MapLoad::Begin("map5", TwoSteps(log), [&log] { log += "after load;"; });
    World::MapLoad::Pump(AfterEachStep);

    World::MapLoad::Cancel();
    CHECK(log == "batch ended;");
    packs.state = Content::PackStatus::State::Ready;
    World::MapLoad::Pump(AfterEachStep);
    CHECK(log == "batch ended;");
    CHECK_FALSE(World::MapLoad::TakeFailure());
}

TEST_CASE("A map load runs its steps across pumps, then its after-load code once")
{
    std::string log;
    World::MapLoad::Begin("", TwoSteps(log), [&log] { log += "after load;"; });
    CHECK(World::MapLoad::Active());

    World::MapLoad::Pump(AfterEachStep);
    CHECK(log == "step 1;");
    World::MapLoad::Pump(AfterEachStep);
    CHECK(log == "step 1;step 2;batch ended;after load;");
    CHECK_FALSE(World::MapLoad::Active());

    World::MapLoad::Pump(AfterEachStep);
    CHECK(log == "step 1;step 2;batch ended;after load;");
}

TEST_CASE("A map load cancelled by a disconnect never runs its after-load code")
{
    std::string log;
    World::MapLoad::Begin("", TwoSteps(log), [&log] { log += "after load;"; });
    World::MapLoad::Pump(AfterEachStep);

    World::MapLoad::Cancel();
    CHECK(log == "step 1;batch ended;");
    CHECK_FALSE(World::MapLoad::Active());

    World::MapLoad::Pump(AfterEachStep);
    CHECK(log == "step 1;batch ended;");
}

TEST_CASE("Quitting during a map load ends what its steps hold before the renderer's teardown")
{
    std::string log;
    World::MapLoad::Begin("", TwoSteps(log), [&log] { log += "after load;"; });
    World::MapLoad::Pump(AfterEachStep);

    // In startup order, as the client pushes them: the renderer first, leaving the server (and the load) last.
    Core::TeardownStack teardown;
    teardown.Push([&log] { log += "renderer;"; });
    teardown.Push([] { World::MapLoad::Cancel(); });
    teardown.Unwind();

    CHECK(log == "step 1;batch ended;renderer;");
}
