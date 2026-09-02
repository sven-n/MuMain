#include "doctest.h"

#include "Render/Effects/ParticleDrawOrder.h"

#include <vector>

using Render::Effects::DrawOrder::Entry;
using Render::Effects::DrawOrder::GroupByTexture;

namespace
{
    Entry Free(int particleIndex, int textureKey)
    {
        return Entry{ particleIndex, textureKey, true };
    }

    Entry Pinned(int particleIndex, int textureKey)
    {
        return Entry{ particleIndex, textureKey, false };
    }

    std::vector<int> IndexOrder(const std::vector<Entry>& entries)
    {
        std::vector<int> order;
        order.reserve(entries.size());
        for (const Entry& entry : entries)
        {
            order.push_back(entry.particleIndex);
        }
        return order;
    }

    void Group(std::vector<Entry>& entries)
    {
        std::vector<Entry> scratch(entries.size());
        GroupByTexture(entries.data(), entries.size(), scratch.data());
    }
}

TEST_CASE("particle draw order: reorderable entries are grouped by texture")
{
    std::vector<Entry> entries{ Free(0, 7), Free(1, 3), Free(2, 7), Free(3, 3) };
    Group(entries);

    CHECK(IndexOrder(entries) == std::vector<int>{ 1, 3, 0, 2 });
}

TEST_CASE("particle draw order: grouping is stable within a texture")
{
    // Same-texture particles keep their relative order, so the grouped path produces the
    // same pixels as the unsorted one rather than merely an equivalent-looking result.
    std::vector<Entry> entries{ Free(10, 1), Free(11, 2), Free(12, 1), Free(13, 2), Free(14, 1) };
    Group(entries);

    CHECK(IndexOrder(entries) == std::vector<int>{ 10, 12, 14, 11, 13 });
}

TEST_CASE("particle draw order: a pinned entry keeps its exact position")
{
    std::vector<Entry> entries{ Free(0, 9), Pinned(1, 4), Free(2, 9) };
    Group(entries);

    CHECK(IndexOrder(entries) == std::vector<int>{ 0, 1, 2 });
}

TEST_CASE("particle draw order: nothing is moved across a barrier")
{
    // The two 5-texture particles sit either side of a barrier. Merging them into one
    // batch would mean crossing it, which is exactly what must not happen -- the barrier
    // is there because it changes state every later draw inherits.
    std::vector<Entry> entries{ Free(0, 5), Free(1, 8), Pinned(2, 1), Free(3, 5), Free(4, 8) };
    Group(entries);

    CHECK(IndexOrder(entries) == std::vector<int>{ 0, 1, 2, 3, 4 });
}

TEST_CASE("particle draw order: each stretch between barriers is grouped independently")
{
    std::vector<Entry> entries{
        Free(0, 2), Free(1, 1), Free(2, 2),
        Pinned(3, 0),
        Free(4, 9), Free(5, 6), Free(6, 9),
    };
    Group(entries);

    CHECK(IndexOrder(entries) == std::vector<int>{ 1, 0, 2, 3, 5, 4, 6 });
}

TEST_CASE("particle draw order: an all-pinned sequence is left untouched")
{
    // The classifier is free to be as conservative as it likes; at the limit this is a no-op.
    std::vector<Entry> entries{ Pinned(0, 3), Pinned(1, 1), Pinned(2, 2) };
    Group(entries);

    CHECK(IndexOrder(entries) == std::vector<int>{ 0, 1, 2 });
}

TEST_CASE("particle draw order: barriers at the ends are handled")
{
    std::vector<Entry> entries{ Pinned(0, 9), Free(1, 4), Free(2, 2), Pinned(3, 9) };
    Group(entries);

    CHECK(IndexOrder(entries) == std::vector<int>{ 0, 2, 1, 3 });
}

TEST_CASE("particle draw order: adjacent barriers do not swallow the stretch between them")
{
    std::vector<Entry> entries{ Pinned(0, 1), Pinned(1, 2), Free(2, 8), Free(3, 3), Pinned(4, 4) };
    Group(entries);

    CHECK(IndexOrder(entries) == std::vector<int>{ 0, 1, 3, 2, 4 });
}

TEST_CASE("particle draw order: degenerate inputs are safe")
{
    std::vector<Entry> single{ Free(0, 1) };
    Group(single);
    CHECK(IndexOrder(single) == std::vector<int>{ 0 });

    std::vector<Entry> empty;
    GroupByTexture(empty.data(), 0, empty.data());
    CHECK(empty.empty());

    GroupByTexture(nullptr, 4, single.data());
    GroupByTexture(single.data(), 4, nullptr);
}

TEST_CASE("particle draw order: every particle is drawn exactly once")
{
    // Grouping is a permutation -- no draw may be dropped or duplicated.
    std::vector<Entry> entries;
    for (int i = 0; i < 64; i++)
    {
        const bool pinned = (i % 7) == 0;
        entries.push_back(pinned ? Pinned(i, i % 5) : Free(i, i % 5));
    }
    Group(entries);

    std::vector<int> order = IndexOrder(entries);
    REQUIRE(order.size() == 64);

    std::vector<bool> seen(64, false);
    for (int index : order)
    {
        REQUIRE(index >= 0);
        REQUIRE(index < 64);
        CHECK_FALSE(seen[index]);
        seen[index] = true;
    }
}
