#pragma once

#ifdef _EDITOR

#include "stdafx.h"

struct SDL_Window;
struct SDL_GPUCommandBuffer;
struct SDL_GPURenderPass;
union SDL_Event;

class CMuEditorCore
{
public:
    static CMuEditorCore& GetInstance();

    // window is the SDL window used by the ImGui SDL3 backend (issue #442).
    void Initialize(SDL_Window* window);
    // Sets the editor's listeners in game code: where the asset record takes
    // the active map from, and the effect browser's world preview removing
    // its objects before the game clears its pools. Initialize calls it.
    void ConnectGameHooks();
    void Shutdown();
    void Update();
    void Render();
    // Updates and draws the editor's own windows (outside the game window)
    // after the frame Render() ended went to the GPU. Does nothing when no
    // frame ended since the last call.
    void RenderPlatformWindows();
    void PrepareDrawData(SDL_GPUCommandBuffer* commandBuffer);
    void RenderDrawData(SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* renderPass);

    bool IsEnabled() const { return m_bEditorMode; }
    void SetEnabled(bool enabled) { m_bEditorMode = enabled; }
    void ToggleEditor() { m_bEditorMode = !m_bEditorMode; }

    bool IsShowingItemEditor() const { return m_bShowItemEditor; }
    bool IsShowingSkillEditor() const { return m_bShowSkillEditor; }
    bool IsShowingDevEditor() const { return m_bShowDevEditor; }
    bool IsShowingMapEditor() const { return m_bShowMapEditor; }
    bool IsShowingConsole() const { return m_bShowConsole; }
    bool IsHoveringUI() const { return m_bHoveringUI; }
    void SetHoveringUI(bool hovering) { m_bHoveringUI = hovering; }

    // Whether the editor takes an input event, so that RmlUi and the game do not
    // get it: everything of the editor's own windows; in the game window a press
    // or the wheel over an editor window and keys and text while it has the
    // keyboard. The release of a press it took is its too; other button
    // releases and all key releases go through.
    bool ClaimsEvent(const SDL_Event& event);

    // Whether the keyboard, or the mouse, is with one of the editor's own
    // windows instead of the game window: what is pressed there is not the
    // game's.
    bool IsKeyboardInEditorWindow() const;
    // As found at the start of the frame (Update).
    bool IsMouseInEditorWindow() const
    {
        return m_bMouseInEditorWindow;
    }
    // Whether a key or button (a virtual-key code) is with one of the
    // editor's own windows: the mouse's buttons by where the mouse is, the
    // other keys by where the keyboard is.
    bool IsWithEditorWindow(int virtualKey) const;

    // Global scale for ALL editor UI (fonts + widget/padding sizes). Driven by the
    // -/+ buttons in the editor window's menu bar. Applied between frames, not mid-frame.
    void  SetUIScale(float scale);
    float GetUIScale() const { return m_UIScale; }

private:
    CMuEditorCore();
    ~CMuEditorCore();

    void ApplyUIScale();
    void LogVideoDriver() const;
    void SelectWindowMode();
    void BlockGameInputOverOpenButton();
    void BlockGameInputOverEditor();
    void UpdateCursorVisibility();
    void ToggleFullscreenOnF11();
    bool IsEditorWindow(SDL_Window* window) const;
    bool FindMouseInEditorWindow() const;

    bool m_bEditorMode;
    bool m_bInitialized;
    bool m_bFrameStarted;
    bool m_bDrawDataReady;
    bool m_bShowItemEditor;
    bool m_bShowSkillEditor;
    bool m_bShowDevEditor;
    bool m_bShowMapEditor;
    bool m_bShowEffectBrowser;
    bool m_bShowConsole;
    bool m_bHoveringUI;
    bool m_bPreviousFrameHoveringUI;  // Store previous frame's hover state for input blocking

    float m_UIScale;        // 1.0 = default ImGui size
    bool  m_bScaleDirty;    // apply the new scale at the start of the next frame
    // The mouse buttons whose press the editor took (SDL_BUTTON_MASK bits), so
    // that their releases stay with it too.
    unsigned int m_claimedButtons = 0;
    // The window the game draws into; any other window of ours is the editor's.
    SDL_Window* m_pGameWindow = nullptr;
    // Render() ended a frame whose own windows are not updated and drawn yet.
    bool m_bPlatformWindowsPending = false;
    bool m_bMouseInEditorWindow = false;
    // The game draws the frame Render() ended. When it skips one (the GPU is
    // behind), the editor's windows keep their picture instead of one without
    // the game's offscreen views (the effect preview).
    bool m_bGameFrameActive = false;
    int m_skippedGameFrames = 0;
};

// Global accessor
#define g_MuEditorCore CMuEditorCore::GetInstance()

#endif // _EDITOR
