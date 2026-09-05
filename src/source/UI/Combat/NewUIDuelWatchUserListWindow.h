// NewUIDuelWatchUserListWindow.h: interface for the CDuelWatchUserListWindow class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Core/NewUIManager.h"

namespace mu::ui::window
{
    class CDuelWatchUserListWindow : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_DUELWATCH_USERLIST_BOX = BITMAP_BUFFWATCH_USERLIST_BEGIN,
        };
    private:
        CManager* m_pNewUIMng;
        POINT						m_Pos;

    public:
        CDuelWatchUserListWindow();
        virtual ~CDuelWatchUserListWindow();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        bool BtnProcess();

        float GetLayerDepth();	//. 5.4f

        void OpeningProcess();
        void ClosingProcess();
    private:
        void LoadImages();
        void UnloadImages();

        void RenderFrame();
    };
}
