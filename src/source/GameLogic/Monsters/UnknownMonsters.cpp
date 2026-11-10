#include "stdafx.h"

#include "GameLogic/Monsters/UnknownMonsters.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Core/Utilities/Random.h"
#include "Data/GameData/Common/DataPaths.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzOpenData.h"
#include "GameLogic/Monsters/MonsterSetupApply.h"
#include "I18N/All.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "UI/Chat/Chat.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cwchar>
#include <iterator>
#include <numbers>
#include <optional>
#include <set>
#include <string_view>
#include <vector>

namespace GameLogic::Monsters::Unknown
{
namespace
{
using Data::Monsters::UnknownMonsterLook;
using Data::Monsters::UnknownReason;

// The glow: its height on the body and its size, and how long a pulse takes.
constexpr float GlowHeight = 80.0f;
constexpr float GlowScale = 2.5f;
constexpr std::uint32_t PulseMs = 1000;
// The marks: a new one this often, each rising this long and this fast, at
// most so many at once, around the body.
constexpr std::uint32_t MarkEveryMs = 400;
constexpr std::uint32_t MarkLifeMs = 1600;
constexpr float MarkRisePerMs = 0.06f;
constexpr std::size_t MaxMarks = 8;
constexpr int MarkSpread = 35;
constexpr int MarkLowest = 60;
constexpr int MarkHeights = 60;
constexpr float MarkScale = 0.6f;

struct Mark
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    std::uint32_t born = 0;
};

// A character Create gave the look, while its slot holds that monster.
struct Shown
{
    CHARACTER* character = nullptr;
    int number = 0;
    int key = 0;
    bool said = false;
    std::uint32_t lastSaid = 0;
    std::uint32_t lastMark = 0;
    std::vector<Mark> marks;
};

std::vector<Shown> g_shown;
std::uint32_t (*g_clock)() = nullptr;
void (*g_say)(CHARACTER& character, const wchar_t* text) = nullptr;

std::uint32_t Now()
{
    return g_clock != nullptr ? g_clock() : static_cast<std::uint32_t>(timeGetTime());
}

std::string_view Why(UnknownReason reason)
{
    switch (reason)
    {
    case UnknownReason::NoEntry:
        return "the monster list has no entry for it";
    case UnknownReason::NoModel:
        return "its entry names no model";
    case UnknownReason::MissingFile:
        return "its model's file is missing";
    }
    return "";
}

// Each number the game cannot show as itself, once in the log: with the look,
// or with the Bull Fighter's body when the look is not for its reason.
void LogOnce(int number, UnknownReason reason, bool look)
{
    static std::set<int> logged;
    if (!logged.insert(number).second)
        return;
    MU_LOG_WARN(mu::log::Get("data"), "Monster {} has {}: {}", number,
                look ? "the unknown monsters' look" : "the Bull Fighter's body", Why(reason));
}

std::vector<Shown>::iterator Find(const CHARACTER& character)
{
    return std::ranges::find(g_shown, &character, &Shown::character);
}

// The marks' texture, loaded at the first mark; nullopt when its file is
// missing (logged once by the loader).
std::optional<GLuint> MarkTexture()
{
    static std::optional<GLuint> texture;
    static bool tried = false;
    if (!tried)
    {
        tried = true;
        const GLuint loaded = Bitmaps.LoadImage(Data::Paths::ToLoaderPath("Data/Effect/UnknownMonsterMark.tga"),
                                                GL_LINEAR, GL_CLAMP_TO_EDGE);
        if (loaded != BITMAP_UNKNOWN)
            texture = loaded;
    }
    return texture;
}

void ShowGlow(OBJECT& object, const UnknownMonsterLook& look, std::uint32_t now)
{
    const float phase = static_cast<float>(now % PulseMs) / static_cast<float>(PulseMs);
    const float pulse = 0.5f + 0.5f * std::sin(phase * 2.0f * std::numbers::pi_v<float>);
    vec3_t position;
    Vector(object.Position[0], object.Position[1], object.Position[2] + GlowHeight * object.Scale, position);
    vec3_t light;
    const float strength = 0.4f + 0.6f * pulse;
    Vector(look.glow[0] * strength, look.glow[1] * strength, look.glow[2] * strength, light);
    CreateSprite(BITMAP_LIGHT, position, GlowScale * object.Scale * (0.85f + 0.3f * pulse), light, &object);
}

void ShowMarks(Shown& shown, const UnknownMonsterLook& look, std::uint32_t now)
{
    OBJECT& object = shown.character->Object;
    std::erase_if(shown.marks, [now](const Mark& mark) { return now - mark.born >= MarkLifeMs; });
    if (shown.marks.size() < MaxMarks && (shown.marks.empty() || now - shown.lastMark >= MarkEveryMs))
    {
        Mark mark;
        mark.x = static_cast<float>(Random::Rand() % (2 * MarkSpread + 1) - MarkSpread) * object.Scale;
        mark.y = static_cast<float>(Random::Rand() % (2 * MarkSpread + 1) - MarkSpread) * object.Scale;
        mark.z = static_cast<float>(MarkLowest + Random::Rand() % MarkHeights) * object.Scale;
        mark.born = now;
        shown.marks.push_back(mark);
        shown.lastMark = now;
    }
    const std::optional<GLuint> texture = MarkTexture();
    if (!texture)
        return;
    for (const Mark& mark : shown.marks)
    {
        const std::uint32_t age = now - mark.born;
        // Fades in and out as it rises.
        const float fade =
            std::sin(static_cast<float>(age) / static_cast<float>(MarkLifeMs) * std::numbers::pi_v<float>);
        vec3_t position;
        Vector(object.Position[0] + mark.x, object.Position[1] + mark.y,
               object.Position[2] + mark.z + MarkRisePerMs * static_cast<float>(age), position);
        vec3_t light;
        Vector(look.glow[0] * fade, look.glow[1] * fade, look.glow[2] * fade, light);
        CreateSprite(static_cast<int>(*texture), position, MarkScale * object.Scale, light, &object);
    }
}

void SayInBubble(CHARACTER& character, const wchar_t* text)
{
    UI::Chat::CreateChat(character.ID, text, &character);
}

void Say(Shown& shown, const UnknownMonsterLook& look, std::uint32_t now)
{
    const bool again =
        look.messageSeconds > 0 && now - shown.lastSaid >= static_cast<std::uint32_t>(look.messageSeconds) * 1000u;
    if (shown.said && !again)
        return;
    shown.said = true;
    shown.lastSaid = now;
    wchar_t text[128];
    std::swprintf(text, std::size(text), I18N::Game::UnknownMonsterMessage, shown.number);
    (g_say != nullptr ? g_say : SayInBubble)(*shown.character, text);
}
} // namespace

CHARACTER* Create(int number, UnknownReason reason, int positionX, int positionY, int key)
{
    const UnknownMonsterLook& look = g_MonsterDatabase.GetUnknownLook();
    const std::optional<int> slot =
        IsFor(look, reason) ? Data::Monsters::FindMonsterModelReference(look.model) : std::nullopt;
    if (!slot)
    {
        LogOnce(number, reason, false);
        OpenMonsterModel(MONSTER_MODEL_BULL_FIGHTER);
        return CreateCharacter(key, MODEL_BULL_FIGHTER, static_cast<unsigned char>(positionX),
                               static_cast<unsigned char>(positionY));
    }
    Setup::OpenModel(*slot);
    CHARACTER* character =
        CreateCharacter(key, *slot, static_cast<unsigned char>(positionX), static_cast<unsigned char>(positionY));
    character->Object.Scale = look.scale;
    LogOnce(number, reason, true);
    Shown shown;
    shown.character = character;
    shown.number = number;
    shown.key = key;
    g_shown.push_back(std::move(shown));
    return character;
}

void Forget(const CHARACTER& character)
{
    if (const auto found = Find(character); found != g_shown.end())
        g_shown.erase(found);
}

void ShowOnScreen(CHARACTER& character)
{
    if (g_shown.empty())
        return;
    const auto found = Find(character);
    if (found == g_shown.end())
        return;
    // The slot holds another monster now.
    if (found->number != character.MonsterIndex || found->key != character.Key)
    {
        g_shown.erase(found);
        return;
    }
    const UnknownMonsterLook& look = g_MonsterDatabase.GetUnknownLook();
    const std::uint32_t now = Now();
    ShowGlow(character.Object, look, now);
    if (look.marks)
        ShowMarks(*found, look, now);
    else
        found->marks.clear();
    Say(*found, look, now);
}

void SetClock(std::uint32_t (*clock)())
{
    g_clock = clock;
}

void SetSpeaker(void (*say)(CHARACTER& character, const wchar_t* text))
{
    g_say = say;
}
} // namespace GameLogic::Monsters::Unknown
