#include "stdafx.h"

#ifdef _EDITOR

#include "MuMonsterAttacksUI.h"

#include "MonsterAttackReaders.h"
#include "MonsterEdits.h"
#include "MonsterSelection.h"
#include "MonsterShownModel.h"
#include "MonsterStepLabel.h"
#include "MonsterToolWindow.h"

#include "../MuEditor/Core/MuEditorCore.h"
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
    case Reader::Hellas:
        return I18N::Editor::ReaderHellas;
    case Reader::BattleCastleGuards:
        return I18N::Editor::ReaderBattleCastleGuards;
    case Reader::HuntingGround:
        return I18N::Editor::ReaderHuntingGround;
    case Reader::Aida:
        return I18N::Editor::ReaderAida;
    case Reader::CrywolfAndDevilSquare:
        return I18N::Editor::ReaderCrywolfAndDevilSquare;
    case Reader::Kanturu1st:
        return I18N::Editor::ReaderKanturu1st;
    case Reader::Kanturu2ndAnd3rd:
        return I18N::Editor::ReaderKanturu2ndAnd3rd;
    case Reader::Kanturu3rd:
        return I18N::Editor::ReaderKanturu3rd;
    case Reader::BalgassBarracks:
        return I18N::Editor::ReaderBalgassBarracks;
    case Reader::IllusionTemple:
        return I18N::Editor::ReaderIllusionTemple;
    case Reader::SwampOfQuiet:
        return I18N::Editor::ReaderSwampOfQuiet;
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
    case Reader::Bali:
        return I18N::Editor::ReaderBali;
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
    if (!monster.attacks)
    {
        RenderNote(I18N::Editor::TheMonsterHasNoAttacksItPlaysTheGenericPattern);
        if (ImGui::Button(I18N::Editor::AddAttacks))
        {
            monster.attacks.emplace();
            m_draft.AfterChange(true, label);
        }
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
        RenderClearSkill(monster);
    }
    if (ImGui::CollapsingHeader(I18N::Editor::AttackSound, ImGuiTreeNodeFlags_DefaultOpen))
    {
        RenderSound(monster);
    }
    ImGui::Separator();
    if (ImGui::Button(I18N::Editor::RemoveTheAttacks))
    {
        monster.attacks.reset();
        m_draft.AfterChange(true, label);
    }
}

void MuMonsterAttacksUI::RenderReaders(const MonsterDefinition& monster, int modelSlot)
{
    const std::vector<AttackReaders::Reader> readers = AttackReaders::FindReaders(monster.number, modelSlot);
    if (readers.empty())
    {
        RenderWarning(I18N::Editor::NoCodeReadsTheseAttacks);
        return;
    }
    ImGui::TextUnformatted(I18N::Editor::ReadBy);
    for (const AttackReaders::Reader reader : readers)
    {
        ImGui::BulletText("%s", ReaderLabel(reader));
    }
}

void MuMonsterAttacksUI::RenderEntries(MonsterDefinition& monster, int modelSlot, bool monsterSkills)
{
    std::vector<Data::Monsters::MonsterAttackEntry>& entries =
        monsterSkills ? monster.attacks->monsterSkills : monster.attacks->skills;
    RenderNote(monsterSkills ? I18N::Editor::MonsterSkillsNote : I18N::Editor::PlayerSkillsNote);
    const std::string label = StepLabel::Attacks(monster);
    std::optional<size_t> removed;
    const float scale = g_MuEditorCore.GetUIScale();
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
    MonsterAttacks& attacks = *monster.attacks;
    const std::string label = StepLabel::Attacks(monster);
    if (ImGui::RadioButton(I18N::Editor::AnAnimation, !attacks.otherwiseRandom))
    {
        attacks.otherwiseRandom.reset();
        m_draft.AfterChange(true, label);
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(attacks.otherwiseRandom.has_value());
    if (AnimationCombo("##Otherwise", attacks.otherwise, modelSlot))
    {
        m_draft.AfterChange(true, label);
    }
    ImGui::EndDisabled();
    if (ImGui::RadioButton(I18N::Editor::ADraw, attacks.otherwiseRandom.has_value()) && !attacks.otherwiseRandom)
    {
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

void MuMonsterAttacksUI::RenderClearSkill(MonsterDefinition& monster)
{
    MonsterAttacks& attacks = *monster.attacks;
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
    MonsterAttacks& attacks = *monster.attacks;
    if (m_soundMonster != monster.number)
    {
        m_soundMonster = monster.number;
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
