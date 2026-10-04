#include "stdafx.h"

#include "UI/MuHelper/MuHelperUpdates.h"

#include "UI/Core/WindowSystem.h"
#include "UI/MuHelper/MuHelperConfigWindow.h"

namespace UI::MuHelper
{
void LoadSavedConfig(const MUHelper::ConfigData& config)
{
    g_pMuHelperConfig->LoadSavedConfig(config);
}
}
