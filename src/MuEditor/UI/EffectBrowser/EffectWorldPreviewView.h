#pragma once

#ifdef _EDITOR

#include "EffectCallSites.h"
#include "EffectWorldPreview.h"

#include <optional>
#include <vector>

// The "In the world" part of the effect browser's preview: the buttons that
// create the selected type in front of the character and stop it, Repeat and
// Mute, the values of the call, the game's own calls of the type to take
// values from, what runs and why nothing shows.
class CEffectWorldPreviewView
{
public:
    // Returns the SubType of a game call chosen with Use.
    std::optional<int> Render(MuEditor::Effects::EffectWorldPreview& world, Data::Effects::EffectKind kind, int type,
                              int subType);

private:
    void Select(Data::Effects::EffectKind kind, int type);
    void RenderButtons(MuEditor::Effects::EffectWorldPreview& world,
                       const MuEditor::Effects::WorldPreviewRequest& request, bool ready);
    void RenderCallValues(Data::Effects::EffectKind kind);
    void RenderTarget();
    std::optional<int> RenderGameCalls();
    std::optional<int> Use(const MuEditor::Effects::EffectCallSite& call);
    void LoadGameCalls();
    void RenderRunning(const MuEditor::Effects::EffectWorldPreview& world,
                       const MuEditor::Effects::WorldPreviewRequest& request) const;
    void RenderNotes(const MuEditor::Effects::EffectWorldPreview& world, bool ready) const;

    MuEditor::Effects::WorldPreviewCall m_call;
    std::optional<MuEditor::Effects::EffectTypeRef> m_selected;
    // The game's calls of every type, read from the sources once, and those
    // of the selected type.
    MuEditor::Effects::EffectCallSiteIndex m_gameCalls;
    bool m_gameCallsFound = false;
    std::vector<const MuEditor::Effects::EffectCallSite*> m_typeCalls;
};

#endif // _EDITOR
