#include <doctest.h>

#include "Render/Renderer/SingleSampleTwins.h"

#include <array>
#include <unordered_map>

namespace
{
struct FakePipeline
{
    int id;
};
} // namespace

TEST_CASE("single-sample twins are complete when every built pipeline has one")
{
    FakePipeline a{1}, b{2}, twinA{11}, twinB{12};
    const std::array<FakePipeline*, 3> multisampled{&a, nullptr, &b};
    const std::unordered_map<FakePipeline*, FakePipeline*> twins{{&a, &twinA}, {&b, &twinB}};
    CHECK(Render::SingleSampleTwinsComplete<FakePipeline>(multisampled, twins));
}

TEST_CASE("single-sample twins are incomplete when one twin failed")
{
    FakePipeline a{1}, b{2}, twinA{11};
    const std::array<FakePipeline*, 2> multisampled{&a, &b};
    std::unordered_map<FakePipeline*, FakePipeline*> twins{{&a, &twinA}};
    CHECK_FALSE(Render::SingleSampleTwinsComplete<FakePipeline>(multisampled, twins));

    twins[&b] = nullptr;
    CHECK_FALSE(Render::SingleSampleTwinsComplete<FakePipeline>(multisampled, twins));
}

TEST_CASE("single-sample twins are incomplete when every twin failed")
{
    FakePipeline a{1};
    const std::array<FakePipeline*, 1> multisampled{&a};
    const std::unordered_map<FakePipeline*, FakePipeline*> twins;
    CHECK_FALSE(Render::SingleSampleTwinsComplete<FakePipeline>(multisampled, twins));
}

TEST_CASE("no built pipelines need no twins")
{
    const std::array<FakePipeline*, 2> multisampled{nullptr, nullptr};
    const std::unordered_map<FakePipeline*, FakePipeline*> twins;
    CHECK(Render::SingleSampleTwinsComplete<FakePipeline>(multisampled, twins));
}
