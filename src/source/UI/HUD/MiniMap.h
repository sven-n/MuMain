#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/HUD/MainFrameWindow.h"
#include "UI/HUD/ChatLogWindow.h"
#include "UI/Inventory/MyInventory.h"
#include "UI/Widgets/Window/Button.h"
#include "UI/HUD/MiniMapLayout.h"
#include "UI/HUD/MiniMapRmlModel.h"
#include "UI/RmlBridge/RmlModelBinder.h"
#include "UI/Widgets/Window/Tooltip.h"

#include <string>

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The full-screen mini map. mini_map.rml draws it; C++ keeps the world's marker data, the
// hero's position, the turned quads of the map and its markers (MiniMapLayout.h), the marker
// name hint, Tab/Escape and the close request.
class CMiniMap : public CObject
{
public:
    bool m_bSuccess;

    CMiniMap();
    virtual ~CMiniMap();

    bool Create(CManager* pNewUIMng, int x, int y);
    void Release();

    void SetPos(int x, int y);
    // Kept for RenderPointRotate() (ZzzOpenglUtil.cpp), which no longer has a caller.
    void SetBtnPos(int Num, float x, float y, float nx, float ny);

    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    bool Update();
    bool Render();

    float GetLayerDepth(); //. 8.1f

    void OpenningProcess();
    void ClosingProcess();
    void LoadImages(const wchar_t* Filename);
    void UnloadImages();

    void ReloadRmlTheme();

private:
    void BuildRmlUi();
    void SyncRmlModel();
    void SyncScreen();
    void SyncMap();
    void SyncHint();
    void SyncClips();

    CManager* m_pNewUIMng;
    MINI_MAP m_Mini_Map_Data[MAX_MINI_MAP_DATA];
    float m_Btn_Loc[MAX_MINI_MAP_DATA][4];

    // The world folder (e.g. L"World1") whose mini_map texture the document shows.
    std::wstring m_WorldName;
    // m_BtnExit's hint ("Close", above the button).
    CTooltip m_ExitTooltip;

    RmlModelBinder<MiniMapRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;
    bool m_PendingClose = false;
    // The screen the border tiles' matrices were built for (SyncScreen()).
    UI::MiniMap::Screen m_SideLinesScreen;
};
}
