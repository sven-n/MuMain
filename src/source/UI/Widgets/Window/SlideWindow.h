#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/HUD/SlideTicker.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#include <RmlUi/Core/Types.h>

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
    class CManager;

    class CSlideWindow : public CObject
    {
        CManager* m_pNewUIMng;
    public:
        CSlideWindow();
        virtual ~CSlideWindow();

        bool Create(CManager* pNewUIMng);
        void Release();

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();		// 1.91f

        // wrapping
        void Init() { m_pSlideMgr->Init(); }
        void CreateSlideText() { m_pSlideMgr->CreateSlideText(); }
        void AddSlide(int iLoopCount, int iLoopDelay, const wchar_t* strText, int iType, float fSpeed, DWORD dwTextColor = (255 << 24) + (200 << 16) + (220 << 8) + (230))
        {
            m_pSlideMgr->AddSlide(iLoopCount, iLoopDelay, strText, iType, fSpeed, dwTextColor);
        }

    private:
        struct SlideNoticeRmlModel
        {
            float rootScale = 1.f;
            float textPx = 0.f;
            bool shown = false;
            float textX = 0.f;
            float textTop = 0.f;
            float bandTop = 0.f;
            float bandHeight = 0.f;
            float alpha = 0.f;
            Rml::String textColor;
            Rml::String text;
        };
        RmlModelBinder<SlideNoticeRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

        void BuildRmlUi();
        void DestroyRmlUi();
        void SyncRmlModel();
        void ReloadRmlTheme();

        UI::HUD::SlideTicker* m_pSlideMgr;
    };
}

