#include "stdafx.h"

#include "LoadSettings.h"

namespace Data::ServerList
{
const LoadText* LoadSettings::Text(LoadState state) const
{
    switch (state)
    {
    case LoadState::Full:
        return &full;
    case LoadState::Overloaded:
        return &overloaded;
    case LoadState::Open:
    default:
        return nullptr;
    }
}

LoadText* LoadSettings::Text(LoadState state)
{
    switch (state)
    {
    case LoadState::Full:
        return &full;
    case LoadState::Overloaded:
        return &overloaded;
    case LoadState::Open:
    default:
        return nullptr;
    }
}

LoadSettings GetDefaultLoadSettings()
{
    LoadSettings settings;
    settings.full.text.Set(LocalizedString::NeutralLocale, "(Full)");
    settings.overloaded.text.Set(LocalizedString::NeutralLocale, "Overloaded");
    return settings;
}
} // namespace Data::ServerList
