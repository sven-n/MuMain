#include "stdafx.h"

#ifdef _EDITOR

#include "MapObjectBrowser.h"
#include "MapObjectImport.h"
#include "ObjectThumbnail.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/Renderer/MuRenderer.h"    // mu::GetRenderer().GetTexturePointer

#include "imgui.h"

#include <algorithm>
#include <cctype>
#include <cwctype>
#include <filesystem>

struct SDL_GPUTexture;

namespace fs = std::filesystem;

namespace
{
    const wchar_t* DATA_ROOT = L"Data";

    // Sentinel for a preview that failed to load/render, so we don't retry it
    // every frame (which would waste the per-frame render budget forever).
    constexpr unsigned int THUMB_FAILED = 0xFFFFFFFFu;

    std::wstring ToLower(std::wstring s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](wchar_t c) { return (wchar_t)towlower(c); });
        return s;
    }

}

CMapObjectBrowser& CMapObjectBrowser::GetInstance()
{
    static CMapObjectBrowser instance;
    return instance;
}

void CMapObjectBrowser::ScanSourceWorlds()
{
    m_worlds.clear();
    std::error_code ec;
    if (fs::is_directory(DATA_ROOT, ec))
    {
        for (const auto& entry : fs::directory_iterator(DATA_ROOT, ec))
        {
            if (!entry.is_directory())
                continue;
            const std::wstring name = ToLower(entry.path().filename().wstring());
            if (name.rfind(L"object", 0) != 0)
                continue;
            int n = 0;
            bool any = false;
            for (size_t i = 6; i < name.size(); ++i)
            {
                if (!iswdigit(name[i])) { any = false; break; }
                n = n * 10 + (name[i] - L'0');
                any = true;
            }
            if (any)
                m_worlds.push_back(n);
        }
    }
    std::sort(m_worlds.begin(), m_worlds.end());
    m_scanned = true;
}

void CMapObjectBrowser::LoadFileList(int sourceWorld)
{
    for (unsigned int t : m_thumbs)
        if (t != THUMB_FAILED)
            CObjectThumbnail::FreeTexture(t);
    m_thumbs.clear();

    // Indices into the old m_files no longer mean anything; any request still
    // in flight for one of them is abandoned (its result, once delivered, is
    // simply never polled).
    m_scratchOwner = -1;

    m_sourceWorld = sourceWorld;
    m_files = Editor::ObjectImport::ListModelFiles(sourceWorld);
    m_thumbs.assign(m_files.size(), 0);
    m_selected = -1;
}

void CMapObjectBrowser::Render(int currentWorld, int* outImportedType)
{
    if (!m_scanned)
        ScanSourceWorlds();

    if (m_worlds.empty())
    {
        ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.6f, 1.0f),
                           "No Data\\Object* folders found.");
        return;
    }

    if (m_sourceWorld < 0)
        LoadFileList(m_worlds.front());

    ImGui::Text("Import an object model from any map onto this map (World %d).", currentWorld);

    // Source map selector.
    ImGui::SetNextItemWidth(160.0f);
    const std::string cur = "Object" + std::to_string(m_sourceWorld);
    if (ImGui::BeginCombo("Source map", cur.c_str()))
    {
        for (int w : m_worlds)
        {
            const std::string label = "Object" + std::to_string(w);
            const bool selected = (w == m_sourceWorld);
            if (ImGui::Selectable(label.c_str(), selected) && w != m_sourceWorld)
                LoadFileList(w);
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    if (ImGui::Button("Rescan"))
    {
        ScanSourceWorlds();
        LoadFileList(m_sourceWorld);
    }

    ImGui::SetNextItemWidth(200.0f);
    ImGui::InputTextWithHint("##filter", "filter by name...", m_filter, sizeof(m_filter));

    // Import button.
    const bool hasSel = (m_selected >= 0 && m_selected < (int)m_files.size());
    ImGui::BeginDisabled(!hasSel);
    if (ImGui::Button("Use selected on current map") && hasSel)
    {
        const int type = Editor::ObjectImport::UseModelOnCurrentMap(
            currentWorld, m_sourceWorld, m_files[m_selected]);
        if (type >= 0)
        {
            if (outImportedType) *outImportedType = type;
            m_status = "Imported to model slot " + std::to_string(type) +
                       " - selected for placing.";
        }
        else
        {
            m_status = "Import failed (no free slot or bad file). See console.";
        }
    }
    ImGui::EndDisabled();
    if (!m_status.empty())
        ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "%s", m_status.c_str());

    ImGui::Separator();
    ImGui::Text("%d models in Object%d", (int)m_files.size(), m_sourceWorld);

    // Case-insensitive name filter.
    std::string filterLower;
    for (size_t i = 0; i < sizeof(m_filter) && m_filter[i]; ++i)
        filterLower.push_back((char)tolower((unsigned char)m_filter[i]));

    // Thumbnail grid. Previews are loaded on demand (a few per frame): each
    // foreign model is loaded into a scratch slot, rendered, and cached.
    const float thumb = (float)g_ObjectThumbnail.Size();
    const int columns = 5;
    int budget = 3;  // heavy (loads model + textures), so throttle
    int col = 0;

    ImGui::BeginChild("ObjModels", ImVec2(0, 0), true);
    for (int i = 0; i < (int)m_files.size() && i < (int)m_thumbs.size(); ++i)
    {
        const std::string name = StringUtils::WideToNarrow(m_files[i].c_str());
        if (!filterLower.empty())
        {
            std::string lower = name;
            std::transform(lower.begin(), lower.end(), lower.begin(),
                           [](char c) { return (char)tolower(c); });
            if (lower.find(filterLower) == std::string::npos)
                continue;
        }

        // Render this preview if not done yet (budget-limited). Failures are
        // marked so they aren't retried every frame. The scratch slot is
        // shared by every candidate (see MapObjectImport::LoadForPreview), so
        // only one request may be in flight at a time - see m_scratchOwner.
        if (m_thumbs[i] == 0 && budget > 0)
        {
            if (m_scratchOwner == i)
            {
                // Already own the scratch slot - poll without reloading,
                // which would stomp the model this render is using.
                const unsigned int tex = g_ObjectThumbnail.PollSlotPreview();
                if (!g_ObjectThumbnail.IsScratchBusy())
                {
                    m_thumbs[i] = (tex != 0) ? tex : THUMB_FAILED;
                    m_scratchOwner = -1;
                    --budget;
                }
            }
            else if (m_scratchOwner == -1 && !g_ObjectThumbnail.IsScratchBusy() &&
                     !mu::GetRenderer().HasPendingOffscreenCaptures())
            {
                // The busy check alone only proves CObjectThumbnail has delivered
                // a result; it doesn't prove the renderer has actually replayed
                // that capture's draw commands yet (replay is deferred to
                // EndFrame(), after this Update() has already run). Reloading the
                // scratch slot's model here would release the texture a
                // still-queued replay is about to sample - see
                // HasPendingOffscreenCaptures()'s doc comment.
                const int slot = Editor::ObjectImport::LoadForPreview(m_sourceWorld, m_files[i]);
                if (slot < 0)
                    m_thumbs[i] = THUMB_FAILED;
                else if (g_ObjectThumbnail.RequestSlotPreview(slot))
                    m_scratchOwner = i;
                --budget;
            }
            // else: the scratch slot is busy with a different entry (or an
            // abandoned request from before a source-world switch), or a
            // previous capture hasn't been replayed by the renderer yet - wait.
        }

        if (col % columns != 0)
            ImGui::SameLine();
        ++col;

        ImGui::PushID(i);
        const bool selected = (i == m_selected);
        if (selected)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));

        bool clicked = false;
        SDL_GPUTexture* const thumbTexPtr = (m_thumbs[i] != 0 && m_thumbs[i] != THUMB_FAILED)
            ? static_cast<SDL_GPUTexture*>(mu::GetRenderer().GetTexturePointer(m_thumbs[i]))
            : nullptr;
        if (thumbTexPtr != nullptr)
            clicked = ImGui::ImageButton("t", (ImTextureID)(intptr_t)thumbTexPtr, ImVec2(thumb, thumb));
        else if (m_thumbs[i] == THUMB_FAILED)
            clicked = ImGui::Button("n/a", ImVec2(thumb, thumb));   // no preview available
        else
            clicked = ImGui::Button("...", ImVec2(thumb, thumb));   // pending

        if (selected)
            ImGui::PopStyleColor();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%s", name.c_str());
        if (clicked)
            m_selected = i;
        ImGui::PopID();
    }
    ImGui::EndChild();
}

#endif // _EDITOR
