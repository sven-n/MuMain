// NewUI3DRenderMng.cpp: implementation of the CNewUI3DRenderMng class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/Core/NewUI3DRenderMng.h"
#include "UI/Core/NewUIManager.h"
#include "Camera/CameraProjection.h"
#include "Render/Renderer/MuRenderer.h"

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
UI::Scaling::Transform TransformForOwner(INewUI3DRenderObj* object)
{
    CNewUIObj* owner = dynamic_cast<CNewUIObj*>(object);
    if (!owner)
        owner = object->GetLayoutOwner();
    if (!owner)
        return UI::Scaling::GetActiveTransform();
    return UI::Scaling::TransformForLayout(owner->GetLayoutMode(), WindowWidth, WindowHeight);
}

void RenderWithOwnerLayout(INewUI3DRenderObj* object)
{
    UI::Scaling::ScopedActiveTransform layout(TransformForOwner(object), true);
    object->Render3D();
}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

mu::ui::window::CNewUI3DCamera::CNewUI3DCamera()
{
}

mu::ui::window::CNewUI3DCamera::~CNewUI3DCamera()
{
    Release();
}

bool mu::ui::window::CNewUI3DCamera::Create(int iCameraIndex, UINT uiWidth, UINT uiHeight, float fZOrder)
{
    Release();

    m_iCameraIndex = iCameraIndex;
    m_uiWidth = uiWidth;
    m_uiHeight = uiHeight;
    m_fZOrder = fZOrder;

    return true;
}

void mu::ui::window::CNewUI3DCamera::Release()
{
    RemoveAll3DRenderObjs();
    m_deque2DEffects.clear();
}

void mu::ui::window::CNewUI3DCamera::UpdateDimensions(UINT uiWidth, UINT uiHeight)
{
    m_uiWidth = uiWidth;
    m_uiHeight = uiHeight;
}

bool mu::ui::window::CNewUI3DCamera::IsEmpty()
{
    return m_list3DObjs.empty();
}

void mu::ui::window::CNewUI3DCamera::Add3DRenderObj(INewUI3DRenderObj* pObj)
{
    if (std::find(m_list3DObjs.begin(), m_list3DObjs.end(), pObj) == m_list3DObjs.end())
    {
        m_list3DObjs.push_back(pObj);
    }
}

void mu::ui::window::CNewUI3DCamera::Remove3DRenderObj(INewUI3DRenderObj* pObj)
{
    auto vi = std::find(m_list3DObjs.begin(), m_list3DObjs.end(), pObj);
    if (vi != m_list3DObjs.end())
    {
        m_list3DObjs.erase(vi);
    }
}

void mu::ui::window::CNewUI3DCamera::RemoveAll3DRenderObjs()
{
    m_list3DObjs.clear();
}

void mu::ui::window::CNewUI3DCamera::RenderUI2DEffect(UI_2DEFFECT_CALLBACK pCallbackFunc, LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
    UI_2DEFFECT_INFO UI2DEffectInfo;
    UI2DEffectInfo.pCallbackFunc = pCallbackFunc;
    UI2DEffectInfo.pClass = pClass;
    UI2DEffectInfo.dwParamA = dwParamA;
    UI2DEffectInfo.dwParamB = dwParamB;
    UI2DEffectInfo.transform = UI::Scaling::GetActiveTransform();

    m_deque2DEffects.push_back(UI2DEffectInfo);
}

void mu::ui::window::CNewUI3DCamera::DeleteUI2DEffectObject(UI_2DEFFECT_CALLBACK pCallbackFunc)
{
    auto di = m_deque2DEffects.begin();
    for (; di != m_deque2DEffects.end(); di++)
    {
        if ((*di).pCallbackFunc == pCallbackFunc)
        {
            m_deque2DEffects.erase(di);
            break;
        }
    }
}

int mu::ui::window::CNewUI3DCamera::GetCameraIndex() const
{
    return m_iCameraIndex;
}

float mu::ui::window::CNewUI3DCamera::GetLayerDepth()
{
    //. fZOrder == fLayerDepth
    return m_fZOrder;
}

// DXP-07d increment 2's shadow-compare diagnostic (proj/view vs. CPU closed form) validated this
// camera's projection formula across multiple soaks; DXP-08a deleted the diagnostic and the FFP
// matrix-stack calls it was validating once GlobalUBO was confirmed the only consumer
// — see Render()'s own comments below. Pre-implementation read of every
// INewUI3DRenderObj::Render3D() implementer registered with this camera (12 call sites across
// NewUIMyInventory, NewUIInventoryCtrl, NewUIEmpireGuardianNPC, NewUINPCQuest,
// NewUIDoppelGangerWindow, NewUICustomMessageBox, NewUICommonMessageBox, and 4 GameShop MsgBoxIGS*
// dialogs) found none touch the GL matrix stack — they all just call RenderItem3D(), which carries no
// GL model transform. (NewUIGoldBowmanLena/NewUIRegistrationLuckyCoin's own Render3D() methods are
// NOT reached through this camera — they call EndBitmap()/gluPerspective2 directly themselves,
// independent of CNewUI3DCamera.)

void mu::ui::window::CNewUI3DCamera::Render3D()
{
    for (auto* object : m_list3DObjs)
    {
        if (object->IsVisible())
        {
            RenderWithOwnerLayout(object);
        }
    }
}

bool mu::ui::window::CNewUI3DCamera::Render()
{
    if (m_list3DObjs.empty())
        return true;

    EndBitmap();
    mu::GetRenderer().SetMatrixMode(GL_PROJECTION);
    mu::GetRenderer().PushMatrix();
    mu::GetRenderer().LoadIdentity();
    SetRenderViewport(0, 0, m_uiWidth, m_uiHeight);
    gluPerspective2(1.f, (float)(m_uiWidth) / (float)(m_uiHeight), RENDER_ITEMVIEW_NEAR, RENDER_ITEMVIEW_FAR);
    mu::GetRenderer().SetMatrixMode(GL_MODELVIEW);
    mu::GetRenderer().PushMatrix();
    mu::GetRenderer().LoadIdentity();
    CameraProjection::GetOpenGLMatrix(g_Camera.Matrix);
    EnableDepthTest();
    EnableDepthMask();
    mu::GetRenderer().ClearDepthBuffer();

    Render3D();
    UpdateMousePositionn();

    mu::GetRenderer().SetMatrixMode(GL_MODELVIEW);
    mu::GetRenderer().PopMatrix();
    mu::GetRenderer().SetMatrixMode(GL_PROJECTION);
    mu::GetRenderer().PopMatrix();
    BeginBitmap();

    while (!m_deque2DEffects.empty())
    {
        UI_2DEFFECT_INFO& UI2DEffectInfo = m_deque2DEffects.front();
        if (UI2DEffectInfo.pCallbackFunc)
        {
            UI::Scaling::ScopedActiveTransform layout(UI2DEffectInfo.transform);
            (*UI2DEffectInfo.pCallbackFunc)(UI2DEffectInfo.pClass, UI2DEffectInfo.dwParamA, UI2DEffectInfo.dwParamB);
        }
        m_deque2DEffects.pop_front();
    }

    return true;
}

bool mu::ui::window::CNewUI3DCamera::Update()
{
    //. DOING NOTHING
    return true;
}

bool mu::ui::window::CNewUI3DCamera::UpdateMouseEvent()
{
    //. DOING NOTHING
    return true;
}

bool mu::ui::window::CNewUI3DCamera::UpdateKeyEvent()
{
    //. DOING NOTHING
    return true;
}

mu::ui::window::CNewUI3DRenderMng::CNewUI3DRenderMng()
{
}

mu::ui::window::CNewUI3DRenderMng::~CNewUI3DRenderMng()
{
    Release();
}

bool mu::ui::window::CNewUI3DRenderMng::Create(CNewUIManager* pNewUIMng)
{
    m_pNewUIMng = pNewUIMng;
    return true;
}

void mu::ui::window::CNewUI3DRenderMng::Release()
{
    RemoveAll3DRenderObjs();
}

void mu::ui::window::CNewUI3DRenderMng::UpdateAllCameraDimensions(UINT uiWidth, UINT uiHeight)
{
    for (auto it = m_listCamera.begin(); it != m_listCamera.end(); ++it)
    {
        if (*it)
        {
            (*it)->UpdateDimensions(uiWidth, uiHeight);
        }
    }
}

void mu::ui::window::CNewUI3DRenderMng::Add3DRenderObj(INewUI3DRenderObj* pObj, float fZOrder/* = INFORMATION_CAMERA_Z_ORDER*/)
{
    CNewUI3DCamera* pCamera = FindCamera(fZOrder);
    if (NULL == pCamera)
    {
        int iAvailableCameraIndex = FindAvailableCameraIndex();
        if (-1 != iAvailableCameraIndex)
        {
            pCamera = new CNewUI3DCamera;
            pCamera->Create(iAvailableCameraIndex, WindowWidth, WindowHeight, fZOrder);
            pCamera->Add3DRenderObj(pObj);
            m_pNewUIMng->AddUIObj(iAvailableCameraIndex, pCamera);
            m_listCamera.push_back(pCamera);
        }
        else
        {
#ifdef _DEBUG
            MU_DEBUG_BREAK();
#endif // _DEBUG
        }
    }
    else
    {
        pCamera->Add3DRenderObj(pObj);
    }
}
void mu::ui::window::CNewUI3DRenderMng::Remove3DRenderObj(INewUI3DRenderObj* pObj)
{
    auto li = m_listCamera.begin();
    for (; li != m_listCamera.end(); li++)
    {
        (*li)->Remove3DRenderObj(pObj);
        if ((*li)->IsEmpty())
        {
            m_pNewUIMng->RemoveUIObj(*li);
            delete (*li);
            m_listCamera.erase(li);
            break;
        }
    }
}

void mu::ui::window::CNewUI3DRenderMng::RemoveAll3DRenderObjs()
{
    auto li = m_listCamera.begin();
    for (; li != m_listCamera.end(); li++)
    {
        delete (*li);
        m_pNewUIMng->RemoveUIObj(*li);
    }
    m_listCamera.clear();
}

void mu::ui::window::CNewUI3DRenderMng::RenderUI2DEffect(float fZOrder, UI_2DEFFECT_CALLBACK pCallbackFunc, LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
    CNewUI3DCamera* pCamera = FindCamera(fZOrder);
    if (pCamera)
        pCamera->RenderUI2DEffect(pCallbackFunc, pClass, dwParamA, dwParamB);
}

void mu::ui::window::CNewUI3DRenderMng::DeleteUI2DEffectObject(UI_2DEFFECT_CALLBACK pCallbackFunc)
{
    auto li = m_listCamera.begin();
    for (; li != m_listCamera.end(); li++)
        (*li)->DeleteUI2DEffectObject(pCallbackFunc);
}

CNewUI3DCamera* mu::ui::window::CNewUI3DRenderMng::FindCamera(float fZOrder)
{
    auto li = m_listCamera.begin();
    for (; li != m_listCamera.end(); li++)
        if ((*li)->GetLayerDepth() == fZOrder)
            return (*li);
    return NULL;
}

int mu::ui::window::CNewUI3DRenderMng::FindAvailableCameraIndex()
{
    for (int iIndex = INTERFACE_3DRENDERING_CAMERA_BEGIN; iIndex < INTERFACE_3DRENDERING_CAMERA_END; iIndex++)
    {
        auto li = m_listCamera.begin();
        for (; li != m_listCamera.end(); li++)
        {
            if ((*li)->GetCameraIndex() == iIndex)
                break;
        }
        if (li == m_listCamera.end())
            return iIndex;
    }
    return -1;
}
