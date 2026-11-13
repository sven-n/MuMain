#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterSetupLists.h"

#include "../MuEditor/UI/ItemTools/ItemFieldWidgets.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "Data/GameData/EffectData/EffectTypeSymbols.h"
#include "Data/GameData/MonsterData/MonsterAnimations.h"
#include "Data/GameData/MonsterData/MonsterSoundIds.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <limits>
#include <optional>
#include <string_view>

namespace MuEditor::Monsters::SetupLists
{
namespace
{
using Data::Monsters::MonsterSpawnEffect;
using Kind = MonsterSpawnEffect::Kind;

constexpr ImVec4 WarningColor{1.0f, 0.4f, 0.4f, 1.0f};

struct KindEntry
{
    Kind kind;
    const char* const* name;
};

const std::array<KindEntry, 5> KindNames{{
    {Kind::Joint, &I18N::Editor::Joint},
    {Kind::Effect, &I18N::Editor::Effect},
    {Kind::Particle, &I18N::Editor::Particle},
    {Kind::Object, &I18N::Editor::Object},
    {Kind::Eyes, &I18N::Editor::Eyes},
}};

const char* KindName(Kind kind)
{
    const auto found = std::ranges::find(KindNames, kind, &KindEntry::kind);
    return found != KindNames.end() ? *found->name : "";
}

float Width(float numbers)
{
    return Items::Fields::ScaledWidth(Items::Fields::NumberWidth * numbers);
}

// The names the effect's type may have: the code's names of its kind, an
// object's model slot.
std::vector<std::string> TypeChoices(Kind kind)
{
    std::vector<std::string> names;
    const auto add = [&](Data::Effects::EffectKind effectKind)
    {
        for (const Data::Effects::EffectTypeSymbol& symbol : Data::Effects::GetEffectTypeSymbols(effectKind))
            names.emplace_back(symbol.code);
    };
    switch (kind)
    {
    case Kind::Joint:
        add(Data::Effects::EffectKind::Joint);
        break;
    case Kind::Effect:
        add(Data::Effects::EffectKind::Effect);
        break;
    case Kind::Particle:
        add(Data::Effects::EffectKind::Particle);
        break;
    case Kind::Object:
        for (const Data::Monsters::MonsterModelDefinition* model : g_MonsterModelDatabase.GetAll())
            names.push_back(model->slot);
        break;
    case Kind::Eyes:
        break;
    }
    std::ranges::sort(names);
    return names;
}

// A drop-down of the names with a search field; true when one was picked.
bool TypeCombo(Kind kind, std::string& type)
{
    static char search[64] = "";
    bool changed = false;
    ImGui::SetNextItemWidth(Width(2.5f));
    if (ImGui::BeginCombo("##Type", type.empty() ? I18N::Editor::None : type.c_str(), ImGuiComboFlags_HeightLarge))
    {
        if (ImGui::IsWindowAppearing())
        {
            search[0] = '\0';
            ImGui::SetKeyboardFocusHere();
        }
        ImGui::SetNextItemWidth(-FLT_MIN);
        ImGui::InputTextWithHint("##Search", I18N::Editor::Search, search, sizeof(search));
        const std::string needle = search;
        for (const std::string& choice : TypeChoices(kind))
        {
            if (!needle.empty() && choice.find(needle) == std::string::npos)
                continue;
            if (ImGui::Selectable(choice.c_str(), choice == type))
            {
                type = choice;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    return changed;
}

bool KnownType(Kind kind, const std::string& type)
{
    if (kind == Kind::Eyes)
        return true;
    const std::vector<std::string> choices = TypeChoices(kind);
    return std::ranges::find(choices, type) != choices.end();
}

// An optional value: a box, and its fields while it is ticked.
template <typename TValue, typename TFields>
bool Optional(const char* label, std::optional<TValue>& value, const TValue& start, TFields&& fields)
{
    bool present = value.has_value();
    bool changed = false;
    if (ImGui::Checkbox(label, &present))
    {
        value = present ? std::optional<TValue>(start) : std::nullopt;
        changed = true;
    }
    if (value)
    {
        ImGui::SameLine();
        ImGui::PushID(label);
        fields(*value);
        ImGui::PopID();
    }
    return changed;
}

} // namespace

MonsterSpawnEffect SwitchedKind(const MonsterSpawnEffect& effect, MonsterSpawnEffect::Kind kind)
{
    MonsterSpawnEffect switched;
    switched.kind = kind;
    // The kind's first type: one the reader takes.
    const std::vector<std::string> types = TypeChoices(kind);
    if (!types.empty())
        switched.type = types.front();
    // A count and an offset mean the same for the kinds that have them.
    if (kind != Kind::Object && kind != Kind::Eyes)
    {
        switched.count = effect.count;
        switched.offset = effect.offset;
    }
    return switched;
}

namespace
{
bool EffectFields(MonsterSpawnEffect& effect, const AfterField& afterField, EffectRows rows)
{
    bool changed = false;
    ImGui::SetNextItemWidth(Width(1.2f));
    if (ImGui::BeginCombo(I18N::Editor::Kind, KindName(effect.kind)))
    {
        for (const KindEntry& kind : KindNames)
        {
            if (!IsEffectKindAllowed(rows, kind.kind))
                continue;
            if (ImGui::Selectable(*kind.name, kind.kind == effect.kind) && kind.kind != effect.kind)
            {
                effect = SwitchedKind(effect, kind.kind);
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    if (effect.kind != Kind::Eyes)
    {
        ImGui::SameLine();
        changed |= TypeCombo(effect.kind, effect.type);
        if (!KnownType(effect.kind, effect.type))
            ImGui::TextColored(WarningColor, "%s", I18N::Format(I18N::Editor::UnknownName, {effect.type}).c_str());
    }
    // Kept to the range the reader takes.
    const auto intField = [&](const char* label, int& value, int low = std::numeric_limits<int>::min(),
                              int high = std::numeric_limits<int>::max())
    {
        ImGui::SetNextItemWidth(Width(0.8f));
        if (ImGui::InputInt(label, &value, 0))
            value = std::clamp(value, low, high);
        afterField();
    };
    const auto floatField = [&](const char* label, float& value)
    {
        ImGui::SetNextItemWidth(Width(0.8f));
        ImGui::InputFloat(label, &value, 0.0f, 0.0f, "%.3f");
        afterField();
    };
    const auto float3 = [&](std::array<float, 3>& value)
    {
        ImGui::SetNextItemWidth(Width(2.0f));
        ImGui::InputFloat3("##Value", value.data(), "%.1f");
        afterField();
    };
    switch (effect.kind)
    {
    case Kind::Eyes:
    {
        // The field whose typed bones the reader would refuse; they are kept
        // out of the row, and the warning says why while it is typed in.
        static ImGuiID s_refused = 0;
        const ImGuiID field = ImGui::GetID(I18N::Editor::Bones);
        std::string bones = FormatNumbers(effect.bones);
        ImGui::SetNextItemWidth(Width(2.0f));
        if (Items::InputString(I18N::Editor::Bones, bones))
        {
            const std::optional<std::vector<int>> parsed = ParseEyeBones(bones);
            if (parsed)
                effect.bones = *parsed;
            s_refused = parsed ? 0 : field;
        }
        if (s_refused == field && !ImGui::IsItemActive())
            s_refused = 0;
        afterField();
        if (s_refused == field || !Data::Monsters::IsEyeBoneCount(effect.bones.size()))
            ImGui::TextColored(WarningColor, "%s", I18N::Editor::EyeBonesNote);
        break;
    }
    case Kind::Object:
        changed |= Optional(I18N::Editor::Angle, effect.angle, 0.0f,
                            [&](float& angle)
                            {
                                ImGui::SetNextItemWidth(Width(0.8f));
                                ImGui::InputFloat("##Value", &angle, 0.0f, 0.0f, "%.1f");
                                afterField();
                            });
        break;
    case Kind::Joint:
    case Kind::Effect:
    case Kind::Particle:
        intField(I18N::Editor::SubType, effect.subType);
        // A scale for a joint or particle, a key for a joint or effect: the
        // fields the game uses and the writer keeps for the kind.
        if (effect.kind != Kind::Effect)
        {
            ImGui::SameLine();
            floatField(I18N::Editor::Scale, effect.scale);
        }
        ImGui::SameLine();
        intField(I18N::Editor::ScaleRandom, effect.scaleRandom, 0, Data::Monsters::MaxSpawnEffectScaleRandom);
        intField(I18N::Editor::Count, effect.count, 1, Data::Monsters::MaxSpawnEffectCount);
        if (effect.kind != Kind::Particle)
        {
            ImGui::SameLine();
            intField(I18N::Editor::Key, effect.key, std::numeric_limits<short>::min(),
                     std::numeric_limits<short>::max());
        }
        changed |= Optional(I18N::Editor::Bone, effect.bone, 0,
                            [&](int& bone)
                            {
                                ImGui::SetNextItemWidth(Width(0.8f));
                                if (ImGui::InputInt("##Value", &bone, 0))
                                    bone = std::max(bone, 0);
                                afterField();
                            });
        changed |= Optional(I18N::Editor::Colour, effect.color, std::array<float, 3>{1.0f, 1.0f, 1.0f},
                            [&](std::array<float, 3>& color)
                            {
                                ImGui::ColorEdit3("##Value", color.data(), ImGuiColorEditFlags_Float);
                                afterField();
                            });
        changed |= Optional(I18N::Editor::Offset, effect.offset, std::array<float, 3>{0.0f, 0.0f, 0.0f}, float3);
        if (effect.kind == Kind::Joint)
            changed |= Optional(I18N::Editor::TargetOffset, effect.targetOffset, std::array<float, 3>{0.0f, 0.0f, 0.0f},
                                float3);
        break;
    }
    return changed;
}
// The animations the setup may name, by their names (stop1 ... attack5).
constexpr int NamedAnimations = 12;

bool AnimationCombo(const char* label, std::string& name)
{
    bool changed = false;
    ImGui::SetNextItemWidth(Width(1.2f));
    if (ImGui::BeginCombo(label, name.c_str()))
    {
        for (int animation = 0; animation < NamedAnimations; ++animation)
        {
            const std::string choice = Data::Monsters::GetMonsterAnimationName(animation);
            if (ImGui::Selectable(choice.c_str(), choice == name))
            {
                name = choice;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    return changed;
}

struct HookCode
{
    std::string_view name;
    std::string_view code;
};

// What World::Monsters::RunSpawnHook runs for each hook.
constexpr std::array<HookCode, 4> HookCodes{{
    {"RaklionBossDrop", "World/GameMaps/MonsterSpawnWorld.cpp: g_Raklion.LetBossDrop()"},
    {"Kanturu2ndEntrance",
     "World/GameMaps/MonsterSpawnWorld.cpp: g_pKanturu2ndEnterNpc->SetNpcObject(&character.Object), "
     "SetNpcAnimation(false)"},
    {"EmpireGuardianRageReset", "World/GameMaps/MonsterSpawnWorld.cpp: g_EmpireGuardian1..4.ResetRage(boss)"},
    {"IllusionTempleStorageReset",
     "World/GameMaps/MonsterSpawnWorld.cpp: g_CursedTemple->HideStoragePointEffect(storage == \"allied\")"},
}};
} // namespace

std::string_view GetHookCode(std::string_view name)
{
    const auto found = std::ranges::find(HookCodes, name, &HookCode::name);
    return found != HookCodes.end() ? found->code : std::string_view();
}

bool Appear(Data::Monsters::MonsterAppear& appear, const AfterField& afterField)
{
    bool changed = false;
    changed |= Optional(I18N::Editor::Animation, appear.action, std::string("stop1"),
                        [&](std::string& action) { changed |= AnimationCombo("##Value", action); });
    if (ImGui::Checkbox(I18N::Editor::BlendFromTheAnimationBefore, &appear.blend))
        changed = true;
    changed |= Optional(I18N::Editor::StartFrame, appear.startFrame, 0.0f,
                        [&](float& frame)
                        {
                            ImGui::SetNextItemWidth(Width(0.8f));
                            ImGui::InputFloat("##Value", &frame, 0.0f, 0.0f, "%.2f");
                            afterField();
                        });
    changed |= Optional(I18N::Editor::FadeIn, appear.fadeIn, true,
                        [&](bool& fadeIn)
                        {
                            if (ImGui::Checkbox("##Value", &fadeIn))
                                changed = true;
                        });
    changed |= Optional(I18N::Editor::AppearTime, appear.appearTime, 60,
                        [&](int& frames)
                        {
                            ImGui::SetNextItemWidth(Width(0.8f));
                            ImGui::InputInt("##Value", &frames, 0);
                            afterField();
                        });
    changed |= Optional(I18N::Editor::Sound, appear.sound, std::string(Data::Monsters::GetFirstMonsterSoundName()),
                        [&](std::string& sound)
                        {
                            ImGui::SetNextItemWidth(Width(2.0f));
                            Items::InputString("##Value", sound);
                            afterField();
                            if (!Data::Monsters::FindMonsterSoundId(sound))
                            {
                                ImGui::SameLine();
                                ImGui::TextColored(WarningColor, "%s",
                                                   I18N::Format(I18N::Editor::UnknownName, {sound}).c_str());
                            }
                        });
    ImGui::TextUnformatted(I18N::Editor::EffectsWhereItComesIn);
    ImGui::Indent();
    ImGui::PushID("Effects");
    changed |= !changed && SpawnEffects(appear.effects, afterField, EffectRows::Appear);
    ImGui::PopID();
    ImGui::Unindent();
    return changed;
}

bool Gate(Data::Monsters::MonsterGate& gate)
{
    bool changed = false;
    changed |= AnimationCombo(I18N::Editor::Closed, gate.closed);
    ImGui::SameLine();
    changed |= AnimationCombo(I18N::Editor::Opening, gate.opening);
    ImGui::SameLine();
    changed |= AnimationCombo(I18N::Editor::Open, gate.open);
    int arrival = gate.arrivalOpening ? 0 : 1;
    const std::string choices = std::string(I18N::Editor::PlaysItsOpening) + '\0' + I18N::Editor::StandsOpen + '\0';
    ImGui::SetNextItemWidth(Width(2.0f));
    if (ImGui::Combo(I18N::Editor::AnOpenGateComingIntoView, &arrival, choices.c_str()))
    {
        gate.arrivalOpening = arrival == 0;
        changed = true;
    }
    return changed;
}

bool Hooks(std::vector<Data::Monsters::MonsterHook>& hooks)
{
    static std::string shownCode;
    bool changed = false;
    for (size_t index = 0; index < hooks.size(); ++index)
    {
        ImGui::PushID(static_cast<int>(index));
        Data::Monsters::MonsterHook& hook = hooks[index];
        ImGui::SetNextItemWidth(Width(2.2f));
        if (ImGui::BeginCombo("##Hook", hook.name.c_str()))
        {
            for (const Data::Monsters::MonsterHookKind& kind : Data::Monsters::GetMonsterHookKinds())
            {
                if (ImGui::Selectable(std::string(kind.name).c_str(), kind.name == hook.name) && kind.name != hook.name)
                {
                    hook.name = std::string(kind.name);
                    hook.values.clear();
                    if (!kind.valueKey.empty() && !kind.values.empty())
                        hook.values.emplace_back(std::string(kind.valueKey), std::string(kind.values.front()));
                    changed = true;
                }
            }
            ImGui::EndCombo();
        }
        if (const Data::Monsters::MonsterHookKind* kind = Data::Monsters::FindMonsterHookKind(hook.name);
            kind != nullptr && !kind->valueKey.empty())
        {
            ImGui::SameLine();
            const std::string value(hook.Value(kind->valueKey));
            ImGui::SetNextItemWidth(Width(1.2f));
            if (ImGui::BeginCombo(std::string(kind->valueKey).c_str(), value.c_str()))
            {
                for (const std::string_view choice : kind->values)
                {
                    if (ImGui::Selectable(std::string(choice).c_str(), choice == value) && choice != value)
                    {
                        hook.values = {{std::string(kind->valueKey), std::string(choice)}};
                        changed = true;
                    }
                }
                ImGui::EndCombo();
            }
        }
        ImGui::SameLine();
        if (ImGui::SmallButton(I18N::Editor::ShowCode))
            shownCode = shownCode == hook.name ? std::string() : hook.name;
        ImGui::SameLine();
        const bool remove = ImGui::SmallButton("x");
        if (shownCode == hook.name && !GetHookCode(hook.name).empty())
            ImGui::TextDisabled("%s", std::string(GetHookCode(hook.name)).c_str());
        ImGui::PopID();
        if (remove)
        {
            hooks.erase(hooks.begin() + static_cast<std::ptrdiff_t>(index));
            return true;
        }
    }
    if (ImGui::SmallButton(I18N::Editor::AddAHook))
    {
        const std::span<const Data::Monsters::MonsterHookKind> kinds = Data::Monsters::GetMonsterHookKinds();
        Data::Monsters::MonsterHook hook;
        if (!kinds.empty())
        {
            hook.name = std::string(kinds.front().name);
            if (!kinds.front().valueKey.empty() && !kinds.front().values.empty())
                hook.values.emplace_back(std::string(kinds.front().valueKey),
                                         std::string(kinds.front().values.front()));
        }
        hooks.push_back(std::move(hook));
        changed = true;
    }
    return changed;
}

std::string FormatNumbers(const std::vector<int>& numbers)
{
    std::string text;
    for (size_t i = 0; i < numbers.size(); ++i)
        text += (i == 0 ? "" : ", ") + std::to_string(numbers[i]);
    return text;
}

std::optional<std::vector<int>> ParseEyeBones(const std::string& text)
{
    std::optional<std::vector<int>> bones = ParseNumbers(text);
    if (!bones || !Data::Monsters::IsEyeBoneCount(bones->size()) ||
        std::ranges::any_of(*bones, [](int bone) { return bone < 0; }))
        return std::nullopt;
    return bones;
}

std::optional<std::vector<int>> ParseNumbers(const std::string& text)
{
    std::vector<int> numbers;
    size_t at = 0;
    while (at < text.size())
    {
        const size_t end = text.find(',', at);
        std::string_view part(text.data() + at, (end == std::string::npos ? text.size() : end) - at);
        while (!part.empty() && part.front() == ' ')
            part.remove_prefix(1);
        while (!part.empty() && part.back() == ' ')
            part.remove_suffix(1);
        if (!part.empty())
        {
            int number = 0;
            const auto [rest, error] = std::from_chars(part.data(), part.data() + part.size(), number);
            if (error != std::errc() || rest != part.data() + part.size())
                return std::nullopt;
            numbers.push_back(number);
        }
        else if (end != std::string::npos)
        {
            return std::nullopt;
        }
        if (end == std::string::npos)
            break;
        at = end + 1;
    }
    return numbers;
}

bool IsEffectKindAllowed(EffectRows rows, Kind kind)
{
    return rows == EffectRows::Spawn || (kind != Kind::Object && kind != Kind::Eyes);
}

bool SpawnEffects(std::vector<MonsterSpawnEffect>& effects, const AfterField& afterField, EffectRows rows)
{
    bool changed = false;
    for (size_t index = 0; index < effects.size(); ++index)
    {
        ImGui::PushID(static_cast<int>(index));
        MonsterSpawnEffect& effect = effects[index];
        const std::string title = std::to_string(index + 1) + ". " + KindName(effect.kind) + " " + effect.type;
        const bool open = ImGui::TreeNode("##Effect", "%s", title.c_str());
        ImGui::SameLine();
        if (index > 0 && ImGui::SmallButton(I18N::Editor::MoveUp))
        {
            std::swap(effects[index], effects[index - 1]);
            changed = true;
        }
        ImGui::SameLine();
        if (index + 1 < effects.size() && ImGui::SmallButton(I18N::Editor::MoveDown))
        {
            std::swap(effects[index], effects[index + 1]);
            changed = true;
        }
        ImGui::SameLine();
        const bool remove = ImGui::SmallButton("x");
        if (open)
        {
            if (!changed && !remove)
                changed |= EffectFields(effects[index], afterField, rows);
            ImGui::TreePop();
        }
        ImGui::PopID();
        if (remove)
        {
            effects.erase(effects.begin() + static_cast<std::ptrdiff_t>(index));
            return true;
        }
        if (changed)
            return true;
    }
    if (ImGui::SmallButton(I18N::Editor::AddAnEffect))
    {
        // A joint of a type the reader takes.
        MonsterSpawnEffect& added = effects.emplace_back();
        const std::vector<std::string> types = TypeChoices(added.kind);
        if (!types.empty())
            added.type = types.front();
        changed = true;
    }
    return changed;
}

bool SpawnBuffs(std::vector<std::string>& buffs)
{
    bool changed = false;
    for (const std::string_view name : Data::Monsters::GetMonsterBuffNames())
    {
        const auto found = std::ranges::find(buffs, name);
        bool present = found != buffs.end();
        if (ImGui::Checkbox(std::string(name).c_str(), &present))
        {
            if (present)
                buffs.emplace_back(name);
            else
                buffs.erase(found);
            changed = true;
        }
        ImGui::SameLine();
    }
    ImGui::NewLine();
    return changed;
}

bool RandomLook(std::vector<int>& skins, const AfterField& afterField)
{
    std::string text = FormatNumbers(skins);
    ImGui::SetNextItemWidth(Width(1.5f));
    if (Items::InputString("##Value", text))
    {
        if (const std::optional<std::vector<int>> parsed = ParseNumbers(text))
            skins = *parsed;
    }
    afterField();
    return false;
}

void SetTileAxis(Data::Monsters::MonsterTileFacing& tile, int axis)
{
    const int value = tile.tileX.value_or(tile.tileY.value_or(0));
    tile.tileX = axis == 0 ? std::optional<int>(value) : std::nullopt;
    tile.tileY = axis == 1 ? std::optional<int>(value) : std::nullopt;
}

int& TileOnAxis(Data::Monsters::MonsterTileFacing& tile)
{
    return tile.tileX ? *tile.tileX : tile.tileY.emplace(tile.tileY.value_or(0));
}

bool FacingByTile(std::vector<Data::Monsters::MonsterTileFacing>& tiles, const AfterField& afterField)
{
    bool changed = false;
    for (size_t index = 0; index < tiles.size(); ++index)
    {
        ImGui::PushID(static_cast<int>(index));
        Data::Monsters::MonsterTileFacing& tile = tiles[index];
        int axis = tile.tileX.has_value() ? 0 : 1;
        ImGui::SetNextItemWidth(Width(0.6f));
        if (ImGui::Combo("##Axis", &axis, "X\0Y\0"))
        {
            SetTileAxis(tile, axis);
            changed = true;
        }
        ImGui::SameLine();
        // The axis as the combo left it.
        int& value = TileOnAxis(tile);
        ImGui::SetNextItemWidth(Width(0.8f));
        if (ImGui::InputInt(I18N::Editor::Tile, &value, 0))
            value = std::clamp(value, 0, Data::Monsters::MaxFacingTile);
        afterField();
        ImGui::SameLine();
        ImGui::SetNextItemWidth(Width(0.8f));
        ImGui::InputFloat(I18N::Editor::FacingStep, &tile.step, 0.0f, 0.0f, "%.1f");
        afterField();
        ImGui::SameLine();
        const bool remove = ImGui::SmallButton("x");
        ImGui::PopID();
        if (remove)
        {
            tiles.erase(tiles.begin() + static_cast<std::ptrdiff_t>(index));
            return true;
        }
    }
    if (ImGui::SmallButton(I18N::Editor::AddATile))
    {
        Data::Monsters::MonsterTileFacing tile;
        tile.tileX = 0;
        tiles.push_back(tile);
        changed = true;
    }
    return changed;
}
} // namespace MuEditor::Monsters::SetupLists

#endif // _EDITOR
