#include "doctest.h"

#include "Render/Renderer/ReorderableDrawRuns.h"

#include <vector>

namespace
{
struct Command
{
    int key;
    bool opaque;
    int recorded; // the order it was recorded in
};

std::vector<int> Order(std::vector<Command> commands)
{
    Render::GroupReorderableFirst(
        std::span<Command>(commands), [](const Command& command) { return command.opaque; },
        [](const Command& left, const Command& right) { return left.key < right.key; });
    std::vector<int> order;
    for (const Command& command : commands)
        order.push_back(command.recorded);
    return order;
}
} // namespace

TEST_CASE("opaque draws come first, grouped by key [render][reorder]")
{
    CHECK(Order({{2, true, 0}, {1, true, 1}, {2, true, 2}, {1, true, 3}}) == std::vector<int>{1, 3, 0, 2});
}

TEST_CASE("blended draws follow the opaque ones in their recorded order [render][reorder]")
{
    CHECK(Order({{2, true, 0}, {9, false, 1}, {1, true, 2}, {0, false, 3}, {2, true, 4}}) ==
          std::vector<int>{2, 0, 4, 1, 3});
}

TEST_CASE("draws with the same key keep their recorded order [render][reorder]")
{
    CHECK(Order({{5, true, 0}, {5, true, 1}, {5, true, 2}}) == std::vector<int>{0, 1, 2});
}

TEST_CASE("nothing reorderable leaves the list as it was [render][reorder]")
{
    CHECK(Order({{3, false, 0}, {1, false, 1}}) == std::vector<int>{0, 1});
    CHECK(Order({}).empty());
}
