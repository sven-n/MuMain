#include "stdafx.h"

#ifdef _EDITOR

#include "ServerListColors.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "I18N/All.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/ThemeInheritance.h"

#include <filesystem>
#include <map>
#include <memory>

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
constexpr int HexBase = 16;
constexpr unsigned int ChannelMask = 0xFF;
constexpr int RedShift = 16;
constexpr int GreenShift = 8;

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

ImVec4 FromRgb(unsigned int rgb)
{
    return {static_cast<float>((rgb >> RedShift) & ChannelMask) / ChannelMax,
            static_cast<float>((rgb >> GreenShift) & ChannelMask) / ChannelMax,
            static_cast<float>(rgb & ChannelMask) / ChannelMax, 1.0f};
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
    if (!Data::ServerList::IsShade(shade))
    {
        return std::nullopt;
    }
    return FromRgb(static_cast<unsigned int>(std::stoul(std::string(shade.substr(1)), nullptr, HexBase)));
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
    const float size = ImGui::GetTextLineHeight();
    ImGui::ColorButton("##Color", color,
                       ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoDragDrop,
                       ImVec2(size, size));
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
