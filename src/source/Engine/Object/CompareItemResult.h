#pragma once

// What CompareItem(item1, item2) returns (ZzzInventory.cpp). It compares the level, the skill, the excellent options
// as one bit-flag number, the number of other options, the additional option and then the durability; the first
// difference decides. It never reads the Jewel of Harmony option, the 380 option, the sockets or the ancient set, so
// copies that differ only there count as the same.
namespace Engine::Items::CompareResult
{
constexpr int FirstIsBetter = 1;
constexpr int Same = 0;
constexpr int SecondIsBetter = -1;
// The two items are of different types and were not compared.
constexpr int OtherType = 2;
} // namespace Engine::Items::CompareResult
