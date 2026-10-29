#include "stdafx.h"

#ifdef _EDITOR

#include "MuMonsterAttacksUI.h"

#include "MonsterAttackReaders.h"
#include "MonsterEdits.h"
#include "MonsterSelection.h"
#include "MonsterShownModel.h"
#include "MonsterStepLabel.h"
#include "MonsterToolWindow.h"
#include "MuMonsterPreviewUI.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MapPicker.h"
#include "../MuEditor/UI/Common/MuEditorLayout.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Data/DataHandler/MonsterData/MonsterDataHandler.h"
#include "Data/GameData/MonsterData/MonsterAnimations.h"
#include "Data/GameData/MonsterData/MonsterAttacks.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Data/GameData/MonsterData/MonsterSoundIds.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "I18N/All.h"
#include "Render/Models/ZzzBMD.h"
#include "imgui.h"

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace MuEditor::Monsters
{
namespace
{
using Data::Monsters::MonsterAttackRandom;
using Data::Monsters::MonsterAttacks;
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterSkillClear;
using Data::Monsters::NoAttackAnimation;

// Pixel widths before the editor UI scale.
constexpr float ComboWidth = 160.0f;
constexpr float NumberWidth = 90.0f;
// The animations offered while the model is not open: up to attack 5.
constexpr int NamedAnimations = MONSTER01_ATTACK5 + 1;
constexpr int MaxSkillNumber = 65534;
constexpr ImVec4 WarningColor{1.0f, 0.75f, 0.3f, 1.0f};

constexpr std::array<MonsterAttackRandom, 3> Randoms = {MonsterAttackRandom::Rand, MonsterAttackRandom::RandFps,
                                                        MonsterAttackRandom::Bali};
constexpr std::array<MonsterSkillClear, 5> Clears = {MonsterSkillClear::Never, MonsterSkillClear::OnMatch,
                                                     MonsterSkillClear::OnNoMatch, MonsterSkillClear::Always,
                                                     MonsterSkillClear::AfterAttack};

std::string AnimationLabel(int animation)
{
    if (animation == NoAttackAnimation)
    {
        return I18N::Editor::NoAnimation;
    }
    return std::to_string(animation) + " " + Data::Monsters::GetMonsterAnimationName(animation);
}

const char* RandomLabel(MonsterAttackRandom random)
{
    switch (random)
    {
    case MonsterAttackRandom::Rand:
        return I18N::Editor::RandomAttack1Or2;
    case MonsterAttackRandom::RandFps:
        return I18N::Editor::RandomAttack1Or2ByFrameRate;
    case MonsterAttackRandom::Bali:
        return I18N::Editor::RandomBalisDraw;
    }
    return "";
}

const char* ClearLabel(MonsterSkillClear clear)
{
    switch (clear)
    {
    case MonsterSkillClear::Never:
        return I18N::Editor::ClearSkillNever;
    case MonsterSkillClear::OnMatch:
        return I18N::Editor::ClearSkillOnMatch;
    case MonsterSkillClear::OnNoMatch:
        return I18N::Editor::ClearSkillOnNoMatch;
    case MonsterSkillClear::Always:
        return I18N::Editor::ClearSkillAlways;
    case MonsterSkillClear::AfterAttack:
        return I18N::Editor::ClearSkillAfterAttack;
    }
    return "";
}

const char* ReaderLabel(AttackReaders::Reader reader)
{
    using AttackReaders::Reader;
    switch (reader)
    {
    case Reader::BattleCastleGuards:
        return I18N::Editor::ReaderBattleCastleGuards;
    case Reader::Kanturu3rd:
        return I18N::Editor::ReaderKanturu3rd;
    case Reader::Raklion:
        return I18N::Editor::ReaderRaklion;
    case Reader::EmpireGuardian:
        return I18N::Editor::ReaderEmpireGuardian;
    case Reader::EmpireGuardian2:
        return I18N::Editor::ReaderEmpireGuardian2;
    case Reader::EmpireGuardian3:
        return I18N::Editor::ReaderEmpireGuardian3;
    case Reader::EmpireGuardian4:
        return I18N::Editor::ReaderEmpireGuardian4;
    case Reader::Traps:
        return I18N::Editor::ReaderTraps;
    }
    return "";
}

void RenderWarning(const char* text)
{
    ImGui::PushStyleColor(ImGuiCol_Text, WarningColor);
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(text);
    ImGui::PopTextWrapPos();
    ImGui::PopStyleColor();
}

void RenderNote(const char* text)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", text);
    ImGui::PopTextWrapPos();
}

int ModelSlotOf(const MonsterDefinition& monster)
{
    return Data::Monsters::FindMonsterModelReference(monster.model).value_or(-1);
}

// The animations the model has with a speed above 0; while it is not open,
// the named ones.
std::vector<int> OfferedAnimations(int modelSlot)
{
    std::vector<int> animations;
    if (modelSlot >= 0 && Models != nullptr && Models[modelSlot].NumActions > 0)
    {
        const BMD& model = Models[modelSlot];
        for (int animation = 0; animation < model.NumActions; ++animation)
        {
            if (model.Actions[animation].PlaySpeed > 0.0f)
            {
                animations.push_back(animation);
            }
        }
        return animations;
    }
    for (int animation = 0; animation < NamedAnimations; ++animation)
    {
        animations.push_back(animation);
    }
    return animations;
}
} // namespace

MuMonsterAttacksUI& MuMonsterAttacksUI::GetInstance()
{
    static MuMonsterAttacksUI instance;
    return instance;
}

MuMonsterAttacksUI::MuMonsterAttacksUI()
    : m_draft([](int number) { return g_MonsterDataHandler.ReadMonster(number); },
              [](const std::string& label, const MonsterDefinition& monster)
              { g_MonsterEdits.SetMonster(label, monster); })
{
}

void MuMonsterAttacksUI::Render(bool* open)
{
    if (ToolWindow::Begin(I18N::Editor::MonsterAttacks, WindowIds::MonsterAttacks, open))
    {
        const int number = g_MonsterSelection.GetNumber();
        MonsterDefinition* monster = m_draft.Begin(number, ToolWindow::GetDataVersion());
        ShownModel::RenderSelectedMonster(number, monster);
        if (monster != nullptr)
        {
            ImGui::PushID(number);
            RenderAttacks(*monster, ModelSlotOf(*monster));
            ImGui::PopID();
        }
        m_draft.End();
    }
    ToolWindow::End();
}

void MuMonsterAttacksUI::RenderAttacks(MonsterDefinition& monster, int modelSlot)
{
    if (modelSlot == MODEL_PLAYER)
    {
        RenderNote(I18N::Editor::PlayerShapedMonstersChooseAPlayerAttack);
        return;
    }
    if (modelSlot >= 0 && ShownModel::FindSlot(monster))
    {
        ShownModel::OpenWhenShown(modelSlot);
    }
    const std::string label = StepLabel::Attacks(monster);
    if (m_variantMonster != monster.number)
    {
        m_variantMonster = monster.number;
        m_variant = AllMaps;
    }
    if (m_variant >= static_cast<int>(monster.mapVariants.size()))
    {
        m_variant = AllMaps;
    }
    RenderMapRow(monster);
    ImGui::PushID(m_variant);
    if (m_variant != AllMaps)
    {
        RenderVariantMaps(monster);
    }
    std::optional<MonsterAttacks>& attacks = ShownAttacks(monster);
    if (!attacks)
    {
        if (m_variant == AllMaps)
        {
            RenderNote(I18N::Editor::TheMonsterHasNoAttacksItPlaysTheGenericPattern);
        }
        if (ImGui::Button(I18N::Editor::AddAttacks))
        {
            attacks.emplace();
            m_draft.AfterChange(true, label);
        }
        ImGui::PopID();
        return;
    }
    RenderReaders(monster, modelSlot);
    if (ImGui::CollapsingHeader(I18N::Editor::MonsterSkills, ImGuiTreeNodeFlags_DefaultOpen))
    {
        RenderEntries(monster, modelSlot, true);
    }
    if (ImGui::CollapsingHeader(I18N::Editor::PlayerSkills, ImGuiTreeNodeFlags_DefaultOpen))
    {
        RenderEntries(monster, modelSlot, false);
    }
    if (ImGui::CollapsingHeader(I18N::Editor::AnyOtherAttack, ImGuiTreeNodeFlags_DefaultOpen))
    {
        RenderOtherwise(monster, modelSlot);
        RenderPlayNext(monster);
        RenderClearSkill(monster);
    }
    if (ImGui::CollapsingHeader(I18N::Editor::AttackSound, ImGuiTreeNodeFlags_DefaultOpen))
    {
        RenderSound(monster);
    }
    ImGui::Separator();
    // A map variant has only attacks yet: it goes as a whole.
    if (m_variant == AllMaps && ImGui::Button(I18N::Editor::RemoveTheAttacks))
    {
        attacks.reset();
        m_draft.AfterChange(true, label);
    }
    ImGui::PopID();
}

void MuMonsterAttacksUI::ShowMapVariant(int monster, std::optional<int> variant)
{
    // RenderAttacks shows all maps for an index the monster does not have.
    m_variantMonster = monster;
    m_variant = variant.value_or(AllMaps);
}

std::optional<MonsterAttacks>& MuMonsterAttacksUI::ShownAttacks(MonsterDefinition& monster) const
{
    if (m_variant == AllMaps)
    {
        return monster.attacks;
    }
    return monster.mapVariants[static_cast<size_t>(m_variant)].attacks;
}

void MuMonsterAttacksUI::RenderMapRow(MonsterDefinition& monster)
{
    ImGui::TextUnformatted(I18N::Editor::Map);
    ImGui::SameLine();
    if (ImGui::RadioButton(I18N::Editor::AllMaps, m_variant == AllMaps))
    {
        m_variant = AllMaps;
    }
    for (size_t index = 0; index < monster.mapVariants.size(); ++index)
    {
        ImGui::SameLine();
        const std::string name = MapPicker::Describe(monster.mapVariants[index].maps, I18N::Editor::NoMapYet) +
                                 "##Variant" + std::to_string(index);
        if (ImGui::RadioButton(name.c_str(), m_variant == static_cast<int>(index)))
        {
            m_variant = static_cast<int>(index);
        }
    }
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::AddAMapVariant))
    {
        // It starts as the attacks of all maps, on the maps picked next.
        Data::Monsters::MonsterMapVariant variant;
        variant.attacks = monster.attacks ? *monster.attacks : MonsterAttacks{};
        monster.mapVariants.push_back(std::move(variant));
        m_variant = static_cast<int>(monster.mapVariants.size()) - 1;
        m_draft.AfterChange(true, StepLabel::Attacks(monster));
    }
}

void MuMonsterAttacksUI::RenderVariantMaps(MonsterDefinition& monster)
{
    Data::Monsters::MonsterMapVariant& variant = monster.mapVariants[static_cast<size_t>(m_variant)];
    RenderNote(I18N::Editor::OnItsMapsTheVariantAttacksInsteadOfTheMonster);
    ImGui::TextUnformatted(I18N::Editor::PlaysOn);
    ImGui::SameLine();
    if (MapPicker::Render("VariantMaps", variant.maps, I18N::Editor::NoMapYet))
    {
        m_draft.AfterChange(true, StepLabel::Attacks(monster));
    }
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::RemoveTheMapVariant))
    {
        monster.mapVariants.erase(monster.mapVariants.begin() + m_variant);
        m_variant = AllMaps;
        m_draft.AfterChange(true, StepLabel::Attacks(monster));
        return;
    }
    if (variant.maps.empty())
    {
        RenderWarning(I18N::Editor::PickTheMapsOfTheVariantUntilThenSaveRefusesTheMonsters);
    }
    const std::vector<Data::Maps::MapRef> shared =
        Data::Monsters::FindMapsOfOtherVariants(monster, static_cast<size_t>(m_variant));
    if (!shared.empty())
    {
        RenderWarning(
            I18N::Format(I18N::Editor::VariantMapsInAnotherVariant, {MapPicker::Describe(shared, "")}).c_str());
    }
}

void MuMonsterAttacksUI::RenderReaders(const MonsterDefinition& monster, int modelSlot)
{
    // The attacks play on every map; this is the code that runs with them.
    const std::vector<AttackReaders::Reader> readers = AttackReaders::FindReaders(monster.number, modelSlot);
    if (readers.empty())
    {
        return;
    }
    ImGui::TextUnformatted(I18N::Editor::MapCodeThatAlsoRuns);
    for (const AttackReaders::Reader reader : readers)
    {
        ImGui::BulletText("%s", ReaderLabel(reader));
    }
}

void MuMonsterAttacksUI::RenderEntries(MonsterDefinition& monster, int modelSlot, bool monsterSkills)
{
    std::vector<Data::Monsters::MonsterAttackEntry>& entries =
        monsterSkills ? ShownAttacks(monster)->monsterSkills : ShownAttacks(monster)->skills;
    RenderNote(monsterSkills ? I18N::Editor::MonsterSkillsNote : I18N::Editor::PlayerSkillsNote);
    const std::string label = StepLabel::Attacks(monster);
    std::optional<size_t> removed;
    const float scale = g_MuEditorCore.GetUIScale();
    // The two lists' rows have ids of their own.
    ImGui::PushID(monsterSkills ? "MonsterSkills" : "Skills");
    for (size_t index = 0; index < entries.size(); ++index)
    {
        ImGui::PushID(static_cast<int>(index));
        auto& [skill, animation] = entries[index];
        ImGui::Text("%d", skill);
        ImGui::SameLine(NumberWidth * scale);
        if (AnimationCombo("##Animation", animation, modelSlot))
        {
            m_draft.AfterChange(true, label);
        }
        ImGui::SameLine();
        RenderPlayButton(monster, animation);
        ImGui::SameLine();
        if (ImGui::SmallButton(I18N::Editor::Remove))
        {
            removed = index;
        }
        // Packet 0x69 plays an attack for the monster skills 1 to 77 only.
        if (monsterSkills && (skill <= ATMON_SKILL_BIGIN || skill >= ATMON_SKILL_END))
        {
            ImGui::SameLine();
            RenderWarning(I18N::Editor::TheGameDoesNotPlayAnAttackForThisSkill);
        }
        ImGui::PopID();
    }
    ImGui::PopID();
    if (removed)
    {
        entries.erase(entries.begin() + static_cast<std::ptrdiff_t>(*removed));
        m_draft.AfterChange(true, label);
    }
    int& newSkill = monsterSkills ? m_newMonsterSkill : m_newSkill;
    ImGui::SetNextItemWidth(NumberWidth * scale);
    ImGui::InputInt(monsterSkills ? "##NewMonsterSkill" : "##NewSkill", &newSkill, 0);
    newSkill = std::clamp(newSkill, 0, MaxSkillNumber);
    ImGui::SameLine();
    const bool taken =
        std::any_of(entries.begin(), entries.end(), [&](const auto& entry) { return entry.first == newSkill; });
    ImGui::BeginDisabled(taken);
    if (ImGui::Button(monsterSkills ? I18N::Editor::AddMonsterSkill : I18N::Editor::AddPlayerSkill))
    {
        entries.emplace_back(newSkill, MONSTER01_ATTACK1);
        std::sort(entries.begin(), entries.end());
        m_draft.AfterChange(true, label);
    }
    ImGui::EndDisabled();
}

void MuMonsterAttacksUI::RenderOtherwise(MonsterDefinition& monster, int modelSlot)
{
    MonsterAttacks& attacks = *ShownAttacks(monster);
    const std::string label = StepLabel::Attacks(monster);
    const bool animations = !attacks.otherwiseRandom && !attacks.otherwiseGeneric;
    const bool rotation = animations && Data::Monsters::IsRotation(attacks);
    if (ImGui::RadioButton(I18N::Editor::AnAnimation, animations && !rotation) && !(animations && !rotation))
    {
        attacks.otherwiseRandom.reset();
        attacks.otherwiseGeneric = false;
        attacks.otherwise.resize(1);
        m_draft.AfterChange(true, label);
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!animations || rotation);
    if (AnimationCombo("##Otherwise", attacks.otherwise.front(), modelSlot))
    {
        m_draft.AfterChange(true, label);
    }
    ImGui::EndDisabled();
    if (ImGui::RadioButton(I18N::Editor::ARotation, rotation) && !rotation)
    {
        attacks.otherwiseRandom.reset();
        attacks.otherwiseGeneric = false;
        // A rotation kept from before the generic pattern or a draw stays
        // whole; one animation becomes two.
        if (attacks.otherwise.size() < 2)
        {
            attacks.otherwise.resize(2, attacks.otherwise.front());
        }
        m_draft.AfterChange(true, label);
    }
    if (rotation)
    {
        RenderRotation(monster, modelSlot);
    }
    if (ImGui::RadioButton(I18N::Editor::TheGenericPattern, attacks.otherwiseGeneric) && !attacks.otherwiseGeneric)
    {
        attacks.otherwiseRandom.reset();
        attacks.otherwiseGeneric = true;
        m_draft.AfterChange(true, label);
    }
    if (ImGui::RadioButton(I18N::Editor::ADraw, attacks.otherwiseRandom.has_value()) && !attacks.otherwiseRandom)
    {
        attacks.otherwiseGeneric = false;
        attacks.otherwiseRandom = MonsterAttackRandom::Rand;
        m_draft.AfterChange(true, label);
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!attacks.otherwiseRandom);
    ImGui::SetNextItemWidth(ComboWidth * 2.0f * g_MuEditorCore.GetUIScale());
    const char* shown = attacks.otherwiseRandom ? RandomLabel(*attacks.otherwiseRandom) : "";
    if (ImGui::BeginCombo("##Random", shown))
    {
        for (const MonsterAttackRandom random : Randoms)
        {
            if (ImGui::Selectable(RandomLabel(random), attacks.otherwiseRandom == random))
            {
                attacks.otherwiseRandom = random;
                m_draft.AfterChange(true, label);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::EndDisabled();
}

void MuMonsterAttacksUI::RenderRotation(MonsterDefinition& monster, int modelSlot)
{
    std::vector<int>& rotation = ShownAttacks(monster)->otherwise;
    const std::string label = StepLabel::Attacks(monster);
    ImGui::Indent();
    RenderNote(I18N::Editor::PlayedInTurnCountedForEachMonsterFromItsCreation);
    std::optional<size_t> removed;
    for (size_t index = 0; index < rotation.size(); ++index)
    {
        ImGui::PushID(static_cast<int>(index));
        ImGui::Text("%zu", index + 1);
        ImGui::SameLine();
        if (AnimationCombo("##Turn", rotation[index], modelSlot))
        {
            m_draft.AfterChange(true, label);
        }
        ImGui::SameLine();
        // A rotation has two animations at least; one is "an animation".
        ImGui::BeginDisabled(rotation.size() <= 2);
        if (ImGui::SmallButton(I18N::Editor::Remove))
        {
            removed = index;
        }
        ImGui::EndDisabled();
        ImGui::PopID();
    }
    if (removed)
    {
        rotation.erase(rotation.begin() + static_cast<std::ptrdiff_t>(*removed));
        m_draft.AfterChange(true, label);
    }
    if (ImGui::SmallButton(I18N::Editor::AddAnAnimation))
    {
        rotation.push_back(rotation.back());
        m_draft.AfterChange(true, label);
    }
    ImGui::Unindent();
}

void MuMonsterAttacksUI::RenderPlayNext(MonsterDefinition& monster)
{
    if (m_countsMonster != monster.number || m_countsVariant != m_variant)
    {
        m_countsMonster = monster.number;
        m_countsVariant = m_variant;
        m_counts = {};
        m_lastPlayed.reset();
    }
    ImGui::BeginDisabled(!ShownModel::FindSlot(monster));
    if (ImGui::SmallButton(I18N::Editor::PlayNextAttack))
    {
        m_lastPlayed = AttackPreview::NextOtherwise(*ShownAttacks(monster), m_counts);
        PlayInPreview(monster, *m_lastPlayed);
    }
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s", I18N::Editor::PlayNextAttackNote);
    }
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::FromTheStart))
    {
        m_counts = {};
        m_lastPlayed.reset();
    }
    if (m_lastPlayed)
    {
        ImGui::SameLine();
        ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::Played0, {AnimationLabel(*m_lastPlayed)}).c_str());
    }
}

void MuMonsterAttacksUI::RenderPlayButton(MonsterDefinition& monster, int animation)
{
    ImGui::BeginDisabled(!ShownModel::FindSlot(monster) || animation == NoAttackAnimation);
    if (ImGui::SmallButton(I18N::Editor::Play))
    {
        PlayInPreview(monster, animation);
    }
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s", I18N::Editor::PlaysItOnceInTheMonsterPreviewWithTheAttackSounds);
    }
}

void MuMonsterAttacksUI::PlayInPreview(MonsterDefinition& monster, int animation)
{
    const std::optional<int> slot = ShownModel::FindSlot(monster);
    if (!slot || animation == NoAttackAnimation)
    {
        return;
    }
    // Shown beside this tool, not as a tab over it.
    g_MuEditorCore.GetToolWindows().monsterPreview = true;
    Layout::ShowBeside(WindowIds::MonsterPreview, WindowIds::MonsterAttacks);
    g_MuMonsterPreviewUI.PlayAttack(*slot, animation, Data::Monsters::FindMonsterSoundId(ShownAttacks(monster)->sound));
}

void MuMonsterAttacksUI::RenderClearSkill(MonsterDefinition& monster)
{
    MonsterAttacks& attacks = *ShownAttacks(monster);
    ImGui::SetNextItemWidth(ComboWidth * 2.0f * g_MuEditorCore.GetUIScale());
    if (ImGui::BeginCombo(I18N::Editor::ClearTheMonsterSkill, ClearLabel(attacks.clearSkill)))
    {
        for (const MonsterSkillClear clear : Clears)
        {
            if (ImGui::Selectable(ClearLabel(clear), attacks.clearSkill == clear))
            {
                attacks.clearSkill = clear;
                m_draft.AfterChange(true, StepLabel::Attacks(monster));
            }
        }
        ImGui::EndCombo();
    }
}

void MuMonsterAttacksUI::RenderSound(MonsterDefinition& monster)
{
    MonsterAttacks& attacks = *ShownAttacks(monster);
    if (m_soundMonster != monster.number || m_soundVariant != m_variant)
    {
        m_soundMonster = monster.number;
        m_soundVariant = m_variant;
        m_soundTextUnknown = false;
    }
    RenderNote(I18N::Editor::AttackSoundNote);
    ImGui::SetNextItemWidth(ComboWidth * 2.0f * g_MuEditorCore.GetUIScale());
    Items::InputStringWithHint("##Sound", "SOUND_TRAP01", m_soundText);
    const bool active = ImGui::IsItemActive();
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        // A text that names no sound is not stored; the field says so.
        m_soundTextUnknown = !m_soundText.empty() && !Data::Monsters::FindMonsterSoundId(m_soundText);
        if (!m_soundTextUnknown && m_soundText != attacks.sound)
        {
            attacks.sound = m_soundText;
            if (attacks.sound.empty())
            {
                attacks.soundAtMonster = false;
            }
            m_draft.AfterChange(true, StepLabel::Attacks(monster));
        }
    }
    if (!active && !m_soundTextUnknown)
    {
        m_soundText = attacks.sound;
    }
    if (m_soundTextUnknown)
    {
        ImGui::SameLine();
        RenderWarning(I18N::Editor::AttackSoundUnknown);
    }
    ImGui::BeginDisabled(attacks.sound.empty());
    m_draft.AfterChange(ImGui::Checkbox(I18N::Editor::AttackSoundAtMonster, &attacks.soundAtMonster),
                        StepLabel::Attacks(monster));
    ImGui::EndDisabled();
}

bool MuMonsterAttacksUI::AnimationCombo(const char* id, int& animation, int modelSlot)
{
    bool picked = false;
    ImGui::SetNextItemWidth(ComboWidth * g_MuEditorCore.GetUIScale());
    if (ImGui::BeginCombo(id, AnimationLabel(animation).c_str()))
    {
        std::vector<int> offered = OfferedAnimations(modelSlot);
        offered.insert(offered.begin(), NoAttackAnimation);
        // The current one stays offered, also when the model lacks it.
        if (std::find(offered.begin(), offered.end(), animation) == offered.end())
        {
            offered.push_back(animation);
        }
        for (const int candidate : offered)
        {
            if (ImGui::Selectable(AnimationLabel(candidate).c_str(), candidate == animation) && candidate != animation)
            {
                animation = candidate;
                picked = true;
            }
        }
        ImGui::EndCombo();
    }
    return picked;
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
