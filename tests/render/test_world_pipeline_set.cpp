#include <doctest.h>

#include "Render/Renderer/WorldPipelineSet.h"

#include <vector>

namespace
{
struct FakePipeline
{
    int id;
};

using FakeSet = Render::WorldPipelineSet<FakePipeline, 2>;

struct FilledSet
{
    FakePipeline culled[2]{{0}, {1}};
    FakePipeline noCull[2]{{10}, {11}};
    FakePipeline depthOff[2]{{20}, {21}};
    FakePipeline depthReadOnly[2]{{30}, {31}};
    FakeSet set;

    FilledSet()
    {
        for (int index = 0; index < 2; ++index)
        {
            set.culled[index] = &culled[index];
            set.noCull[index] = &noCull[index];
            set.depthOff[index] = &depthOff[index];
            set.depthReadOnly[index] = &depthReadOnly[index];
        }
    }
};
} // namespace

TEST_CASE("world pipeline set selects depth state before culling [render][world_pipeline_set]")
{
    const FilledSet filled;

    CHECK(filled.set.Select(1, false, true, true)->id == 21);
    CHECK(filled.set.Select(1, true, false, true)->id == 31);
    CHECK(filled.set.Select(0, true, true, true)->id == 0);
    CHECK(filled.set.Select(0, true, true, false)->id == 10);
}

TEST_CASE("world pipeline set is complete only with every state of every blend mode [render][world_pipeline_set]")
{
    FilledSet filled;
    CHECK(filled.set.IsComplete());

    filled.set.depthReadOnly[1] = nullptr;
    CHECK_FALSE(filled.set.IsComplete());
    CHECK_FALSE(FakeSet{}.IsComplete());
}

TEST_CASE("world pipeline set releases each pipeline once and clears it [render][world_pipeline_set]")
{
    FilledSet filled;
    filled.set.noCull[0] = nullptr;
    std::vector<int> released;

    filled.set.Release([&](FakePipeline* pipeline) { released.push_back(pipeline->id); });

    CHECK(released.size() == 7);
    CHECK_FALSE(filled.set.culled[0]);
    CHECK_FALSE(filled.set.depthReadOnly[1]);
}
