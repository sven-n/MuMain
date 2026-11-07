#include "stdafx.h"
#include "Engine/Object/PlayerBodyModels.h"
#include "Core/Globals/_enum.h"
#include "Data/DataHandler/LoadData.h"

#include <array>
#include <string>

namespace
{
struct BodyPart
{
    int firstSlot;
    const wchar_t* filePrefix;
};

constexpr std::array<BodyPart, 5> BodyParts{{
    {MODEL_BODY_HELM, L"HelmClass"},
    {MODEL_BODY_ARMOR, L"ArmorClass"},
    {MODEL_BODY_PANTS, L"PantClass"},
    {MODEL_BODY_GLOVES, L"GloveClass"},
    {MODEL_BODY_BOOTS, L"BootClass"},
}};

// "HelmClass" for the first form, "HelmClass2" and "HelmClass3" for the others.
std::wstring BodyFileName(const BodyPart& part, int stage)
{
    std::wstring name = part.filePrefix;
    if (stage > 0)
        name += std::to_wstring(stage + 1);
    return name;
}

void OpenBodyPart(int slot, const std::wstring& fileName, int fileNumber, bool useS6PlayerModels)
{
    // An improved file that is missing is skipped quietly; one that is there and fails to load is
    // reported, and the original is used in both cases.
    constexpr bool improvedIsOptional = true;
    if (!useS6PlayerModels && gLoadData.AccessModel(slot, Engine::Object::ImprovedPlayerModelFolder, fileName.c_str(),
                                                    fileNumber, improvedIsOptional))
        return;
    gLoadData.AccessModel(slot, Engine::Object::OriginalPlayerModelFolder, fileName.c_str(), fileNumber);
}
} // namespace

void Engine::Object::OpenPlayerBodyParts(int stage, int baseClass, bool useS6PlayerModels)
{
    const int classOffset = stage * MAX_CLASS + baseClass;
    for (const BodyPart& part : BodyParts)
        OpenBodyPart(part.firstSlot + classOffset, BodyFileName(part, stage), baseClass + 1, useS6PlayerModels);
}
