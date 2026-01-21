#pragma once

// SceneCommon.h - Shared utilities used by multiple scenes

#include <windows.h>
#include "../ZzzInfomation.h"  // For MAX_CHARACTERS_PER_ACCOUNT

//=============================================================================
// Character Selection State
//=============================================================================

class CharacterSelectionState {
private:
    int selectedIndex = -1;

public:
    static constexpr int NO_SELECTION = -1;

    // Check if a character is currently selected
    bool HasSelection() const {
        return selectedIndex >= 0 && selectedIndex < MAX_CHARACTERS_PER_ACCOUNT;
    }

    // Select a character by index (validates bounds)
    void SelectCharacter(int index) {
        if (index >= 0 && index < MAX_CHARACTERS_PER_ACCOUNT) {
            selectedIndex = index;
        }
    }

    // Clear the current selection
    void ClearSelection() {
        selectedIndex = NO_SELECTION;
    }

    // Get the selected character index (-1 if none)
    int GetSelectedIndex() const {
        return selectedIndex;
    }

    // Legacy accessor for backward compatibility (allows direct modification)
    int& GetLegacyReference() {
        return selectedIndex;
    }
};

// Global character selection state
extern CharacterSelectionState g_characterSelection;

// Legacy global (deprecated - use g_characterSelection instead)
// This reference points to g_characterSelection's internal state for backward compatibility
extern int& SelectedHero;

//=============================================================================
// Scene Initialization State
//=============================================================================

class SceneInitializationState {
private:
    bool initLogIn = false;
    bool initLoading = false;
    bool initCharacterScene = false;
    bool initMainScene = false;
    bool enableMainRender = false;

public:
    // Reset all initialization flags
    void ResetAll() {
        initLogIn = false;
        initLoading = false;
        initCharacterScene = false;
        initMainScene = false;
        enableMainRender = false;
    }

    // Reset flags when disconnecting from server
    void ResetForDisconnect() {
        initLogIn = false;
        initCharacterScene = false;
        initMainScene = false;
        enableMainRender = false;
        // Note: initLoading is not reset on disconnect
    }

    // Legacy accessors for backward compatibility
    bool& GetInitLogIn() { return initLogIn; }
    bool& GetInitLoading() { return initLoading; }
    bool& GetInitCharacterScene() { return initCharacterScene; }
    bool& GetInitMainScene() { return initMainScene; }
    bool& GetEnableMainRender() { return enableMainRender; }
};

// Global scene initialization state
extern SceneInitializationState g_sceneInit;

// Legacy globals (deprecated - use g_sceneInit instead)
extern bool& InitLogIn;
extern bool& InitLoading;
extern bool& InitCharacterScene;
extern bool& InitMainScene;
extern bool& EnableMainRender;

//=============================================================================
// Utility functions
//=============================================================================

// Utility functions
bool CheckAbuseFilter(wchar_t* Text, bool bCheckSlash);
bool CheckAbuseNameFilter(wchar_t* Text);
bool CheckName();
BOOL CheckOptionMouseClick(int iOptionPos_y, BOOL bPlayClickSound = TRUE);
int SeparateTextIntoLines(const wchar_t* lpszText, wchar_t* lpszSeparated, int iMaxLine, int iLineSize);
void SetEffectVolumeLevel(int level);
void SetViewPortLevel(int level);

// Rendering utilities
void RenderInfomation3D();
void RenderInfomation();
BOOL ShowCheckBox(int num, int index, int message);

// Time utilities
bool GetTimeCheck(int DelayTime);
bool IsEnterPressed();
void SetEnterPressed(bool enterpressed);
