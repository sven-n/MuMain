#include "doctest.h"

#include "Scenes/SceneNames.h"

#include <iterator>
#include <set>
#include <string>

TEST_CASE("Every scene has its own protocol id and display name [core][scenes]")
{
    constexpr EGameScene kScenes[] = {SERVER_LIST_SCENE, WEBZEN_SCENE,    LOG_IN_SCENE,
                                      LOADING_SCENE,     CHARACTER_SCENE, MAIN_SCENE};
    std::set<std::string> ids;
    std::set<std::string> displayNames;
    for (EGameScene scene : kScenes)
    {
        const Scenes::SceneNames names = Scenes::NamesOf(scene);
        CHECK(std::string(names.id) != "unknown");
        ids.insert(names.id);
        displayNames.insert(names.displayName);
    }
    CHECK(ids.size() == std::size(kScenes));
    CHECK(displayNames.size() == std::size(kScenes));
}

TEST_CASE("Scene ids match the control socket protocol [core][scenes]")
{
    CHECK(std::string(Scenes::NamesOf(LOG_IN_SCENE).id) == "login");
    CHECK(std::string(Scenes::NamesOf(CHARACTER_SCENE).id) == "character_list");
    CHECK(std::string(Scenes::NamesOf(MAIN_SCENE).id) == "world");
    CHECK(std::string(Scenes::NamesOf(static_cast<EGameScene>(-1)).id) == "unknown");
}
