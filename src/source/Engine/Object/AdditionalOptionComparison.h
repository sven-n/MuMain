#pragma once

namespace Engine::Items
{
// Compares two copies' additional option (the AT_IMPROVE_* option a Jewel of Life raises) for CompareItem: having it
// beats not having it, and a higher value beats a lower one. Returns one of Engine::Items::CompareResult.
[[nodiscard]] int CompareAdditionalOption(bool firstHasIt, int firstValue, bool secondHasIt, int secondValue);
} // namespace Engine::Items
