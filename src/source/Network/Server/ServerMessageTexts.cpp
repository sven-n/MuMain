#include "stdafx.h"
#include "Network/Server/ServerMessageTexts.h"

#include "I18N/All.h"

#include <cstddef>

namespace Network::ServerMessages
{
namespace
{
// Command 1 numbers from this one on are Blood Castle's lines.
constexpr std::uint8_t kFirstBloodCastleNumber = 20;

// Devil Square's NPC lines (Charon, the Chaos Goblin), by number.
const wchar_t* const* const kDevilSquareNpcLines[] = {
    &I18N::Game::RumorHasItThatRecentlyMonsters,
    &I18N::Game::IfYouWantToFindOutAboutTheDevilSquareGoMeetCharonInNoria,
    &I18N::Game::TheDevilSquareIsAPlace,
    &I18N::Game::YouHaveComeTooSoonYou,
    &I18N::Game::SomeSayThatTheDevilEyes,
    &I18N::Game::ManyAdventurersAndWarriorsAreCrowding,
    &I18N::Game::DoNotYouWantToProve,
    &I18N::Game::ThousandsOfYearsAgoTheDevil,
    &I18N::Game::AreYouLookingForTheDevil,
    &I18N::Game::BringMeTheDevilEyeDevil,
    &I18N::Game::YouVeBroughtTheDevilS,
    &I18N::Game::FinallyTheGateOfDevilSquare,
    &I18N::Game::ManyAdventurersAreLookingForThe,
};

// Blood Castle's lines, by number from kFirstBloodCastleNumber: the NPCs' lines, then the event's
// texts up to "The weapon has been found", which the server sends when the event is finished.
const wchar_t* const* const kBloodCastleNpcLines[] = {
    &I18N::Game::ISeeThatYouHaveThe,
    &I18N::Game::YourCourageIsAdmirableButYou,
    &I18N::Game::YourWillToHelpTheArchangel,
    &I18N::Game::AhGreatWarriorThanksToYour,
    &I18N::Game::YouReAWarriorInTraining,
    &I18N::Game::IfYouThinkYouAreBrave,
    &I18N::Game::YouVeComeToPurchaseA,
    &I18N::Game::HaveYouComeToRepairSomething,
    &I18N::Game::BloodCastleIsAnExtremelyDangerous,
    &I18N::Game::AreYouGoingToTheBloodCastlePleaseHelpOutTheArchangelPlease,
    &I18N::Game::YouLlFindTheScrollOf,
    &I18N::Game::TheArchangelHasBeenProtectingThis,
    &I18N::Game::ItSeemsAsThoughTheOnlyOneWhoCanHelpTheArchangelIsYou,
    &I18N::Game::TheLiquorThatISellHere,
    &I18N::Game::IHeardTheCreaturesInBlood,
    &I18N::Game::TheArchangelIsFightingTheEvil,
    &I18N::Game::MessengerOfArchangel,
    &I18N::Game::CastleDLevelDD,
    &I18N::Game::CastleDOverLevelD,
    &I18N::Game::Archangel,
    &I18N::Game::YouCanEnterSNow,
    &I18N::Game::AfterDMinutesYouMayEnterS,
    &I18N::Game::TheTimeToEnterSHasPassed,
    &I18N::Game::TheMaximumCapacityOfSHasBeenReachedTheMaxNumberAllowedIsD,
    &I18N::Game::TheLevelOfTheCloakOfInvisibilityIsIncorrect,
    &I18N::Game::EvenIfYouDieOrUse,
    &I18N::Game::TheWeaponHasBeenFoundThank,
};

// The Golden Archer's lines, by number.
const wchar_t* const* const kGoldenArcherLines[] = {
    &I18N::Game::TheSeasonOfBlessingHasCome,
    &I18N::Game::RenaIsATypeOfGold,
    &I18N::Game::AfterTheRessurectionOfKundunSome,
    &I18N::Game::YouVeBrought10RenaAs,
    &I18N::Game::RenaEmbodiesAVeryStrongPower,
    &I18N::Game::AfterThenScholarsOfMUStudying,
    &I18N::Game::ButThroughConstantWarAllThe,
    &I18N::Game::AfterTheResurrectionOfKundunI,
    &I18N::Game::BeforeKundunFindsRenaHiddenIn,
    &I18N::Game::June78MULevelUP2003EventWillBeHeldInCoexMall,
    &I18N::Game::AnEventSolvingTheSecretOfHeavenWillBeHeldFromJune7thTo8th,
    &I18N::Game::BringRenaFromTheBoxOfHeaven,
    &I18N::Game::WhenYouBring10RenaYouWillBeGivenANumberBlessedByLugard,
    &I18N::Game::YouCanExchangeTheRegisteredRenaToZen,
    &I18N::Game::TheGoldenArcherWillStayAt,
};

// The Rena hints, by number.
const wchar_t* const* const kRenaLines[] = {
    &I18N::Game::AreYouThrowingRenaOnThe,
    &I18N::Game::RyanTheMaidOfTheBar,
    &I18N::Game::RenaIsATypeOfCoin,
};

// The line `number` of a family, read now in the current language; empty past its last line.
template <std::size_t N> const wchar_t* LineAt(const wchar_t* const* const (&lines)[N], std::size_t number)
{
    if (number >= N)
    {
        return L"";
    }
    return *lines[number];
}
} // namespace

const wchar_t* GetEventNpcLine(std::uint8_t number)
{
    if (number < kFirstBloodCastleNumber)
    {
        return LineAt(kDevilSquareNpcLines, number);
    }
    return LineAt(kBloodCastleNpcLines, static_cast<std::size_t>(number - kFirstBloodCastleNumber));
}

const wchar_t* GetGoldenArcherLine(std::uint8_t number)
{
    return LineAt(kGoldenArcherLines, number);
}

const wchar_t* GetRenaLine(std::uint8_t number)
{
    return LineAt(kRenaLines, number);
}
} // namespace Network::ServerMessages
