#include "stdafx.h"

#include "Engine/Object/CharacterAttachments.h"

#include "Engine/Object/ZzzCharacter.h"

#include <array>
#include <deque>

namespace
{
struct CharacterSlots
{
    // A deque keeps slots in place as later ones are added.
    std::deque<Engine::Object::Attachments::Slot> slots;
    std::size_t next = 0;
};

std::array<CharacterSlots, MAX_CHARACTERS_CLIENT> s_characters;
unsigned int s_drawFrame = 0;

CharacterSlots* SlotsOf(const CHARACTER* c)
{
    if (CharactersClient == nullptr || c < CharactersClient || c >= CharactersClient + MAX_CHARACTERS_CLIENT)
        return nullptr;
    return &s_characters[static_cast<std::size_t>(c - CharactersClient)];
}

bool IsPartOf(const CHARACTER* c, const void* part)
{
    const auto* begin = reinterpret_cast<const unsigned char*>(c);
    const auto* at = static_cast<const unsigned char*>(part);
    return at >= begin && at < begin + sizeof(CHARACTER);
}
} // namespace

namespace Engine::Object::Attachments
{
void BeginFrame()
{
    ++s_drawFrame;
}

void BeginCharacter(const CHARACTER* c)
{
    if (CharacterSlots* character = SlotsOf(c))
        character->next = 0;
}

Slot* Record(CHARACTER* c, _PART_t* part, int type, float x, float y, float z, int level, int option1, bool link,
             bool translate, int renderType, bool rightHandItem, unsigned char linkBone, float ownerScale)
{
    CharacterSlots* character = SlotsOf(c);
    if (character == nullptr)
        return nullptr;
    if (character->next == character->slots.size())
        character->slots.emplace_back();
    Slot& slot = character->slots[character->next++];
    _PART_t* const own = IsPartOf(c, part) ? part : nullptr;
    slot.sameAsLast = slot.drawnFrame + 1 == s_drawFrame && slot.type == type && slot.part == own && slot.x == x &&
                      slot.y == y && slot.z == z && slot.link == link && slot.rightHandItem == rightHandItem &&
                      slot.linkBone == linkBone && slot.ownerScale == ownerScale;
    slot.type = type;
    slot.x = x;
    slot.y = y;
    slot.z = z;
    slot.level = level;
    slot.option1 = option1;
    slot.link = link;
    slot.translate = translate;
    slot.renderType = renderType;
    slot.rightHandItem = rightHandItem;
    slot.linkBone = linkBone;
    slot.ownerScale = ownerScale;
    slot.part = own;
    slot.drawnFrame = s_drawFrame;
    return &slot;
}

float (*Bones(Slot& slot))[3][4]
{
    if (!slot.bones)
        slot.bones = std::make_unique<vec34_t[]>(MAX_BONES);
    return slot.bones.get();
}

bool WasAdvanced(const Slot& slot)
{
    return slot.part != nullptr && slot.sameAsLast && slot.advancedFor + 1 == s_drawFrame;
}

void ForEachToAdvance(const CHARACTER* c, const std::function<void(Slot&)>& advance)
{
    CharacterSlots* character = SlotsOf(c);
    if (character == nullptr)
        return;
    for (std::size_t i = 0; i < character->next; ++i)
    {
        Slot& slot = character->slots[i];
        if (slot.part == nullptr || slot.drawnFrame != s_drawFrame || slot.advancedFor == s_drawFrame)
            continue;
        slot.advancedFor = s_drawFrame;
        advance(slot);
    }
}
} // namespace Engine::Object::Attachments
