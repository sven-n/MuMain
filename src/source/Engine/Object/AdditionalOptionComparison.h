#pragma once

namespace Engine::Items
{
// Compares presence and rank for the Jewel of Life option, including HP recovery, max mana and max AG.
// Callers pass a zero value for an absent option and compare values only within the same option kind.
// Returns one of Engine::Items::CompareResult.
[[nodiscard]] int CompareAdditionalOption(bool firstHasIt, int firstValue, bool secondHasIt, int secondValue);
} // namespace Engine::Items
