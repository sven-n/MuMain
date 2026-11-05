#include "stdafx.h"

#ifdef _EDITOR

#include "Audio/SoundPlaces.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "Render/Effects/ZzzEffect.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <unordered_map>

namespace Audio
{
namespace
{
std::unordered_map<const void*, SoundPlace>& Makers()
{
    static std::unordered_map<const void*, SoundPlace> makers;
    return makers;
}

// Whether `object` is one of the `count` objects from `first` on.
bool IsIn(const OBJECT* object, const OBJECT* first, size_t count)
{
    const std::less<const OBJECT*> before;
    return !before(object, first) && before(object, first + count);
}

// The element of `count` holders from `first` whose `Object` member is
// `object`, or nullptr.
template <typename Holder> const Holder* HolderOf(const OBJECT* object, const Holder* first, size_t count)
{
    if (first == nullptr)
    {
        return nullptr;
    }
    const auto at = reinterpret_cast<std::uintptr_t>(object);
    const auto start = reinterpret_cast<std::uintptr_t>(&first->Object);
    if (at < start || (at - start) % sizeof(Holder) != 0 || (at - start) / sizeof(Holder) >= count)
    {
        return nullptr;
    }
    const Holder* holder = first + (at - start) / sizeof(Holder);
    return &holder->Object == object ? holder : nullptr;
}

std::optional<SoundPlace> CharacterPlace(const CHARACTER& c)
{
    const std::optional<SoundKind> kind = CharacterSoundKind(c.Object.Kind, &c == Hero);
    if (!kind)
    {
        return std::nullopt;
    }
    const bool player = *kind == SoundKind::Hero || *kind == SoundKind::Player;
    const float* position = c.Object.Position;
    return SoundPlace{*kind, player ? -1 : static_cast<int>(c.MonsterIndex),
                      std::array<float, 3>{position[0], position[1], position[2]}};
}

std::array<float, 3> PositionOf(const OBJECT& object)
{
    return {object.Position[0], object.Position[1], object.Position[2]};
}
} // namespace

std::optional<SoundKind> CharacterSoundKind(int objectKind, bool isHero)
{
    switch (objectKind)
    {
    case KIND_PLAYER:
        return isHero ? SoundKind::Hero : SoundKind::Player;
    case KIND_MONSTER:
    case KIND_TRAP:
        return SoundKind::Monster;
    case KIND_NPC:
        return SoundKind::Npc;
    default:
        return std::nullopt;
    }
}

void PlaceCharacter(std::optional<ScopedSoundPlace>& place, const CHARACTER& c)
{
    if (const std::optional<SoundPlace> found = CharacterPlace(c))
    {
        place.emplace(*found);
    }
}

void RememberMaker(const void* effect)
{
    if (const SoundPlace* maker = CurrentSoundPlace())
    {
        Makers()[effect] = SoundPlace{maker->kind, maker->number, std::nullopt};
    }
    else
    {
        Makers().erase(effect);
    }
}

SoundPlace EffectPlace(const void* effect, int type, const float* position)
{
    SoundPlace place{SoundKind::Effect, type, std::array<float, 3>{position[0], position[1], position[2]}};
    if (const auto maker = Makers().find(effect); maker != Makers().end())
    {
        place.kind = maker->second.kind;
        place.number = maker->second.number;
    }
    return place;
}

void PlaceEffect(std::optional<ScopedSoundPlace>& place, const void* effect, int type, const float* position)
{
    place.emplace(EffectPlace(effect, type, position));
}

std::optional<SoundPlace> PlaceOfObject(const OBJECT* object)
{
    if (object == nullptr)
    {
        return std::nullopt;
    }
    if (const CHARACTER* c = HolderOf(object, CharactersClient, MAX_CHARACTERS_CLIENT + 1))
    {
        if (std::optional<SoundPlace> place = CharacterPlace(*c))
        {
            return place;
        }
    }
    if (IsIn(object, Effects, MAX_EFFECTS))
    {
        return EffectPlace(object, object->Type, object->Position);
    }
    if (HolderOf(object, Items, MAX_ITEMS) != nullptr)
    {
        return SoundPlace{SoundKind::Item, object->Type, PositionOf(*object)};
    }
    if (IsIn(object, Boids, MAX_BOIDS) || IsIn(object, Fishs, MAX_FISHS))
    {
        return SoundPlace{SoundKind::Creature, object->Type, PositionOf(*object)};
    }
    if (IsIn(object, Mounts, MAX_MOUNTS) && object->Owner != nullptr)
    {
        if (const CHARACTER* rider = HolderOf(object->Owner, CharactersClient, MAX_CHARACTERS_CLIENT + 1))
        {
            if (std::optional<SoundPlace> place = CharacterPlace(*rider))
            {
                place->position = PositionOf(*object);
                return place;
            }
        }
    }
    return SoundPlace{SoundKind::Other, object->Type, PositionOf(*object)};
}

void ForgetMakers()
{
    Makers().clear();
}
} // namespace Audio

#endif // _EDITOR
