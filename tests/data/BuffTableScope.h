#pragma once

#include "Data/GameData/BuffData/BuffDatabase.h"

#include <string_view>
#include <vector>

// Puts the buffs into the buff table for one test case, and empties it again when it goes, also when one of its checks
// stops the test.
class BuffTableScope
{
public:
    BuffTableScope(const std::vector<Data::Buffs::BuffDefinition>& buffs, std::string_view locale)
    {
        g_BuffDatabase.Build(buffs, locale);
    }

    ~BuffTableScope()
    {
        g_BuffDatabase.Build({}, "en");
    }

    BuffTableScope(const BuffTableScope&) = delete;
    BuffTableScope& operator=(const BuffTableScope&) = delete;
};
