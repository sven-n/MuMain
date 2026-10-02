#pragma once

#include <string>

namespace Data::Effects
{
// Loads the effect catalogue (Data/Effects) into g_EffectTypeCatalogue and
// logs the load time. On errors nothing is built and `errorMessage` describes
// them; the game must not start then.
bool LoadEffectTypes(std::string& errorMessage);
} // namespace Data::Effects
