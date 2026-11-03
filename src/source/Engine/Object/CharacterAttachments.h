#pragma once

#include "Core/Math/ZzzMathLib.h"

#include <functional>
#include <memory>

class CHARACTER;
struct _PART_t;

// The parts a character carries (weapons, wings, back items, set objects), as its update plans them from the
// character's state: one slot per attachment, in draw order, each with the pose the update advanced it to. Draws
// find their slot and show it; nothing a draw does changes a plan.
namespace Engine::Object::Attachments
{
// The animation state and placement a slot's bones were posed with.
struct Posed
{
    float animationFrame = 0.f;
    float priorAnimationFrame = 0.f;
    unsigned short priorAction = 0;
    unsigned short action = 0;
    float parent[3][4]{};
    vec3_t origin{};
    vec3_t angle{};
    float scale = 0.f;
};

struct Slot
{
    // The model attached, and where (RenderLinkObject's placement arguments).
    int type = -1;
    float x = 0.f;
    float y = 0.f;
    float z = 0.f;
    bool link = false;
    bool rightHandItem = false;
    unsigned char linkBone = 0;
    // The character's scale for this attachment; some draws change it around one part.
    float ownerScale = 1.f;
    // The character's part, or nullptr for a part built for this attachment only (a weapon on the back).
    _PART_t* part = nullptr;
    // That part, for an attachment that has none of its own.
    std::shared_ptr<_PART_t> copy;
    // How fast the update advances the part for this attachment; two attachments can share one part.
    float playSpeed = 0.f;

    Posed posed;
    std::unique_ptr<vec34_t[]> bones;
};

// Starts the frame the next draws show; the last step of the update, after every plan of the frame.
void BeginFrame();
// Gives a character outside CharactersClient (a preview) slots of its own, until Untrack; its owner updates it.
void Track(const CHARACTER* c);
void Untrack(const CHARACTER* c);
// Every character an owner tracks: the previews.
void ForEachTracked(const std::function<void(const CHARACTER*)>& visit);
// Starts the character's plan for the coming draw; false for a character neither in CharactersClient nor tracked.
bool BeginPlan(const CHARACTER* c);
// Adds an attachment to the plan the update is making, in draw order.
Slot& Plan(CHARACTER* c, _PART_t* part, int type, float x, float y, float z, bool link, bool rightHandItem,
           unsigned char linkBone, float ownerScale, float playSpeed);
// The slots of the plan the update just made.
void ForEachPlanned(const CHARACTER* c, const std::function<void(Slot&)>& visit);
// The bones of the first slot the update just planned for this part of the character, or nullptr.
vec34_t* PlannedBonesOf(const CHARACTER* c, const _PART_t* part);
// The slot of the current plan a draw of this part as this model shows; nullptr when the update planned none (the
// character's first drawn frame). A part built for one draw matches a slot planned without a part of its own.
const Slot* Find(const CHARACTER* c, const _PART_t* part, int type, bool rightHandItem);
// The slot's own bones, MAX_BONES long.
float (*Bones(const Slot& slot))[3][4];
// Bones a draw poses a part into itself, MAX_BONES long; valid until the next frame.
float (*DrawBones())[3][4];
// Whether the character has slots: a client character or a tracked preview.
bool HasSlots(const CHARACTER* c);
// Whether `part` is one of the character's own parts.
bool IsPartOf(const CHARACTER* c, const void* part);
} // namespace Engine::Object::Attachments
