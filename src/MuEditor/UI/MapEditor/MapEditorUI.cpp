#include "stdafx.h"

#ifdef _EDITOR

#include "MapEditorUI.h"
#include "MapEditorSave.h"
#include "MapTextureBrowser.h"
#include "MapObjectPlace.h"
#include "MapObjectBrowser.h"
#include "ObjectThumbnail.h"

#include "Render/Textures/ZzzOpenglUtil.h"  // CollisionPosition (world hit point)
#include "Engine/Object/w_ObjectInfo.h"      // class OBJECT (transform fields)
#include "Camera/CameraManager.h"            // top-down minimap view
#include "Camera/CameraMode.h"
#include "Camera/CameraState.h"
#include "Camera/FreeFlyCamera.h"

#include "imgui.h"
#include "Core/Globals/_define.h"          // EDIT_MAPPING / EDIT_NONE
#include "Core/Globals/_TextureIndex.h"    // BITMAP_MAPTILE
#include "Render/Sprites/GlobalBitmap.h"   // Bitmaps[]
#include "Render/Terrain/ZzzLodTerrain.h"  // CurrentLayer
#include "World/MapInfra/MapManager.h"     // gMapManager.WorldActive
#include "Core/MuEditorCore.h"             // hover flag for input blocking
#include "UI/Console/MuEditorConsoleUI.h"

#include <cstring>

// Brush/selection globals owned by ZzzInterface.cpp. Declared locally in the
// same style as EditObjects.cpp rather than pulling in a wide interface header.
extern int EditFlag;
extern int SelectMapping;
extern int BrushSize;

// Terrain arrays + picking state. SelectFlag/SelectXF/SelectYF are updated by the
// terrain render pass each frame while EditFlag != EDIT_NONE (the mouse-over-tile
// under the cursor).
extern unsigned char TerrainMappingLayer1[];
extern unsigned char TerrainMappingLayer2[];
extern float         TerrainMappingAlpha[];
extern float         BackTerrainHeight[];
extern bool SelectFlag;

// Terrain height sculpt (ZzzLodTerrain.cpp).
void AddTerrainHeight(float xf, float yf, float Height, int Range, float* Buffer);
void CreateTerrainNormal();
void CreateTerrainLight();
void SaveTerrainHeight(wchar_t* name);

// Live mouse-button states. We capture and clear these before the game consumes
// them (CaptureInputForPainting) so painting doesn't also move/attack the hero.
extern bool MouseLButton, MouseLButtonPop, MouseLButtonPush, MouseLButtonDBClick;
extern bool MouseRButton, MouseRButtonPop, MouseRButtonPush;

namespace
{
    // Number of terrain tile slots the loader fills (BITMAP_MAPTILE + 0..29):
    // 14 base Tile* slots followed by 16 ExtTile slots. Index N in a mapping
    // layer selects BITMAP_MAPTILE + N.
    constexpr int TILE_SLOT_COUNT = 30;

    // Human labels for the fixed base slots; the rest are ExtTile01..16.
    // Mirrors the slot->filename chain in MapManager::CreateTerrain.
    const char* TileSlotName(int slot)
    {
        static const char* kBase[] = {
            "TileGrass01", "TileGrass02", "TileGround01", "TileGround02",
            "TileGround03", "TileWater01", "TileWood01",  "TileRock01",
            "TileRock02",   "TileRock03",  "TileRock04",  "TileRock05",
            "TileRock06",   "TileRock07",
        };
        if (slot >= 0 && slot < (int)(sizeof(kBase) / sizeof(kBase[0])))
            return kBase[slot];
        static char ext[16];
        snprintf(ext, sizeof(ext), "ExtTile%02d", slot - 13);  // slot 14 -> ExtTile01
        return ext;
    }

    // Palette thumbnail size (pixels) and grid width (columns).
    constexpr float TILE_THUMB_SIZE = 56.0f;
    constexpr int   TILE_COLUMNS    = 5;

    // Extra font scale applied to the Map Editor window so it's readable at high
    // display resolutions (the base ImGui font is small on 2560-wide screens).
    constexpr float UI_FONT_SCALE = 1.30f;

    // Brush half-size clamp. The painted square is (BrushSize*2 + 1) tiles wide.
    constexpr int MAX_BRUSH_HALF = 10;

    // Layer-2 sentinel meaning "no overlay tile here" (matches InitTerrainMappingLayer).
    constexpr unsigned char NO_OVERLAY_TILE = 255;

    // Total terrain cells (256x256), used for undo snapshots.
    constexpr int CELLS = TERRAIN_SIZE * TERRAIN_SIZE;
}

CMapEditorUI& CMapEditorUI::GetInstance()
{
    static CMapEditorUI instance;
    return instance;
}

int CMapEditorUI::ResolveWorldNumber() const
{
    if (m_TargetWorldOverride >= 0)
        return m_TargetWorldOverride;
    // Off-by-one: the world enum value maps to folder World{value + 1}. Special
    // per-map remaps (Blood Castle, Chaos Castle, ...) exist in MapManager, but
    // for the common case WorldActive + 1 is correct; the user can override the
    // number in the UI when a special map needs it.
    return gMapManager.WorldActive + 1;
}

void CMapEditorUI::EnterEditMode(int editFlag)
{
    EditFlag = editFlag;
    m_bEditActive = true;
}

void CMapEditorUI::RestoreGameMode()
{
    if (!m_bEditActive)
        return;
    EditFlag = EDIT_NONE;
    m_bEditActive = false;
}

void CMapEditorUI::Render(bool* p_open)
{
    // While the minimap top-down view is active (and still on FreeFly), render the
    // whole map: full-terrain bounds + g_Camera.TopViewEnable, which the engine
    // itself uses for minimap capture to bypass per-tile terrain AND object
    // culling.
    //
    // IMPORTANT: only WRITE these flags while we own the mode, and release them
    // exactly once on exit. The game's own minimap uses TopViewEnable too, so
    // writing it every frame (even false) would fight the game minimap and crash it.
    {
        extern bool g_bMapEditorFullTerrain;
        extern CameraState g_Camera;
        const bool topdown = m_bMinimapMode &&
                             (CameraManager::Instance().GetCurrentMode() == CameraMode::FreeFly);
        if (topdown)
        {
            g_bMapEditorFullTerrain = true;
            g_Camera.TopViewEnable = true;
        }
        else if (m_topdownWasActive)
        {
            g_bMapEditorFullTerrain = false;
            g_Camera.TopViewEnable = false;
        }
        m_topdownWasActive = topdown;
    }

    const bool show = (p_open != nullptr && *p_open);
    if (!show)
    {
        RestoreGameMode();
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(480, 640), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Map Editor", p_open))
    {
        ImGui::End();
        // Collapsed but still open: keep edit mode as-is.
        return;
    }

    ImGui::SetWindowFontScale(UI_FONT_SCALE);

    // Reset the object-thumbnail render budget for this frame.
    g_ObjectThumbnail.BeginFrame();

    // Count hovering the scrollable child regions (e.g. the texture browser grid)
    // as hovering the panel, so the game cursor stays hidden and clicks don't leak
    // to the world while the mouse is over editor content.
    if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows |
                               ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
        g_MuEditorCore.SetHoveringUI(true);

    // Each tab decides what edit mode it needs this frame; default to none.
    m_desiredEditFlag = EDIT_NONE;

    if (ImGui::BeginTabBar("MapEditorTabs"))
    {
        if (ImGui::BeginTabItem("Texture"))
        {
            RenderTextureTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Objects"))
        {
            RenderObjectsTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Height"))
        {
            RenderHeightTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("T. Browse"))
        {
            g_MapTextureBrowser.Render(ResolveWorldNumber());
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Minimap"))
        {
            RenderMinimapTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("O. Browse"))
        {
            // Importing a model selects it for placing: jump to the Objects tab's
            // Place mode with the new type ready.
            int imported = -1;
            g_MapObjectBrowser.Render(ResolveWorldNumber(), &imported);
            if (imported >= 0)
            {
                m_selectedModelType = imported;
                m_objMode = 0;              // Place mode
                m_bObjEditEnabled = true;
                m_modelsWorld = -1;         // force the Objects model list to rebuild
            }
            ImGui::EndTabItem();
        }
        // Later phases add tabs here (Attribute / Height).
        ImGui::EndTabBar();
    }

    ImGui::End();

    // The window's close (X) button flips *p_open. Restore the game the moment
    // that happens so the client doesn't stay stuck in edit mode.
    if (!*p_open)
    {
        RestoreGameMode();
        return;
    }

    // Apply the active tab's requested edit mode (or hand the game back if the
    // current tab isn't an editing tool, e.g. Browse).
    if (m_desiredEditFlag != EDIT_NONE)
        EnterEditMode(m_desiredEditFlag);
    else
        RestoreGameMode();
}

void CMapEditorUI::RenderTextureTab()
{
    // Painting toggle. When on, take the game into EDIT_MAPPING (mouse picking +
    // paint); when off, hand the game back so you can walk without painting.
    ImGui::Checkbox("Enable painting", &m_bPaintingEnabled);
    if (m_bPaintingEnabled)
    {
        m_desiredEditFlag = EDIT_MAPPING;
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "PAINTING");
    }
    else
    {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "off (walk freely)");
    }

    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f),
                       "Left-click terrain to paint. Right-click clears layer 2.");
    ImGui::Separator();

    // Target world / file the Save button writes to.
    int autoWorld = gMapManager.WorldActive + 1;
    bool useAuto = (m_TargetWorldOverride < 0);
    if (ImGui::Checkbox("Auto world number", &useAuto))
        m_TargetWorldOverride = useAuto ? -1 : autoWorld;
    if (!useAuto)
    {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(80.0f);
        ImGui::InputInt("##world", &m_TargetWorldOverride);
        if (m_TargetWorldOverride < 0) m_TargetWorldOverride = 0;
    }
    ImGui::Text("Saves to: Data\\World%d\\EncTerrain%d.map", ResolveWorldNumber(), ResolveWorldNumber());

    ImGui::Separator();

    // Layer + brush controls. Both layers honor the brush size; the base layer
    // (0) replaces the tile outright, while the overlay (1) blends over the base
    // using the opacity below.
    ImGui::RadioButton("Layer 1 (base)", &CurrentLayer, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Layer 2 (overlay)", &CurrentLayer, 1);

    ImGui::SetNextItemWidth(160.0f);
    ImGui::SliderInt("Brush", &BrushSize, 0, MAX_BRUSH_HALF, "");
    ImGui::SameLine();
    ImGui::Text("%d x %d tiles", BrushSize * 2 + 1, BrushSize * 2 + 1);

    // Overlay blend strength (how strongly the layer-2 tile shows over the base).
    if (CurrentLayer == 1)
    {
        ImGui::SetNextItemWidth(160.0f);
        ImGui::SliderFloat("Overlay opacity", &m_OverlayAlpha, 0.0f, 1.0f, "%.2f");
    }

    // Dropper: click picks the tile under the cursor (from the current layer)
    // into the selection instead of painting. Holding Alt does the same.
    ImGui::Checkbox("Dropper - pick tile under cursor (or hold Alt)", &m_bDropperMode);

    ImGui::Separator();
    ImGui::Text("Tile palette (selected: %s)", TileSlotName(SelectMapping));

    // Palette of the current world's loaded tile textures. Each cell is the
    // actual GPU texture the terrain samples, so it always matches this map.
    for (int slot = 0; slot < TILE_SLOT_COUNT; ++slot)
    {
        if (slot % TILE_COLUMNS != 0)
            ImGui::SameLine();

        ImGui::PushID(slot);
        const GLuint tex = Bitmaps[BITMAP_MAPTILE + slot].TextureNumber;
        const bool selected = (SelectMapping == slot);

        if (selected)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));

        bool clicked = false;
        if (tex != 0)
        {
            clicked = ImGui::ImageButton("t", (ImTextureID)tex,
                                         ImVec2(TILE_THUMB_SIZE, TILE_THUMB_SIZE));
        }
        else
        {
            // Slot has no texture on this map (e.g. missing ExtTile) - show a
            // labeled placeholder so the index is still selectable.
            clicked = ImGui::Button("--", ImVec2(TILE_THUMB_SIZE, TILE_THUMB_SIZE));
        }

        if (selected)
            ImGui::PopStyleColor();

        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("%d: %s", slot, TileSlotName(slot));
        if (clicked)
            SelectMapping = slot;

        ImGui::PopID();
    }

    ImGui::Separator();

    // Undo the last paint stroke (one level).
    ImGui::BeginDisabled(!m_bHasUndo);
    if (ImGui::Button("Undo last stroke"))
        Undo();
    ImGui::EndDisabled();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
    if (ImGui::Button("Save terrain textures", ImVec2(-1.0f, 0.0f)))
    {
        const int world = ResolveWorldNumber();
        Editor::MapSave::SaveMappingEncrypted(world, world);
    }
    ImGui::PopStyleColor(2);
    ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.3f, 1.0f),
                       "Overwrites the live map file (no backup).");

    // Apply the brush this frame (after the UI, so hover state is known).
    PaintMapping();
}

void CMapEditorUI::PaintMapping()
{
    // Uses m_PaintLDown/m_PaintRDown captured in CaptureInputForPainting (the raw
    // MouseLButton was cleared there to stop the hero walking).
    if (!m_bPaintingEnabled || !SelectFlag || g_MuEditorCore.IsHoveringUI())
    {
        m_StrokeActive = false;
        return;
    }

    const int x = (int)SelectXF;
    const int y = (int)SelectYF;
    const int idx0 = TERRAIN_INDEX_REPEAT(x, y);

    // Dropper: a click samples the tile under the cursor into the selection.
    const bool dropper = m_bDropperMode || ImGui::GetIO().KeyAlt;
    if (dropper)
    {
        if (m_PaintLDown)
        {
            const unsigned char picked =
                (CurrentLayer == 0) ? TerrainMappingLayer1[idx0] : TerrainMappingLayer2[idx0];
            if (!(CurrentLayer == 1 && picked == NO_OVERLAY_TILE))
                SelectMapping = picked;
        }
        m_StrokeActive = false;
        return;
    }

    // Alpha/RGBA tiles are overlay-only material; skip them for base painting to
    // avoid writing a translucent tile as an opaque base.
    if (CurrentLayer == 0 && Bitmaps[BITMAP_MAPTILE + SelectMapping].Components == 4)
        return;

    const bool leftPaint  = m_PaintLDown;
    const bool rightErase = m_PaintRDown && CurrentLayer == 1;
    if (!leftPaint && !rightErase)
    {
        m_StrokeActive = false;
        return;
    }

    // Snapshot once at the start of a stroke so Undo reverts the whole drag.
    if (!m_StrokeActive)
    {
        TakeUndoSnapshot();
        m_StrokeActive = true;
    }

    const auto tile = (unsigned char)SelectMapping;
    for (int i = y - BrushSize; i <= y + BrushSize; ++i)
        for (int j = x - BrushSize; j <= x + BrushSize; ++j)
        {
            const int idx = TERRAIN_INDEX_REPEAT(j, i);
            if (leftPaint && CurrentLayer == 0)
            {
                TerrainMappingLayer1[idx] = tile;
            }
            else if (leftPaint)  // overlay paint
            {
                TerrainMappingLayer2[idx] = tile;
                TerrainMappingAlpha[idx] = m_OverlayAlpha;
            }
            else                 // overlay erase (right-click)
            {
                TerrainMappingLayer2[idx] = NO_OVERLAY_TILE;
                TerrainMappingAlpha[idx] = 0.0f;
            }
        }
}

void CMapEditorUI::CaptureInputForPainting()
{
    m_PaintLDown = false;
    m_PaintRDown = false;

    // Only intercept when a tool owns edit mode and the cursor is on the world
    // (not the UI). m_bEditActive covers both the texture and object tools.
    if (!m_bEditActive || g_MuEditorCore.IsHoveringUI())
        return;

    // Read the HELD state from ImGui, not the game's MouseLButton: the game's
    // button flags are set on the Windows press/release edges, and we clear them
    // below every frame, so they'd only ever read true on the press frame (which
    // breaks click-drag). ImGui tracks the physical button independently.
    m_PaintLDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    m_PaintRDown = ImGui::IsMouseDown(ImGuiMouseButton_Right);

    // Consume the click so the game doesn't also move/attack the hero.
    MouseLButton = MouseLButtonPop = MouseLButtonPush = MouseLButtonDBClick = false;
    MouseRButton = MouseRButtonPop = MouseRButtonPush = false;
}

void CMapEditorUI::RenderObjectsTab()
{
    const int world = ResolveWorldNumber();

    // (Re)load the model list when the target world changes. Clear any selection
    // too, since a map change invalidates object pointers.
    if (world != m_modelsWorld)
    {
        m_models = Editor::ObjectPlace::EnumerateModels(world);
        m_modelsWorld = world;
        m_pSelected = nullptr;
        g_ObjectThumbnail.Invalidate();  // model slots differ per map
        if (!m_models.empty())
            m_selectedModelType = m_models.front().type;
        else
            m_selectedModelType = -1;
    }

    ImGui::Checkbox("Enable object editing", &m_bObjEditEnabled);
    if (m_bObjEditEnabled)
    {
        m_desiredEditFlag = EDIT_OBJECT;
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "EDITING");
    }
    else
    {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "off (walk freely)");
    }

    ImGui::RadioButton("Place new", &m_objMode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("Select & edit", &m_objMode, 1);
    ImGui::Separator();

    if (m_objMode == 0)
    {
        ImGui::Text("Loaded object models on this map: %d", (int)m_models.size());
        if (m_models.empty())
        {
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.6f, 1.0f),
                               "No world-object models are loaded on this map.");
        }
        else
        {
            // Placement transform.
            ImGui::SetNextItemWidth(160.0f);
            ImGui::SliderFloat("Yaw", &m_objYaw, 0.0f, 360.0f, "%.0f deg");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(120.0f);
            ImGui::InputFloat("Scale", &m_objScale, 0.05f, 0.25f, "%.2f");
            if (m_objScale < 0.05f) m_objScale = 0.05f;
            ImGui::Checkbox("Snap to tile centre", &m_bObjSnap);

            // Model palette: a grid of live 3D thumbnails of the loaded models.
            char selName[80] = "(none)";
            for (const auto& m : m_models)
                if (m.type == m_selectedModelType)
                {
                    snprintf(selName, sizeof(selName), "%d: %ls", m.type, m.file.c_str());
                    break;
                }
            ImGui::Text("Model palette (selected: %s)", selName);
            ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Left-click the ground to place.");

            const float thumb = (float)g_ObjectThumbnail.Size();
            const int columns = 5;
            // Fill the remaining window height, leaving room for the Undo/Save
            // buttons and status text below (~110px).
            float paletteH = ImGui::GetContentRegionAvail().y - 110.0f;
            if (paletteH < 140.0f) paletteH = 140.0f;
            ImGui::BeginChild("ObjPalette", ImVec2(0, paletteH), true);
            int col = 0;
            for (const auto& m : m_models)
            {
                if (col % columns != 0)
                    ImGui::SameLine();
                ++col;

                ImGui::PushID(m.type);
                const bool selected = (m.type == m_selectedModelType);
                if (selected)
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));

                const unsigned int tex = g_ObjectThumbnail.Get(m.type);
                bool clicked = false;
                if (tex != 0)
                    // Flip V: FBO textures are bottom-up vs ImGui's top-down.
                    clicked = ImGui::ImageButton("t", (ImTextureID)tex, ImVec2(thumb, thumb),
                                                 ImVec2(0, 1), ImVec2(1, 0));
                else  // still rendering this frame - show a placeholder button
                    clicked = ImGui::Button("...", ImVec2(thumb, thumb));

                if (selected)
                    ImGui::PopStyleColor();
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%d: %ls", m.type, m.file.c_str());
                if (clicked)
                    m_selectedModelType = m.type;
                ImGui::PopID();
            }
            ImGui::EndChild();
        }
    }
    else
    {
        ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f),
                           "Left-click an object to select. Drag to move it.");
        RenderSelectedObjectPanel();
    }

    ImGui::Separator();
    ImGui::BeginDisabled(!m_bObjHasUndo);
    if (ImGui::Button("Undo last object edit") || (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z)))
        UndoObjects();
    ImGui::EndDisabled();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
    if (ImGui::Button("Save objects", ImVec2(-1.0f, 0.0f)))
        m_objStatus = Editor::ObjectPlace::Save(world) ? "Saved objects (EncTerrain.obj)."
                                                       : "Save failed.";
    ImGui::PopStyleColor(2);
    ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.3f, 1.0f),
                       "Overwrites the live map objects file (no backup).");
    if (!m_objStatus.empty())
        ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "%s", m_objStatus.c_str());

    if (m_objMode == 0)
        PlaceObjects(world);
    else
        HandleObjectSelect();
}

void CMapEditorUI::RenderSelectedObjectPanel()
{
    if (m_pSelected == nullptr)
    {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(nothing selected)");
        return;
    }

    OBJECT* o = m_pSelected;
    ImGui::Text("Selected: type %d", o->Type);

    // Position: editing may cross an object-grid block, so route through Reposition.
    float pos[3] = { o->Position[0], o->Position[1], o->Position[2] };
    ImGui::SetNextItemWidth(280.0f);
    const bool posEdited = ImGui::InputFloat3("Pos", pos, "%.0f");
    if (ImGui::IsItemActivated()) TakeObjectUndoSnapshot();
    if (posEdited)
        m_pSelected = Editor::ObjectPlace::Reposition(o, pos[0], pos[1], pos[2]);

    // Angle + scale edit safely in place. Snapshot when a field is first grabbed.
    ImGui::SetNextItemWidth(280.0f);
    ImGui::InputFloat3("Angle", o->Angle, "%.0f");
    if (ImGui::IsItemActivated()) TakeObjectUndoSnapshot();
    ImGui::SetNextItemWidth(200.0f);
    ImGui::InputFloat("Scale ", &o->Scale, 0.05f, 0.25f, "%.2f");
    if (ImGui::IsItemActivated()) TakeObjectUndoSnapshot();
    if (o->Scale < 0.05f) o->Scale = 0.05f;

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
    if (ImGui::Button("Delete object") || ImGui::IsKeyPressed(ImGuiKey_Delete))
    {
        TakeObjectUndoSnapshot();
        Editor::ObjectPlace::Remove(m_pSelected);
        m_pSelected = nullptr;
        m_objStatus = "Deleted object.";
    }
    ImGui::PopStyleColor(2);
}

void CMapEditorUI::RenderMinimapTab()
{
    ImGui::TextWrapped("Capture a top-down screenshot of the whole map to make a minimap.");
    ImGui::Separator();

    auto& mgr = CameraManager::Instance();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 0.9f, 1.0f));
    if (ImGui::Button("Top-down view (for minimap)", ImVec2(-1.0f, 0.0f)))
    {
        mgr.SetCameraMode(CameraMode::FreeFly);
        ICamera* cam = mgr.GetActiveCamera();
        if (cam && strcmp(cam->GetName(), "FreeFly") == 0)
            static_cast<FreeFlyCamera*>(cam)->SnapTopDown();
        m_bMinimapMode = true;   // render the whole terrain for the shot
    }
    ImGui::PopStyleColor(2);

    // Rotate the top-down view to orient the map for the shot.
    auto rotateTopDown = [&mgr](float deg)
    {
        ICamera* cam = mgr.GetActiveCamera();
        if (cam && strcmp(cam->GetName(), "FreeFly") == 0)
            static_cast<FreeFlyCamera*>(cam)->RotateYaw(deg);
    };
    if (ImGui::Button("Rotate 90 CW"))
        rotateTopDown(90.0f);
    ImGui::SameLine();
    if (ImGui::Button("Rotate 90 CCW"))
        rotateTopDown(-90.0f);

    if (ImGui::Button("Back to game camera", ImVec2(-1.0f, 0.0f)))
    {
        m_bMinimapMode = false;
        mgr.SetCameraMode(CameraMode::Default);
    }

    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Tips:");
    ImGui::BulletText("Arrow keys pan, PageUp/PageDown change height,");
    ImGui::BulletText("hold right-mouse to tilt, Shift to move faster.");
    ImGui::BulletText("Press F12 to hide the editor UI for a clean shot,");
    ImGui::BulletText("then screenshot and crop to the map square.");
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                       "The map is captured north-up; the in-game minimap applies its own 45-deg spin.");
}

void CMapEditorUI::RenderHeightTab()
{
    ImGui::Checkbox("Enable height editing", &m_bHeightEnabled);
    if (m_bHeightEnabled)
    {
        m_desiredEditFlag = EDIT_HEIGHT;
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "SCULPTING");
    }
    else
    {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "off (walk freely)");
    }

    ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f),
                       "Left-click raises the ground, right-click lowers it.");
    ImGui::Separator();

    ImGui::SetNextItemWidth(200.0f);
    ImGui::SliderInt("Brush radius", &m_heightBrush, 1, 12, "%d tiles");
    ImGui::SetNextItemWidth(200.0f);
    ImGui::SliderFloat("Strength", &m_heightStrength, 1.0f, 40.0f, "%.0f");

    ImGui::Separator();
    ImGui::BeginDisabled(!m_bHeightHasUndo);
    if (ImGui::Button("Undo last height stroke") ||
        (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z)))
        UndoHeight();
    ImGui::EndDisabled();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
    if (ImGui::Button("Save height", ImVec2(-1.0f, 0.0f)))
    {
        const int world = ResolveWorldNumber();
        wchar_t fileName[128];
        swprintf_s(fileName, L"Data\\World%d\\TerrainHeight.OZB", world);
        SaveTerrainHeight(fileName);
        g_MuEditorConsoleUI.LogEditor("[MapEditor] Saved terrain height to TerrainHeight.OZB");
    }
    ImGui::PopStyleColor(2);
    ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.3f, 1.0f),
                       "Overwrites the live map height file (no backup).");

    SculptHeight();
}

void CMapEditorUI::SculptHeight()
{
    if (!m_bHeightEnabled || !SelectFlag || g_MuEditorCore.IsHoveringUI())
    {
        m_heightStrokeActive = false;
        return;
    }

    const bool raise = m_PaintLDown;
    const bool lower = m_PaintRDown;
    if (!raise && !lower)
    {
        m_heightStrokeActive = false;
        return;
    }

    // Snapshot once at the start of the stroke for undo.
    if (!m_heightStrokeActive)
    {
        TakeHeightUndoSnapshot();
        m_heightStrokeActive = true;
    }

    const float delta = raise ? m_heightStrength : -m_heightStrength;
    AddTerrainHeight(CollisionPosition[0], CollisionPosition[1], delta, m_heightBrush, BackTerrainHeight);
    // Rebuild lighting/normals so the change is visible immediately.
    CreateTerrainNormal();
    CreateTerrainLight();
}

void CMapEditorUI::TakeHeightUndoSnapshot()
{
    m_heightUndo.assign(BackTerrainHeight, BackTerrainHeight + CELLS);
    m_bHeightHasUndo = true;
}

void CMapEditorUI::UndoHeight()
{
    if (!m_bHeightHasUndo)
        return;
    std::memcpy(BackTerrainHeight, m_heightUndo.data(), CELLS * sizeof(float));
    CreateTerrainNormal();
    CreateTerrainLight();
    m_bHeightHasUndo = false;
    g_MuEditorConsoleUI.LogEditor("[MapEditor] Undo: reverted last height stroke");
}

void CMapEditorUI::TakeObjectUndoSnapshot()
{
    m_objUndo = Editor::ObjectPlace::SnapshotAll();
    m_bObjHasUndo = true;
}

void CMapEditorUI::UndoObjects()
{
    if (!m_bObjHasUndo)
        return;
    Editor::ObjectPlace::RestoreAll(m_objUndo);
    m_pSelected = nullptr;          // pointers are invalid after a full rebuild
    m_bObjHasUndo = false;          // single-level undo
    m_objStatus = "Undo: reverted last object edit.";
    g_MuEditorConsoleUI.LogEditor("[MapEditor] Undo objects");
}

void CMapEditorUI::PlaceObjects(int world)
{
    (void)world;
    if (!m_bObjEditEnabled || m_selectedModelType < 0)
    {
        m_objWasDown = false;
        return;
    }

    const bool down = m_PaintLDown;   // captured left button (game click suppressed)

    // Place one object on the press edge, only when the cursor is over terrain
    // (SelectFlag) and not over the editor UI.
    if (down && !m_objWasDown && SelectFlag && !g_MuEditorCore.IsHoveringUI())
    {
        TakeObjectUndoSnapshot();
        const bool ok = Editor::ObjectPlace::Place(
            m_selectedModelType, CollisionPosition[0], CollisionPosition[1],
            m_objYaw, m_objScale, m_bObjSnap);
        char s[64];
        snprintf(s, sizeof(s), ok ? "Placed object type %d." : "Place failed (type %d).",
                 m_selectedModelType);
        m_objStatus = s;
    }
    m_objWasDown = down;
}

void CMapEditorUI::HandleObjectSelect()
{
    if (!m_bObjEditEnabled)
    {
        m_objWasDown = false;
        m_objDragging = false;
        return;
    }

    const bool down = m_PaintLDown;

    // Press edge: pick the object under the cursor. Record where we grabbed it
    // relative to the object's origin so a plain click doesn't yank it, and a
    // drag keeps that same grip (no jump).
    if (down && !m_objWasDown && !g_MuEditorCore.IsHoveringUI())
    {
        OBJECT* hit = Editor::ObjectPlace::PickUnderCursor();
        if (hit != nullptr)
        {
            m_pSelected = hit;
            m_objDragging = true;
            m_grabOffsetX = hit->Position[0] - CollisionPosition[0];
            m_grabOffsetY = hit->Position[1] - CollisionPosition[1];
            // Snapshot at grab so Undo reverts the whole drag as one step.
            TakeObjectUndoSnapshot();
        }
    }

    // While held, move the selected object so the grabbed point stays under the
    // cursor. Height is left unchanged (only X/Y follow the ground cursor).
    if (down && m_objDragging && m_pSelected != nullptr && SelectFlag &&
        !g_MuEditorCore.IsHoveringUI())
    {
        const float nx = CollisionPosition[0] + m_grabOffsetX;
        const float ny = CollisionPosition[1] + m_grabOffsetY;
        m_pSelected = Editor::ObjectPlace::Reposition(m_pSelected, nx, ny, m_pSelected->Position[2]);
    }

    if (!down)
        m_objDragging = false;
    m_objWasDown = down;
}

void CMapEditorUI::TakeUndoSnapshot()
{
    m_UndoLayer1.assign(TerrainMappingLayer1, TerrainMappingLayer1 + CELLS);
    m_UndoLayer2.assign(TerrainMappingLayer2, TerrainMappingLayer2 + CELLS);
    m_UndoAlpha.assign(TerrainMappingAlpha, TerrainMappingAlpha + CELLS);
    m_bHasUndo = true;
}

void CMapEditorUI::Undo()
{
    if (!m_bHasUndo)
        return;
    std::memcpy(TerrainMappingLayer1, m_UndoLayer1.data(), CELLS);
    std::memcpy(TerrainMappingLayer2, m_UndoLayer2.data(), CELLS);
    std::memcpy(TerrainMappingAlpha, m_UndoAlpha.data(), CELLS * sizeof(float));
    m_bHasUndo = false;  // single-level undo
    g_MuEditorConsoleUI.LogEditor("[MapEditor] Undo: reverted last paint stroke");
}

#endif // _EDITOR
