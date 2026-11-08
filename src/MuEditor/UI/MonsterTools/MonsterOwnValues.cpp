#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterOwnValues.h"

#include "MonsterEdits.h"
#include "MonsterStepLabel.h"

#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <optional>
#include <set>
#include <tuple>
#include <utility>

namespace MuEditor::Monsters::OwnValues
{
namespace
{
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterValueField;

// Grey for a shared value, yellow for an own one (Q-MN50).
constexpr ImVec4 SharedTextColor{0.62f, 0.62f, 0.62f, 1.0f};
constexpr ImVec4 OwnTextColor{0.95f, 0.75f, 0.25f, 1.0f};
constexpr ImVec4 OwnFrameColor{0.55f, 0.40f, 0.08f, 0.60f};
constexpr ImVec4 OwnButtonColor{0.60f, 0.45f, 0.10f, 0.70f};

// A field unlocked on a number, in the monster it was unlocked in. An action
// that takes the number out or in locks it (LockNumber); an undo that takes
// it into another monster leaves the unlock behind, so it is locked there.
using UnlockKey = std::tuple<int, int, MonsterValueField>;

std::set<UnlockKey>& Unlocked()
{
    static std::set<UnlockKey> unlocked;
    return unlocked;
}

// Nullopt for a number of no monster with several numbers, which has nothing
// to unlock.
std::optional<UnlockKey> FindUnlockKey(int number, MonsterValueField field)
{
    const std::optional<int> monster = g_MonsterDataHandler.FindGroupOf(number);
    return monster ? std::optional<UnlockKey>(UnlockKey{number, *monster, field}) : std::nullopt;
}

int& UnlockVersion()
{
    static int version = 0;
    return version;
}

// "Attacks: Shared by the 3 numbers of Death Centurion".
std::string SharedLine(int number, MonsterValueField field)
{
    return std::string(GetFieldName(field)) + ": " +
           I18N::Format(
               I18N::Editor::SharedByThe0NumbersOf1,
               {std::to_string(g_MonsterEdits.ReadMonsterNumbers(number).size()), ShownValues::MonsterName(number)});
}

void RenderSourcesMenu(int number, MonsterValueField field)
{
    for (const Source& source : FindSources(number, field))
    {
        if (ImGui::MenuItem(source.label.c_str()))
        {
            TakeValue(number, field, source);
        }
    }
}
} // namespace

State GetNumberState(int number, MonsterValueField field)
{
    // Read each frame for each field shown: no copies.
    const MonsterDefinition* entry = g_MonsterDatabase.Find(number);
    const MonsterDefinition* shared = g_MonsterEdits.FindSharedValues(number);
    if (entry == nullptr || shared == nullptr)
    {
        return State::Plain;
    }
    if (!Data::Monsters::HasSameValue(*entry, *shared, field))
    {
        return State::Own;
    }
    const std::optional<UnlockKey> key = FindUnlockKey(number, field);
    return key && Unlocked().contains(*key) ? State::Unlocked : State::Shared;
}

bool SharesValue(int number, MonsterValueField field)
{
    const State state = GetNumberState(number, field);
    return state == State::Shared || state == State::Unlocked;
}

State GetState(const ShownValues::ShownMonster& shown, MonsterValueField field)
{
    return shown.wholeMonster ? State::Plain : GetNumberState(shown.monster.number, field);
}

void RenderLine(const ShownValues::ShownMonster& shown, MonsterValueField field)
{
    const int number = shown.monster.number;
    ImGui::PushID(static_cast<int>(field));
    switch (GetState(shown, field))
    {
    case State::Plain:
        break;
    case State::Shared:
        ImGui::TextColored(SharedTextColor, "%s", SharedLine(number, field).c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton(I18N::Editor::MakeItThisNumberSOwn))
        {
            Unlock(number, field);
        }
        break;
    case State::Unlocked:
        ImGui::TextColored(SharedTextColor, "%s", SharedLine(number, field).c_str());
        break;
    case State::Own:
        ImGui::TextColored(OwnTextColor, "%s: %s", GetFieldName(field),
                           I18N::Format(I18N::Editor::OwnValueOf0, {std::to_string(number)}).c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton(I18N::Editor::BackTo))
        {
            ImGui::OpenPopup("##BackTo");
        }
        if (ImGui::BeginPopup("##BackTo"))
        {
            RenderSourcesMenu(number, field);
            ImGui::EndPopup();
        }
        break;
    }
    ImGui::PopID();
}

void BeginField(State state)
{
    if (state == State::Shared)
    {
        ImGui::BeginDisabled(true);
    }
    else if (state == State::Own)
    {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, OwnFrameColor);
        ImGui::PushStyleColor(ImGuiCol_Button, OwnButtonColor);
    }
}

void RenderValueText(State state, const std::string& text)
{
    if (state == State::Own)
    {
        ImGui::TextColored(OwnTextColor, "%s", text.c_str());
    }
    else if (state != State::Plain)
    {
        ImGui::TextColored(SharedTextColor, "%s", text.c_str());
    }
    else
    {
        ImGui::TextUnformatted(text.c_str());
    }
}

void EndField(State state)
{
    if (state == State::Shared)
    {
        ImGui::EndDisabled();
    }
    else if (state == State::Own)
    {
        ImGui::PopStyleColor(2);
    }
}

void RenderMenuItems(int number, MonsterValueField field)
{
    const State state = GetNumberState(number, field);
    const std::string prefix = std::string(GetFieldName(field)) + ": ";
    if (state == State::Shared)
    {
        if (ImGui::MenuItem((prefix + I18N::Editor::MakeItThisNumberSOwn).c_str()))
        {
            Unlock(number, field);
        }
    }
    else if (state == State::Own && ImGui::BeginMenu((prefix + I18N::Editor::BackTo).c_str()))
    {
        RenderSourcesMenu(number, field);
        ImGui::EndMenu();
    }
}

const char* GetFieldName(MonsterValueField field)
{
    switch (field)
    {
    case MonsterValueField::Names:
        return I18N::Editor::Name;
    case MonsterValueField::Kind:
        return I18N::Editor::Kind;
    case MonsterValueField::Model:
        return I18N::Editor::Model;
    case MonsterValueField::SoundSet:
        return I18N::Editor::SoundSet;
    case MonsterValueField::Attacks:
        return I18N::Editor::Attacks;
    case MonsterValueField::MapVariants:
        return I18N::Editor::MapVariants;
    case MonsterValueField::RandomVariants:
        return I18N::Editor::RandomVariants;
    }
    return "";
}

std::vector<Source> FindSources(int number, MonsterValueField field)
{
    std::vector<Source> sources;
    const std::optional<MonsterDefinition> entry = g_MonsterDataHandler.ReadMonster(number);
    const std::optional<MonsterDefinition> shared = g_MonsterEdits.ReadSharedValues(number);
    if (!entry || !shared)
    {
        return sources;
    }
    const auto offered = [&](const MonsterDefinition& values)
    {
        return Data::Monsters::HasSameValue(values, *entry, field) ||
               std::ranges::any_of(sources, [&](const Source& source)
                                   { return Data::Monsters::HasSameValue(source.values, values, field); });
    };
    if (!offered(*shared))
    {
        sources.push_back({I18N::Editor::TheMonsterSValue, *shared});
    }
    for (const int other : g_MonsterEdits.ReadMonsterNumbers(number))
    {
        const std::optional<MonsterDefinition> values = g_MonsterDataHandler.ReadMonster(other);
        if (other != number && values && !offered(*values))
        {
            sources.push_back({I18N::Format(I18N::Editor::LikeNumber0, {std::to_string(other)}), *values});
        }
    }
    return sources;
}

bool TakeValue(int number, MonsterValueField field, const Source& source)
{
    std::optional<MonsterDefinition> entry = g_MonsterDataHandler.ReadMonster(number);
    if (!entry)
    {
        return false;
    }
    Data::Monsters::CopyValue(*entry, source.values, field);
    const std::string label =
        I18N::Format(I18N::Editor::StepBackTo, {StepLabel::LogName(*entry), GetFieldName(field), source.label});
    if (!g_MonsterEdits.SetMonster(label, *entry))
    {
        return false;
    }
    const std::optional<UnlockKey> key = FindUnlockKey(number, field);
    if (key && GetNumberState(number, field) != State::Own)
    {
        Unlocked().erase(*key);
        ++UnlockVersion();
    }
    return true;
}

void Unlock(int number, MonsterValueField field)
{
    if (const std::optional<UnlockKey> key = FindUnlockKey(number, field))
    {
        Unlocked().insert(*key);
        ++UnlockVersion();
    }
}

void LockNumber(int number)
{
    const auto locked =
        std::erase_if(Unlocked(), [number](const UnlockKey& key) { return std::get<0>(key) == number; });
    if (locked > 0)
    {
        ++UnlockVersion();
    }
}

void LockAll()
{
    Unlocked().clear();
    ++UnlockVersion();
}

int GetUnlockVersion()
{
    return UnlockVersion();
}
} // namespace MuEditor::Monsters::OwnValues

#endif // _EDITOR
