#include "stdafx.h"

#ifdef _EDITOR

#include "EffectPreviewView.h"

#include "EffectBrowserLayout.h"
#include "EffectPreviewSubject.h"
#include "../MuEditor/Core/MuEditorCore.h"
#include "Core/Globals/_define.h"
#include "Core/Globals/_struct.h"
#include "Core/Text/Utf8.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "I18N/All.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Renderer/MuRenderer.h"
#include "UI/Common/ScopedOffscreenCapture.h"
#include "imgui.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <string>

extern ITEM_ATTRIBUTE* ItemAttribute;

using Data::Effects::EffectKind;
using MuEditor::Effects::EffectPreviewRequest;
using MuEditor::Effects::PreviewShowOn;
using MuEditor::Effects::PreviewSpriteBlend;
namespace Layout = MuEditor::Effects::Layout;

namespace
{
// The view's height is this share of its width, within limits (UI scale 1).
constexpr float ViewAspect = 0.75f;
constexpr float MinViewHeight = 160.0f;
constexpr float MaxViewHeight = 420.0f;
constexpr float ComboWidth = 130.0f;
constexpr float ItemComboWidth = 220.0f;
constexpr float LevelSliderWidth = 90.0f;
// The item drop-down lists this many items at once; the list scrolls.
constexpr int MaxListedItems = 12;
// The levels an item can have.
constexpr int MaxItemLevel = 15;

const char* ShowOnLabel(PreviewShowOn showOn)
{
    switch (showOn)
    {
    case PreviewShowOn::Plane:
        return I18N::Editor::ShowOnPlane;
    case PreviewShowOn::Cube:
        return I18N::Editor::ShowOnCube;
    case PreviewShowOn::Item:
        return I18N::Editor::ShowOnItem;
    case PreviewShowOn::Nothing:
        break;
    }
    return I18N::Editor::ShowOnNothing;
}

const char* SpriteBlendLabel(PreviewSpriteBlend blend)
{
    switch (blend)
    {
    case PreviewSpriteBlend::Subtract:
        return I18N::Editor::BlendSubtract;
    case PreviewSpriteBlend::AlphaTest:
        return I18N::Editor::BlendAlphaTest;
    case PreviewSpriteBlend::Luminance:
        return I18N::Editor::BlendLuminance;
    case PreviewSpriteBlend::Glow:
        break;
    }
    return I18N::Editor::BlendGlow;
}

// A combo that sets `value` to one of `choices`, labelled by `labelOf`.
template <typename Choice, size_t Count, typename LabelOf>
void RenderChoiceCombo(const char* id, const char* label, Choice& value, const std::array<Choice, Count>& choices,
                       LabelOf labelOf)
{
    ImGui::TextUnformatted(label);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ComboWidth * g_MuEditorCore.GetUIScale());
    if (!ImGui::BeginCombo(id, labelOf(value)))
        return;
    for (const Choice choice : choices)
    {
        if (ImGui::Selectable(labelOf(choice), choice == value))
            value = choice;
    }
    ImGui::EndCombo();
}

bool IsItemDrawable(int itemType)
{
    const BMD& model = Models[Data::Items::ToModelSlot(itemType)];
    return model.NumMeshs > 0 && model.Meshs != nullptr;
}

const char* NoteText(MuEditor::Effects::PreviewNote note)
{
    using namespace MuEditor::Effects;
    switch (note)
    {
    case NoteNothingLoaded:
        return I18N::Editor::NothingLoaded;
    case NoteTextureChosenInCode:
        return I18N::Editor::SlotTextureChosenInCode;
    case NoteNotDrawnByGame:
        return I18N::Editor::PreviewNotDrawnByGame;
    case NoteCodeMayChooseTexture:
        return I18N::Editor::SlotDefaultTexture;
    case NoteWorldObjectSlot:
        return I18N::Editor::WorldObjectSlot;
    case NoteStartsInvisible:
        return I18N::Editor::PreviewStartsInvisible;
    case NoteHiddenModel:
        return I18N::Editor::PreviewHiddenModel;
    case NoteNoAnimation:
        return I18N::Editor::PreviewNoAnimation;
    case NoteItemEffectsLeftOut:
        return I18N::Editor::PreviewItemEffectsLeftOut;
    case NoteNotItsModel:
        break;
    }
    return I18N::Editor::PreviewNotItsModel;
}
} // namespace

void CEffectPreviewView::Render(const MuEditor::Effects::EffectBrowserRow& row, EffectKind kind,
                                const MuEditor::Effects::EffectBrowserDetails& details)
{
    const MuEditor::Effects::EffectTypeRef selected{kind, row.type};
    if (m_selected != selected)
        SelectType(row, kind, details);
    const EffectPreviewRequest request = MakeRequest(row, kind);
    RenderControls(request);
    RenderView(request);
    std::uint16_t notes = request.subject.notes | m_scene.GetObjectNotes();
    if (request.showOn == PreviewShowOn::Item && request.itemType >= 0)
        notes |= MuEditor::Effects::NoteItemEffectsLeftOut;
    RenderNotes(notes);
}

void CEffectPreviewView::SelectType(const MuEditor::Effects::EffectBrowserRow& row, EffectKind kind,
                                    const MuEditor::Effects::EffectBrowserDetails& details)
{
    m_selected = MuEditor::Effects::EffectTypeRef{kind, row.type};
    const std::vector<std::vector<int>> none;
    m_subTypes = MuEditor::Effects::PreviewSubTypes(details.creation ? details.creation->variantSubTypes : none);
    m_subTypeIndex = 0;
}

EffectPreviewRequest CEffectPreviewView::MakeRequest(const MuEditor::Effects::EffectBrowserRow& row,
                                                     EffectKind kind) const
{
    EffectPreviewRequest request;
    request.kind = kind;
    request.type = row.type;
    request.subType = m_subTypes.empty() ? 0 : m_subTypes[std::min<size_t>(m_subTypeIndex, m_subTypes.size() - 1)];
    request.subject = MuEditor::Effects::DescribePreviewSubject(kind, row.type, row.assetSlot, row.asset.loaded,
                                                                row.foreignMapObject, row.stages);
    request.showOn = m_showOn;
    request.itemType = m_itemType;
    request.itemLevel = m_itemLevel;
    request.itemExcellent = m_itemExcellent;
    request.itemAncient = m_itemAncient;
    request.spriteBlend = m_spriteBlend;
    request.turn = m_turn;
    return request;
}

void CEffectPreviewView::RenderControls(const EffectPreviewRequest& request)
{
    RenderShowOn();
    if (m_subTypes.size() > 1)
    {
        Layout::SameLineIfFits(Layout::LabeledComboWidth(I18N::Editor::SubType, ComboWidth));
        RenderSubTypes();
    }
    if (request.subject.draw == MuEditor::Effects::PreviewDraw::Sprite)
    {
        Layout::SameLineIfFits(Layout::LabeledComboWidth(I18N::Editor::Blend, ComboWidth));
        RenderSpriteBlend();
    }
    Layout::SameLineIfFits(Layout::CheckboxWidth(I18N::Editor::Turn));
    ImGui::Checkbox(I18N::Editor::Turn, &m_turn);
    Layout::SameLineIfFits(Layout::ButtonWidth(I18N::Editor::ResetView));
    if (ImGui::Button(I18N::Editor::ResetView))
        m_camera.Reset();
    if (m_showOn == PreviewShowOn::Item)
        RenderItemPicker();
}

void CEffectPreviewView::RenderShowOn()
{
    constexpr std::array<PreviewShowOn, 4> choices = {PreviewShowOn::Nothing, PreviewShowOn::Plane, PreviewShowOn::Cube,
                                                      PreviewShowOn::Item};
    RenderChoiceCombo("##showOn", I18N::Editor::ShowOn, m_showOn, choices, ShowOnLabel);
}

// The first SubType stands for those without a variant: the row's own
// values.
void CEffectPreviewView::RenderSubTypes()
{
    char preview[64];
    const auto label = [&](size_t index)
    {
        if (index == 0)
            std::snprintf(preview, sizeof(preview), "%d (%s)", m_subTypes[index], I18N::Editor::OtherSubTypes);
        else
            std::snprintf(preview, sizeof(preview), "%d", m_subTypes[index]);
        return preview;
    };
    ImGui::TextUnformatted(I18N::Editor::SubType);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ComboWidth * g_MuEditorCore.GetUIScale());
    if (!ImGui::BeginCombo("##subType", label(static_cast<size_t>(m_subTypeIndex))))
        return;
    for (size_t i = 0; i < m_subTypes.size(); ++i)
    {
        ImGui::PushID(static_cast<int>(i));
        if (ImGui::Selectable(label(i), static_cast<int>(i) == m_subTypeIndex))
            m_subTypeIndex = static_cast<int>(i);
        ImGui::PopID();
    }
    ImGui::EndCombo();
}

void CEffectPreviewView::RenderSpriteBlend()
{
    constexpr std::array<PreviewSpriteBlend, 4> choices = {PreviewSpriteBlend::Glow, PreviewSpriteBlend::Subtract,
                                                           PreviewSpriteBlend::AlphaTest,
                                                           PreviewSpriteBlend::Luminance};
    RenderChoiceCombo("##blend", I18N::Editor::Blend, m_spriteBlend, choices, SpriteBlendLabel);
}

// One line: a drop-down with a search field and the items it finds, the
// level and the excellent and ancient looks.
void CEffectPreviewView::RenderItemPicker()
{
    RefreshItems();
    const float scale = g_MuEditorCore.GetUIScale();
    const MuEditor::Effects::PreviewItem* item = m_items.Find(m_itemType);
    ImGui::SetNextItemWidth(ItemComboWidth * scale);
    if (ImGui::BeginCombo("##item", item != nullptr ? item->name.c_str() : I18N::Editor::FindAnItem,
                          ImGuiComboFlags_HeightLargest))
    {
        if (ImGui::IsWindowAppearing())
            ImGui::SetKeyboardFocusHere();
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::InputTextWithHint("##itemSearch", I18N::Editor::FindAnItem, m_itemSearch, sizeof(m_itemSearch)) ||
            !m_itemMatchesValid)
        {
            m_itemMatches = m_items.Filter(m_itemSearch);
            m_itemMatchesValid = true;
        }
        RenderItemList();
        ImGui::EndCombo();
    }
    Layout::SameLineIfFits(LevelSliderWidth * scale + ImGui::CalcTextSize(I18N::Editor::Level).x);
    ImGui::SetNextItemWidth(LevelSliderWidth * scale);
    ImGui::SliderInt(I18N::Editor::Level, &m_itemLevel, 0, MaxItemLevel);
    Layout::SameLineIfFits(Layout::CheckboxWidth(I18N::Editor::Excellent));
    ImGui::Checkbox(I18N::Editor::Excellent, &m_itemExcellent);
    Layout::SameLineIfFits(Layout::CheckboxWidth(I18N::Editor::Ancient));
    ImGui::Checkbox(I18N::Editor::Ancient, &m_itemAncient);
}

// The items the search finds, in a box of fixed height under the search
// field; choosing one closes the drop-down.
void CEffectPreviewView::RenderItemList()
{
    const ImVec2 size(0.0f, ImGui::GetTextLineHeightWithSpacing() * MaxListedItems);
    const bool open = ImGui::BeginChild("##items", size);
    if (!open)
    {
        ImGui::EndChild();
        return;
    }
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(m_itemMatches.size()));
    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            const MuEditor::Effects::PreviewItem& item = m_items.GetItems()[m_itemMatches[i]];
            ImGui::PushID(item.type);
            if (ImGui::Selectable(item.name.c_str(), item.type == m_itemType))
            {
                m_itemType = item.type;
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopID();
        }
    }
    ImGui::EndChild();
}

// The names come from the items as the game has them now; the list is made
// again when the item editor changes them or the language changes.
void CEffectPreviewView::RefreshItems()
{
    const int version = g_ItemDatabase.GetVersion();
    const char* locale = I18N::GetCurrentLocale();
    if (m_itemsVersion == version && m_itemsLocale == locale)
        return;
    m_itemsVersion = version;
    m_itemsLocale = locale;
    m_items.Build(
        MAX_ITEM, [](int itemType) { return Core::Text::ToUtf8(ItemAttribute[itemType].Name); }, IsItemDrawable);
    m_itemMatchesValid = false;
}

void CEffectPreviewView::RenderView(const EffectPreviewRequest& request)
{
    const float width = ImGui::GetContentRegionAvail().x;
    if (width < 1.0f)
        return;
    const float scale = g_MuEditorCore.GetUIScale();
    // The width without the vertical scrollbar, so the height does not make
    // the scrollbar show and hide by turns.
    const ImGuiStyle& style = ImGui::GetStyle();
    const float widthWithoutScrollbar = ImGui::GetWindowWidth() - 2.0f * style.WindowPadding.x - style.ScrollbarSize;
    const float height = std::clamp(widthWithoutScrollbar * ViewAspect, MinViewHeight * scale, MaxViewHeight * scale);
    ImGui::InvisibleButton("##view", ImVec2(width, height), ImGuiButtonFlags_MouseButtonLeft);
    ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY);
    HandleViewInput();
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("%s", I18N::Editor::PreviewControls);
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    Frame(request);
    if (!ImGui::IsItemVisible() || !mu::GetRenderer().IsFrameActive())
        return;
    const float framebufferScale = ImGui::GetIO().DisplayFramebufferScale.x;
    const MuEditor::Effects::PreviewTextureSize size =
        MuEditor::Effects::ChoosePreviewTextureSize(width, height, framebufferScale);
    const std::uint32_t texture = m_target.Begin(size.width, size.height, !ImGui::IsMouseDown(ImGuiMouseButton_Left));
    if (texture == 0)
        return;
    {
        const MuEditor::ScopedOffscreenCapture endCapture;
        m_scene.Draw(request, m_camera, width / height);
    }
    if (void* pointer = mu::GetRenderer().GetTexturePointer(texture))
        ImGui::GetWindowDrawList()->AddImage((ImTextureID)(intptr_t)pointer, min, max);
}

void CEffectPreviewView::HandleViewInput()
{
    const ImGuiIO& io = ImGui::GetIO();
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f))
        m_camera.Turn(io.MouseDelta.x, io.MouseDelta.y);
    if (!ImGui::IsItemHovered())
        return;
    if (io.MouseWheel != 0.0f)
        m_camera.Zoom(io.MouseWheel);
    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        m_camera.Reset();
}

// Prepares the scene every frame; frames the camera again only when what it
// shows changed, so turning and zooming stay.
void CEffectPreviewView::Frame(const EffectPreviewRequest& request)
{
    const MuEditor::Effects::PreviewFraming framing = m_scene.Prepare(request);
    const FramingKey key{request.kind,   request.type,     request.subType,
                         request.showOn, request.itemType, request.subject.draw};
    if (m_framedFor == key)
        return;
    m_framedFor = key;
    m_camera.Frame(framing.center, framing.radius);
}

void CEffectPreviewView::RenderNotes(std::uint16_t notes) const
{
    for (std::uint16_t bit = 1; bit != 0 && bit <= notes; bit = static_cast<std::uint16_t>(bit << 1))
    {
        if ((notes & bit) == 0)
            continue;
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::TextWrapped("%s", NoteText(static_cast<MuEditor::Effects::PreviewNote>(bit)));
        ImGui::PopStyleColor();
    }
}

#endif // _EDITOR
