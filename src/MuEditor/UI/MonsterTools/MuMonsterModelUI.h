#pragma once

#ifdef _EDITOR

#include "MonsterModelFiles.h"

#include "../MuEditor/UI/ItemTools/ItemEditDraft.h"
#include "Data/GameData/MonsterData/MonsterModelDefinition.h"

#include <string>
#include <vector>

namespace MuEditor::Monsters
{
// The Monster Model tool (monsters design, section 9): the model entry of the
// selected monster's model in the model list (Data/Monsters/Models.json): its
// file, texture folders, animation speeds and held last frames, its sounds,
// sound slots and sound events (MonsterSoundEventsEditor), and what the
// game's code does on top of them (the slot's range, the speed factors, the
// next animation, the impact frame). Each
// change is an undo step of the monster tools; the game opens the model
// again between frames, so the change shows at once (D55).
class MuMonsterModelUI
{
public:
    static MuMonsterModelUI& GetInstance();

    void Render(bool* open);

    // The model values the tool shows for the slot in this frame, with an
    // edit that is not stored yet (a speed being dragged), for the Monster
    // Preview; nullptr when the tool did not show the slot this frame.
    const Data::Monsters::MonsterModelDefinition* GetShownDraft(int slot) const;

private:
    using ModelDraft = Items::ItemEditDraft<Data::Monsters::MonsterModelDefinition>;

    MuMonsterModelUI();

    void RenderModel(Data::Monsters::MonsterModelDefinition& model, int slot);
    void RenderSlot(int slot);
    void RenderUsers(int slot);
    void RenderFile(Data::Monsters::MonsterModelDefinition& model);
    void RenderFilePicker(Data::Monsters::MonsterModelDefinition& model);
    void RenderTextureFolders(Data::Monsters::MonsterModelDefinition& model);
    void RenderAnimations(Data::Monsters::MonsterModelDefinition& model, int slot);
    void RenderSounds(const Data::Monsters::MonsterModelDefinition& model, int slot);
    void RenderSoundSlots(Data::Monsters::MonsterModelDefinition& model);
    void RenderMore(const Data::Monsters::MonsterModelDefinition& model);

    ModelDraft m_draft;
    MonsterModelFiles m_files;
    std::string m_fileSearch;
    std::string m_newFolder;
    std::string m_soundEventMaps;
    // The draft (m_draft's value, which lives until its next Begin), the slot
    // and the frame it was shown in.
    const Data::Monsters::MonsterModelDefinition* m_shownDraft = nullptr;
    int m_shownSlot = -1;
    int m_shownFrame = -1;
    // What the frames would find again until the data changes: the monsters
    // that use the slot, and the files of the model list.
    std::vector<int> m_users;
    int m_usersSlot = -1;
    long long m_usersVersion = -1;
    std::vector<std::string> m_listedFiles;
    long long m_listedVersion = -1;
};
} // namespace MuEditor::Monsters

#define g_MuMonsterModelUI MuEditor::Monsters::MuMonsterModelUI::GetInstance()

#endif // _EDITOR
