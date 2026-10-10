#include "stdafx.h"
#include "RmlTheme.h"
#include "RmlNativeText.h"
#include "RmlStackingOrder.h"
#include "Data/GameConfig/GameConfig.h"
#include "Core/Platform/WinIni.h"
#include "Render/RmlUi/RmlUiRuntime.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Factory.h>

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>

extern EGameScene SceneFlag;

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

    }

    namespace
    {
        // Function-local statics, not namespace-scope globals, purely to sidestep static
        // initialization order -- both are written after their first read (SetActiveThemeName(),
        // ThemeExists()'s callers), so "mutable" is the operative word here, not "cached once".
        std::string& ActiveThemeNameStorage()
        {
            static std::string name = ToLower(NarrowAscii(GameConfig::GetInstance().GetRmlTheme()));
            return name;
        }

        bool ComputeUsesNativeTextSize(const std::string& themeName)
        {
            const std::string iniPath = "Data/Interface/RmlUi/themes/" + themeName + "/theme.ini";
            return GetPrivateProfileIntW(L"Capabilities", L"NativeTextSize", 0, WidenAscii(iniPath).c_str()) != 0;
        }

        bool& UsesNativeTextSizeStorage()
        {
            static bool usesNativeTextSize = ComputeUsesNativeTextSize(ActiveThemeNameStorage());
            return usesNativeTextSize;
        }

    }

    const std::string& GetActiveThemeName()
    {
        return ActiveThemeNameStorage();
    }

    bool ThemeUsesNativeTextSize()
    {
        return UsesNativeTextSizeStorage();
    }

    void SetActiveThemeName(const std::string& themeName)
    {
        ActiveThemeNameStorage() = ToLower(themeName);
        UsesNativeTextSizeStorage() = ComputeUsesNativeTextSize(ActiveThemeNameStorage());

        // Both themes deliberately reuse the same <template name="..."> for a shared concept
        // (e.g. "window_shell_bg") in their own themes/<theme>/ copy. Rml::TemplateCache caches a
        // template both by its resolved file path (fine -- each theme's path is distinct) and by
        // that declared name (Factory.h's GetTemplate()/TemplateCache::GetTemplate(), used to
        // resolve <body template="...">) -- and the by-name entry is only refreshed when a path is
        // loaded for the first time, not on every lookup. Once both themes' copies of a shared
        // template name have been loaded at least once each, the by-name entry keeps pointing at
        // whichever theme's copy was most recently loaded fresh, regardless of which theme is
        // active now -- so a document reloaded against the *other* theme could still splice in the
        // wrong theme's template content. Clearing here forces every <body template="..."> lookup
        // after this point to reload fresh from the now-active theme's own file.
        // A theme reload must read updated token values and RCSS, including when switching
        // back to a theme whose sheets were cached earlier this session.
        Rml::Factory::ClearStyleSheetCache();
        Rml::Factory::ClearTemplateCache();
    }

    bool ThemeExists(const std::string& themeName)
    {
        const std::string basePath = "Data/Interface/RmlUi/themes/" + ToLower(themeName) + "/base.rcss";
        std::ifstream file(basePath, std::ios::binary);
        return file.good();
    }

    std::string ThemedDocumentSourceUrl(const char* documentName, const std::string& themeName)
    {
        return std::string("Data/Interface/RmlUi/themes/") + ToLower(themeName) + "/" + documentName;
    }

    std::vector<std::string> DiscoverAvailableThemes()
    {
        std::vector<std::string> themes;
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator("Data/Interface/RmlUi/themes", ec))
        {
            if (!entry.is_directory())
                continue;
            std::string name = ToLower(entry.path().filename().string());
            if (ThemeExists(name))
                themes.push_back(std::move(name));
        }
        std::sort(themes.begin(), themes.end());
        return themes;
    }

    std::string GetThemeDisplayName(const std::string& themeName)
    {
        const std::string iniPath = "Data/Interface/RmlUi/themes/" + ToLower(themeName) + "/theme.ini";
        wchar_t buffer[128] = {};
        GetPrivateProfileStringW(L"Meta", L"DisplayName", L"", buffer, static_cast<DWORD>(std::size(buffer)),
            WidenAscii(iniPath).c_str());
        std::string displayName = NarrowAscii(buffer);
        if (!displayName.empty())
            return displayName;

        // No theme.ini/[Meta]/DisplayName -- fall back to the folder name, capitalized.
        displayName = themeName;
        if (!displayName.empty())
            displayName[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(displayName[0])));
        return displayName;
    }

    namespace
    {
    // Set on the document element of every main-scene document, see
    // SuspendMainSceneDocumentsOutsideMainScene().
    constexpr const char* MainSceneDocumentAttribute = "data-main-scene-document";
    // A DocumentScene::Every document's depths, in and outside the main scene.
    constexpr const char* MainSceneDepthAttribute = "data-main-scene-depth";
    constexpr const char* OtherSceneDepthAttribute = "data-other-scene-depth";
    bool s_mainSceneDocumentsSuspended = false;

    template <typename Visit> void ForEachMainSceneDocument(Visit&& visit)
    {
        for (int contextIndex = 0; contextIndex < Rml::GetNumContexts(); ++contextIndex)
        {
            Rml::Context* context = Rml::GetContext(contextIndex);
            for (int documentIndex = 0; documentIndex < context->GetNumDocuments(); ++documentIndex)
            {
                Rml::ElementDocument* document = context->GetDocument(documentIndex);
                if (document->HasAttribute(MainSceneDocumentAttribute))
                    visit(document);
            }
        }
    }
    } // namespace

    Rml::ElementDocument* ThemedDocumentLoader::Load(Rml::Context* context, const char* documentPath)
    {
        return Load(context, documentPath, std::string(), std::string());
    }

    Rml::ElementDocument* ThemedDocumentLoader::Load(Rml::Context* context, const char* documentPath,
                                                     const std::string& modelPlaceholder, const std::string& modelName)
    {
        // documentPath's basename (the part after the last '/') is what the per-theme source URL
        // needs -- e.g. "Data/Interface/RmlUi/login.rml" -> "login.rml".
        const std::string path(documentPath);
        const size_t lastSlash = path.find_last_of('/');
        const std::string documentName = (lastSlash == std::string::npos) ? path : path.substr(lastSlash + 1);

        const std::string sourceUrl = ThemedDocumentSourceUrl(documentName.c_str(), GetActiveThemeName());

        // Prefer a per-theme override of the document's own MARKUP (not just its styling) at
        // themes/<theme>/<name>.rml, falling back to the shared documentPath when no such override
        // exists -- most windows have none, so this is a no-op for them
        // (one extra failed ifstream open, immediately falls through). This exists for content
        // that must genuinely differ, not just look different, per theme; CSS-only hiding
        // (`display: none`) and a data-model boolean gating `data-if` both put per-theme awareness
        // into C++, which is exactly the kind of per-context branching this branch's architecture
        // avoids everywhere else. A per-theme markup override keeps theme differentiation where it
        // already lives for every other window: which file gets loaded, decided purely by
        // directory convention, zero runtime "which theme" logic in C++.
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

        std::string rmlText = buffer.str();
        if (!modelPlaceholder.empty())
        {
            for (size_t at = rmlText.find(modelPlaceholder); at != std::string::npos;
                 at = rmlText.find(modelPlaceholder, at + modelName.size()))
                rmlText.replace(at, modelPlaceholder.size(), modelName);
        }

        Rml::ElementDocument* doc = context->LoadDocumentFromMemory(rmlText, sourceUrl);
        ApplyNativeTextSize(doc);
        ApplyStackingDepth(doc, documentName);
        ApplyDocumentScene(doc, documentName);
        if (!doc)
            g_ErrorReport.Write(L"> [RmlTheme] Failed to load '%hs' as theme '%hs' (source url '%hs').\r\n",
                documentPath, GetActiveThemeName().c_str(), sourceUrl.c_str());

        return doc;
    }

    void ApplyStackingDepth(Rml::ElementDocument* document, const std::string& documentName)
    {
        if (document == nullptr)
            return;

        const std::optional<float> depth = StackingDepthForDocument(documentName);
        if (!depth)
        {
            g_ErrorReport.Write(L"> [RmlTheme] No stacking depth for '%hs'.\r\n", documentName.c_str());
            return;
        }
        document->SetProperty(Rml::PropertyId::ZIndex, Rml::Property(*depth, Rml::Unit::NUMBER));
    }

    void ApplyDocumentScene(Rml::ElementDocument* document, const std::string& documentName)
    {
        if (document == nullptr)
            return;
        if (SceneForDocument(documentName) == DocumentScene::Main)
            document->SetAttribute(MainSceneDocumentAttribute, true);
        const std::optional<float> mainDepth = StackingDepthForDocument(documentName);
        if (const std::optional<float> otherDepth = StackingDepthOutsideMainScene(documentName); otherDepth && mainDepth)
        {
            document->SetAttribute(MainSceneDepthAttribute, *mainDepth);
            document->SetAttribute(OtherSceneDepthAttribute, *otherDepth);
        }
    }

    void ApplySceneStackingDepths()
    {
        const char* attribute = SceneFlag == MAIN_SCENE ? MainSceneDepthAttribute : OtherSceneDepthAttribute;
        for (int contextIndex = 0; contextIndex < Rml::GetNumContexts(); ++contextIndex)
        {
            Rml::Context* context = Rml::GetContext(contextIndex);
            for (int documentIndex = 0; documentIndex < context->GetNumDocuments(); ++documentIndex)
            {
                Rml::ElementDocument* document = context->GetDocument(documentIndex);
                if (!document->HasAttribute(attribute))
                    continue;
                const float depth = document->GetAttribute<float>(attribute, 0.f);
                const Rml::Property* current = document->GetLocalProperty(Rml::PropertyId::ZIndex);
                if (current == nullptr || current->Get<float>() != depth)
                    document->SetProperty(Rml::PropertyId::ZIndex, Rml::Property(depth, Rml::Unit::NUMBER));
            }
        }
    }

    void SuspendMainSceneDocumentsOutsideMainScene()
    {
        if (SceneFlag == MAIN_SCENE)
            return;

        ForEachMainSceneDocument(
            [](Rml::ElementDocument* document)
            {
                if (document->GetLocalProperty(Rml::PropertyId::Display) == nullptr)
                    document->SetProperty(Rml::PropertyId::Display, Rml::Property(Rml::Style::Display::None));
            });
        s_mainSceneDocumentsSuspended = true;
    }

    void ResumeMainSceneDocuments()
    {
        if (!s_mainSceneDocumentsSuspended)
            return;

        s_mainSceneDocumentsSuspended = false;
        ForEachMainSceneDocument([](Rml::ElementDocument* document)
                                 { document->RemoveProperty(Rml::PropertyId::Display); });
    }

    namespace
    {
        struct ThemeReloadEntry
        {
            const void* owner;
            ThemeReloadRegistry::Callback callback;
            ThemeReloadRegistry::Document document;
        };

        // Never destroyed: an owner destroyed at exit after it (a static window) still unregisters.
        // In registration order, which owners without a document keep on a theme switch.
        std::vector<ThemeReloadEntry>& Registry()
        {
            static auto* registry = new std::vector<ThemeReloadEntry>();
            return *registry;
        }

        std::vector<ThemeReloadEntry>::iterator FindThemeReloadEntry(const void* owner)
        {
            auto& registry = Registry();
            return std::find_if(registry.begin(), registry.end(),
                                [owner](const ThemeReloadEntry& entry) { return entry.owner == owner; });
        }

        // Where `document` sits among its context's documents, bottom first; -1 if it has none.
        int StackPosition(const ThemeReloadEntry& entry)
        {
            Rml::ElementDocument* document = entry.document ? entry.document() : nullptr;
            Rml::Context* context = document != nullptr ? document->GetContext() : nullptr;
            if (context == nullptr)
                return -1;
            for (int i = 0; i < context->GetNumDocuments(); ++i)
            {
                if (context->GetDocument(i) == document)
                    return i;
            }
            return -1;
        }

        // The registry, with the owners whose documents are loaded reordered among themselves from
        // the bottom of their stacks up.
        std::vector<ThemeReloadEntry> ReloadOrder()
        {
            std::vector<ThemeReloadEntry> entries = Registry();
            std::vector<size_t> slots;
            std::vector<std::pair<int, size_t>> stacked;
            for (size_t i = 0; i < entries.size(); ++i)
            {
                const int position = StackPosition(entries[i]);
                if (position < 0)
                    continue;
                slots.push_back(i);
                stacked.emplace_back(position, i);
            }
            std::stable_sort(stacked.begin(), stacked.end(),
                             [](const auto& a, const auto& b) { return a.first < b.first; });
            std::vector<ThemeReloadEntry> ordered = entries;
            for (size_t k = 0; k < slots.size(); ++k)
                ordered[slots[k]] = entries[stacked[k].second];
            return ordered;
        }
    }

    void ThemeReloadRegistry::Register(const void* owner, Callback callback, Document document)
    {
        const auto entry = FindThemeReloadEntry(owner);
        if (entry != Registry().end())
        {
            entry->callback = std::move(callback);
            entry->document = std::move(document);
        }
        else
            Registry().push_back({owner, std::move(callback), std::move(document)});
    }

    void ThemeReloadRegistry::Unregister(const void* owner)
    {
        const auto entry = FindThemeReloadEntry(owner);
        if (entry != Registry().end())
            Registry().erase(entry);
    }

    void ReloadAllThemedDocuments()
    {
        // Drop the focus before any document is unloaded. RmlUi clears Context::focus directly in
        // both UnloadDocument() and OnElementDetach() without dispatching Blur, so a focused
        // <input> is destroyed without its WidgetTextInput ever calling DeactivateKeyboard() --
        // and RmlUiSystemInterface's text-input latch then stays set with no field left to clear
        // it. CManager::UpdateKeyEvent() reads that latch to decide the whole UI is typing, so it
        // would skip every window from then on: no hotkeys, no Enter-to-chat, until a relaunch.
        // Blurring here goes through Context::Focus(), which does dispatch the event.
        //
        // This is also the right behaviour on its own terms: every document is about to be
        // rebuilt, so nothing should still be holding focus across the swap.
        if (Rml::Context* context = RmlUiRuntime::Instance().IsCreated()
                ? RmlUiRuntime::Instance().GetContext()
                : nullptr)
        {
            if (Rml::Element* focused = context->GetFocusElement())
                focused->Blur();
        }

        // Copy first -- see this function's own header comment (RmlTheme.h) for why.
        for (const ThemeReloadEntry& entry : ReloadOrder())
        {
            if (entry.callback)
                entry.callback();
        }
    }
}
