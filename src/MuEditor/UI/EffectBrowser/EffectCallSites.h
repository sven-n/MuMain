#pragma once

#ifdef _EDITOR

#include "EffectPreviewCamera.h"
#include "Data/GameData/EffectData/EffectKind.h"

#include <cstddef>
#include <filesystem>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace MuEditor::Effects
{
// A call of the game that creates an effect, particle, lightning or sprite
// (CreateEffect, CreateParticle, CreateJoint, CreateSprite and their
// FpsChecked forms), read from the source text, with the values it writes
// out.
struct EffectCallSite
{
    Data::Effects::EffectKind kind = Data::Effects::EffectKind::Effect;
    int type = 0;
    // The call picks its type at random among this many (NAME + rand() % N).
    int randomTypes = 1;
    // The file relative to the source folder, with '/', and its line.
    std::string file;
    int line = 0;
    // The arguments as written; empty when the call leaves them out.
    std::string subType;
    std::string scale;
    // The owner (effects, particles, sprites) or the target (lightning).
    std::string owner;
    // The values of the arguments written as numbers; a SubType or a scale
    // left out gets the call's default.
    std::optional<int> subTypeValue;
    std::optional<float> scaleValue;
    // The light: the last write of the call's light variable that reaches
    // the call in its function, Vector(r, g, b, light) or a declaration with
    // r, g, b; for lightning, the colour it passes. Unknown when that write
    // is anything else or runs only on some paths.
    std::optional<PreviewVector> light;
    bool withoutOwner = false;
    // Lightning only: the values some SubTypes read from PK and SkillIndex.
    std::optional<int> pkValue;
    std::optional<int> skillIndexValue;
};

// Whether one of the calls creates its type with this SubType, written as a
// number, and no owner. The code of other SubTypes may read their owner
// without checking it.
bool CreatesWithoutOwner(std::span<const EffectCallSite* const> calls, int subType);

// The calls in the text of one source file. Code under #if 0, and under
// #ifdef or #if defined of a macro of `macrosOff`, is left out.
std::vector<EffectCallSite> ReadEffectCallSites(std::string_view source, std::string_view file,
                                                const std::unordered_set<std::string>& macrosOff = {});

// The calls of every .cpp file under a source folder, by kind and type.
class EffectCallSiteIndex
{
public:
    // Reads the folder; false when it holds no source. A macro that a file
    // tests but none defines, and that the build files beside the folder do
    // not name, counts as off.
    bool Load(const std::filesystem::path& sourceDirectory);
    bool IsLoaded() const
    {
        return m_loaded;
    }
    std::vector<const EffectCallSite*> Find(Data::Effects::EffectKind kind, int type) const;
    std::size_t Count() const
    {
        return m_calls.size();
    }

private:
    std::vector<EffectCallSite> m_calls;
    std::map<std::pair<int, int>, std::vector<std::size_t>> m_byType;
    bool m_loaded = false;
};
} // namespace MuEditor::Effects

#endif // _EDITOR
