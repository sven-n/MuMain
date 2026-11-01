#include "stdafx.h"

#ifdef _EDITOR

#include "ServerListLoadUI.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "Data/GameData/ServerListData/ButtonColors.h"
#include "Data/GameData/ServerListData/ChannelNameBuilder.h"
#include "Data/GameData/ServerListData/LoadBar.h"
#include "I18N/All.h"
#include "ServerListColors.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "imgui.h"

#include <algorithm>
#include <array>
#include <optional>
#include <vector>

namespace MuEditor::ServerList::LoadUI
{
namespace
{
using Core::Color::Rgb;
using Data::ServerList::LoadBarPart;
using Data::ServerList::LoadText;
using Data::ServerList::ServerListData;

// Pixel sizes before the editor UI scale: the color lists, and the preview's bar, as large as
// the legacy theme's.
constexpr float ColorComboWidth = 140.0f;
constexpr float BarWidth = 160.0f;
constexpr float BarHeight = 4.0f;
constexpr float BarGap = 2.0f;
// The load of the preview's first row.
constexpr int PreviewLoadPercent = 35;
constexpr std::array<int, 3> PreviewLoads = {PreviewLoadPercent, Data::ServerList::FullLoadPercent,
                                             Data::ServerList::OverloadedLoadPercent};
// The columns before the themes' bars: the load and the button.
constexpr int PreviewTextColumns = 2;
// ImGui tables have at most this many columns; further themes are left out.
constexpr int MaxPreviewColumns = 64;
// An unlit segment whose theme token is no color the editor can read.
constexpr ImU32 UnlitFallback = IM_COL32(0, 0, 0, 160);
constexpr ImU32 Opaque = 255;

void RenderNote(const char* text)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", text);
    ImGui::PopTextWrapPos();
}

ImU32 ToImColor(Rgb color)
{
    return IM_COL32(color.red, color.green, color.blue, Opaque);
}

// A token's color with its alpha.
ImU32 ToImColor(const Colors::TokenColor& color)
{
    const Rgb rgb = Colors::ToRgb(color.rgb);
    return IM_COL32(rgb.red, rgb.green, rgb.blue, color.alpha);
}

Data::ServerList::ThemeTokenReader TokenReader(std::string_view theme)
{
    return [theme](const char* token) { return Colors::ReadThemeToken(theme, token); };
}

// The swatch of `color` as a part of the load bar in the game's theme.
ImVec4 GetBarSwatch(const ServerListData& list, LoadBarPart part, std::string_view color)
{
    const std::string_view theme = Colors::GetGameTheme();
    const std::optional<Rgb> resolved =
        Data::ServerList::ResolveLoadBarColor(list, color, part, theme, TokenReader(theme));
    return resolved ? Colors::ToSwatch(*resolved) : ImGui::ColorConvertU32ToFloat4(UnlitFallback);
}

bool PickBarColor(ServerListData& list, LoadBarPart part, const char* label, const std::function<void()>& beforeChange)
{
    std::string color = Data::ServerList::GetLoadBarColor(list.load.bar, part);
    const bool picked = Colors::PickColor(label, list, color, ColorComboWidth, [&list, part](std::string_view candidate)
                                          { return GetBarSwatch(list, part, candidate); });
    ImGui::SameLine();
    ImGui::TextUnformatted(label);
    if (!picked)
    {
        return false;
    }
    beforeChange();
    Data::ServerList::GetLoadBarColor(list.load.bar, part) = std::move(color);
    return true;
}

// A load text's color, bold and italic.
bool RenderLoadTextLook(ServerListData& list, const char* label, LoadText& text,
                        const std::function<void()>& beforeChange)
{
    ImGui::PushID(label);
    ImGui::TextUnformatted(label);
    const std::string_view theme = Colors::GetGameTheme();
    std::string color = text.color;
    bool changed = false;
    if (Colors::PickColor("##TextColor", list, color, ColorComboWidth,
                          [&list, theme](std::string_view candidate)
                          {
                              return Colors::GetButtonColorSwatch(
                                  Data::ServerList::ResolveButtonColor(list, candidate, theme), theme);
                          }))
    {
        beforeChange();
        text.color = std::move(color);
        changed = true;
    }
    ImGui::SameLine();
    bool bold = text.bold;
    if (ImGui::Checkbox(I18N::Editor::Bold, &bold))
    {
        beforeChange();
        text.bold = bold;
        changed = true;
    }
    ImGui::SameLine();
    bool italic = text.italic;
    if (ImGui::Checkbox(I18N::Editor::Italic, &italic))
    {
        beforeChange();
        text.italic = italic;
        changed = true;
    }
    ImGui::PopID();
    return changed;
}

ImU32 GetUnlitColor(const ServerListData& list, std::string_view theme)
{
    if (!Data::ServerList::IsThemeLoadBarColor(list, LoadBarPart::Empty, theme))
    {
        const std::optional<Rgb> own =
            Data::ServerList::ResolveLoadBarColor(list, LoadBarPart::Empty, theme, TokenReader(theme));
        return own ? ToImColor(*own) : UnlitFallback;
    }
    const std::optional<std::string> token =
        Colors::ReadThemeToken(theme, Data::ServerList::GetLoadBarToken(LoadBarPart::Empty));
    const std::optional<Colors::TokenColor> color = token ? Colors::ParseTokenColor(*token) : std::nullopt;
    return color ? ToImColor(*color) : UnlitFallback;
}

// A full or overloaded server's load text on its bar, in its color.
struct BarText
{
    std::string text;
    ImVec4 color;
};

// The bar's segments from `left` on, `width` wide in all.
std::vector<Data::ServerList::SegmentSpan> LayOutSegments(float left, float width, float gap)
{
    constexpr int Count = Data::ServerList::LoadBarSegmentCount;
    const float segmentWidth = (width - gap * (Count - 1)) / Count;
    std::vector<Data::ServerList::SegmentSpan> spans;
    for (int index = 0; index < Count; ++index)
    {
        spans.push_back({left + static_cast<float>(index) * (segmentWidth + gap), segmentWidth});
    }
    return spans;
}

// The load bar as the theme draws it, in the legacy theme's size. A load text sits centered on it and hides the
// segments under it, as on the login screen.
void DrawLoadBar(const ServerListData& list, std::string_view theme, int percent, const std::optional<BarText>& text)
{
    const float scale = g_MuEditorCore.GetUIScale();
    const ImVec2 size(BarWidth * scale, BarHeight * scale);
    const ImVec2 start = ImGui::GetCursorScreenPos();
    const float top = start.y + (ImGui::GetTextLineHeight() - size.y) / 2.0f;
    ImGui::Dummy(ImVec2(size.x, ImGui::GetTextLineHeight()));

    const std::vector<Data::ServerList::SegmentSpan> spans = LayOutSegments(start.x, size.x, BarGap * scale);
    const float textWidth = text ? ImGui::CalcTextSize(text->text.c_str()).x : 0.0f;
    const float textLeft = start.x + (size.x - textWidth) / 2.0f;
    const Data::ServerList::CoveredSegments covered =
        text ? Data::ServerList::GetCoveredSegments(spans, textLeft, textLeft + textWidth)
             : Data::ServerList::CoveredSegments{};

    const Data::ServerList::ThemeTokenReader readToken = TokenReader(theme);
    const auto base = Data::ServerList::ResolveLoadBarColor(list, LoadBarPart::Base, theme, readToken);
    const auto full = Data::ServerList::ResolveLoadBarColor(list, LoadBarPart::Full, theme, readToken);
    const Data::ServerList::LoadBarSegments segments =
        Data::ServerList::GetLoadBarSegments(base.value_or(Rgb{}), full.value_or(Rgb{}), percent);
    const ImU32 unlit = GetUnlitColor(list, theme);
    ImDrawList* draw = ImGui::GetWindowDrawList();
    for (int index = 0; index < static_cast<int>(spans.size()); ++index)
    {
        if (index >= covered.first && index < covered.end)
        {
            continue;
        }
        const Data::ServerList::SegmentSpan& span = spans[static_cast<size_t>(index)];
        const Data::ServerList::LoadBarSegment& segment = segments[static_cast<size_t>(index)];
        draw->AddRectFilled(ImVec2(span.left, top), ImVec2(span.left + span.width, top + size.y), unlit);
        if (segment.lit > 0.0f)
        {
            draw->AddRectFilled(ImVec2(span.left, top), ImVec2(span.left + span.width * segment.lit, top + size.y),
                                ToImColor(segment.color));
        }
    }
    if (text)
    {
        draw->AddText(ImVec2(textLeft, start.y), ImGui::ColorConvertFloat4ToU32(text->color), text->text.c_str());
    }
}

std::string GetLoadLabel(int percent)
{
    switch (Data::ServerList::GetLoadState(percent))
    {
    case Data::ServerList::LoadState::Full:
        return I18N::Editor::FullServer;
    case Data::ServerList::LoadState::Overloaded:
        return I18N::Editor::OverloadedServer;
    case Data::ServerList::LoadState::Open:
    default:
        return I18N::Format(I18N::Editor::Load0, {std::to_string(percent)});
    }
}

// The button's name as the login screen shows it, without its load text, in the game's theme.
void RenderButtonName(const ServerListData& list, const Data::ServerList::ChannelSettings& channel,
                      const std::string& groupName, std::string_view locale)
{
    Data::ServerList::ChannelNameValues values;
    values.name = groupName;
    values.channel = 1;
    const std::string name =
        Data::ServerList::BuildChannelName(list, Data::ServerList::ChannelNamePlace::Button, channel, values, locale);
    const std::string_view theme = Colors::GetGameTheme();
    ImGui::TextColored(Colors::GetButtonColorSwatch(Data::ServerList::GetButtonColor(list, channel, theme), theme),
                       "%s", name.c_str());
}

// The load text the bar shows in the theme, in its color or else the button's; none while the server can be joined
// or the template has no {load}.
std::optional<BarText> GetBarText(const ServerListData& list, const Data::ServerList::ChannelSettings& channel,
                                  std::string_view locale, int percent, std::string_view theme)
{
    const LoadText* loadText = list.load.Text(Data::ServerList::GetLoadState(percent));
    if (loadText == nullptr)
    {
        return std::nullopt;
    }
    Data::ServerList::ChannelNameValues values;
    values.load = loadText->text.Get(locale);
    std::string text = Data::ServerList::BuildButtonTextParts(list, channel, values, locale).load;
    if (text.empty())
    {
        return std::nullopt;
    }
    const Data::ServerList::ButtonColor loadColor = Data::ServerList::ResolveButtonColor(list, loadText->color, theme);
    const bool ownColor = !loadColor.shade.empty() || loadColor.theme != Data::ServerList::ThemeColor::Default;
    const Data::ServerList::ButtonColor color =
        ownColor ? loadColor : Data::ServerList::GetButtonColor(list, channel, theme);
    return BarText{std::move(text), Colors::GetButtonColorSwatch(color, theme)};
}
} // namespace

bool RenderLoadSettings(ServerListData& list, const std::function<void()>& beforeChange)
{
    ImGui::TextUnformatted(I18N::Editor::Load);
    RenderNote(I18N::Editor::LoadNote);
    ImGui::PushID("##Load");
    ImGui::TextUnformatted(I18N::Editor::LoadBar);
    bool changed = PickBarColor(list, LoadBarPart::Base, I18N::Editor::BaseColor, beforeChange);
    changed |= PickBarColor(list, LoadBarPart::Full, I18N::Editor::FullColor, beforeChange);
    changed |= PickBarColor(list, LoadBarPart::Empty, I18N::Editor::UnlitColor, beforeChange);
    changed |= RenderLoadTextLook(list, I18N::Editor::FullServer, list.load.full, beforeChange);
    changed |= RenderLoadTextLook(list, I18N::Editor::OverloadedServer, list.load.overloaded, beforeChange);
    ImGui::PopID();
    return changed;
}

void RenderLoadPreview(const ServerListData& list, const Data::ServerList::ChannelSettings& channel,
                       const std::string& groupName, std::string_view locale, std::span<const std::string> themes)
{
    RenderNote(I18N::Editor::LoadPreviewNote);
    constexpr ImGuiTableFlags TableFlags =
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;
    const std::span<const std::string> shown =
        themes.first(std::min(themes.size(), static_cast<size_t>(MaxPreviewColumns - PreviewTextColumns)));
    if (!ImGui::BeginTable("##LoadPreview", PreviewTextColumns + static_cast<int>(shown.size()), TableFlags))
    {
        return;
    }
    ImGui::TableSetupColumn(I18N::Editor::Load);
    ImGui::TableSetupColumn(I18N::Editor::ServerListButton);
    for (const std::string& theme : shown)
    {
        ImGui::TableSetupColumn(UI::RmlBridge::GetThemeDisplayName(theme).c_str());
    }
    ImGui::TableHeadersRow();
    for (const int percent : PreviewLoads)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(GetLoadLabel(percent).c_str());
        ImGui::TableNextColumn();
        RenderButtonName(list, channel, groupName, locale);
        for (const std::string& theme : shown)
        {
            ImGui::TableNextColumn();
            DrawLoadBar(list, theme, percent, GetBarText(list, channel, locale, percent, theme));
        }
    }
    ImGui::EndTable();
}
} // namespace MuEditor::ServerList::LoadUI

#endif // _EDITOR
