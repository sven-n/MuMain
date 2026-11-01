#include "stdafx.h"

#ifdef _EDITOR

#include "ServerListColors.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "Data/DataHandler/TextFile.h"
#include "I18N/All.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/ThemeInheritance.h"

#include <array>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

namespace MuEditor::ServerList::Colors
{
namespace
{
using Data::ServerList::ThemeColor;

// Where the theme folders are, relative to the client folder.
constexpr const char* ThemesFolder = "Data/Interface/RmlUi/themes";
// The swatch of a token a theme does not have.
constexpr ImVec4 MissingShadeSwatch{1.0f, 1.0f, 1.0f, 1.0f};
constexpr float ChannelMax = 255.0f;

using ThemeCache = std::map<std::string, std::unique_ptr<UI::RmlBridge::ThemeInheritance>, std::less<>>;

ThemeCache& GetThemeCache()
{
    static ThemeCache themes;
    return themes;
}

// Each theme folder's inheritance chain, read once.
const UI::RmlBridge::ThemeInheritance& GetTheme(std::string_view theme)
{
    ThemeCache& themes = GetThemeCache();
    auto found = themes.find(theme);
    if (found == themes.end())
    {
        auto inheritance = std::make_unique<UI::RmlBridge::ThemeInheritance>(
            std::filesystem::u8path(ThemesFolder) / std::filesystem::u8path(std::string(theme)));
        found = themes.emplace(std::string(theme), std::move(inheritance)).first;
    }
    return *found->second;
}

// The text's lines without their line ends.
std::vector<std::string> SplitLines(std::string_view text)
{
    std::vector<std::string> lines;
    std::size_t start = 0;
    while (start < text.size())
    {
        std::size_t end = text.find('\n', start);
        if (end == std::string_view::npos)
        {
            end = text.size();
        }
        std::string line(text.substr(start, end - start));
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        lines.push_back(std::move(line));
        start = end + 1;
    }
    return lines;
}

std::string JoinLines(const std::vector<std::string>& lines, std::string_view newline, bool endsWithNewline)
{
    std::string text;
    for (std::size_t index = 0; index < lines.size(); ++index)
    {
        text += lines[index];
        if (index + 1 < lines.size() || endsWithNewline)
        {
            text += newline;
        }
    }
    return text;
}

// Sets `token` in [Tokens], with the lines read as the themes read them (UI::RmlBridge::ThemeInheritance): each line
// that has it, as the last one counts, else a new one after the section's last value line, else the section with it
// at the end.
void PlaceToken(std::vector<std::string>& lines, std::string_view token, const std::string& value)
{
    using UI::RmlBridge::ThemeInheritance;
    const std::string line = std::string(token) + "=" + value;
    const std::optional<std::string> key = ThemeInheritance::ParseIniLine(line).key;
    bool inTokens = false;
    bool written = false;
    std::optional<std::size_t> lastInTokens;
    for (std::size_t index = 0; index < lines.size(); ++index)
    {
        const ThemeInheritance::IniLine parsed = ThemeInheritance::ParseIniLine(lines[index]);
        if (parsed.section)
        {
            inTokens = *parsed.section == "tokens";
        }
        else if (!inTokens || !parsed.key)
        {
            continue;
        }
        else if (parsed.key == key)
        {
            lines[index] = line;
            written = true;
        }
        if (inTokens)
        {
            lastInTokens = index;
        }
    }
    if (written)
    {
        return;
    }
    if (lastInTokens)
    {
        lines.insert(lines.begin() + static_cast<std::ptrdiff_t>(*lastInTokens + 1), line);
        return;
    }
    lines.push_back("[Tokens]");
    lines.push_back(line);
}

std::uint8_t ToChannel(float value)
{
    return static_cast<std::uint8_t>(std::lround(std::clamp(value, 0.0f, 1.0f) * ChannelMax));
}
} // namespace

std::string_view GetGameTheme()
{
    return UI::RmlBridge::GetActiveThemeName();
}

void ForgetThemeTokens()
{
    GetThemeCache().clear();
}

std::optional<std::string> FindThemeTokenSource(std::string_view theme, const char* token)
{
    const auto setting = GetTheme(theme).GetSetting("Tokens", token);
    if (!setting)
    {
        return std::nullopt;
    }
    const std::u8string folder = setting->directory.filename().u8string();
    return std::string(folder.begin(), folder.end());
}

bool WriteThemeToken(std::string_view theme, const char* token, const std::string& value)
{
    const std::filesystem::path folder =
        std::filesystem::u8path(ThemesFolder) / std::filesystem::u8path(std::string(theme));
    std::error_code fileError;
    if (!std::filesystem::is_directory(folder, fileError))
    {
        return false;
    }
    const std::filesystem::path file = folder / "tokens.ini";
    std::string text;
    // A theme that inherits all its tokens has no tokens.ini of its own: it gets one.
    const bool exists = std::filesystem::exists(file, fileError);
    if (exists)
    {
        std::ifstream in(file, std::ios::binary);
        if (!in)
        {
            return false;
        }
        std::stringstream read;
        read << in.rdbuf();
        text = read.str();
    }
    std::string updated = SetTokenInText(text, token, value);
    if (!exists)
    {
        updated += '\n';
    }

    // Through a temporary file: a write that fails leaves the theme's tokens as they were.
    std::string error;
    const bool written = Data::WriteTextFile(file, updated, error);
    ForgetThemeTokens();
    return written;
}

std::string SetTokenInText(std::string_view text, std::string_view token, const std::string& value)
{
    std::vector<std::string> lines = SplitLines(text);
    PlaceToken(lines, token, value);
    const std::string_view newline = text.find("\r\n") != std::string_view::npos ? "\r\n" : "\n";
    return JoinLines(lines, newline, text.ends_with('\n'));
}

std::optional<TokenColor> ParseTokenColor(std::string_view value)
{
    TokenColor color;
    if (const std::optional<ImVec4> shade = ParseShade(value))
    {
        color.rgb[0] = shade->x;
        color.rgb[1] = shade->y;
        color.rgb[2] = shade->z;
        return color;
    }
    constexpr std::string_view RgbaStart = "rgba(";
    if (!value.starts_with(RgbaStart) || !value.ends_with(')'))
    {
        return std::nullopt;
    }
    // Four whole numbers from 0 to 255 between commas, spaces around them, and nothing else: "0.5" or a
    // fifth number is no color the editor could write back as the theme has it.
    std::array<int, 4> channels{};
    const char* position = value.data() + RgbaStart.size();
    const char* end = value.data() + value.size() - 1;
    const auto skipSpaces = [&position, end]
    {
        while (position < end && *position == ' ')
        {
            ++position;
        }
    };
    for (std::size_t index = 0; index < channels.size(); ++index)
    {
        skipSpaces();
        if (index > 0)
        {
            if (position == end || *position != ',')
            {
                return std::nullopt;
            }
            ++position;
            skipSpaces();
        }
        int& channel = channels[index];
        const auto [parsed, error] = std::from_chars(position, end, channel);
        if (error != std::errc{} || channel < 0 || channel > static_cast<int>(ChannelMax))
        {
            return std::nullopt;
        }
        position = parsed;
    }
    skipSpaces();
    if (position != end)
    {
        return std::nullopt;
    }
    for (int index = 0; index < 3; ++index)
    {
        color.rgb[index] = static_cast<float>(channels[static_cast<std::size_t>(index)]) / ChannelMax;
    }
    color.alpha = channels[3];
    return color;
}

std::string FormatTokenColor(const TokenColor& color)
{
    const Core::Color::Rgb rgb = ToRgb(color.rgb);
    char text[32] = {};
    // In small letters, as the shipped tokens.ini files write them.
    if (color.alpha >= static_cast<int>(ChannelMax))
    {
        std::snprintf(text, sizeof(text), "#%02x%02x%02x", rgb.red, rgb.green, rgb.blue);
    }
    else
    {
        std::snprintf(text, sizeof(text), "rgba(%d, %d, %d, %d)", rgb.red, rgb.green, rgb.blue, color.alpha);
    }
    return text;
}

std::optional<std::string> ReadThemeToken(std::string_view theme, const char* token)
{
    const auto setting = GetTheme(theme).GetSetting("Tokens", token);
    if (!setting)
    {
        return std::nullopt;
    }
    return setting->value;
}

std::optional<ImVec4> ParseShade(std::string_view shade)
{
    const std::optional<Core::Color::Rgb> rgb = Core::Color::ParseRgb(shade);
    if (!rgb)
    {
        return std::nullopt;
    }
    return ToSwatch(*rgb);
}

ImVec4 ToSwatch(Core::Color::Rgb color)
{
    return {static_cast<float>(color.red) / ChannelMax, static_cast<float>(color.green) / ChannelMax,
            static_cast<float>(color.blue) / ChannelMax, 1.0f};
}

Core::Color::Rgb ToRgb(const float rgb[3])
{
    return {ToChannel(rgb[0]), ToChannel(rgb[1]), ToChannel(rgb[2])};
}

ImVec4 GetTokenSwatch(const char* token, std::string_view theme)
{
    const std::optional<std::string> value = ReadThemeToken(theme, token);
    return (value ? ParseShade(*value) : std::nullopt).value_or(MissingShadeSwatch);
}

ImVec4 GetThemeColorSwatch(ThemeColor color, std::string_view theme)
{
    return GetTokenSwatch(Data::ServerList::GetThemeColorToken(color), theme);
}

ImVec4 GetButtonColorSwatch(const Data::ServerList::ButtonColor& color, std::string_view theme)
{
    const std::optional<ImVec4> shade = ParseShade(color.shade);
    return shade ? *shade : GetThemeColorSwatch(color.theme, theme);
}

void RenderSwatch(const ImVec4& color)
{
    // Drawn, not a widget: a swatch takes no clicks, so it needs no ID that could meet the list's or another
    // swatch's in the same scope.
    const float size = ImGui::GetTextLineHeight();
    const ImVec2 from = ImGui::GetCursorScreenPos();
    const ImVec2 to(from.x + size, from.y + size);
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(from, to, ImGui::ColorConvertFloat4ToU32(ImVec4(color.x, color.y, color.z, 1.0f)));
    drawList->AddRect(from, to, ImGui::GetColorU32(ImGuiCol_Border));
    ImGui::Dummy(ImVec2(size, size));
}

const char* GetThemeColorLabel(ThemeColor color)
{
    switch (color)
    {
    case ThemeColor::Gray:
        return I18N::Editor::Gray;
    case ThemeColor::Orange:
        return I18N::Editor::Orange;
    case ThemeColor::Silver:
        return I18N::Editor::Silver;
    case ThemeColor::Default:
    default:
        return I18N::Editor::Default;
    }
}

std::string GetColorLabel(const Data::ServerList::ServerListData& list, std::string_view color)
{
    if (const std::optional<ThemeColor> themeColor = Data::ServerList::FindThemeColor(color))
    {
        return GetThemeColorLabel(*themeColor);
    }
    const Data::ServerList::CustomColor* custom = list.FindCustomColor(color);
    if (custom == nullptr)
    {
        return std::string(color);
    }
    return custom->name.empty() ? custom->id : custom->name;
}

bool SelectableText(const char* id, const std::string& text, bool selected, ImGuiSelectableFlags flags)
{
    const ImVec2 textPosition = ImGui::GetCursorScreenPos();
    const bool clicked = ImGui::Selectable(id, selected, flags);
    ImGui::GetWindowDrawList()->AddText(textPosition, ImGui::GetColorU32(ImGuiCol_Text), text.c_str());
    return clicked;
}

bool PickColor(const char* id, const Data::ServerList::ServerListData& list, std::string& color, float width,
               const ColorSwatch& swatch)
{
    ImGui::PushID(id);
    RenderSwatch(swatch(color));
    ImGui::SameLine();
    ImGui::SetNextItemWidth(width * g_MuEditorCore.GetUIScale());
    bool picked = false;
    if (ImGui::BeginCombo("##Color", GetColorLabel(list, color).c_str()))
    {
        int option = 0;
        const auto offer = [&](const std::string& candidate)
        {
            ImGui::PushID(option++);
            RenderSwatch(swatch(candidate));
            ImGui::SameLine();
            if (SelectableText("##ColorOption", GetColorLabel(list, candidate), candidate == color) &&
                candidate != color)
            {
                color = candidate;
                picked = true;
            }
            ImGui::PopID();
        };
        for (int value = 0; value < Data::ServerList::ThemeColorCount; ++value)
        {
            offer(Data::ServerList::GetThemeColorName(static_cast<ThemeColor>(value)));
        }
        for (const Data::ServerList::CustomColor& custom : list.colors)
        {
            offer(custom.id);
        }
        ImGui::EndCombo();
    }
    ImGui::PopID();
    return picked;
}
} // namespace MuEditor::ServerList::Colors

#endif // _EDITOR
