#include "stdafx.h"
#include "SceneNames.h"

namespace Scenes
{
namespace
{
struct SceneNameEntry
{
    EGameScene scene;
    SceneNames names;
};

constexpr SceneNameEntry kSceneNames[] = {
    {SERVER_LIST_SCENE, {"server_list", "Server list"}},
    {WEBZEN_SCENE, {"webzen", "Intro"}},
    {LOG_IN_SCENE, {"login", "Login"}},
    {LOADING_SCENE, {"loading", "Loading"}},
    {CHARACTER_SCENE, {"character_list", "Character select"}},
    {MAIN_SCENE, {"world", "In game"}},
};

constexpr SceneNames kUnknownScene{"unknown", "Unknown"};
} // namespace

SceneNames NamesOf(EGameScene scene)
{
    for (const SceneNameEntry& entry : kSceneNames)
    {
        if (entry.scene == scene)
            return entry.names;
    }
    return kUnknownScene;
}
} // namespace Scenes
