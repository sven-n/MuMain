#pragma once

#ifdef _EDITOR

#include "EffectWorldPreview.h"

// The "In the world" part of the effect browser's preview: the buttons that
// create the selected type in front of the character and stop it, Repeat and
// Mute, what runs and why nothing shows.
class CEffectWorldPreviewView
{
public:
    void Render(MuEditor::Effects::EffectWorldPreview& world, const MuEditor::Effects::WorldPreviewRequest& request);

private:
    void RenderButtons(MuEditor::Effects::EffectWorldPreview& world,
                       const MuEditor::Effects::WorldPreviewRequest& request, bool ready);
    void RenderRunning(const MuEditor::Effects::EffectWorldPreview& world,
                       const MuEditor::Effects::WorldPreviewRequest& request) const;
    void RenderNotes(const MuEditor::Effects::EffectWorldPreview& world, bool ready) const;
};

#endif // _EDITOR
