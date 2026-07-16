#pragma once

#ifdef _EDITOR

#include <vector>

#include "MapObjectPlace.h"  // Editor::ObjectPlace::ModelEntry

class OBJECT;

// In-game Map Editor panel.
//
// Rather than reimplement terrain picking and painting, this panel drives the
// client's existing edit pipeline: setting the global EditFlag makes the terrain
// render loop ray-cast the mouse onto the ground each frame (SelectXF/SelectYF/
// CollisionPosition), and Editor::EditObjects() (called every frame in
// MainScene) performs the actual paint using the brush/selection globals this
// panel exposes. The panel adds a texture palette and an encrypting Save that
// writes the edit back to the map's real, reloadable file.
//
// Phase 1 covers the terrain texture (mapping) painter. Later phases (attribute,
// height, objects) add tabs to the same window.
class CMapEditorUI
{
public:
    static CMapEditorUI& GetInstance();

    // Called once per frame while the editor is open. Pass the show flag; pass
    // nullptr (or a false flag) to signal "not shown" so the panel can hand the
    // game back to normal (EDIT_NONE) mode.
    void Render(bool* p_open);

    // Called from the editor Update (before the game consumes input) to capture
    // the mouse-button state for the active tool and suppress the click from
    // reaching the game, so the character doesn't walk/attack while editing.
    void CaptureInputForPainting();

private:
    CMapEditorUI() = default;
    ~CMapEditorUI() = default;

    void RenderTextureTab();
    void RenderObjectsTab();
    void RenderHeightTab();
    void RenderMinimapTab();
    void RenderAttributeTab();
    // Paints TerrainWall[] with the selected attribute under the cursor.
    void PaintAttribute();
    void TakeAttrUndoSnapshot();
    void UndoAttr();
    // Raises/lowers terrain under the cursor while the Height tab is active.
    void SculptHeight();
    void TakeHeightUndoSnapshot();
    void UndoHeight();

    // Paints the terrain mapping arrays from the current brush/selection when the
    // mouse is over terrain and pressed. Runs each frame the Texture tab is
    // active; the legacy Editor::EditObjects() paint path is compiled out in this
    // build (it's gated on ENABLE_EDIT, not _EDITOR), so the editor owns it.
    void PaintMapping();

    // One-level undo: snapshot the mapping arrays at the start of a paint stroke,
    // and restore them on demand.
    void TakeUndoSnapshot();
    void Undo();

    // Places an object on left-click (Objects tab, Place mode).
    void PlaceObjects(int world);
    // Picks/drags the object under the cursor (Objects tab, Select mode).
    void HandleObjectSelect();
    // Draws the selected object's editable transform fields + delete.
    void RenderSelectedObjectPanel();
    // One-level undo for object edits (place/move/rotate/scale/delete).
    void TakeObjectUndoSnapshot();
    void UndoObjects();

    // Puts the game into the given edit mode (sets EditFlag). Idempotent.
    void EnterEditMode(int editFlag);
    // Returns the game to normal play (EDIT_NONE) if this panel had taken it.
    void RestoreGameMode();

    // Resolves which World{N}/EncTerrain{N} the live map maps to. -1 = auto from
    // the active world; a user override wins when >= 0.
    int ResolveWorldNumber() const;

    bool m_bEditActive = false;      // this panel currently owns EditFlag
    int  m_desiredEditFlag = 0;      // EDIT_* the active tab wants this frame (0 = EDIT_NONE)
    bool m_bPaintingEnabled = false; // when off, the panel stays open but the
                                     // game plays normally (walk without painting)
    int  m_TargetWorldOverride = -1; // -1 = auto (gMapManager.WorldActive + 1)
    float m_OverlayAlpha = 1.0f;     // blend strength applied to layer-2 overlay paint
    bool m_bDropperMode = false;     // when on (or Alt held), a click picks the tile

    // Mouse state captured before the game consumes it (see CaptureInputForPainting).
    bool m_PaintLDown = false;
    bool m_PaintRDown = false;
    bool m_StrokeActive = false;     // a left-drag stroke is in progress (undo edge)

    // One-level undo snapshot of the three mapping arrays.
    bool m_bHasUndo = false;
    std::vector<unsigned char> m_UndoLayer1;
    std::vector<unsigned char> m_UndoLayer2;
    std::vector<float>         m_UndoAlpha;

    // --- Objects tab state ---
    bool  m_bObjEditEnabled = false;   // master: take EDIT_OBJECT (else walk freely)
    int   m_objMode = 0;               // 0 = Place new, 1 = Select & edit
    int   m_selectedModelType = -1;    // CreateObject type to place
    float m_objYaw = 0.0f;             // placement yaw (degrees)
    float m_objScale = 1.0f;           // placement scale
    bool  m_bObjSnap = false;          // snap placement to tile centre
    bool  m_objWasDown = false;        // rising-edge guard so one click = one object
    bool  m_objDragging = false;       // a select-mode drag is in progress
    float m_grabOffsetX = 0.0f;        // object.Pos - cursor at grab (so clicks don't jump)
    float m_grabOffsetY = 0.0f;
    OBJECT* m_pSelected = nullptr;     // currently selected object (Select mode)
    std::string m_objStatus;           // last placement/save result
    int   m_modelsWorld = -1;          // world the cached model list is for
    std::vector<Editor::ObjectPlace::ModelEntry> m_models;

    // One-level object undo (snapshot of all objects before the last edit).
    bool m_bObjHasUndo = false;
    std::vector<Editor::ObjectPlace::SavedObject> m_objUndo;

    // --- Height tab state ---
    bool  m_bHeightEnabled = false;   // master: take EDIT_HEIGHT (else walk freely)
    int   m_heightBrush = 2;          // brush radius (tiles)
    float m_heightStrength = 8.0f;    // world units added per application
    bool  m_heightStrokeActive = false;
    bool  m_bHeightHasUndo = false;
    std::vector<float> m_heightUndo;  // BackTerrainHeight snapshot before a stroke

    // Minimap top-down mode: while active (and camera is FreeFly), force the whole
    // terrain to render for a full-map screenshot.
    bool m_bMinimapMode = false;
    bool m_topdownWasActive = false;  // edge-detect so we only own TopViewEnable while active
    std::string m_minimapStatus;      // result text for the generate button

    // --- Attribute (walkability) tab state ---
    bool m_bAttrEnabled = false;      // master: take EDIT_WALL (else walk freely)
    bool m_bAttrOverlay = true;       // tint tiles by their attribute
    int  m_attrBrushValue = 4;        // TW_* value the brush writes (0 = walkable)
    int  m_attrBrush = 1;             // brush radius (tiles)
    bool m_attrStrokeActive = false;
    bool m_bAttrHasUndo = false;
    std::vector<unsigned short> m_attrUndo;   // TerrainWall snapshot before a stroke
    int  m_serverMapOverride = -1;    // -1 = auto (gMapManager.WorldActive)
    std::string m_attrStatus;

    // Server export is a MERGE onto the server's current TerrainData, because the two
    // walk maps legitimately differ (the client blocks object footprints, the server
    // does not - 1084 tiles on Tarkan). We therefore need to know exactly which tiles
    // the user touched, and write only those. See MapAttributeSave.h.
    std::vector<BYTE>  m_attrBaseline;      // client attr bytes at session start (65536)
    std::vector<bool>  m_attrEdited;        // which tiles the user painted (65536)
    int  m_attrBaselineWorld = -1;          // world the baseline was taken on
    std::vector<BYTE>  m_serverBase;        // server's current TerrainData (65539)
    std::string m_serverBaseName;           // shown in the UI ("" = none loaded)

    // Snapshots the baseline / clears the edit set when the map changes.
    void EnsureAttrBaseline(int world);
};

#define g_MapEditorUI CMapEditorUI::GetInstance()

#endif // _EDITOR
