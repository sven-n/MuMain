#pragma once

namespace Engine::Object
{
// The original body models, and the improved ones that replace them when the S6 player models are off.
inline constexpr const wchar_t* OriginalPlayerModelFolder = L"Data\\Player\\";
inline constexpr const wchar_t* ImprovedPlayerModelFolder = L"Data\\Player\\Updated\\";

// Opens the five unequipped body parts (helm, armor, pants, gloves, boots) of one class form into
// the body slots: `stage` 0, 1 or 2 is the first, second or third form of `baseClass`. Without the
// S6 player models, each part comes from the improved folder when it has the file and from the
// original folder otherwise, so the improved set is whatever files that folder holds.
void OpenPlayerBodyParts(int stage, int baseClass, bool useS6PlayerModels);
} // namespace Engine::Object
