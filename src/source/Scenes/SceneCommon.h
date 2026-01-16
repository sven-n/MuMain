#pragma once

// SceneCommon.h - Shared utilities used by multiple scenes

#include <windows.h>

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
