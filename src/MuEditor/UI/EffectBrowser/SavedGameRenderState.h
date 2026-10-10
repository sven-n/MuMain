#pragma once

#ifdef _EDITOR

namespace MuEditor::Effects
{
// While it lives the effect preview draws with the game's render state
// functions (EnableAlphaBlend, DisableDepthTest, ...). They keep the state in
// flags of their own and skip calls that would not change them, so the flags
// have to be what the renderer has. This saves the flags and BoneScale, sets
// a known state in the renderer and the flags alike, and puts the saved
// state back into both. Fog is off while it lives.
class SavedGameRenderState
{
public:
    SavedGameRenderState();
    ~SavedGameRenderState();

    SavedGameRenderState(const SavedGameRenderState&) = delete;
    SavedGameRenderState& operator=(const SavedGameRenderState&) = delete;

private:
    bool m_texture;
    bool m_depthTest;
    bool m_cullFace;
    bool m_depthMask;
    bool m_alphaTest;
    int m_blendType;
    bool m_fog;
    float m_boneScale;
};
} // namespace MuEditor::Effects

#endif // _EDITOR
