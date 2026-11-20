#include "stdafx.h"

#include "Render/Look/LookLayers.h"

#include <utility>

namespace Render::Look
{

LookLayers::LookLayers(MoodRebuild rebuildMood) : m_rebuildMood(std::move(rebuildMood)) {}

void LookLayers::Changed(bool moodChanged)
{
    ++m_revision;
    m_cacheValid = false;
    if (moodChanged && m_rebuildMood)
    {
        m_rebuildMood(Data::Maps::MoodsOf(m_live.lookData));
    }
}

void LookLayers::UseFiles(LookFiles files)
{
    m_live = files;
    m_loaded = std::move(files);
    ++m_filesGeneration;
    Changed(true);
}

bool LookLayers::StartAuthoring()
{
    if (m_preview == LookPreview::Authoring)
    {
        return false;
    }
    m_preview = LookPreview::Authoring;
    return true;
}

TuningGroupSet LookLayers::Applying() const
{
    return m_preview == LookPreview::Authoring ? TuningGroupSet{} : m_applying;
}

void LookLayers::SetPreview(LookPreview preview)
{
    if (preview == m_preview)
    {
        return;
    }
    m_preview = preview;
    Changed(true);
}

LookEditResult LookLayers::Replace(const Data::Maps::LookProfileData& profile)
{
    LookEditResult result = ReplaceLookProfile(m_live, profile);
    if (result.applied)
    {
        Changed(StartAuthoring() || result.moodChanged);
    }
    return result;
}

const Data::Maps::LookValues& LookLayers::Active(int world)
{
    if (!m_cacheValid || world != m_cacheWorld)
    {
        m_values = ResolveLookValues(m_live.lookData, world, m_personal, Applying());
        m_cacheWorld = world;
        m_cacheValid = true;
    }
    return m_values;
}

void LookLayers::SetPersonal(const Data::Maps::LookOverrides& personal)
{
    if (personal == m_personal)
    {
        return;
    }
    m_personal = personal;
    Changed(false);
}

void LookLayers::SetApplyingGroups(const TuningGroupSet& applying)
{
    if (applying == m_applying)
    {
        return;
    }
    m_applying = applying;
    Changed(false);
}

Data::Maps::LookValues LookLayers::Inherited(int world) const
{
    return m_live.lookData.ForWorld(world);
}

std::vector<LookValueState> LookLayers::Describe(int world, std::optional<std::string_view> name) const
{
    std::vector<LookValueState> states = DescribeLookValues(m_live, world, name);
    const TuningGroupSet applying = Applying();
    for (LookValueState& state : states)
    {
        const std::optional<Data::Maps::LookValue> value = Data::Maps::FindLookValue(state.name);
        if (value && PersonalValueApplies(m_personal, applying, *value))
        {
            state.value = *m_personal[*value];
            state.source = LookValueSource::Personal;
        }
    }
    return states;
}

LookEditResult LookLayers::Set(std::span<const LookEdit> edits, std::optional<int> world, std::string_view mapName)
{
    LookEditResult result = SetLookValues(m_live, edits, world, mapName);
    if (result.applied)
    {
        Changed(StartAuthoring() || result.moodChanged);
    }
    return result;
}

LookEditResult LookLayers::Reset(std::optional<std::string_view> name, int world)
{
    LookEditResult result = ResetLookValues(m_live, m_loaded, name, world);
    if (result.applied)
    {
        Changed(StartAuthoring() || result.moodChanged);
    }
    return result;
}

void LookLayers::MarkSaved()
{
    m_loaded = m_live;
}

} // namespace Render::Look
