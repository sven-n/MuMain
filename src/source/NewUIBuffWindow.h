// NewUIBuffWindow.h: interface for the CNewUIBuffWindow class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NewUIManager.h"

namespace SEASON3B
{
    class CNewUIBuffWindow : public CNewUIObj
    {
    public:
        enum IMAGE_LIST
        {
            IMAGE_BUFF_STATUS = BITMAP_BUFFWINDOW_BEGIN,
            IMAGE_BUFF_STATUS2,
            IMAGE_BUFF_STATUS3,
        };

        enum BUFF_RENDER
        {
            BUFF_RENDER_ICON = 0,
            BUFF_RENDER_TOOLTIP
        };

    public:
        CNewUIBuffWindow();
        virtual ~CNewUIBuffWindow();

        bool Create(CNewUIManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);
        void SetPos(int iScreenWidth);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 5.3f

        void OpenningProcess();
        void ClosingProcess();

    private:
        void LoadImages();
        void UnloadImages();

        void BuffSort(std::list<eBuffState>& buffstate);
        void RenderBuffStatus(BUFF_RENDER renderstate);
        void RenderBuffIcon(eBuffState& eBuffType, float x, float y, float width, float height);
        void RenderBuffTooltip(eBuffClass& eBuffClassType, eBuffState& eBuffType, float x, float y);
        bool SetDisableRenderBuff(const eBuffState& _BuffState);

        CNewUIManager* m_pNewUIMng;
        POINT m_Pos;
    };
}
