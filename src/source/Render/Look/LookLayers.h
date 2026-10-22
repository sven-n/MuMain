#pragma once

#include "Render/Look/LookFiles.h"
#include "Render/Look/TuningControls.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

// The live look profile and light mood with everything layered over them: the player's values (of
// the groups whose feature is in effect) and the values a developer changed this session, per scope.
// Every change invalidates the effective values before anything reads them again: the light mood's
// rebuild, which reads the look values, sees the change it was started for. Render/Look/
// LookProfileSetting.h keeps the client's one instance.
namespace Render::Look
{

class LookLayers
{
public:
    // Rebuilds the map light from the live light mood (Lighting::LightMood::SetMoodData).
    using MoodRebuild = std::function<void(const Data::Maps::LightMoodData&)>;

    explicit LookLayers(MoodRebuild rebuildMood = {});

    // New files (start-up, `$look load`): live and loaded both, no developer edits, mood rebuilt.
    void UseFiles(LookFiles files);

    // The effective values on `world`, cached until a change or another world.
    [[nodiscard]] const Data::Maps::LookValues& Active(int world);
    // Changes whenever the effective values may have (not with the world).
    [[nodiscard]] std::uint32_t Revision() const
    {
        return m_revision;
    }

    void SetPersonal(const Data::Maps::LookOverrides& personal);
    void SetApplyingGroups(const TuningGroupSet& applying);
    [[nodiscard]] const Data::Maps::LookOverrides& Personal() const
    {
        return m_personal;
    }
    [[nodiscard]] const TuningGroupSet& ApplyingGroups() const
    {
        return m_applying;
    }

    // The values on `world` without the player's.
    [[nodiscard]] Data::Maps::LookValues Inherited(int world) const;
    // Every value (or one) on `world` with where it comes from, a player's value included.
    [[nodiscard]] std::vector<LookValueState> Describe(int world, std::optional<std::string_view> name) const;

    // `$look set` (no `world`) or `$look set map` on `world`: refused edits change nothing.
    LookEditResult Set(std::span<const LookEdit> edits, std::optional<int> world, std::string_view mapName = {});
    // `$look reset`: every value, or `name` in the default and `world`'s entry, with their edit marks.
    LookEditResult Reset(std::optional<std::string_view> name, int world);
    // `$look save` wrote the live files: a reset goes back to them, and the edits saved in them are
    // no longer marked, so the player's values apply over them again.
    void MarkSaved();

    [[nodiscard]] const LookFiles& Live() const
    {
        return m_live;
    }
    [[nodiscard]] const DeveloperLookEdits& DeveloperEdits() const
    {
        return m_developerEdits;
    }

private:
    void Changed(bool moodChanged);

    MoodRebuild m_rebuildMood;
    LookFiles m_live;
    LookFiles m_loaded;
    Data::Maps::LookOverrides m_personal;
    // None applies until the look's features are applied (Classic keeps the player's values unseen).
    TuningGroupSet m_applying{};
    DeveloperLookEdits m_developerEdits;
    std::uint32_t m_revision = 0;
    bool m_cacheValid = false;
    int m_cacheWorld = 0;
    Data::Maps::LookValues m_values = Data::Maps::BuiltInLookValues();
};

} // namespace Render::Look
