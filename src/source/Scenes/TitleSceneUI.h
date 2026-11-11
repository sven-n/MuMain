//*****************************************************************************
// File: TitleSceneUI.h
//*****************************************************************************
#pragma once

// The title screen's loading-bar sprites -- extracted out of CUIMng since this has nothing to do
// with the CWin/CObject window-system
// merger that class otherwise exists for; it just happened to live there historically. Used only
// from WebzenScene.cpp (the title scene itself), so plain free functions rather than a singleton
// class -- there's no shared state anyone else needs to reach.
namespace TitleSceneUI
{
    void CreateSceneUI();
    void ReleaseSceneUI();
    // Draws the title and its progress into the frame that is open.
    void RenderSceneUI(HDC hDC, DWORD dwNow, DWORD dwTotal);
}
