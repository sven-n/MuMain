#pragma once

#include <string>
#include <string_view>

namespace Data::Monsters
{
// Loads the monster list (Data/Monsters/Monsters.json) into the monster table
// (g_MonsterDatabase).
class MonsterDataHandler
{
public:
    static MonsterDataHandler& GetInstance();

    // Reads the file, logs its problems and the load time, and fills the table
    // with the names shown in `nameLocale`. On errors the table stays as it was
    // and `errorMessage` describes them; the game must not start then.
    bool Load(std::string_view nameLocale, std::string& errorMessage);
};
} // namespace Data::Monsters

#define g_MonsterDataHandler Data::Monsters::MonsterDataHandler::GetInstance()
