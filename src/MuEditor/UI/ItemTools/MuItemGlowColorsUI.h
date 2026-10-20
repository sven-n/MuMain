#pragma once

#ifdef _EDITOR

#include "GlowColorDialog.h"
#include "ItemLookUseCounts.h"

#include "Data/GameData/EffectData/GlowColors.h"

#include <array>
#include <string>
#include <vector>

// The glow color tool (D26 for the glow colors of the item models): the
// colors with how many items use each; a new color, a changed value (the
// items it reaches are listed before it is applied) and the colors no item
// uses removed. The model tool picks an item's colors from them.
class CMuItemGlowColorsUI
{
public:
    static CMuItemGlowColorsUI& GetInstance();

    void Render(bool* open);

private:
    CMuItemGlowColorsUI() = default;

    void RenderList();
    void RenderPicked();
    void RenderConfirm();
    void RenderUnused();
    void ApplyEdit();
    // The edited value follows the color (an undo, a redo) until it is changed.
    void UpdateEdit(const Data::Effects::GlowColorValue& value);

    // The color picked in the list; "" before one is.
    std::string m_picked;
    std::array<float, 3> m_edit{};
    Data::Effects::GlowColorValue m_loaded{};
    std::string m_editOf;
    int m_editVersion = -1;
    // The items the confirmed change reaches.
    std::vector<int> m_confirmUsers;
    MuEditor::Items::ItemLookUseCounts m_counts;
    MuEditor::Items::CGlowColorDialog m_dialog;
};

#define g_MuItemGlowColorsUI CMuItemGlowColorsUI::GetInstance()

#endif // _EDITOR
