#pragma once

#ifdef _EDITOR

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ColorFolders/ColorFolder.h"
#include "Data/GameData/LookData/LookDefinition.h"
#include "Data/GameData/LookData/LookStore.h"
#include "UI/ItemTools/ItemEdits.h"
#include "UI/StyleEditor/LookEdits.h"
#include "UI/StyleEditor/MuStyleEditorUI.h"

#include <optional>
#include <utility>
#include <vector>

// The looks of the Style Editor and of the game for one test, with nothing to undo, nothing to save and no draft of the
// Style Editor's; the effects are the test's, or the game's. Afterwards, also after a failed check, the game has
// its looks from before again and the editor reads the look file again when next asked for, as before the test.
class LookEditsScope
{
public:
    explicit LookEditsScope(std::vector<Data::Looks::LookDefinition> looks,
                            std::vector<Data::Colors::ColorFolder> folders = {},
                            std::optional<std::vector<Data::Looks::LookDefinition>> effects = std::nullopt,
                            std::vector<Data::Colors::ColorFolder> effectFolders = {})
        : m_previous(Data::Looks::g_LookStore.GetRenderStyles().begin(),
                     Data::Looks::g_LookStore.GetRenderStyles().end()),
          m_previousEffects(Data::Looks::g_LookStore.GetEffects().begin(),
                                Data::Looks::g_LookStore.GetEffects().end())
    {
        g_LookEdits.ResetForTest({std::move(looks), std::move(folders)});
        // Without effects of the test's, the game's.
        g_EffectEdits.ResetForTest(
            {effects ? std::move(*effects) : m_previousEffects, std::move(effectFolders)});
        g_ItemEdits.GetHistory().Clear();
        g_ItemEdits.GetHistory().MarkSaved();
        g_MuStyleEditorUI.ForgetDraftsForTest();
    }

    ~LookEditsScope()
    {
        g_MuStyleEditorUI.ForgetDraftsForTest();
        g_ItemEdits.GetHistory().Clear();
        g_ItemEdits.GetHistory().MarkSaved();
        g_ItemDataHandler.ForgetRenderStylesForTest();
        Data::Looks::g_LookStore.SetRenderStyles(std::move(m_previous));
        Data::Looks::g_LookStore.SetEffects(std::move(m_previousEffects));
    }

    LookEditsScope(const LookEditsScope&) = delete;
    LookEditsScope& operator=(const LookEditsScope&) = delete;

private:
    std::vector<Data::Looks::LookDefinition> m_previous;
    std::vector<Data::Looks::LookDefinition> m_previousEffects;
};

#endif // _EDITOR
