#include "stdafx.h"
#include "RmlTheme.h"
#include "Data/GameConfig/GameConfig.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace UI::RmlBridge
{
    namespace
    {
        // Theme names are simple ASCII identifiers ("legacy", "modern", ...) by convention, so a
        // plain narrow cast is safe here -- no need for the WideToUtf8 machinery LoginWin.cpp
        // uses for real (possibly non-ASCII) localized strings.
        std::string NarrowAscii(const std::wstring& s)
        {
            std::string out(s.size(), '\0');
            std::transform(s.begin(), s.end(), out.begin(), [](wchar_t c) { return static_cast<char>(c); });
            return out;
        }

        std::string ToLower(std::string s)
        {
            std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)std::tolower(c); });
            return s;
        }
    }

    const std::string& GetActiveThemeName()
    {
        static const std::string cached = ToLower(NarrowAscii(GameConfig::GetInstance().GetRmlTheme()));
        return cached;
    }

    std::string ThemedDocumentSourceUrl(const char* documentName, const std::string& themeName)
    {
        return std::string("Data/Interface/RmlUi/themes/") + ToLower(themeName) + "/" + documentName;
    }

    Rml::ElementDocument* LoadThemedDocument(Rml::Context* context, const char* documentPath)
    {
        // documentPath's basename (the part after the last '/') is what the per-theme source URL
        // needs -- e.g. "Data/Interface/RmlUi/login.rml" -> "login.rml".
        const std::string path(documentPath);
        const size_t lastSlash = path.find_last_of('/');
        const std::string documentName = (lastSlash == std::string::npos) ? path : path.substr(lastSlash + 1);

        const std::string sourceUrl = ThemedDocumentSourceUrl(documentName.c_str(), GetActiveThemeName());

        // 2026-09-02: prefer a per-theme override of the document's own MARKUP (not just its
        // styling) at themes/<theme>/<name>.rml, falling back to the shared documentPath when no
        // such override exists -- every window but main_frame.rml still has none, so this is a
        // no-op for them (one extra failed ifstream open, immediately falls through). Exists
        // because CSS-only hiding proved unreliable for content that must genuinely differ, not
        // just look different, per theme (modern's old bottom-HUD button row leaked through a
        // stylesheet `display: none` rule no matter how it was hardened; a data-model boolean
        // gating `data-if` also worked but put per-theme awareness into C++, which is exactly the
        // kind of per-context branching this branch's architecture avoids everywhere else -- see
        // NewUIMainFrameWindow's git history for that attempt). This keeps theme differentiation
        // where it already lives for every other window: which file gets loaded, decided purely
        // by directory convention, zero runtime "which theme" logic in C++.
        std::ifstream file(sourceUrl, std::ios::binary);
        if (!file)
            file.open(documentPath, std::ios::binary);
        if (!file)
        {
            g_ErrorReport.Write(L"> [RmlTheme] Failed to open '%hs' or '%hs'.\r\n", sourceUrl.c_str(), documentPath);
            return nullptr;
        }

        std::ostringstream buffer;
        buffer << file.rdbuf();

        Rml::ElementDocument* doc = context->LoadDocumentFromMemory(buffer.str(), sourceUrl);
        if (!doc)
            g_ErrorReport.Write(L"> [RmlTheme] Failed to load '%hs' as theme '%hs' (source url '%hs').\r\n",
                documentPath, GetActiveThemeName().c_str(), sourceUrl.c_str());

        return doc;
    }
}
