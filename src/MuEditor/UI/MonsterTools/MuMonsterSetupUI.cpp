#include "stdafx.h"

#ifdef _EDITOR

#include "MuMonsterSetupUI.h"

#include "MonsterEdits.h"
#include "MonsterSelection.h"
#include "MonsterSetupLists.h"
#include "MonsterShownModel.h"
#include "MonsterStepLabel.h"
#include "MonsterToolWindow.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/Common/MapPicker.h"
#include "../MuEditor/UI/Common/MuEditorLayout.h"
#include "GameLogic/Monsters/MonsterSetupApply.h"
#include "World/GameMaps/MonsterSpawnWorld.h"
#include "World/MapInfra/MapManager.h"
#include "Engine/Object/MonsterModelBody.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/ItemTools/ItemFieldWidgets.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "Core/Globals/_define.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelDatabase.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Data/GameData/MonsterData/MonsterSoundIds.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace MuEditor::Monsters
{
namespace
{
using Data::Monsters::MonsterDefinition;
using Data::Monsters::MonsterSetup;
using SetupFields::Section;
using SetupFields::Takes;

constexpr float LabelWidth = 170.0f;
constexpr ImVec4 AtOnceColor{0.50f, 0.82f, 0.54f, 1.0f};
constexpr ImVec4 NextSpawnColor{0.91f, 0.72f, 0.38f, 1.0f};
constexpr ImVec4 WarningColor{1.0f, 0.4f, 0.4f, 1.0f};

// The values a field takes when it is set: a variant starts from the
// monster's value, the monster's own setup from these.
MonsterSetup MakeStartValues()
{
    MonsterSetup setup;
    setup.scale = 1.0f;
    setup.outline = 1.0f;
    setup.subType = 0;
    setup.lookLevel = 0;
    setup.glowMesh = 0;
    setup.glowLight = 1.0f;
    setup.hiddenMesh = 0;
    setup.shadow = false;
    setup.hideShadow = true;
    setup.alphaTarget = 1.0f;
    setup.light = true;
    setup.pkColor = PVP_NEUTRAL;
    setup.helperPlate = true;
    setup.playerClass = 0;
    setup.classStep = 0;
    setup.startAction = 0;
    setup.startFrame = 0.0f;
    setup.positionOffset = std::array<float, 2>{0.0f, 0.0f};
    setup.tileOffset = std::array<int, 2>{0, 0};
    setup.heightOffset = 0.0f;
    setup.height = 0.0f;
    setup.keepGroundHeight = true;
    setup.boxMin = std::array<float, 3>{-40.0f, -40.0f, 0.0f};
    setup.boxMax = std::array<float, 3>{40.0f, 40.0f, 120.0f};
    setup.facing = 0.0f;
    setup.facingStep = 45.0f;
    setup.randomStart = true;
    setup.spawnSound = std::string(Data::Monsters::GetFirstMonsterSoundName());
    setup.moveSpeed = 10;
    setup.noTurnOnHit = true;
    setup.noBloodPool = true;
    setup.selectable = false;
    setup.live = false;
    setup.deathEffect = 100;
    setup.appear = Data::Monsters::MonsterAppear{};
    setup.teleport = Data::Monsters::MonsterAppear{};
    setup.gate = Data::Monsters::MonsterGate{};
    return setup;
}

const MonsterSetup& StartValues()
{
    static const MonsterSetup values = MakeStartValues();
    return values;
}

// The fields shown as a summary until their own editors come (nested
// values).
bool IsSummaryField(std::string_view key)
{
    return key == "equipment";
}

// The fields of several values, edited on the lines under their label:
// appear, teleport and the gate once set.
bool IsBlockField(std::string_view key)
{
    return key == "appear" || key == "teleport" || key == "gate";
}

// The fields that are lists: on the monster always edited (a row added or
// removed); on a variant edited when it has the list or the monster has none.
bool IsListField(std::string_view key)
{
    constexpr std::array<std::string_view, 5> Lists{"spawnEffects", "spawnBuffs", "randomLook", "facingByTile",
                                                    "hooks"};
    return std::ranges::find(Lists, key) != Lists.end();
}

std::string RowCount(size_t count)
{
    return I18N::Format(I18N::Editor::SetupRowCount, {std::to_string(count)});
}

std::string Number(float value)
{
    char text[32];
    std::snprintf(text, sizeof(text), "%.3g", value);
    return text;
}

std::string YesNo(bool value)
{
    return value ? I18N::Editor::Yes : I18N::Editor::No;
}

std::string ClassName(int playerClass)
{
    const std::span<const char* const> keys = Data::Items::GetClassKeys();
    return playerClass >= 0 && playerClass < static_cast<int>(keys.size()) ? keys[static_cast<size_t>(playerClass)]
                                                                           : std::to_string(playerClass);
}

// The value of a field as text, for a field the rows show but do not edit
// here: the monster's value under a variant, and the summaries.
std::string Describe(std::string_view key, const MonsterSetup& setup)
{
    if (!SetupFields::IsSet(key, setup))
        return I18N::Editor::TheGameSetsIt;
    const auto optionalNumber = [](const auto& value) { return Number(static_cast<float>(*value)); };
    if (key == "scale")
        return setup.scaleByCharacter ? I18N::Editor::SizeOfAPlayer : Number(*setup.scale);
    if (key == "outline")
        return optionalNumber(setup.outline);
    if (key == "subType")
        return optionalNumber(setup.subType);
    if (key == "lookLevel")
        return optionalNumber(setup.lookLevel);
    if (key == "glowMesh")
        return optionalNumber(setup.glowMesh);
    if (key == "glowLight")
        return optionalNumber(setup.glowLight);
    if (key == "hiddenMesh")
        return optionalNumber(setup.hiddenMesh);
    if (key == "shadow")
        return YesNo(*setup.shadow);
    if (key == "hideShadow")
        return YesNo(*setup.hideShadow);
    if (key == "alphaTarget")
        return optionalNumber(setup.alphaTarget);
    if (key == "light")
        return YesNo(*setup.light);
    if (key == "pkColor")
        return std::string(Data::Monsters::GetPkColorName(*setup.pkColor));
    if (key == "helperPlate")
        return YesNo(*setup.helperPlate);
    if (key == "class")
        return ClassName(*setup.playerClass);
    if (key == "classStep")
        return optionalNumber(setup.classStep);
    if (key == "equipment")
    {
        size_t items = 0;
        for (const auto& weapon : setup.equipment.weapons)
            items += weapon.has_value();
        for (const auto* slot :
             {&setup.equipment.helm, &setup.equipment.armor, &setup.equipment.pants, &setup.equipment.gloves,
              &setup.equipment.boots, &setup.equipment.wings, &setup.equipment.helper})
            items += slot->has_value();
        return RowCount(items);
    }
    if (key == "startAction")
        return optionalNumber(setup.startAction);
    if (key == "startFrame")
        return optionalNumber(setup.startFrame);
    if (key == "positionOffset")
        return Number((*setup.positionOffset)[0]) + ", " + Number((*setup.positionOffset)[1]);
    if (key == "tileOffset")
        return std::to_string((*setup.tileOffset)[0]) + ", " + std::to_string((*setup.tileOffset)[1]);
    if (key == "heightOffset")
        return optionalNumber(setup.heightOffset);
    if (key == "height")
        return optionalNumber(setup.height);
    if (key == "keepGroundHeight")
        return YesNo(*setup.keepGroundHeight);
    if (key == "boundingBox")
    {
        const auto box = [](const std::optional<std::array<float, 3>>& corner)
        { return corner ? Number((*corner)[0]) + " " + Number((*corner)[1]) + " " + Number((*corner)[2]) : "-"; };
        return box(setup.boxMin) + " .. " + box(setup.boxMax);
    }
    if (key == "facing")
        return optionalNumber(setup.facing);
    if (key == "facingStep")
        return optionalNumber(setup.facingStep);
    if (key == "facingByTile")
        return RowCount(setup.facingByTile.size());
    if (key == "randomLook")
        return RowCount(setup.randomLook.size());
    if (key == "randomStart")
        return YesNo(setup.randomStart);
    if (key == "spawnBuffs")
        return RowCount(setup.spawnBuffs.size());
    if (key == "spawnSound")
        return *setup.spawnSound;
    if (key == "spawnEffects")
        return RowCount(setup.spawnEffects.size());
    if (key == "appear" || key == "teleport" || key == "gate")
        return I18N::Editor::Yes;
    if (key == "moveSpeed")
        return optionalNumber(setup.moveSpeed);
    if (key == "noTurnOnHit")
        return YesNo(*setup.noTurnOnHit);
    if (key == "noBloodPool")
        return YesNo(*setup.noBloodPool);
    if (key == "selectable")
        return YesNo(*setup.selectable);
    if (key == "live")
        return YesNo(*setup.live);
    if (key == "deathEffect")
        return optionalNumber(setup.deathEffect);
    if (key == "hooks")
        return RowCount(setup.hooks.size());
    return std::string();
}

void RenderTakes(Takes takes)
{
    ImGui::SameLine();
    const bool atOnce = takes == Takes::AtOnce;
    ImGui::TextColored(atOnce ? AtOnceColor : NextSpawnColor, "%s",
                       atOnce ? I18N::Editor::AtOnce : I18N::Editor::NextSpawn);
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", atOnce ? I18N::Editor::LivingMonstersTakeItAtOnce
                                       : I18N::Editor::LivingMonstersTakeItWhenTheyAppearNext);
}

const char* SectionName(Section section)
{
    switch (section)
    {
    case Section::Look:
        return I18N::Editor::Look;
    case Section::Spawn:
        return I18N::Editor::Spawn;
    case Section::Appear:
        return I18N::Editor::AppearAndTeleport;
    case Section::Behaviour:
        return I18N::Editor::Behaviour;
    case Section::GateAndHooks:
        return I18N::Editor::GateAndHooks;
    }
    return "";
}

// The model references the model list offers, sorted, and "player" and "none".
std::vector<std::string> ModelChoices()
{
    std::vector<std::string> models;
    for (const Data::Monsters::MonsterModelDefinition* model : g_MonsterModelDatabase.GetAll())
    {
        const std::optional<int> slot = Data::Monsters::FindMonsterModelSlot(model->slot);
        std::string reference = slot ? Data::Monsters::GetMonsterModelReference(*slot) : std::string();
        if (!reference.empty())
            models.push_back(std::move(reference));
    }
    std::ranges::sort(models);
    models.insert(models.begin(), {"player", "none"});
    return models;
}

// A combo of names; true when one was picked.
bool NameCombo(const char* id, std::string_view shown, std::span<const std::string_view> names, int& picked)
{
    bool changed = false;
    ImGui::SetNextItemWidth(Items::Fields::ScaledWidth(Items::Fields::NumberWidth * 1.5f));
    if (ImGui::BeginCombo(id, std::string(shown).c_str()))
    {
        for (size_t index = 0; index < names.size(); ++index)
        {
            if (ImGui::Selectable(std::string(names[index]).c_str(), names[index] == shown))
            {
                picked = static_cast<int>(index);
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    return changed;
}
} // namespace

MuMonsterSetupUI& MuMonsterSetupUI::GetInstance()
{
    static MuMonsterSetupUI instance;
    return instance;
}

MuMonsterSetupUI::MuMonsterSetupUI()
    : m_draft([](int key) { return ShownValues::Load(key); },
              [](const std::string& label, const ShownValues::ShownMonster& shown)
              { ShownValues::Store(label, shown); })
{
}

void MuMonsterSetupUI::Render(bool* open)
{
    if (ToolWindow::Begin(I18N::Editor::MonsterSetup, WindowIds::MonsterSetup, open))
    {
        const int number = g_MonsterSelection.GetNumber();
        const int key = ShownValues::GetSelectedKey();
        if (key != m_scopeKey)
        {
            m_scopeKey = key;
            m_mapVariant = NoVariant;
            m_stateVariant = NoVariant;
        }
        ShownValues::ShownMonster* shown = m_draft.Begin(key, ToolWindow::GetDataVersion());
        MonsterDefinition* monster = shown != nullptr ? &shown->monster : nullptr;
        ShownModel::RenderSelectedMonster(number, monster, shown != nullptr && shown->wholeMonster);
        if (monster != nullptr)
        {
            m_shown = shown;
            ImGui::PushID(key);
            RenderSetup(*monster);
            ImGui::PopID();
            m_shown = nullptr;
        }
        m_draft.End();
        RenderUnknownMonsters();
    }
    ToolWindow::End();
}

void MuMonsterSetupUI::RenderTakesOf(std::string_view key, Takes takes) const
{
    RenderTakes(takes);
    if (takes != Takes::NextSpawn)
        return;
    const auto found = m_waiting.find(key);
    const int waiting = found != m_waiting.end() ? found->second : 0;
    if (waiting == 0)
        return;
    ImGui::SameLine();
    ImGui::TextColored(NextSpawnColor, "%s", I18N::Format(I18N::Editor::Waiting0, {std::to_string(waiting)}).c_str());
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", I18N::Format(I18N::Editor::WaitingTooltip, {std::to_string(waiting)}).c_str());
}

// The living monsters once a frame; the monsters waiting for their next
// spawn, by field, only when the data, the living monsters, the map or the
// states changed: each count merges every living monster's setup.
void MuMonsterSetupUI::CountLiveAndWaiting()
{
    WaitingSource source;
    source.dataVersion = ToolWindow::GetDataVersion();
    source.numbers = m_numbers;
    for (const int number : m_numbers)
    {
        for (const CHARACTER* character : GameLogic::Monsters::Setup::FindLive(number))
            source.live.push_back(character);
    }
    m_liveCount = static_cast<int>(source.live.size());
    // Without living monsters none waits, and the game's states are not asked.
    if (source.live.empty())
    {
        m_waiting.clear();
        m_waitingCounted = false;
        return;
    }
    source.spawns = GameLogic::Monsters::Setup::GetLiveSpawnCount();
    const GameLogic::Monsters::Setup::SpawnWorld& world = World::Monsters::GetSpawnWorld();
    for (const std::string_view state : Data::Monsters::GetMonsterStates())
        source.states.push_back(world.isStateOn(state));
    source.serverMap = gMapManager.GetServerMap();
    source.serverStage = gMapManager.GetServerStage();
    if (m_waitingCounted && source == m_waitingSource)
        return;
    m_waiting.clear();
    for (const int number : m_numbers)
        GameLogic::Monsters::Setup::CountWaiting(number, world, m_waiting);
    m_waitingSource = std::move(source);
    m_waitingCounted = true;
}

void MuMonsterSetupUI::RenderRespawn()
{
    const int live = m_liveCount;
    ImGui::BeginDisabled(live == 0);
    const std::string label = I18N::Format(I18N::Editor::RespawnLiveMonsters0, {std::to_string(live)}) + "###Respawn";
    if (ImGui::SmallButton(label.c_str()))
    {
        for (const int number : m_numbers)
            World::Monsters::RespawnLive(number, m_playAppear);
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::Checkbox(I18N::Editor::PlayAppear, &m_playAppear);
}

void MuMonsterSetupUI::RenderUnknownMonsters()
{
    if (!ImGui::CollapsingHeader(I18N::Editor::UnknownMonsters))
        return;
    ImGui::PushID("UnknownMonsters");
    ImGui::TextDisabled("%s", I18N::Editor::UnknownMonstersNote);
    if (!m_unknownEditing)
        m_unknown = g_MonsterDatabase.GetUnknownLook();
    Data::Monsters::UnknownMonsterLook& look = m_unknown;
    bool store = false;
    bool active = false;
    const auto row = [](const char* label)
    {
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(label);
        ImGui::SameLine(Items::Fields::ScaledWidth(LabelWidth));
    };
    // A field stores when it is left; a box or a pick at once.
    const auto afterField = [&]
    {
        active |= ImGui::IsItemActive();
        store |= ImGui::IsItemDeactivatedAfterEdit();
    };
    row(I18N::Editor::Model);
    ImGui::SetNextItemWidth(Items::Fields::ScaledWidth(Items::Fields::NumberWidth * 2.5f));
    if (ImGui::BeginCombo("##Model", look.model.c_str(), ImGuiComboFlags_HeightLarge))
    {
        for (const std::string& choice : ModelChoices())
        {
            if (choice != "player" && choice != "none" && ImGui::Selectable(choice.c_str(), choice == look.model))
            {
                look.model = choice;
                store = true;
            }
        }
        ImGui::EndCombo();
    }
    RenderTakes(Takes::NextSpawn);
    row(I18N::Editor::Scale);
    ImGui::SetNextItemWidth(Items::Fields::ScaledWidth(Items::Fields::NumberWidth));
    ImGui::InputFloat("##Scale", &look.scale, 0.0f, 0.0f, "%.2f");
    afterField();
    RenderTakes(Takes::NextSpawn);
    row(I18N::Editor::Glow);
    ImGui::ColorEdit3("##Glow", look.glow.data(), ImGuiColorEditFlags_Float);
    afterField();
    RenderTakes(Takes::AtOnce);
    row(I18N::Editor::RedXMarks);
    store |= ImGui::Checkbox("##Marks", &look.marks);
    RenderTakes(Takes::AtOnce);
    row(I18N::Editor::MessageEverySeconds);
    ImGui::SetNextItemWidth(Items::Fields::ScaledWidth(Items::Fields::NumberWidth));
    ImGui::InputInt("##Message", &look.messageSeconds, 0);
    afterField();
    RenderTakes(Takes::AtOnce);
    row(I18N::Editor::For);
    store |= ImGui::Checkbox(I18N::Editor::ANumberWithoutAnEntry, &look.forNoEntry);
    ImGui::SameLine();
    store |= ImGui::Checkbox(I18N::Editor::AnEntryWithoutAModel, &look.forNoModel);
    ImGui::SameLine();
    store |= ImGui::Checkbox(I18N::Editor::AModelWhoseFileIsMissing, &look.forMissingFile);
    row(I18N::Editor::PreviewAs);
    ImGui::SetNextItemWidth(Items::Fields::ScaledWidth(Items::Fields::NumberWidth));
    ImGui::InputInt("##PreviewNumber", &m_previewNumber, 0);
    m_previewNumber = std::clamp(m_previewNumber, 0, Data::Monsters::MonsterNumberCount - 1);
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::ShowInMonsterPreview))
    {
        g_MonsterSelection.Select(m_previewNumber);
        g_MuEditorCore.GetToolWindows().monsterPreview = true;
        Layout::ShowBeside(WindowIds::MonsterPreview, WindowIds::MonsterSetup);
    }
    m_unknownEditing = active && !store;
    if (store)
    {
        look.scale = std::max(look.scale, 0.01f);
        look.messageSeconds = std::clamp(look.messageSeconds, 0, Data::Monsters::MaxUnknownMessageSeconds);
        g_MonsterEdits.SetUnknownLook(I18N::Editor::UnknownMonsters, look);
        m_unknownEditing = false;
    }
    ImGui::PopID();
}

std::string MuMonsterSetupUI::SetupLabel() const
{
    return m_shown != nullptr ? ShownValues::LogName(*m_shown) + ": " + I18N::Editor::MonsterSetup : std::string();
}

void MuMonsterSetupUI::RenderSetup(MonsterDefinition& monster)
{
    // A variant the monster no longer has (an undo, another tool) shows the
    // monster's setup.
    if (m_mapVariant >= static_cast<int>(monster.mapVariants.size()))
        m_mapVariant = NoVariant;
    if (m_stateVariant >= static_cast<int>(SetupLists::SetupOfMap(monster, m_mapVariant).stateVariants.size()))
        m_stateVariant = NoVariant;
    // The model shown: the map variant's own, else the monster's look (its
    // shown random look's, else its own).
    const int look = ShownModel::GetShownVariant(monster);
    std::string_view modelName = look >= 0 ? monster.randomVariants[static_cast<size_t>(look)].model : monster.model;
    if (m_mapVariant != NoVariant && monster.mapVariants[static_cast<size_t>(m_mapVariant)].model)
        modelName = *monster.mapVariants[static_cast<size_t>(m_mapVariant)].model;
    const std::optional<int> modelSlot = Data::Monsters::FindMonsterModelReference(modelName);
    const Data::Monsters::MonsterModelDefinition* model = modelSlot ? g_MonsterModelDatabase.Find(*modelSlot) : nullptr;
    m_modelBoxMin = Engine::Object::MonsterBody::GetBoxMin(model != nullptr ? &model->body : nullptr);
    // A player-shaped monster has the player's box, as its character slot.
    m_modelBoxMax = model == nullptr && modelSlot == MODEL_PLAYER
                        ? Engine::Object::MonsterBody::PlayerBoxMax
                        : Engine::Object::MonsterBody::GetBoxMax(model != nullptr ? &model->body : nullptr);
    // A map variant's state variant falls back to the map variant's fields,
    // then the monster's.
    if (m_mapVariant != NoVariant && m_stateVariant != NoVariant)
    {
        m_mapParent = monster.setup;
        const MonsterSetup& map = SetupLists::SetupOfMap(monster, m_mapVariant);
        for (const Data::Monsters::MonsterSetupField& field : Data::Monsters::GetMonsterSetupFields())
        {
            if (field.key != "stateVariants" && field.isSet(map))
                field.copy(m_mapParent, map);
        }
    }
    if (!m_shown->wholeMonster && m_shown->numberCount > 1)
    {
        ImGui::TextDisabled(
            "%s", I18N::Format(I18N::Editor::SetupSharedHint,
                               {std::to_string(m_shown->numberCount), ShownValues::MonsterName(monster.number)})
                      .c_str());
    }
    // The maps the selection appears on (its numbers' maps).
    m_numbers =
        m_shown->wholeMonster ? g_MonsterEdits.ReadMonsterNumbers(monster.number) : std::vector<int>{monster.number};
    CountLiveAndWaiting();
    const std::vector<Data::Maps::MapRef> appearsOn = g_MonsterEdits.ReadMaps(m_numbers);
    ImGui::TextDisabled(
        "%s", I18N::Format(I18N::Editor::AppearsOn0, {MapPicker::Describe(appearsOn, I18N::Editor::NoMapYet)}).c_str());
    RenderRespawn();
    RenderMapRow(monster);
    RenderStateRow(monster);
    RenderVariantLine(monster);
    ImGui::Separator();
    for (const Section section :
         {Section::Look, Section::Spawn, Section::Appear, Section::Behaviour, Section::GateAndHooks})
    {
        RenderSection(section, monster);
    }
}

void MuMonsterSetupUI::RenderMapRow(MonsterDefinition& monster)
{
    ImGui::PushID("MapRow");
    ImGui::TextUnformatted(I18N::Editor::Map);
    ImGui::SameLine();
    if (ImGui::RadioButton(I18N::Editor::AllMaps, m_mapVariant == NoVariant) && m_mapVariant != NoVariant)
    {
        // A picked state was the map variant's.
        m_mapVariant = NoVariant;
        m_stateVariant = NoVariant;
    }
    for (size_t index = 0; index < monster.mapVariants.size(); ++index)
    {
        ImGui::SameLine();
        const std::string name = MapPicker::Describe(monster.mapVariants[index].maps, I18N::Editor::NoMapYet) +
                                 "##Variant" + std::to_string(index);
        if (ImGui::RadioButton(name.c_str(), m_mapVariant == static_cast<int>(index)))
        {
            m_mapVariant = static_cast<int>(index);
            m_stateVariant = NoVariant;
        }
    }
    ImGui::SameLine();
    const OwnValues::State state = OwnValues::GetState(*m_shown, Data::Monsters::MonsterValueField::MapVariants);
    OwnValues::BeginField(state);
    const bool add = ImGui::SmallButton(I18N::Editor::AddAMapVariant);
    OwnValues::EndField(state);
    if (add)
    {
        // It changes nothing until a field is set on it, on the maps picked next.
        monster.mapVariants.emplace_back();
        m_mapVariant = static_cast<int>(monster.mapVariants.size()) - 1;
        m_stateVariant = NoVariant;
        m_draft.AfterChange(true, SetupLabel());
    }
    ImGui::PopID();
}

void MuMonsterSetupUI::RenderStateRow(MonsterDefinition& monster)
{
    ImGui::PushID("StateRow");
    ImGui::TextUnformatted(I18N::Editor::State);
    ImGui::SameLine();
    if (ImGui::RadioButton(I18N::Editor::Always, m_stateVariant == NoVariant))
        m_stateVariant = NoVariant;
    // The picked map variant's own state variants, or the monster's.
    std::vector<Data::Monsters::MonsterStateVariant>& variants =
        SetupLists::SetupOfMap(monster, m_mapVariant).stateVariants;
    for (size_t index = 0; index < variants.size(); ++index)
    {
        ImGui::SameLine();
        const std::string name = variants[index].state + "##State" + std::to_string(index);
        if (ImGui::RadioButton(name.c_str(), m_stateVariant == static_cast<int>(index)))
            m_stateVariant = static_cast<int>(index);
    }
    std::vector<std::string_view> free;
    for (const std::string_view state : Data::Monsters::GetMonsterStates())
    {
        if (std::ranges::none_of(variants, [&](const auto& variant) { return variant.state == state; }))
            free.push_back(state);
    }
    if (!free.empty())
    {
        ImGui::SameLine();
        const OwnValues::State own = FieldState("stateVariants");
        OwnValues::BeginField(own);
        int picked = -1;
        const bool add = NameCombo("##AddState", I18N::Editor::AddAStateVariant, free, picked);
        OwnValues::EndField(own);
        if (add && picked >= 0)
        {
            // A map variant's first state variant replaces the monster's on its
            // maps: it starts from them, so they keep applying there.
            if (m_mapVariant != NoVariant && variants.empty())
                variants = monster.setup.stateVariants;
            const std::string state(free[static_cast<size_t>(picked)]);
            const auto found = std::ranges::find(variants, state, &Data::Monsters::MonsterStateVariant::state);
            if (found != variants.end())
            {
                m_stateVariant = static_cast<int>(found - variants.begin());
            }
            else
            {
                Data::Monsters::MonsterStateVariant variant;
                variant.state = state;
                variants.push_back(std::move(variant));
                m_stateVariant = static_cast<int>(variants.size()) - 1;
            }
            m_draft.AfterChange(true, SetupLabel());
        }
    }
    if (const std::string inherited = SetupLists::MonsterStatesOnMap(monster, m_mapVariant); !inherited.empty())
        ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::MapVariantMonsterStates0, {inherited}).c_str());
    RenderOwnStatesNote(monster);
    ImGui::PopID();
}

void MuMonsterSetupUI::RenderOwnStatesNote(const MonsterDefinition& monster) const
{
    // A map variant's own state variants replace the monster's on its maps,
    // so a change of the monster's does not reach them: said on both sides.
    if (m_mapVariant != NoVariant)
    {
        if (!SetupLists::SetupOfMap(monster, m_mapVariant).stateVariants.empty())
            ImGui::TextDisabled("%s", I18N::Editor::MapVariantOwnStatesNote);
        return;
    }
    std::string maps;
    for (const Data::Monsters::MonsterMapVariant& variant : monster.mapVariants)
    {
        if (variant.setup.stateVariants.empty())
            continue;
        if (!maps.empty())
            maps += "; ";
        maps += MapPicker::Describe(variant.maps, I18N::Editor::NoMapYet);
    }
    if (!maps.empty())
        ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::OwnMapVariantStates0, {maps}).c_str());
}

void MuMonsterSetupUI::RenderVariantLine(MonsterDefinition& monster)
{
    if (m_mapVariant != NoVariant)
    {
        ImGui::PushID("MapVariant");
        Data::Monsters::MonsterMapVariant& variant = monster.mapVariants[static_cast<size_t>(m_mapVariant)];
        ImGui::TextDisabled("%s", I18N::Editor::SetupMapVariantNote);
        const OwnValues::State state = FieldState("");
        OwnValues::BeginField(state);
        ImGui::TextUnformatted(I18N::Editor::PlaysOn);
        ImGui::SameLine();
        if (MapPicker::Render("VariantMaps", variant.maps, I18N::Editor::NoMapYet))
            m_draft.AfterChange(true, SetupLabel());
        ImGui::SameLine();
        const bool remove = ImGui::SmallButton(I18N::Editor::RemoveTheMapVariant);
        OwnValues::EndField(state);
        if (variant.maps.empty())
            ImGui::TextColored(WarningColor, "%s",
                               I18N::Editor::PickTheMapsOfTheVariantUntilThenSaveRefusesTheMonsters);
        if (remove)
        {
            monster.mapVariants.erase(monster.mapVariants.begin() + m_mapVariant);
            m_mapVariant = NoVariant;
            m_stateVariant = NoVariant;
            m_draft.AfterChange(true, SetupLabel());
            ImGui::PopID();
            return;
        }
        ImGui::PopID();
    }
    if (m_stateVariant != NoVariant)
    {
        ImGui::PushID("StateVariant");
        ImGui::TextDisabled("%s", I18N::Editor::SetupStateVariantNote);
        const OwnValues::State state = FieldState("");
        OwnValues::BeginField(state);
        const bool remove = ImGui::SmallButton(I18N::Editor::RemoveTheStateVariant);
        OwnValues::EndField(state);
        if (remove)
        {
            std::vector<Data::Monsters::MonsterStateVariant>& states =
                SetupLists::SetupOfMap(monster, m_mapVariant).stateVariants;
            states.erase(states.begin() + m_stateVariant);
            m_stateVariant = NoVariant;
            m_draft.AfterChange(true, SetupLabel());
        }
        ImGui::PopID();
    }
}

MonsterSetup& MuMonsterSetupUI::EditedSetup(MonsterDefinition& monster)
{
    MonsterSetup& map = SetupLists::SetupOfMap(monster, m_mapVariant);
    if (m_stateVariant != NoVariant)
        return map.stateVariants[static_cast<size_t>(m_stateVariant)].setup;
    return map;
}

const MonsterSetup* MuMonsterSetupUI::InheritedSetup(const MonsterDefinition& monster) const
{
    if (m_mapVariant != NoVariant && m_stateVariant != NoVariant)
        return &m_mapParent;
    return m_mapVariant != NoVariant || m_stateVariant != NoVariant ? &monster.setup : nullptr;
}

OwnValues::State MuMonsterSetupUI::FieldState(std::string_view key) const
{
    using Data::Monsters::MonsterValueField;
    if (m_mapVariant != NoVariant)
        return OwnValues::GetState(*m_shown, MonsterValueField::MapVariants);
    if (m_stateVariant != NoVariant)
        key = "stateVariants";
    const std::optional<MonsterValueField> field = SetupFields::GetValueField(key);
    return field ? OwnValues::GetState(*m_shown, *field) : OwnValues::State::Plain;
}

void MuMonsterSetupUI::RenderModel(MonsterDefinition& monster)
{
    // The model is the monster's on all maps; a map variant may name its own.
    // A state variant has none.
    if (m_stateVariant != NoVariant)
        return;
    std::optional<std::string>* variantModel =
        m_mapVariant != NoVariant ? &monster.mapVariants[static_cast<size_t>(m_mapVariant)].model : nullptr;
    const std::string model = variantModel != nullptr ? variantModel->value_or(monster.model) : monster.model;
    ImGui::PushID("model");
    const OwnValues::State state = m_mapVariant != NoVariant
                                       ? FieldState("")
                                       : OwnValues::GetState(*m_shown, Data::Monsters::MonsterValueField::Model);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(I18N::Editor::Model);
    ImGui::SameLine(Items::Fields::ScaledWidth(LabelWidth));
    OwnValues::BeginField(state);
    ImGui::SetNextItemWidth(Items::Fields::ScaledWidth(Items::Fields::NumberWidth * 2.5f));
    bool changed = false;
    if (ImGui::BeginCombo("##Model", model.c_str(), ImGuiComboFlags_HeightLarge))
    {
        for (const std::string& choice : ModelChoices())
        {
            if (ImGui::Selectable(choice.c_str(), choice == model) && choice != model)
            {
                if (variantModel == nullptr)
                    monster.model = choice;
                else if (choice == monster.model)
                    variantModel->reset(); // the monster's again: no model of its own
                else
                    *variantModel = choice;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    OwnValues::EndField(state);
    RenderTakesOf("model", Takes::NextSpawn);
    if (model == "player")
        ImGui::TextDisabled("%s", I18N::Editor::SetupPlayerModelNote);
    m_draft.AfterChange(changed, SetupLabel());
    ImGui::PopID();
}

void MuMonsterSetupUI::RenderSection(Section section, MonsterDefinition& monster)
{
    if (!ImGui::CollapsingHeader(SectionName(section), section == Section::Look ? ImGuiTreeNodeFlags_DefaultOpen : 0))
        return;
    if (section == Section::Look)
        RenderModel(monster);
    for (const SetupFields::Field& field : SetupFields::GetFields())
    {
        // The state variants are the State row.
        if (field.section == section && field.key != "stateVariants")
            RenderField(field, monster);
    }
}

void MuMonsterSetupUI::RenderField(const SetupFields::Field& field, MonsterDefinition& monster)
{
    const std::string key(field.key);
    MonsterSetup& setup = EditedSetup(monster);
    const MonsterSetup* inherited = InheritedSetup(monster);
    const bool set = SetupFields::IsSet(key, setup);
    const OwnValues::State state = FieldState(key);
    ImGui::PushID(key.c_str());
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(*field.label);
    if (inherited == nullptr && state != OwnValues::State::Plain && ImGui::BeginPopupContextItem("##Own"))
    {
        if (const std::optional<Data::Monsters::MonsterValueField> valueField = SetupFields::GetValueField(key))
            OwnValues::RenderMenuItems(monster.number, *valueField);
        ImGui::EndPopup();
    }
    ImGui::SameLine(Items::Fields::ScaledWidth(LabelWidth));
    OwnValues::BeginField(state);
    bool changed = false;
    const bool listEditor =
        (IsListField(key) && (inherited == nullptr || set || !SetupFields::IsSet(key, *inherited))) ||
        (IsBlockField(key) && set);
    if (IsSummaryField(key))
    {
        ImGui::TextUnformatted(Describe(key, set || inherited == nullptr ? setup : *inherited).c_str());
        // The equipment has a tool of its own (Q-MN33).
        ImGui::SameLine();
        if (ImGui::SmallButton(I18N::Editor::OpenMonsterEquipment))
        {
            g_MuEditorCore.GetToolWindows().monsterEquipment = true;
            Layout::ShowBeside(WindowIds::MonsterEquipment, WindowIds::MonsterSetup);
        }
    }
    else if (listEditor)
    {
        // A list starts on the next line, after when it takes effect.
        RenderTakesOf(field.key, field.takes);
        if ((inherited != nullptr || IsBlockField(key)) && set)
        {
            ImGui::SameLine();
            if (ImGui::SmallButton("x"))
            {
                SetupFields::ClearField(key, setup);
                changed = true;
            }
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", inherited != nullptr ? I18N::Editor::UseTheMonsterValue
                                                             : I18N::Editor::LeaveItToTheGame);
        }
        ImGui::Indent();
        changed |= !changed && RenderValue(key, setup, SetupLabel());
        ImGui::Unindent();
    }
    else if (!set)
    {
        // Not set: the game's value, or the monster's under a variant.
        ImGui::TextDisabled("%s", Describe(key, inherited != nullptr ? *inherited : setup).c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton(I18N::Editor::Set))
        {
            const bool fromMonster = inherited != nullptr && SetupFields::IsSet(key, *inherited);
            SetupFields::CopyField(key, setup, fromMonster ? *inherited : StartValues());
            // A box starts as the model's.
            if (!fromMonster && key == "boundingBox")
            {
                setup.boxMin = m_modelBoxMin;
                setup.boxMax = m_modelBoxMax;
            }
            changed = true;
        }
    }
    else
    {
        changed = RenderValue(key, setup, SetupLabel());
        ImGui::SameLine();
        if (ImGui::SmallButton("x"))
        {
            SetupFields::ClearField(key, setup);
            changed = true;
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s",
                              inherited != nullptr ? I18N::Editor::UseTheMonsterValue : I18N::Editor::LeaveItToTheGame);
    }
    OwnValues::EndField(state);
    if (!listEditor)
        RenderTakesOf(field.key, field.takes);
    m_draft.AfterChange(changed, SetupLabel());
    ImGui::PopID();
}

bool MuMonsterSetupUI::RenderValue(std::string_view key, MonsterSetup& setup, const std::string& undoLabel)
{
    namespace Fields = Items::Fields;
    const float width = Fields::ScaledWidth(Fields::NumberWidth);
    const auto floatField = [&](std::optional<float>& value)
    {
        ImGui::SetNextItemWidth(width);
        ImGui::InputFloat("##Value", &*value, 0.0f, 0.0f, "%.3f");
        m_draft.AfterField(undoLabel);
        return false;
    };
    // Kept to the range the reader takes.
    const auto intField = [&](std::optional<int>& value, int low = std::numeric_limits<int>::min(),
                              int high = std::numeric_limits<int>::max())
    {
        ImGui::SetNextItemWidth(width);
        if (ImGui::InputInt("##Value", &*value))
            *value = std::clamp(*value, low, high);
        m_draft.AfterField(undoLabel);
        return false;
    };
    const auto boolField = [&](std::optional<bool>& value)
    {
        bool checked = *value;
        if (!ImGui::Checkbox("##Value", &checked))
            return false;
        value = checked;
        return true;
    };
    if (key == "scale")
    {
        bool byCharacter = setup.scaleByCharacter;
        if (!byCharacter)
        {
            if (!setup.scale)
                setup.scale = 1.0f;
            floatField(setup.scale);
            ImGui::SameLine();
        }
        if (ImGui::Checkbox(I18N::Editor::SizeOfAPlayer, &byCharacter))
        {
            setup.scaleByCharacter = byCharacter;
            setup.scale = byCharacter ? std::nullopt : std::optional<float>(1.0f);
            return true;
        }
        return false;
    }
    if (key == "outline")
        return floatField(setup.outline);
    if (key == "subType")
        return intField(setup.subType);
    if (key == "lookLevel")
        return intField(setup.lookLevel, 0, Data::Monsters::MaxLookLevel);
    if (key == "glowMesh")
        return intField(setup.glowMesh);
    if (key == "glowLight")
        return floatField(setup.glowLight);
    if (key == "hiddenMesh")
        return intField(setup.hiddenMesh);
    if (key == "shadow")
        return boolField(setup.shadow);
    if (key == "hideShadow")
        return boolField(setup.hideShadow);
    if (key == "alphaTarget")
        return floatField(setup.alphaTarget);
    if (key == "light")
        return boolField(setup.light);
    if (key == "pkColor")
    {
        static const std::array<std::string_view, 6> Colors{"hero2",   "hero1",     "neutral",
                                                            "caution", "murderer1", "murderer2"};
        int picked = -1;
        if (!NameCombo("##Value", Data::Monsters::GetPkColorName(*setup.pkColor), Colors, picked))
            return false;
        setup.pkColor = Data::Monsters::FindPkColor(Colors[static_cast<size_t>(picked)]);
        return true;
    }
    if (key == "helperPlate")
        return boolField(setup.helperPlate);
    if (key == "class")
    {
        std::vector<std::string_view> classes;
        for (const char* name : Data::Items::GetClassKeys())
            classes.emplace_back(name);
        int picked = -1;
        if (!NameCombo("##Value", ClassName(*setup.playerClass), classes, picked))
            return false;
        setup.playerClass = picked;
        return true;
    }
    if (key == "classStep")
        return intField(setup.classStep, 0, Data::Monsters::MaxClassStep);
    if (key == "startAction")
        return intField(setup.startAction);
    if (key == "startFrame")
        return floatField(setup.startFrame);
    if (key == "positionOffset")
    {
        ImGui::SetNextItemWidth(width * 1.5f);
        ImGui::InputFloat2("##Value", setup.positionOffset->data(), "%.1f");
        m_draft.AfterField(undoLabel);
        return false;
    }
    if (key == "tileOffset")
    {
        ImGui::SetNextItemWidth(width * 1.5f);
        if (ImGui::InputInt2("##Value", setup.tileOffset->data()))
        {
            for (int& offset : *setup.tileOffset)
                offset = std::clamp(offset, -Data::Monsters::MaxTileOffset, Data::Monsters::MaxTileOffset);
        }
        m_draft.AfterField(undoLabel);
        return false;
    }
    if (key == "heightOffset")
        return floatField(setup.heightOffset);
    if (key == "height")
        return floatField(setup.height);
    if (key == "keepGroundHeight")
        return boolField(setup.keepGroundHeight);
    if (key == "boundingBox")
    {
        // Each corner is the model's until its own box is ticked, so a
        // corner the file leaves out is never written by drawing it.
        bool changed = false;
        const auto corner = [&](const char* label, const char* id, std::optional<std::array<float, 3>>& value,
                                const std::optional<std::array<float, 3>>& start)
        {
            bool own = value.has_value();
            if (ImGui::Checkbox(label, &own))
            {
                value = own ? start : std::nullopt;
                changed = true;
            }
            ImGui::SameLine();
            if (value)
            {
                ImGui::SetNextItemWidth(width * 2.0f);
                ImGui::InputFloat3(id, value->data(), "%.0f");
                m_draft.AfterField(undoLabel);
            }
            else
            {
                ImGui::TextDisabled("%s", I18N::Editor::AsTheModel);
            }
        };
        // A corner ticked on starts from the model's.
        corner(I18N::Editor::BoxMin, "##Min", setup.boxMin, m_modelBoxMin);
        ImGui::SameLine();
        corner(I18N::Editor::BoxMax, "##Max", setup.boxMax, m_modelBoxMax);
        return changed;
    }
    if (key == "facing")
        return floatField(setup.facing);
    if (key == "facingStep")
        return floatField(setup.facingStep);
    if (key == "randomStart")
    {
        bool checked = setup.randomStart;
        if (!ImGui::Checkbox("##Value", &checked))
            return false;
        setup.randomStart = checked;
        return true;
    }
    if (key == "spawnSound")
    {
        ImGui::SetNextItemWidth(width * 2.0f);
        Items::InputString("##Value", *setup.spawnSound);
        m_draft.AfterField(undoLabel);
        if (!Data::Monsters::FindMonsterSoundId(*setup.spawnSound))
        {
            ImGui::SameLine();
            ImGui::TextColored(WarningColor, "%s",
                               I18N::Format(I18N::Editor::UnknownName, {*setup.spawnSound}).c_str());
        }
        return false;
    }
    if (key == "moveSpeed")
        return intField(setup.moveSpeed, 0, Data::Monsters::MaxMoveSpeed);
    if (key == "noTurnOnHit")
        return boolField(setup.noTurnOnHit);
    if (key == "noBloodPool")
        return boolField(setup.noBloodPool);
    if (key == "selectable")
        return boolField(setup.selectable);
    if (key == "live")
        return boolField(setup.live);
    if (key == "deathEffect")
        return intField(setup.deathEffect);
    const auto afterField = [&] { m_draft.AfterField(undoLabel); };
    if (key == "spawnEffects")
        return SetupLists::SpawnEffects(setup.spawnEffects, afterField, SetupLists::EffectRows::Spawn);
    if (key == "spawnBuffs")
        return SetupLists::SpawnBuffs(setup.spawnBuffs);
    if (key == "randomLook")
        return SetupLists::RandomLook(setup.randomLook, afterField);
    if (key == "facingByTile")
        return SetupLists::FacingByTile(setup.facingByTile, afterField);
    if (key == "appear" && setup.appear)
        return SetupLists::Appear(*setup.appear, afterField);
    if (key == "teleport" && setup.teleport)
        return SetupLists::Appear(*setup.teleport, afterField);
    if (key == "gate" && setup.gate)
        return SetupLists::Gate(*setup.gate);
    if (key == "hooks")
        return SetupLists::Hooks(setup.hooks);
    return false;
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
