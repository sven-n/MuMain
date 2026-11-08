#include "stdafx.h"
#include "Engine/Object/PlayerBodyModels.h"
#include "Data/DataHandler/LoadData.h"
#include "Core/Utilities/Log/MuLogger.h"

#include <algorithm>
#include <span>
#include <string>
#include <vector>

namespace
{
void OpenBodyPart(int slot, const std::wstring& fileName, bool useS6PlayerModels)
{
    // A missing improved file is skipped quietly; an invalid header is reported by AccessModel.
    constexpr bool improvedIsOptional = true;
    if (!useS6PlayerModels && gLoadData.AccessModel(slot, Engine::Object::ImprovedPlayerModelFolder, fileName.c_str(),
                                                    -1, improvedIsOptional))
        return;
    gLoadData.AccessModel(slot, Engine::Object::OriginalPlayerModelFolder, fileName.c_str());
}

void OpenBodyTextures(int slot, const std::wstring& fileName)
{
    const std::wstring improvedFile = std::wstring(Engine::Object::ImprovedPlayerModelFolder) + fileName + L".bmd";
    if (gLoadData.GetModelFile(slot) != improvedFile)
    {
        gLoadData.OpenTexture(slot, L"Player\\");
        return;
    }

    const std::wstring textureFolder = L"Player\\";
    std::vector<TextureProblem> problems;
    gLoadData.OpenTexture(slot, std::span<const std::wstring>(&textureFolder, 1), problems);
    // A cached texture from another folder is a legitimate fallback. Hidden and unsupported names
    // keep the normal loader's semantics; only a texture it tried but could not load rejects a part.
    const auto missing = std::ranges::find_if(problems, [](const TextureProblem& problem)
                                              { return !problem.unsupportedType && problem.usedInstead.empty(); });
    if (missing == problems.end())
        return;

    MU_LOG_ERROR(mu::log::Get("data"), "Improved player body {} has no loadable texture {}; using original body",
                 mu_wchar_to_utf8(improvedFile.c_str()), mu_wchar_to_utf8(missing->fileName.c_str()));
    // Release also drops any texture references acquired before the failed texture.
    Models[slot].Release();
    gLoadData.AccessModel(slot, Engine::Object::OriginalPlayerModelFolder, fileName.c_str());
    gLoadData.OpenTexture(slot, L"Player\\");
}
} // namespace

std::wstring Engine::Object::PlayerBodyFileName(int stage, const PlayerBodyPart& part, int baseClass)
{
    std::wstring name = part.filePrefix;
    if (stage > 0)
        name += std::to_wstring(stage + 1);
    const int number = baseClass + 1;
    if (number < 10)
        name += L'0';
    return name + std::to_wstring(number);
}

bool Engine::Object::HasPlayerBodyForm(int stage, int baseClass)
{
    // The Magic Gladiator, Dark Lord and Rage Fighter have no second form.
    return stage != 1 || (baseClass != CLASS_DARK && baseClass != CLASS_DARK_LORD && baseClass != CLASS_RAGEFIGHTER);
}

void Engine::Object::OpenPlayerBodyParts(int stage, int baseClass, bool useS6PlayerModels)
{
    const int classOffset = stage * MAX_CLASS + baseClass;
    for (const PlayerBodyPart& part : PlayerBodyParts)
        OpenBodyPart(part.firstSlot + classOffset, PlayerBodyFileName(stage, part, baseClass), useS6PlayerModels);
}

void Engine::Object::OpenPlayerClassBodyParts(int baseClass, bool useS6PlayerModels)
{
    for (int stage = 0; stage < MAX_CLASS_STAGES; ++stage)
    {
        if (HasPlayerBodyForm(stage, baseClass))
            OpenPlayerBodyParts(stage, baseClass, useS6PlayerModels);
    }
}

void Engine::Object::OpenPlayerBodyTextures(int stage, int baseClass)
{
    const int classOffset = stage * MAX_CLASS + baseClass;
    for (const PlayerBodyPart& part : PlayerBodyParts)
        OpenBodyTextures(part.firstSlot + classOffset, PlayerBodyFileName(stage, part, baseClass));
}
