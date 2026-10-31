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

#include <array>
#include <charconv>
#include <optional>

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
constexpr std::array<std::string_view, 2> PreviewThemes = {"legacy", "modern"};
constexpr int PreviewColumns = 2 + static_cast<int>(PreviewThemes.size());
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

ImVec4 ToSwatch(Rgb color)
{
    return ImGui::ColorConvertU32ToFloat4(ToImColor(color));
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
    return resolved ? ToSwatch(*resolved) : ImGui::ColorConvertU32ToFloat4(UnlitFallback);
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

// "#RRGGBB" or "rgba(r, g, b, a)", as tokens.ini writes colors.
std::optional<ImU32> ParseTokenColor(const std::string& value)
{
    if (const std::optional<Rgb> rgb = Core::Color::ParseRgb(value))
    {
        return ToImColor(*rgb);
    }
    constexpr std::string_view RgbaStart = "rgba(";
    if (!value.starts_with(RgbaStart) || value.back() != ')')
    {
        return std::nullopt;
    }
    std::array<int, 4> channels{};
    const char* position = value.data() + RgbaStart.size();
    const char* end = value.data() + value.size() - 1;
    for (int& channel : channels)
    {
        while (position < end && (*position == ' ' || *position == ','))
        {
            ++position;
        }
        const auto [parsed, error] = std::from_chars(position, end, channel);
        if (error != std::errc{} || channel < 0 || channel > static_cast<int>(Opaque))
        {
            return std::nullopt;
        }
        position = parsed;
    }
    return IM_COL32(channels[0], channels[1], channels[2], channels[3]);
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
    return (token ? ParseTokenColor(*token) : std::nullopt).value_or(UnlitFallback);
}

// The load bar as the theme draws it, in the legacy theme's size.
void DrawLoadBar(const ServerListData& list, std::string_view theme, int percent)
{
    const float scale = g_MuEditorCore.GetUIScale();
    const ImVec2 size(BarWidth * scale, BarHeight * scale);
    const float gap = BarGap * scale;
    const float textHeight = ImGui::GetTextLineHeight();
    const ImVec2 start = ImGui::GetCursorScreenPos();
    const float top = start.y + (textHeight - size.y) / 2.0f;
    ImGui::Dummy(ImVec2(size.x, textHeight));

    const Data::ServerList::ThemeTokenReader readToken = TokenReader(theme);
    const auto base = Data::ServerList::ResolveLoadBarColor(list, LoadBarPart::Base, theme, readToken);
    const auto full = Data::ServerList::ResolveLoadBarColor(list, LoadBarPart::Full, theme, readToken);
    const Data::ServerList::LoadBarSegments segments =
        Data::ServerList::GetLoadBarSegments(base.value_or(Rgb{}), full.value_or(Rgb{}), percent);
    const ImU32 unlit = GetUnlitColor(list, theme);
    const float segmentWidth =
        (size.x - gap * (Data::ServerList::LoadBarSegmentCount - 1)) / Data::ServerList::LoadBarSegmentCount;
    ImDrawList* draw = ImGui::GetWindowDrawList();
    for (size_t index = 0; index < segments.size(); ++index)
    {
        const float left = start.x + static_cast<float>(index) * (segmentWidth + gap);
        draw->AddRectFilled(ImVec2(left, top), ImVec2(left + segmentWidth, top + size.y), unlit);
        const float lit = segmentWidth * segments[index].lit;
        if (lit > 0.0f)
        {
            draw->AddRectFilled(ImVec2(left, top), ImVec2(left + lit, top + size.y), ToImColor(segments[index].color));
        }
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

// The button's text in the game's theme, its load text in that text's color.
void RenderButtonText(const ServerListData& list, const Data::ServerList::ChannelSettings& channel,
                      const std::string& groupName, std::string_view locale, int percent)
{
    const LoadText* loadText = list.load.Text(Data::ServerList::GetLoadState(percent));
    Data::ServerList::ChannelNameValues values;
    values.name = groupName;
    values.channel = 1;
    if (loadText != nullptr)
    {
        values.load = loadText->text.Get(locale);
    }
    const Data::ServerList::ButtonTextParts parts =
        Data::ServerList::BuildButtonTextParts(list, channel, values, locale);
    const std::string_view theme = Colors::GetGameTheme();
    const ImVec4 buttonColor =
        Colors::GetButtonColorSwatch(Data::ServerList::GetButtonColor(list, channel, theme), theme);
    ImGui::TextColored(buttonColor, "%s", parts.before.c_str());
    if (parts.load.empty())
    {
        return;
    }
    const Data::ServerList::ButtonColor loadColor = Data::ServerList::ResolveButtonColor(list, loadText->color, theme);
    const bool ownColor = !loadColor.shade.empty() || loadColor.theme != Data::ServerList::ThemeColor::Default;
    ImGui::SameLine(0.0f, 0.0f);
    ImGui::TextColored(ownColor ? Colors::GetButtonColorSwatch(loadColor, theme) : buttonColor, "%s",
                       parts.load.c_str());
    ImGui::SameLine(0.0f, 0.0f);
    ImGui::TextColored(buttonColor, "%s", parts.after.c_str());
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
                       const std::string& groupName, std::string_view locale)
{
    RenderNote(I18N::Editor::LoadPreviewNote);
    constexpr ImGuiTableFlags TableFlags =
        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingFixedFit;
    if (!ImGui::BeginTable("##LoadPreview", PreviewColumns, TableFlags))
    {
        return;
    }
    ImGui::TableSetupColumn(I18N::Editor::Load);
    ImGui::TableSetupColumn(I18N::Editor::ServerListButton);
    for (const std::string_view theme : PreviewThemes)
    {
        ImGui::TableSetupColumn(UI::RmlBridge::GetThemeDisplayName(std::string(theme)).c_str());
    }
    ImGui::TableHeadersRow();
    for (const int percent : PreviewLoads)
    {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(GetLoadLabel(percent).c_str());
        ImGui::TableNextColumn();
        RenderButtonText(list, channel, groupName, locale, percent);
        for (const std::string_view theme : PreviewThemes)
        {
            ImGui::TableNextColumn();
            DrawLoadBar(list, theme, percent);
        }
    }
    ImGui::EndTable();
}
} // namespace MuEditor::ServerList::LoadUI

#endif // _EDITOR
