#include "stdafx.h"
#include <doctest.h>
#include "RmlLayoutFixture.h"
#include "Data/GameConfig/GameConfig.h"
#include "UI/RmlBridge/RmlScaleInputs.h"
#include "UI/Scaling/UITransform.h"
#include <fstream>
#include <regex>
#include <sstream>

namespace UI::Tests
{
std::string ReadFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    CAPTURE(path.generic_string());
    REQUIRE(file.is_open());
    std::ostringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

std::string AssetMarkup(const std::string& theme, const std::string& name)
{
    const std::filesystem::path root = MU_RMLUI_DIR;
    const auto themed = root / "themes" / theme;
    auto path = themed / (name + ".rml");
    if (!std::filesystem::exists(path))
        path = root / (name + ".rml");
    // Supply state directly to the real DOM without registering the game's networked models.
    const std::regex bindings(R"(\sdata-(?:model|for|if|class-[\w-]+|style-[\w-]+|attr-[\w-]+|event-[\w-]+)="[^"]*")");
    std::string markup = std::regex_replace(ReadFile(path), bindings, "");
    markup = std::regex_replace(markup, std::regex(R"(\{\{[^}]*\}\})"), "");
    const std::regex links(R"rcss(href="([^"]+\.rcss)")rcss");
    std::string resolved;
    size_t end = 0;
    for (auto it = std::sregex_iterator(markup.begin(), markup.end(), links); it != std::sregex_iterator(); ++it)
    {
        auto stylesheet = themed / (*it)[1].str();
        if (!std::filesystem::exists(stylesheet))
            stylesheet = root / (*it)[1].str();
        resolved.append(markup, end, static_cast<size_t>(it->position()) - end);
        resolved += "href=\"" + stylesheet.generic_string() + "\"";
        end = static_cast<size_t>(it->position() + it->length());
    }
    resolved.append(markup, end, std::string::npos);
    return resolved;
}

RmlLayoutFixture::RmlLayoutFixture()
    : m_PreviousPercent(GameConfig::GetInstance().GetUIScalePercent()),
      m_PreviousContentScale(UI::Scaling::GetWindowContentScale())
{
    Rml::SetRenderInterface(&m_Renderer);
    Rml::SetFileInterface(&m_Files);
    REQUIRE(Rml::Initialise());
    const std::string fonts = MU_FONT_DIR;
    REQUIRE(Rml::LoadFontFace(fonts + "/LiberationSans-Regular.ttf", true));
    REQUIRE(Rml::LoadFontFace(fonts + "/LiberationSans-Bold.ttf"));
    REQUIRE(Rml::LoadFontFace(fonts + "/DejaVuSans.ttf"));
    REQUIRE(Rml::LoadFontFace(fonts + "/DejaVuSans-Bold.ttf"));
    context = Rml::CreateContext("layout-fixture", {1280, 720});
    REQUIRE(context != nullptr);
}

RmlLayoutFixture::~RmlLayoutFixture()
{
    Rml::Shutdown();
    Rml::SetFileInterface(nullptr);
    Rml::SetRenderInterface(nullptr);
    GameConfig::GetInstance().SetUIScalePercent(m_PreviousPercent);
    UI::Scaling::SetWindowContentScale(m_PreviousContentScale);
}

void RmlLayoutFixture::Configure(Rml::Vector2i viewport, int percent, float contentScale)
{
    GameConfig::GetInstance().SetUIScalePercent(percent);
    UI::Scaling::SetWindowContentScale(contentScale);
    context->SetDimensions(viewport);
    context->SetDensityIndependentPixelRatio(UI::Scaling::CompanionRatio(viewport.x, viewport.y));
    UI::RmlBridge::ApplyScaleInputs(context);
}

Rml::ElementDocument* RmlLayoutFixture::Load(const std::string& theme, const std::string& name)
{
    return LoadMarkup(AssetMarkup(theme, name));
}

Rml::ElementDocument* RmlLayoutFixture::LoadMarkup(const std::string& markup)
{
    auto* document = context->LoadDocumentFromMemory(markup);
    REQUIRE(document != nullptr);
    document->Show();
    Refresh();
    return document;
}

void RmlLayoutFixture::Refresh() { context->Update(); context->Render(); }

} // namespace UI::Tests
