#include "stdafx.h"

#include "Engine/Object/CharacterAttachments.h"

#include "Engine/Object/ZzzCharacter.h"

#include <array>
#include <deque>
#include <unordered_map>
#include <vector>

namespace
{
struct CharacterSlots
{
    // A deque keeps slots in place as later ones are added.
    std::deque<Engine::Object::Attachments::Slot> slots;
    std::size_t count = 0;
    // The draw frame the plan is for.
    unsigned int plannedFor = 0;
};

std::array<CharacterSlots, MAX_CHARACTERS_CLIENT> s_characters;
unsigned int s_drawFrame = 0;

// Bones the draws pose parts into themselves this frame; reused from the next frame on.
std::vector<std::unique_ptr<vec34_t[]>> s_drawBones;
std::size_t s_drawBonesUsed = 0;

// Built on first use: owners that are globals track their previews during static initialization.
std::unordered_map<const CHARACTER*, CharacterSlots>& Tracked()
{
    static std::unordered_map<const CHARACTER*, CharacterSlots> tracked;
    return tracked;
}

CharacterSlots* SlotsOf(const CHARACTER* c)
{
    if (CharactersClient != nullptr && c >= CharactersClient && c < CharactersClient + MAX_CHARACTERS_CLIENT)
        return &s_characters[static_cast<std::size_t>(c - CharactersClient)];
    const auto tracked = Tracked().find(c);
    return tracked != Tracked().end() ? &tracked->second : nullptr;
}
} // namespace

namespace Engine::Object::Attachments
{
void BeginFrame()
{
    ++s_drawFrame;
    s_drawBonesUsed = 0;
}

void Track(const CHARACTER* c)
{
    Tracked().try_emplace(c);
}

void ForEachTracked(const std::function<void(const CHARACTER*)>& visit)
{
    for (const auto& [c, slots] : Tracked())
        visit(c);
}

void Untrack(const CHARACTER* c)
{
    Tracked().erase(c);
}

bool BeginPlan(const CHARACTER* c)
{
    CharacterSlots* character = SlotsOf(c);
    if (character == nullptr)
        return false;
    character->count = 0;
    character->plannedFor = s_drawFrame + 1;
    return true;
}

Slot& Plan(CHARACTER* c, _PART_t* part, int type, float x, float y, float z, bool link, bool rightHandItem,
           unsigned char linkBone, float ownerScale, float playSpeed)
{
    CharacterSlots* character = SlotsOf(c);
    if (character->count == character->slots.size())
        character->slots.emplace_back();
    Slot& slot = character->slots[character->count++];
    slot.type = type;
    slot.x = x;
    slot.y = y;
    slot.z = z;
    slot.link = link;
    slot.rightHandItem = rightHandItem;
    slot.linkBone = linkBone;
    slot.ownerScale = ownerScale;
    slot.playSpeed = playSpeed;
    slot.part = IsPartOf(c, part) ? part : nullptr;
    if (slot.part == nullptr)
    {
        if (!slot.copy)
            slot.copy = std::make_shared<_PART_t>();
        *slot.copy = *part;
    }
    if (!slot.bones)
        slot.bones = std::make_unique<vec34_t[]>(MAX_BONES);
    return slot;
}

void ForEachPlanned(const CHARACTER* c, const std::function<void(Slot&)>& visit)
{
    CharacterSlots* character = SlotsOf(c);
    if (character == nullptr)
        return;
    for (std::size_t i = 0; i < character->count; ++i)
        visit(character->slots[i]);
}

vec34_t* PlannedBonesOf(const CHARACTER* c, const _PART_t* part)
{
    CharacterSlots* character = SlotsOf(c);
    if (character == nullptr)
        return nullptr;
    for (std::size_t i = 0; i < character->count; ++i)
    {
        if (character->slots[i].part == part)
            return character->slots[i].bones.get();
    }
    return nullptr;
}

const Slot* Find(const CHARACTER* c, const _PART_t* part, int type, bool rightHandItem)
{
    const CharacterSlots* character = SlotsOf(c);
    if (character == nullptr || character->plannedFor != s_drawFrame)
        return nullptr;
    const _PART_t* own = IsPartOf(c, part) ? part : nullptr;
    for (std::size_t i = 0; i < character->count; ++i)
    {
        const Slot& slot = character->slots[i];
        if (slot.part == own && slot.type == type && slot.rightHandItem == rightHandItem)
            return &slot;
    }
    return nullptr;
}

float (*Bones(const Slot& slot))[3][4]
{
    return slot.bones.get();
}

float (*DrawBones())[3][4]
{
    if (s_drawBonesUsed == s_drawBones.size())
        s_drawBones.push_back(std::make_unique<vec34_t[]>(MAX_BONES));
    return s_drawBones[s_drawBonesUsed++].get();
}

bool HasSlots(const CHARACTER* c)
{
    return SlotsOf(c) != nullptr;
}

bool IsPartOf(const CHARACTER* c, const void* part)
{
    const auto* begin = reinterpret_cast<const unsigned char*>(c);
    const auto* at = static_cast<const unsigned char*>(part);
    return at >= begin && at < begin + sizeof(CHARACTER);
}
} // namespace Engine::Object::Attachments
