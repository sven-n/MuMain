// NewUIDuelWatchUserListWindow.h: interface for the CNewUIDuelWatchUserListWindow class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NewUIBase.h"
#include "NewUIManager.h"

namespace SEASON3B
{
    class CNewUIDuelWatchUserListWindow : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_DUELWATCH_USERLIST_BOX = BITMAP_BUFFWATCH_USERLIST_BEGIN,
        };
    private:
        CNewUIManager* m_pNewUIMng;
        POINT						m_Pos;

    public:
        CNewUIDuelWatchUserListWindow();
        virtual ~CNewUIDuelWatchUserListWindow();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
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
