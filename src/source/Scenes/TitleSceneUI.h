//*****************************************************************************
// File: TitleSceneUI.h
//*****************************************************************************
#pragma once

// The title screen's loading-bar sprites -- extracted out of CUIMng (Phase 4,
// docs/newui-legacy-merger.md) since this has nothing to do with the CWin/CNewUIObj window-system
// merger that class otherwise exists for; it just happened to live there historically. Used only
// from WebzenScene.cpp (the title scene itself) and ZzzOpenData.cpp's OpenBasicData() (the
// asset-loading progress callbacks the title scene drives), so plain free functions rather than a
// singleton class -- there's no shared state anyone outside this pair needs to reach.
namespace TitleSceneUI
{
    void CreateSceneUI();
    void ReleaseSceneUI();
    void RenderSceneUI(HDC hDC, DWORD dwNow, DWORD dwTotal);
}
