#pragma once

#include "GameLogic/Items/GoldAmountTier.h"

// Style keys: the value a data model binds so an RML document can pick a theme-defined look with
// data-class-<name>="<field> == '<key>'", instead of C++ pushing a literal color. The themes'
// .rcss files own what each key looks like.
namespace UI::RmlBridge
{
// Keys of GameLogic::Items::GoldAmountTier -- base.rcss's .gold-* classes.
[[nodiscard]] constexpr const char* GoldTierKey(GameLogic::Items::GoldAmountTier tier)
{
    switch (tier)
    {
    case GameLogic::Items::GoldAmountTier::TenMillion:
        return "ten-million";
    case GameLogic::Items::GoldAmountTier::Million:
        return "million";
    case GameLogic::Items::GoldAmountTier::HundredThousand:
        return "hundred-thousand";
    case GameLogic::Items::GoldAmountTier::Base:
        break;
    }
    return "base";
}
} // namespace UI::RmlBridge
