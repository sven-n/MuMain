#pragma once

#ifdef _EDITOR

#include "MonsterSoundEventsEditor.h"

#include "Data/GameData/MonsterData/MonsterDefinition.h"
#include "Data/GameData/MonsterData/MonsterModelDefinition.h"
#include "Data/GameData/MonsterData/MonsterSoundSet.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace MuEditor::Monsters
{
// The Monster Sounds tool (monsters design, Q-MN78 to Q-MN83): the sound set
// the selected monster names and the sets of its random variants; the sound
// sets, added, copied, renamed and removed, with their sound files, sound
// events and maps, and the monsters that play them; on a cue's event the
// frames of the cue of the shown look's model.
class MuMonsterSoundsUI
{
public:
    static MuMonsterSoundsUI& GetInstance();

    void Render(bool* open);

    // Shows the set, with the event marked and scrolled to once (Monster
    // Preview's cue tags, Q-MN80); opens and focuses the tool.
    void ShowEvent(const std::string& set, std::optional<size_t> event);

    // The model of the shown look with a cue edit that is not stored yet, for
    // Monster Preview's tags; nullptr when the tool shows no draft of the slot.
    const Data::Monsters::MonsterModelDefinition* GetShownModelDraft(int slot) const;

private:
    MuMonsterSoundsUI();

    void FollowSelection(const Data::Monsters::MonsterDefinition* monster);
    void RenderMonster(const Data::Monsters::MonsterDefinition& monster);
    void RenderVariants(const Data::Monsters::MonsterDefinition& monster);
    void RenderSetList();
    void RenderSet(Data::Monsters::MonsterSoundSet& set, const SoundEventsEditor::Context& context);
    void RenderName(const Data::Monsters::MonsterSoundSet& set);
    void RenderUsers(const Data::Monsters::MonsterSoundSet& set);
    void RenderFiles(Data::Monsters::MonsterSoundSet& set);
    void RenderAddFile(Data::Monsters::MonsterSoundSet& set);
    // A combo of the sets; `noneLabel` is the choice of no set. True when
    // another was picked.
    bool SetCombo(const char* id, std::string& value, const char* noneLabel);
    // The monsters that name each set, also in a random variant.
    const std::map<std::string, std::vector<int>, std::less<>>& GetUsers();
    // A number for the name, for the set draft, which keys its item by number.
    int IdOf(const std::string& name);

    SoundEventsEditor::SetDraft m_setDraft;
    SoundEventsEditor::ModelDraft m_modelDraft;
    std::vector<std::string> m_idNames;
    // The set shown, and the event to mark once.
    std::string m_shown;
    std::optional<size_t> m_focusEvent;
    // The selection the tool showed last: a new one shows the monster's set.
    int m_shownSerial = -1;
    int m_shownVariant = -2;
    // The model draft, its slot and the frame it was shown in.
    const Data::Monsters::MonsterModelDefinition* m_shownModel = nullptr;
    int m_shownSlot = -1;
    int m_shownFrame = -1;
    // The name being edited, and the set it belongs to.
    std::string m_nameEdit;
    std::string m_nameEditOf;
    bool m_nameActive = false;
    // The file Add a file adds.
    std::string m_newFile;
    std::string m_newId;
    int m_newChannels = 2;
    bool m_newSpatial = true;
    std::map<std::string, std::vector<int>, std::less<>> m_users;
    long long m_usersVersion = -1;
};
} // namespace MuEditor::Monsters

#define g_MuMonsterSoundsUI MuEditor::Monsters::MuMonsterSoundsUI::GetInstance()

#endif // _EDITOR
