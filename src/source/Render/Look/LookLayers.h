#pragma once

#include "Render/Look/LookFiles.h"
#include "Render/Look/TuningControls.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

// The live look profile and light mood with the player's values (of the groups whose feature is in
// effect) layered over them, unless the Authoring preview leaves them out. Every change invalidates the effective values before anything reads them again: the light mood's
// rebuild, which reads the look values, sees the change it was started for. Render/Look/
// LookProfileSetting.h keeps the client's one instance.
namespace Render::Look
{

class LookLayers
{
public:
    // Rebuilds the map light from every map's mood in the live profile (Lighting::LightMood::SetMoodData).
    using MoodRebuild = std::function<void(const Data::Maps::LightMoodData&)>;

    explicit LookLayers(MoodRebuild rebuildMood = {});

    // New files (start-up, `$look load`): live and loaded both, mood rebuilt. The preview stays.
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

    // What the effective values preview. A change rebuilds the mood: the player's values and light mood
    // strength come or go.
    void SetPreview(LookPreview preview);
    [[nodiscard]] LookPreview Preview() const
    {
        return m_preview;
    }

    // The values on `world` without the player's.
    [[nodiscard]] Data::Maps::LookValues Inherited(int world) const;
    // Every value (or one) on `world` with where it comes from, a player's value included while it applies.
    [[nodiscard]] std::vector<LookValueState> Describe(int world, std::optional<std::string_view> name) const;

    // The edits below switch the Player preview to Authoring: a developer editing the look sees the
    // profile as authored, not covered by their own player values.

    // `$look set` (no `world`) or `$look set map` on `world`: refused edits change nothing.
    LookEditResult Set(std::span<const LookEdit> edits, std::optional<int> world, std::string_view mapName = {});
    // `$look reset`: every value, or `name` in the default and `world`'s entry.
    LookEditResult Reset(std::optional<std::string_view> name, int world);
    // The Graphics Look window's change, undo or redo: `profile` becomes the live profile when the
    // files can be edited and it passes ValidateLookProfile.
    LookEditResult Replace(const Data::Maps::LookProfileData& profile);
    // `$look save` wrote the live files: a reset goes back to them.
    void MarkSaved();

    [[nodiscard]] const LookFiles& Live() const
    {
        return m_live;
    }
    // The files as loaded or last saved.
    [[nodiscard]] const LookFiles& Loaded() const
    {
        return m_loaded;
    }
    // Changes with every UseFiles: other files are live, so an edit history of the old ones ends.
    [[nodiscard]] std::uint32_t FilesGeneration() const
    {
        return m_filesGeneration;
    }

private:
    void Changed(bool moodChanged);
    // An applied edit: the Player preview becomes Authoring; true when it did.
    bool StartAuthoring();
    // The groups whose player values apply in the preview.
    [[nodiscard]] TuningGroupSet Applying() const;

    MoodRebuild m_rebuildMood;
    LookFiles m_live;
    LookFiles m_loaded;
    Data::Maps::LookOverrides m_personal;
    // None applies until the look's features are applied (Classic keeps the player's values unseen).
    TuningGroupSet m_applying{};
    LookPreview m_preview = LookPreview::Player;
    std::uint32_t m_revision = 0;
    std::uint32_t m_filesGeneration = 0;
    bool m_cacheValid = false;
    int m_cacheWorld = 0;
    Data::Maps::LookValues m_values = Data::Maps::BuiltInLookValues();
};

} // namespace Render::Look
