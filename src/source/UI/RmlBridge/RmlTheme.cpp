#include "stdafx.h"
#include "RmlTheme.h"
#include "Data/GameConfig/GameConfig.h"
#include "Core/Platform/WinIni.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <regex>
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

        // Inverse of NarrowAscii -- theme.ini paths built from GetActiveThemeName() are plain
        // ASCII by the same convention, so a naive widen is safe here.
        std::wstring WidenAscii(const std::string& s)
        {
            std::wstring out(s.size(), L'\0');
            std::transform(s.begin(), s.end(), out.begin(), [](char c) { return static_cast<wchar_t>(c); });
            return out;
        }

        // Directory portion of `path` (up to and including the last '/'), empty if `path` has no
        // '/'. Used to resolve a <link href="X.rcss"> the same way RmlUi's own AbsolutePath()
        // would -- relative to whichever RML file actually loaded, not always documentPath.
        std::string DirectoryOf(const std::string& path)
        {
            const size_t lastSlash = path.find_last_of('/');
            return (lastSlash == std::string::npos) ? std::string() : path.substr(0, lastSlash + 1);
        }

        // Reads themes/<theme>/tokens.ini's [Tokens] section (modern-theme-visual-direction.md's
        // token table) via the same private-profile API theme.ini's capability reader uses.
        // Missing file/theme/key all resolve to an empty string -- an RCSS rule referencing an
        // undefined token renders visibly wrong (empty value), which is enough: this is an
        // authoring-time mistake to catch in review, not a runtime condition worth handling more
        // gracefully than that.
        std::string ResolveToken(const std::string& tokenName)
        {
            const std::string iniPath = "Data/Interface/RmlUi/themes/" + GetActiveThemeName() + "/tokens.ini";
            wchar_t buffer[256] = {};
            GetPrivateProfileStringW(L"Tokens", WidenAscii(tokenName).c_str(), L"", buffer,
                static_cast<DWORD>(std::size(buffer)), WidenAscii(iniPath).c_str());
            return NarrowAscii(buffer);
        }

        // Replaces every token(name) in `rcssText` with ResolveToken(name). Plain text
        // substitution, not CSS-aware -- the author is responsible for quoting a token() call the
        // same way they'd quote a literal (e.g. font-family: "token(font-body)";).
        std::string SubstituteTokens(const std::string& rcssText)
        {
            static const std::regex tokenPattern(R"(token\(([a-zA-Z0-9_-]+)\))");
            std::string out;
            out.reserve(rcssText.size());
            size_t lastEnd = 0;
            for (auto it = std::sregex_iterator(rcssText.begin(), rcssText.end(), tokenPattern);
                 it != std::sregex_iterator(); ++it)
            {
                const std::smatch& m = *it;
                out.append(rcssText, lastEnd, static_cast<size_t>(m.position(0)) - lastEnd);
                out += ResolveToken(m[1].str());
                lastEnd = static_cast<size_t>(m.position(0) + m.length(0));
            }
            out.append(rcssText, lastEnd, rcssText.size() - lastEnd);
            return out;
        }

        // Design-token substitution (modern-theme-visual-direction.md) -- this vendored RmlUi has
        // no var()/custom-property mechanism, so a themed .rcss authored with token(name) markers
        // needs its tokens resolved before RmlUi ever sees the text. RmlUi's own
        // XMLNodeHandlerHead supports an inline <style> block in <head>, handled identically to an
        // external <link type="text/rcss"> for cascade purposes -- so this finds every <link> in
        // the RML's <head> (every migrated document links at least two: base.rcss, then its own
        // <name>.rcss), reads the .rcss file each one points at, and (only for a file that
        // actually contains a token(...) marker) splices the substituted text in as <style>
        // instead, in the same position, preserving cascade order.
        //
        // 2026-09-04 fix: this used to std::regex_search for a single match and return after
        // handling it -- correct for a document with exactly one stylesheet link, silently wrong
        // for the (universal, in practice) two-link case: base.rcss's link is first in every
        // document's <head>, so it was the only one ever substituted, and every window's own
        // <name>.rcss -- where nearly every token(...) call actually lives -- was left as a
        // literal, unsubstituted <link>, sending literal text like `token(font-body)`/
        // `token(semantic-warning)` straight to RmlUi's parser. Symptom: labels/tooltips/button
        // text silently failing to render across every migrated window, not just login. Iterating
        // every <link> match fixes this for any number of stylesheet links, not just two.
        //
        // Content-driven, not theme-name-driven, on purpose (architecture-principles.md §30 --
        // same reasoning as ThemeProvidesOwnIconChrome() above): a stylesheet with no token(...)
        // marker leaves its own <link> completely untouched, the exact code path every window
        // already took before this existed. `legacy` (and any future non-tokenized file) never
        // enters the substitution branch at all, rather than being special-cased by name.
        std::string InlineTokenizedStylesheet(const std::string& rmlText, const std::string& resolvedRmlPath)
        {
            // Custom raw-string delimiter (R"re(...)re") -- the pattern's own text contains `)"`
            // (the capture group closing right before a literal quote), which would otherwise be
            // misread as the raw string's own terminator.
            static const std::regex linkPattern(R"re(<link\s+type="text/rcss"\s+href="([^"]+)"\s*/>)re");

            std::string out;
            out.reserve(rmlText.size());
            size_t lastEnd = 0;
            for (auto it = std::sregex_iterator(rmlText.begin(), rmlText.end(), linkPattern);
                 it != std::sregex_iterator(); ++it)
            {
                const std::smatch& m = *it;
                out.append(rmlText, lastEnd, static_cast<size_t>(m.position(0)) - lastEnd);

                std::string replacement = m.str(0); // default: leave this <link> exactly as authored
                std::ifstream rcssFile(DirectoryOf(resolvedRmlPath) + m[1].str(), std::ios::binary);
                if (rcssFile)
                {
                    std::ostringstream rcssBuffer;
                    rcssBuffer << rcssFile.rdbuf();
                    const std::string rcssText = rcssBuffer.str();
                    if (rcssText.find("token(") != std::string::npos)
                        replacement = "<style>" + SubstituteTokens(rcssText) + "</style>";
                }
                // If the file can't be read here, replacement stays the original <link> -- let
                // RmlUi's own <link> loading attempt it and surface the real error, same as before.

                out += replacement;
                lastEnd = static_cast<size_t>(m.position(0) + m.length(0));
            }
            out.append(rmlText, lastEnd, rmlText.size() - lastEnd);
            return out;
        }
    }

    const std::string& GetActiveThemeName()
    {
        static const std::string cached = ToLower(NarrowAscii(GameConfig::GetInstance().GetRmlTheme()));
        return cached;
    }

    bool ThemeProvidesOwnIconChrome()
    {
        // Cached the same way GetActiveThemeName() is -- read once, the active theme is fixed for
        // the process lifetime (see this file's own theme-hot-swap comment on LoadThemedDocument).
        static const bool cached = []
        {
            const std::string iniPath = "Data/Interface/RmlUi/themes/" + GetActiveThemeName() + "/theme.ini";
            return GetPrivateProfileIntW(L"Capabilities", L"ProvidesOwnIconChrome", 0, WidenAscii(iniPath).c_str()) != 0;
        }();
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
        std::string resolvedPath = sourceUrl;
        if (!file)
        {
            file.open(documentPath, std::ios::binary);
            resolvedPath = documentPath;
        }
        if (!file)
        {
            g_ErrorReport.Write(L"> [RmlTheme] Failed to open '%hs' or '%hs'.\r\n", sourceUrl.c_str(), documentPath);
            return nullptr;
        }

        std::ostringstream buffer;
        buffer << file.rdbuf();

        // Design-token substitution -- see InlineTokenizedStylesheet()'s own comment. No-op for
        // any stylesheet that doesn't use token(...) markers.
        const std::string rmlText = InlineTokenizedStylesheet(buffer.str(), resolvedPath);

        Rml::ElementDocument* doc = context->LoadDocumentFromMemory(rmlText, sourceUrl);
        if (!doc)
            g_ErrorReport.Write(L"> [RmlTheme] Failed to load '%hs' as theme '%hs' (source url '%hs').\r\n",
                documentPath, GetActiveThemeName().c_str(), sourceUrl.c_str());

        return doc;
    }
}
