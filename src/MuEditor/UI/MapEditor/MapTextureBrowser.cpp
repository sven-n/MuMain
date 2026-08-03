#include "stdafx.h"

#ifdef _EDITOR

#include "MapTextureBrowser.h"

#include "imgui.h"
#include "MapTextureImport.h"
#include "Render/Sprites/GlobalBitmap.h"    // Bitmaps[]
#include "Render/Textures/ZzzTexture.h"     // LoadBitmap / DeleteBitmap
#include "UI/Console/MuEditorConsoleUI.h"

// Painter's current tile selection (ZzzInterface.cpp) - set it after import so the
// freshly-added texture is ready to paint on the Texture tab.
extern int SelectMapping;

#include <algorithm>
#include <filesystem>
#include <cwctype>

namespace fs = std::filesystem;

namespace
{
    // Root of the client data tree (relative to the working directory, same base
    // LoadBitmap prepends).
    const wchar_t* DATA_ROOT = L"Data";

    // Dedicated scratch bitmap-index base for preview textures. Sits far above the
    // game's own indices (which top out around the low 33000s + dynamic range) so
    // browser previews never collide with real textures.
    constexpr unsigned int BROWSER_SLOT_BASE = 40'000'000u;

    // Cap on previews loaded at once (one world's tile set is well under this).
    constexpr int MAX_PREVIEWS = 128;

    // Thumbnail grid.
    constexpr float THUMB_SIZE = 64.0f;
    constexpr int   COLUMNS    = 5;

    std::wstring ToLower(std::wstring s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](wchar_t c) { return (wchar_t)std::towlower(c); });
        return s;
    }

    // A terrain tile texture is Tile*/ExtTile*/AlphaTile* with an image extension.
    bool IsTileTextureFile(const fs::path& p)
    {
        if (!p.has_extension())
            return false;
        const std::wstring ext = ToLower(p.extension().wstring());
        const bool imageExt = (ext == L".jpg" || ext == L".jpeg" || ext == L".tga" ||
                               ext == L".ozj" || ext == L".ozt" || ext == L".png" ||
                               ext == L".bmp");
        if (!imageExt)
            return false;

        const std::wstring stem = ToLower(p.stem().wstring());
        return stem.rfind(L"tile", 0) == 0 ||       // TileGrass01, TileRock01, ...
               stem.rfind(L"exttile", 0) == 0 ||    // ExtTile01..16
               stem.rfind(L"alphatile", 0) == 0;    // AlphaTile*/AlphaTileGround*
    }

    // The client stores terrain textures as OZJ/OZT but its loader is driven by
    // the plain .jpg/.tga names (LoadBitmap swaps .jpg->.OZJ, .tga->.OZT). So to
    // load a file we discovered on disk, hand LoadBitmap the .jpg/.tga form and
    // let it swap back to the real OZJ/OZT.
    std::wstring LoaderName(const fs::path& p)
    {
        const std::wstring ext = ToLower(p.extension().wstring());
        if (ext == L".ozj")
            return p.stem().wstring() + L".jpg";
        if (ext == L".ozt")
            return p.stem().wstring() + L".tga";
        return p.filename().wstring();  // already .jpg/.tga/etc.
    }
}

CMapTextureBrowser& CMapTextureBrowser::GetInstance()
{
    static CMapTextureBrowser instance;
    return instance;
}

void CMapTextureBrowser::ScanWorlds()
{
    m_worlds.clear();
    std::error_code ec;
    if (!fs::is_directory(DATA_ROOT, ec))
    {
        m_scanned = true;
        return;
    }

    for (const auto& entry : fs::directory_iterator(DATA_ROOT, ec))
    {
        if (!entry.is_directory())
            continue;
        const std::wstring name = ToLower(entry.path().filename().wstring());
        if (name.rfind(L"world", 0) != 0)
            continue;
        // Parse the trailing number of "World<N>".
        int n = 0;
        bool hasDigit = false;
        for (size_t i = 5; i < name.size(); ++i)
        {
            if (!iswdigit(name[i])) { hasDigit = false; break; }
            n = n * 10 + (name[i] - L'0');
            hasDigit = true;
        }
        if (hasDigit)
            m_worlds.push_back(n);
    }
    std::sort(m_worlds.begin(), m_worlds.end());
    m_scanned = true;
}

void CMapTextureBrowser::ClearPreviews()
{
    for (const Preview& p : m_previews)
        DeleteBitmap(p.slot, true);
    m_previews.clear();
}

void CMapTextureBrowser::LoadWorld(int world)
{
    ClearPreviews();
    m_selectedWorld = world;
    m_selectedIndex = -1;

    const fs::path folder = fs::path(DATA_ROOT) / (L"World" + std::to_wstring(world));
    std::error_code ec;
    if (!fs::is_directory(folder, ec))
        return;

    // Gather matching files first so we can present them sorted by name.
    std::vector<fs::path> files;
    for (const auto& entry : fs::directory_iterator(folder, ec))
    {
        if (entry.is_regular_file() && IsTileTextureFile(entry.path()))
            files.push_back(entry.path());
    }
    std::sort(files.begin(), files.end());

    unsigned int slot = BROWSER_SLOT_BASE;
    for (const fs::path& file : files)
    {
        if ((int)m_previews.size() >= MAX_PREVIEWS)
            break;

        // LoadBitmap prepends "Data\\", so pass the path relative to Data root:
        // "World<N>\<jpg-or-tga name>". bCheck=false so a bad file logs quietly
        // instead of popping an error box.
        const std::wstring relative = L"World" + std::to_wstring(world) + L"\\" +
                                      LoaderName(file);
        const bool ok = LoadBitmap(relative.c_str(), slot, GL_LINEAR, GL_REPEAT, false);

        m_previews.push_back({ file.filename().wstring(), slot, ok });
        ++slot;
    }

    g_MuEditorConsoleUI.LogEditor("[MapEditor] Texture browser loaded World textures");
}

void CMapTextureBrowser::Render(int defaultWorld)
{
    if (!m_scanned)
        ScanWorlds();

    if (m_worlds.empty())
    {
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.6f, 1.0f),
                           "No Data\\World* folders found in the working directory.");
        return;
    }

    // Seed the selection to the current map the first time.
    if (m_selectedWorld < 0)
        LoadWorld(defaultWorld > 0 ? defaultWorld : m_worlds.front());

    ImGui::Text("Browse the tile textures shipped by any map.");

    // World selector.
    ImGui::SetNextItemWidth(160.0f);
    const std::string current = "World " + std::to_string(m_selectedWorld);
    if (ImGui::BeginCombo("World", current.c_str()))
    {
        for (int w : m_worlds)
        {
            const std::string label = "World " + std::to_string(w);
            const bool selected = (w == m_selectedWorld);
            if (ImGui::Selectable(label.c_str(), selected) && w != m_selectedWorld)
                LoadWorld(w);
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Rescan"))
    {
        ScanWorlds();
        LoadWorld(m_selectedWorld);
    }

    // Import actions. Textures are added to the CURRENT map (defaultWorld), not
    // the world being browsed - so you can borrow another map's tile onto yours.
    const bool hasSelection = (m_selectedIndex >= 0 && m_selectedIndex < (int)m_previews.size());
    const Preview* sel = hasSelection ? &m_previews[m_selectedIndex] : nullptr;

    ImGui::BeginDisabled(!hasSelection);
    if (ImGui::Button("Use selected texture on this map") && sel)
    {
        const std::wstring src = L"Data\\World" + std::to_wstring(m_selectedWorld) + L"\\" + sel->file;
        const int slot = Editor::TextureImport::UseTextureFile(defaultWorld, src);
        if (slot >= 0)
        {
            SelectMapping = slot;  // ready to paint with it on the Texture tab
            char s[96];
            snprintf(s, sizeof(s), "Added to slot %d - selected on the Texture tab.", slot);
            m_status = s;
        }
        else
        {
            m_status = "Failed - no free ExtTile slot, or unsupported format (OZT).";
        }
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    if (ImGui::Button("Upload image..."))
    {
        std::wstring picked;
        if (Editor::TextureImport::PickImageFile(picked))
        {
            const int slot = Editor::TextureImport::UseTextureFile(defaultWorld, picked);
            if (slot >= 0)
            {
                SelectMapping = slot;
                char s[96];
                snprintf(s, sizeof(s), "Uploaded to slot %d - selected on the Texture tab.", slot);
                m_status = s;
            }
            else
            {
                m_status = "Upload failed - use a .jpg/.jpeg or .ozj, and ensure a free ExtTile slot.";
            }
        }
        else
        {
            m_status = "Upload cancelled.";
        }
    }
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                       "Adds to current map (World %d) as a free ExtTile slot, then paint it on the Texture tab.",
                       defaultWorld);
    if (!m_status.empty())
        ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "%s", m_status.c_str());

    ImGui::Separator();
    ImGui::Text("%d textures in World %d", (int)m_previews.size(), m_selectedWorld);

    // Always-visible label for the texture under the cursor. A tooltip alone is
    // unreliable here because the game's 3D cursor sprite draws over it.
    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Hovered: %s",
                       m_hoveredName.empty() ? "(hover a texture)" : m_hoveredName.c_str());

    ImGui::BeginChild("BrowserThumbs", ImVec2(0, 0), false);
    std::string hoveredThisFrame;
    for (int idx = 0; idx < (int)m_previews.size(); ++idx)
    {
        const Preview& p = m_previews[idx];
        if (idx % COLUMNS != 0)
            ImGui::SameLine();

        char name[128];
        snprintf(name, sizeof(name), "%ls", p.file.c_str());

        const bool isSelected = (idx == m_selectedIndex);
        if (isSelected)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));

        ImGui::PushID(idx);
        bool clicked = false;
        if (p.loaded)
            clicked = ImGui::ImageButton("t", (ImTextureID)Bitmaps[p.slot].TextureNumber,
                                         ImVec2(THUMB_SIZE, THUMB_SIZE));
        else
            clicked = ImGui::Button("fail", ImVec2(THUMB_SIZE, THUMB_SIZE));

        if (ImGui::IsItemHovered())
        {
            hoveredThisFrame = name;
            ImGui::SetTooltip("%s", name);
        }
        if (clicked)
            m_selectedIndex = idx;
        ImGui::PopID();

        if (isSelected)
            ImGui::PopStyleColor();
    }
    ImGui::EndChild();

    m_hoveredName = hoveredThisFrame;
}

#endif // _EDITOR
