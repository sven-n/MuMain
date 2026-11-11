#include "stdafx.h"

#include "UI/Theme/ThemeSelection.h"

#include "Core/Utilities/StringUtils.h"
#include "Data/GameConfig/GameConfig.h"
#include "UI/RmlBridge/RmlTheme.h"

namespace UI::Theme
{
bool Select(const std::string& name, Persistence persistence)
{
    if (!UI::RmlBridge::ThemeExists(name))
        return false;

    auto& config = GameConfig::GetInstance();
    config.SetRmlTheme(StringUtils::NarrowToWide(name));
    if (persistence == Persistence::Saved)
        config.Save();
    UI::RmlBridge::SetActiveThemeName(name);
    UI::RmlBridge::ReloadAllThemedDocuments();
    return true;
}
} // namespace UI::Theme
