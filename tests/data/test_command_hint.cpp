#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "GameLogic/Character/CommandEffects.h"
#include "I18N/All.h"
#include "UI/Character/CommandHint.h"
#include "UI/Character/CommandHintTexts.h"

#include <filesystem>
#include <string>
#include <vector>

namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

using GameLogic::Character::GetCommandEffects;

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

    const std::vector<UI::RmlBridge::Tooltip::Line> hint = UI::Character::BuildCommandHint(GetCommandEffects(250, 300));
    CHECK(Texts(hint) == std::vector<std::string>{"Command", "Attack rate (PvM): +30", "Dark Raven damage: +37~75",
                                                  "Dark Raven attack speed: +6", "Dark Horse damage: +30",
                                                  "Dark Horse attack speed: +6", "Guild members as guild master: +30"});
    CHECK(hint.front().bold);
    CHECK(hint.front().color == UI::RmlBridge::Tooltip::LineColor::Yellow);

    // German names the pets in the genitive.
    I18N::SetLocale("de");
    const std::vector<std::string> german = Texts(UI::Character::BuildCommandHint(GetCommandEffects(250, 300)));
    CHECK(german[2] == "Schaden des Dunklen Raben: +37~75");
    CHECK(german[4] == "Schaden des Dunklen Pferdes: +30");
}

// A part that adds nothing has no line, like the damage and critical damage rate hints; without any part the row has
// no hint.
TEST_CASE("The command row's hint leaves out what the command does not add [data][items]")
{
    CommandHintScope scope;
    REQUIRE(scope.Loaded());

    // Under 50 command the pets' attack speed gets nothing; at the cap of 80 members the command adds none.
    CHECK(Texts(UI::Character::BuildCommandHint(GetCommandEffects(800, 49))) ==
          std::vector<std::string>{"Command", "Attack rate (PvM): +4", "Dark Raven damage: +6~12",
                                   "Dark Horse damage: +4"});
    CHECK(UI::Character::BuildCommandHint(GetCommandEffects(400, 0)).empty());
    CHECK(UI::Character::BuildCommandHint(GetCommandEffects(400, 3)).empty());
}

// The hint is formatted again only when what it shows changes: the command's effects, the language, or the pets'
// names ("Translate item names", the item editor's names and grammatical forms).
TEST_CASE("The command hint is built again only when what it shows changes [data][items]")
{
    CommandHintScope scope;
    REQUIRE(scope.Loaded());
    UI::Character::CommandHintTexts texts;

    CHECK(texts.Update(GetCommandEffects(250, 300)));
    CHECK_FALSE(texts.Update(GetCommandEffects(250, 300)));
    CHECK(texts.Update(GetCommandEffects(250, 310)));
    CHECK(Texts(texts.Lines())[1] == "Attack rate (PvM): +31");
    CHECK_FALSE(texts.Update(GetCommandEffects(250, 310)));

    I18N::SetLocale("es");
    CHECK(texts.Update(GetCommandEffects(250, 310)));
    CHECK(Texts(texts.Lines())[2] == "Daño del Cuervo Oscuro: +38~77");
    CHECK_FALSE(texts.Update(GetCommandEffects(250, 310)));

    g_ItemDataHandler.SetTranslateItemNames(false);
    CHECK(texts.Update(GetCommandEffects(250, 310)));
    CHECK(Texts(texts.Lines())[2] == "Daño del Dark Raven: +38~77");
    CHECK_FALSE(texts.Update(GetCommandEffects(250, 310)));
}
