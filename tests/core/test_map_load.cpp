#include <doctest.h>

#include "Core/Utilities/TeardownStack.h"
#include "World/MapInfra/MapLoad.h"

#include <memory>
#include <string>
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
} // namespace

TEST_CASE("A map load runs its steps across pumps, then its after-load code once")
{
    std::string log;
    World::MapLoad::Begin(TwoSteps(log), [&log] { log += "after load;"; });
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
    World::MapLoad::Begin(TwoSteps(log), [&log] { log += "after load;"; });
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
    World::MapLoad::Begin(TwoSteps(log), [&log] { log += "after load;"; });
    World::MapLoad::Pump(AfterEachStep);

    // In startup order, as the client pushes them: the renderer first, leaving the server (and the load) last.
    Core::TeardownStack teardown;
    teardown.Push([&log] { log += "renderer;"; });
    teardown.Push([] { World::MapLoad::Cancel(); });
    teardown.Unwind();

    CHECK(log == "step 1;batch ended;renderer;");
}
