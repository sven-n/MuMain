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
