#pragma once

#include "Core/Globals/_enum.h"

#include <array>
#include <string>

namespace Engine::Object
{
// The original body models, and the improved ones that replace them when the S6 player models are off.
inline constexpr const wchar_t* OriginalPlayerModelFolder = L"Data\\Player\\";
inline constexpr const wchar_t* ImprovedPlayerModelFolder = L"Data\\Player\\Updated\\";

struct PlayerBodyPart
{
    int firstSlot;
    const wchar_t* filePrefix;
};

inline constexpr std::array<PlayerBodyPart, 5> PlayerBodyParts{{
    {MODEL_BODY_HELM, L"HelmClass"},
    {MODEL_BODY_ARMOR, L"ArmorClass"},
    {MODEL_BODY_PANTS, L"PantClass"},
    {MODEL_BODY_GLOVES, L"GloveClass"},
    {MODEL_BODY_BOOTS, L"BootClass"},
}};

// The file stem, e.g. HelmClass201 for the second form of the Dark Wizard.
std::wstring PlayerBodyFileName(int stage, const PlayerBodyPart& part, int baseClass);
bool HasPlayerBodyForm(int stage, int baseClass);

// Opens the five unequipped body parts into the slots of one class form. Each part comes from
// the improved folder when present and from the original otherwise. Textures are opened later.
void OpenPlayerBodyParts(int stage, int baseClass, bool useS6PlayerModels);
void OpenPlayerClassBodyParts(int baseClass, bool useS6PlayerModels);

// Uses the normal Player texture lookup; an improved part with an unresolved texture is replaced
// by its original, without a missing-texture dialog for the optional improved part.
void OpenPlayerBodyTextures(int stage, int baseClass);
} // namespace Engine::Object
