#include "stdafx.h"
#include "RmlTheme.h"
#include "Data/GameConfig/GameConfig.h"
#include "Core/Platform/WinIni.h" // GetPrivateProfileIntW -- reading each theme's own theme.ini

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

        std::wstring WidenAscii(const std::string& s)
        {
            std::wstring out(s.size(), L'\0');
            std::transform(s.begin(), s.end(), out.begin(), [](char c) { return static_cast<wchar_t>(c); });
            return out;
        }
    }

    const std::string& GetActiveThemeName()
    {
        static const std::string cached = ToLower(NarrowAscii(GameConfig::GetInstance().GetRmlTheme()));
        return cached;
    }

    bool UsesLegacySpriteChrome(const std::string& themeName)
    {
        // Data-driven, not a hardcoded C++ list -- see this header's own comment on why. Absence
        // of the manifest file (GetPrivateProfileIntW's own contract) or the key inside it both
        // fall through to the same default (0/false), so a theme folder with no theme.ini at all
        // -- the expected shape for a plain sprite-free/modder theme -- just works.
        const std::wstring manifestPath =
            L"Data/Interface/RmlUi/themes/" + WidenAscii(ToLower(themeName)) + L"/theme.ini";
        return GetPrivateProfileIntW(L"Theme", L"UsesLegacySpriteChrome", 0, manifestPath.c_str()) != 0;
    }

    std::string ThemedDocumentSourceUrl(const char* documentName, const std::string& themeName)
    {
        return std::string("Data/Interface/RmlUi/themes/") + ToLower(themeName) + "/" + documentName;
    }

    Rml::ElementDocument* LoadThemedDocument(Rml::Context* context, const char* documentPath)
    {
        std::ifstream file(documentPath, std::ios::binary);
        if (!file)
        {
            g_ErrorReport.Write(L"> [RmlTheme] Failed to open '%hs'.\r\n", documentPath);
            return nullptr;
        }

        std::ostringstream buffer;
        buffer << file.rdbuf();

        // documentPath's basename (the part after the last '/') is what the per-theme source URL
        // needs -- e.g. "Data/Interface/RmlUi/login.rml" -> "login.rml".
        const std::string path(documentPath);
        const size_t lastSlash = path.find_last_of('/');
        const std::string documentName = (lastSlash == std::string::npos) ? path : path.substr(lastSlash + 1);

        const std::string sourceUrl = ThemedDocumentSourceUrl(documentName.c_str(), GetActiveThemeName());
        Rml::ElementDocument* doc = context->LoadDocumentFromMemory(buffer.str(), sourceUrl);
        if (!doc)
            g_ErrorReport.Write(L"> [RmlTheme] Failed to load '%hs' as theme '%hs' (source url '%hs').\r\n",
                documentPath, GetActiveThemeName().c_str(), sourceUrl.c_str());

        return doc;
    }
}
