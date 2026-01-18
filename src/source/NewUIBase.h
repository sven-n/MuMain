#ifndef _NEWUIBASE_H_
#define _NEWUIBASE_H_

#pragma once

 
namespace SEASON3B
{
    class INewUIBase
    {
    public:
        virtual bool Render() = 0;
        virtual bool Update() = 0;
        virtual bool UpdateMouseEvent() = 0;
        virtual bool UpdateKeyEvent() = 0;

        virtual float GetLayerDepth() = 0;
        virtual float GetKeyEventOrder() = 0;

        virtual bool IsVisible() const = 0;
        virtual bool IsEnabled() const = 0;
    };

    class CNewUIObj : public INewUIBase
    {
        HWND m_hRelatedWnd;
        bool m_bRender, m_bUpdate;
    public:
        CNewUIObj() : m_hRelatedWnd(nullptr), m_bRender(true), m_bUpdate(true) {}
        virtual ~CNewUIObj() {}

        void SetRelatedWnd(HWND hWnd = g_hWnd)
        {
            m_hRelatedWnd = hWnd;
        }
        HWND GetRelatedWnd() const { return m_hRelatedWnd; }

        void Show(bool bShow)
        {
            m_bRender = bShow;
        }
        void Enable(bool bEnable)
        {
            m_bUpdate = bEnable;
        }

        bool IsVisible() const override { return m_bRender; }
        bool IsEnabled() const override { return m_bUpdate; }

        virtual float GetKeyEventOrder() { return 3.0f; }		//. Default
    };
}

#endif // _NEWUIBASE_H_