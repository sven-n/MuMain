// NewUI3DRenderMng.h: interface for the CNewUI3DRenderMng class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUI3DRENDERMNG_H__BD260911_DD18_4B6D_8BA0_C6A062043C7A__INCLUDED_)
#define AFX_NEWUI3DRENDERMNG_H__BD260911_DD18_4B6D_8BA0_C6A062043C7A__INCLUDED_

#pragma once

#include "NewUIBase.h"

#pragma warning(disable : 4786)
#include <list>
#include <deque>

#define INVENTORY_CAMERA_Z_ORDER		5.5f
#define INFORMATION_CAMERA_Z_ORDER		10.9f
#define ITEMHOTKEYNUMBER_CAMERA_Z_ORDER 11.f
#define TOOLTIP_CAMERA_Z_ORDER			5.6f

namespace SEASON3B
{
    typedef void (*UI_2DEFFECT_CALLBACK)(LPVOID pClass, DWORD dwParamA, DWORD dwParamB);

#pragma pack(push, 1)
    typedef struct tagUI_2DEFFECT_INFO
    {
        UI_2DEFFECT_CALLBACK	pCallbackFunc;
        LPVOID					pClass;
        DWORD					dwParamA, dwParamB;
    } UI_2DEFFECT_INFO;
#pragma pack(pop)

    class CNewUIManager;

    class INewUI3DRenderObj
    {
    public:
        virtual void Render3D() = 0;
        virtual bool IsVisible() const = 0;
    };

    class CNewUI3DCamera : public CNewUIObj
    {
        typedef std::list<INewUI3DRenderObj*> type_list_3dobj;
        typedef std::deque<UI_2DEFFECT_INFO> type_deque_2deffect;

        type_list_3dobj	m_list3DObjs;
        type_deque_2deffect m_deque2DEffects;

        UINT	m_uiWidth, m_uiHeight;
        float	m_fZOrder;
        int		m_iCameraIndex;

    public:
        CNewUI3DCamera();
        virtual ~CNewUI3DCamera();

        bool Create(int iCameraIndex, UINT uiWidth, UINT uiHeight, float fZOrder);
        void Release();

        bool IsEmpty();

        void Add3DRenderObj(INewUI3DRenderObj* pObj);
        void Remove3DRenderObj(INewUI3DRenderObj* pObj);
        void RemoveAll3DRenderObjs();

        void RenderUI2DEffect(UI_2DEFFECT_CALLBACK pCallbackFunc, LPVOID pClass, DWORD dwParamA, DWORD dwParamB);
        void DeleteUI2DEffectObject(UI_2DEFFECT_CALLBACK pCallbackFunc);

        float GetLayerDepth();			//. fZOrder == fLayerDepth
        int GetCameraIndex() const;

        bool Render();
        bool Update();					//. DOING NOTHING
        bool UpdateMouseEvent();		//. DOING NOTHING
        bool UpdateKeyEvent();			//. DOING NOTHING

    protected:
        void Render3D();
    };

    class CNewUI3DRenderMng
    {
        typedef std::list<CNewUI3DCamera*>	type_list_camera;
        type_list_camera	m_listCamera;

        CNewUIManager* m_pNewUIMng;

    public:
        CNewUI3DRenderMng();
        virtual ~CNewUI3DRenderMng();

        bool Create(CNewUIManager* pNewUIMng);
        void Release();

        void Add3DRenderObj(INewUI3DRenderObj* pObj, float fZOrder = INFORMATION_CAMERA_Z_ORDER);
        void Remove3DRenderObj(INewUI3DRenderObj* pObj);
        void RemoveAll3DRenderObjs();

        void RenderUI2DEffect(float fZOrder, UI_2DEFFECT_CALLBACK pCallbackFunc, LPVOID pClass, DWORD dwParamA, DWORD dwParamB);
        void DeleteUI2DEffectObject(UI_2DEFFECT_CALLBACK pCallbackFunc);

    protected:
        CNewUI3DCamera* FindCamera(float fZOrder);
        int FindAvailableCameraIndex();
    };
}

#endif // !defined(AFX_NEWUI3DRENDERMNG_H__BD260911_DD18_4B6D_8BA0_C6A062043C7A__INCLUDED_)
