#pragma once

#ifdef _EDITOR

#include "EffectBrowserModel.h"
#include "EffectPreviewCamera.h"
#include "EffectPreviewItems.h"
#include "EffectPreviewScene.h"
#include "EffectPreviewTarget.h"
#include "EffectWorldPreviewView.h"

#include <cstdint>
#include <optional>
#include <vector>

// The preview in the effect browser's details: a view of what the selected
// type's slot holds, shown on nothing, a plane, a cube or an item, and below it
// the type created in the world. The mouse turns the camera, the wheel zooms,
// a double click puts it back.
class CEffectPreviewView
{
public:
    void Render(const MuEditor::Effects::EffectBrowserRow& row, Data::Effects::EffectKind kind,
                const MuEditor::Effects::EffectBrowserDetails& details, MuEditor::Effects::EffectWorldPreview& world);

    // Called between frames: the preview's texture can only be released
    // then.
    void BeforeFrame()
    {
        m_target.BeforeFrame();
    }

private:
    // What the camera was framed for; it is framed again when this changes.
    struct FramingKey
    {
        Data::Effects::EffectKind kind = Data::Effects::EffectKind::Effect;
        int type = 0;
        int subType = 0;
        MuEditor::Effects::PreviewShowOn showOn = MuEditor::Effects::PreviewShowOn::Nothing;
        int itemType = -1;
        MuEditor::Effects::PreviewDraw draw = MuEditor::Effects::PreviewDraw::None;

        bool operator==(const FramingKey&) const = default;
    };

    void SelectType(const MuEditor::Effects::EffectBrowserRow& row, Data::Effects::EffectKind kind,
                    const MuEditor::Effects::EffectBrowserDetails& details);
    MuEditor::Effects::EffectPreviewRequest MakeRequest(const MuEditor::Effects::EffectBrowserRow& row,
                                                        Data::Effects::EffectKind kind) const;
    void RenderControls(const MuEditor::Effects::EffectPreviewRequest& request);
    void RenderShowOn();
    void RenderSubType(Data::Effects::EffectKind kind);
    void RenderSubTypePresets(Data::Effects::EffectKind kind);
    void RenderSpriteBlend();
    void RenderItemPicker();
    void RenderItemList();
    void RefreshItems();
    void RenderView(const MuEditor::Effects::EffectPreviewRequest& request);
    void HandleViewInput();
    void Frame(const MuEditor::Effects::EffectPreviewRequest& request);
    void RenderNotes(std::uint16_t notes) const;

    MuEditor::Effects::EffectPreviewCamera m_camera;
    MuEditor::Effects::EffectPreviewScene m_scene;
    MuEditor::Effects::EffectPreviewTarget m_target;
    std::optional<FramingKey> m_framedFor;

    // The selected type, the SubType both previews use (a sprite's is its
    // blend, the formation mark's its frame), and the SubTypes to choose from:
    // one of each column of the creation table, a sprite's blends.
    std::optional<MuEditor::Effects::EffectTypeRef> m_selected;
    int m_subType = 0;
    std::vector<int> m_subTypePresets;

    MuEditor::Effects::PreviewShowOn m_showOn = MuEditor::Effects::PreviewShowOn::Nothing;
    // The blend the texture of a type of another kind than sprites is shown
    // with.
    MuEditor::Effects::PreviewSpriteBlend m_spriteBlend = MuEditor::Effects::PreviewSpriteBlend::Glow;
    bool m_turn = true;

    MuEditor::Effects::EffectPreviewItems m_items;
    int m_itemsVersion = -1;
    // The language the item names are in (a pointer into I18N's locales).
    const char* m_itemsLocale = nullptr;
    char m_itemSearch[64] = {};
    std::vector<int> m_itemMatches;
    bool m_itemMatchesValid = false;
    int m_itemType = -1;
    int m_itemLevel = 0;
    bool m_itemExcellent = false;
    bool m_itemAncient = false;

    CEffectWorldPreviewView m_world;
};

#endif // _EDITOR
