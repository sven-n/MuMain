#pragma once

#include "Data/DataHandler/EffectData/EffectTypeStorage.h"
#include "Data/GameData/EffectData/EffectCreateParams.h"

#include <filesystem>
#include <span>

// The shipped effect catalogue (src/bin/Data/Effects) and the effect registry
// as the game builds it from it, for the effect tests. The test target defines
// MU_TEST_DATA_DIR.
namespace EffectTestData
{
// src/bin/Data/Effects.
std::filesystem::path ShippedEffectDirectory();

const Data::Effects::EffectTypesLoadResult& ShippedTypes();

// The registry as the game builds it on the loading screen, without the
// creation values of `withoutCreationValuesOf` (so their legacy cases run
// while they are still in ZzzEffect.cpp) and with `extraRows` in place of the
// rows of their types. A type in `withoutCreationValuesOf` that has no row
// fails the test: both sides of a comparison would run its legacy case.
void BuildShippedRegistry(std::span<const int> withoutCreationValuesOf = {},
                          std::span<const Data::Effects::EffectTypeCreateParams> extraRows = {});
} // namespace EffectTestData
