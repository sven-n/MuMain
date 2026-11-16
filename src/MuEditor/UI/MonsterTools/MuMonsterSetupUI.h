#pragma once

#ifdef _EDITOR

#include "MonsterOwnValues.h"
#include "MonsterSetupFields.h"
#include "MonsterShownValues.h"

#include "../MuEditor/UI/ItemTools/ItemEditDraft.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"
#include "Data/GameData/MonsterData/UnknownMonsterLook.h"

#include <array>
#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace MuEditor::Monsters
{
// The Monster Setup tool (monsters design, section 9, D63): what the selected
// monster's character gets when it appears, in collapsing sections (Look,
// Spawn, Appear and teleport, Behaviour, Gate and hooks). The Map and State
// rows pick what the rows show and edit: the monster's setup, a map
// variant's or a state variant's, which change only the fields they have.
// Each field says when a living monster takes a change: at once or when it
// appears next (D55). A monster with several numbers as a whole edits the
// setup its numbers share; a number's shared fields are grey and locked
// until made its own (right-click, Q-MN96). Each change is an undo step of
// the monster tools.
class MuMonsterSetupUI
{
public:
    static MuMonsterSetupUI& GetInstance();

    void Render(bool* open);

    // The Map and State rows' choice: the monster's setup, or the variant at
    // this index.
    static constexpr int NoVariant = -1;

private:
    using MonsterDraft = Items::ItemEditDraft<ShownValues::ShownMonster>;

    MuMonsterSetupUI();

    void RenderSetup(Data::Monsters::MonsterDefinition& monster);
    // All maps, each map variant, Add a map variant; Always, each state
    // variant, Add a state variant.
    void RenderMapRow(Data::Monsters::MonsterDefinition& monster);
    void RenderStateRow(Data::Monsters::MonsterDefinition& monster);
    // Under All maps the map variants whose own state variants replace the
    // monster's; on such a map variant that its own replace them.
    void RenderOwnStatesNote(const Data::Monsters::MonsterDefinition& monster) const;
    // The picked variant's maps or state, and its Remove.
    void RenderVariantLine(Data::Monsters::MonsterDefinition& monster);
    // The setup the rows edit, and the setup the variant takes the other
    // fields from (nullptr for the monster's setup): the monster's, or for a
    // map variant's state variant the map variant's fields over it.
    Data::Monsters::MonsterSetup& EditedSetup(Data::Monsters::MonsterDefinition& monster);
    const Data::Monsters::MonsterSetup* InheritedSetup(const Data::Monsters::MonsterDefinition& monster) const;
    void RenderModel(Data::Monsters::MonsterDefinition& monster);
    void RenderSection(SetupFields::Section section, Data::Monsters::MonsterDefinition& monster);
    void RenderField(const SetupFields::Field& field, Data::Monsters::MonsterDefinition& monster);
    // The field's control on a setup that has it; true when a control that
    // changes at once changed it.
    bool RenderValue(std::string_view key, Data::Monsters::MonsterSetup& setup, const std::string& undoLabel);
    // The field's own-value state, by where the rows edit: the field itself
    // on the monster's setup, the map or state variants on a variant.
    OwnValues::State FieldState(std::string_view key) const;
    // When living monsters take a change of the field, and for a field of the
    // next spawn how many living monsters of the selection still wait for it.
    void RenderTakesOf(std::string_view key, SetupFields::Takes takes) const;
    // Respawn live monsters (D55) with its count, and Play appear.
    // The selection's living monsters and those waiting for their next spawn.
    void CountLiveAndWaiting();
    void RenderRespawn();
    // The unknown monsters' look (D64), for every monster: its body, size,
    // glow, marks, message and who gets it, and Preview as a number.
    void RenderUnknownMonsters();
    // The talk texts several NPCs share (the list's talkTexts): each set's
    // texts in English, Add a set and x.
    void RenderSharedTalkTexts();
    std::string SetupLabel() const;

    MonsterDraft m_draft;
    // While the window draws: the shown values.
    const ShownValues::ShownMonster* m_shown = nullptr;
    // The selection the variant choices belong to; a new one shows the
    // monster's setup.
    int m_scopeKey = -1;
    int m_mapVariant = NoVariant;
    // The state variant, of the picked map variant's or the monster's own.
    int m_stateVariant = NoVariant;
    // The fields a map variant's state variant falls back to, made a frame.
    Data::Monsters::MonsterSetup m_mapParent;
    // The box of the model shown (the map variant's or the monster's), made a
    // frame: a corner ticked on starts from it.
    std::array<float, 3> m_modelBoxMin{};
    std::array<float, 3> m_modelBoxMax{};
    // The selection's numbers while the window draws.
    std::vector<int> m_numbers;
    // How many of their living monsters wait for their next spawn, by field.
    std::map<std::string, int, std::less<>> m_waiting;
    // What the counts were made from: they are made again when it changes.
    struct WaitingSource
    {
        int dataVersion = 0;
        std::vector<int> numbers;
        std::vector<const void*> live;
        int spawns = 0;
        std::vector<bool> states;
        int serverMap = 0;
        int serverStage = 0;

        bool operator==(const WaitingSource&) const = default;
    };
    WaitingSource m_waitingSource;
    bool m_waitingCounted = false;
    // The selection's living monsters, found once a frame.
    int m_liveCount = 0;
    bool m_playAppear = false;
    // The look while a field of it is edited, and the number Preview as shows.
    Data::Monsters::UnknownMonsterLook m_unknown;
    bool m_unknownEditing = false;
    // The shared talk texts while a field of them is edited, and the name a
    // new set gets.
    Data::Monsters::MonsterTalkTexts m_talkTexts;
    bool m_talkTextsEditing = false;
    std::string m_newSetName;
    int m_previewNumber = 700;
};
} // namespace MuEditor::Monsters

#define g_MuMonsterSetupUI MuEditor::Monsters::MuMonsterSetupUI::GetInstance()

#endif // _EDITOR
