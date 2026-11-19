#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "GameLogic/Character/CommandEffects.h"
#include "GameLogic/Pets/PetStats.h"
#include "I18N/All.h"
#include "UI/Character/CommandHint.h"
#include "UI/Character/CommandHintTexts.h"
#include "UI/Character/TextsChangedWatch.h"

#include <filesystem>
#include <string>
#include <vector>

namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

// The command's effects with the shipped pet stats, for a Dark Lord without pets and without strength, so the
// horse's maximum damage gets exactly half again the command's part of its minimum.
GameLogic::Character::CommandEffects Effects(int level, int command)
{
    const GameLogic::Character::DarkLordPets pets{
        GameLogic::Character::DarkLordPet{GameLogic::Pets::GetItemPetStats(ITEM_DARK_RAVEN_ITEM), 0, 0},
        GameLogic::Character::DarkLordPet{GameLogic::Pets::GetItemPetStats(ITEM_DARK_HORSE_ITEM), 0, 0}};
    return GameLogic::Character::GetCommandEffects(level, command, pets);
}

// The item data loaded (the hint names the pets by their item names) in English, with translated item names; puts
// both back at the end.
class CommandHintScope
{
public:
    CommandHintScope() : m_client(ClientDirectory)
    {
        I18N::SetLocale("en");
        std::string errorMessage;
        m_loaded = g_ItemDataHandler.Load(errorMessage);
    }

    ~CommandHintScope()
    {
        g_ItemDataHandler.SetTranslateItemNames(true);
        I18N::SetLocale("en");
    }

    bool Loaded() const
    {
        return m_loaded;
    }

private:
    ClientDataScope m_client;
    bool m_loaded = false;
};

std::vector<std::string> Texts(const std::vector<UI::RmlBridge::Tooltip::Line>& lines)
{
    std::vector<std::string> texts;
    for (const UI::RmlBridge::Tooltip::Line& line : lines)
    {
        texts.push_back(line.text);
    }
    return texts;
}
} // namespace

// The command row's hint lists what the command adds, the pets by their item names, and the guild members the command
// adds to the level's.
TEST_CASE("The command row's hint lists what the Dark Lord's command adds [data][items]")
{
    CommandHintScope scope;
    REQUIRE(scope.Loaded());

    const std::vector<UI::RmlBridge::Tooltip::Line> hint = UI::Character::BuildCommandHint(Effects(250, 300));
    CHECK(Texts(hint) == std::vector<std::string>{"Command", "Attack rate (PvM): +30", "Dark Raven damage: +37~75",
                                                  "Dark Raven attack speed: +6", "Dark Horse damage: +30~45",
                                                  "Dark Horse attack speed: +6", "Guild members as guild master: +30"});
    CHECK(hint.front().bold);
    CHECK(hint.front().color == UI::RmlBridge::Tooltip::LineColor::Yellow);

    // German names the pets in the genitive.
    I18N::SetLocale("de");
    const std::vector<std::string> german = Texts(UI::Character::BuildCommandHint(Effects(250, 300)));
    CHECK(german[2] == "Schaden des Dunklen Raben: +37~75");
    CHECK(german[4] == "Schaden des Dunklen Pferdes: +30~45");
}

// A part that adds nothing has no line, like the damage and critical damage rate hints; without any part the row has
// no hint.
TEST_CASE("The command row's hint leaves out what the command does not add [data][items]")
{
    CommandHintScope scope;
    REQUIRE(scope.Loaded());

    // Under 50 command the pets' attack speed gets nothing; at the cap of 80 members the command adds none.
    CHECK(Texts(UI::Character::BuildCommandHint(Effects(800, 49))) ==
          std::vector<std::string>{"Command", "Attack rate (PvM): +4", "Dark Raven damage: +6~12",
                                   "Dark Horse damage: +4~6"});
    CHECK(UI::Character::BuildCommandHint(Effects(400, 0)).empty());
    CHECK(UI::Character::BuildCommandHint(Effects(400, 3)).empty());

    GameLogic::Character::CommandEffects minimumOnly;
    minimumOnly.raven.minimumDamage = 37;
    minimumOnly.horse.minimumDamage = 30;
    CHECK(Texts(UI::Character::BuildCommandHint(minimumOnly)) ==
          std::vector<std::string>{"Command", "Dark Raven damage: +37~0", "Dark Horse damage: +30~0"});
}

// The hint is formatted again only when the command's effects change, or when the window invalidates it after its
// TextsChangedWatch saw a language switch or a refresh of the item texts (the pets' names: "Translate item names", the
// item editor), as the window's sync does.
TEST_CASE("The command hint is built again only when what it shows changes [data][items]")
{
    CommandHintScope scope;
    REQUIRE(scope.Loaded());
    UI::Character::CommandHintTexts texts;
    UI::Character::TextsChangedWatch watch;
    // What the window's sync does before it builds the hint.
    const auto sync = [&texts, &watch]
    {
        if (watch.TakeChanged())
        {
            texts.Invalidate();
        }
    };

    sync();
    CHECK(texts.Update(Effects(250, 300)));
    CHECK_FALSE(texts.Update(Effects(250, 300)));
    CHECK(texts.Update(Effects(250, 310)));
    CHECK(Texts(texts.Lines())[1] == "Attack rate (PvM): +31");
    sync();
    CHECK_FALSE(texts.Update(Effects(250, 310)));

    I18N::SetLocale("es");
    sync();
    CHECK(texts.Update(Effects(250, 310)));
    CHECK(Texts(texts.Lines())[2] == "Daño del Cuervo Oscuro: +38~77");
    sync();
    CHECK_FALSE(texts.Update(Effects(250, 310)));

    g_ItemDataHandler.SetTranslateItemNames(false);
    sync();
    CHECK(texts.Update(Effects(250, 310)));
    CHECK(Texts(texts.Lines())[2] == "Daño del Dark Raven: +38~77");
    sync();
    CHECK_FALSE(texts.Update(Effects(250, 310)));
}

// The pet stats moved from CalcPetInfo into the item data: the shipped Dark Raven and Dark Horse give what the code
// gave, for every pet level, and for strength and command from 0 up.
TEST_CASE("The shipped pets' stats give what the pet window's code gave [data][items]")
{
    CommandHintScope scope;
    REQUIRE(scope.Loaded());
    const Data::Items::ItemPetStats& raven = GameLogic::Pets::GetItemPetStats(ITEM_DARK_RAVEN_ITEM);
    const Data::Items::ItemPetStats& horse = GameLogic::Pets::GetItemPetStats(ITEM_DARK_HORSE_ITEM);
    REQUIRE_FALSE(raven.IsEmpty());
    REQUIRE_FALSE(horse.IsEmpty());

    constexpr int MaxPetLevel = 50;
    constexpr int StatStep = 37;
    constexpr int MaxStat = 32767;
    int mismatches = 0;
    for (int level = 0; level <= MaxPetLevel; ++level)
    {
        for (int strength = 0; strength <= MaxStat; strength += StatStep * 11)
        {
            for (int command = 0; command <= MaxStat; command += StatStep)
            {
                const Data::Items::PetStatInputs inputs{level, strength, command};
                const Data::Items::PetStatValues oldRaven{
                    180 + level * 15 + command / 8, 200 + level * 15 + command / 4, 20 + level * 4 / 5 + command / 50};
                const int horseMinimum = strength / 10 + command / 10 + level * 5;
                const Data::Items::PetStatValues oldHorse{horseMinimum, horseMinimum + horseMinimum / 2,
                                                          20 + level * 4 / 5 + command / 50};
                mismatches += Data::Items::EvaluatePetStats(raven, inputs) != oldRaven;
                mismatches += Data::Items::EvaluatePetStats(horse, inputs) != oldHorse;
            }
        }
    }
    CHECK(mismatches == 0);
}
