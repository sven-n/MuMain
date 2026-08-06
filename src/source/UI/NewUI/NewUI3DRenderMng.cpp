// NewUI3DRenderMng.cpp: implementation of the CNewUI3DRenderMng class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/NewUI/NewUI3DRenderMng.h"
#include "UI/NewUI/NewUIManager.h"
#include "Camera/CameraProjection.h"
#include "Render/Core/RenderConfig.h"
#include "Render/Core/GlobalUBO.h"
#include "Core/Utilities/Log/ErrorReport.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUI3DCamera::CNewUI3DCamera()
{
}

SEASON3B::CNewUI3DCamera::~CNewUI3DCamera()
{
    Release();
}

bool SEASON3B::CNewUI3DCamera::Create(int iCameraIndex, UINT uiWidth, UINT uiHeight, float fZOrder)
{
    Release();

    m_iCameraIndex = iCameraIndex;
    m_uiWidth = uiWidth;
    m_uiHeight = uiHeight;
    m_fZOrder = fZOrder;

    return true;
}

void SEASON3B::CNewUI3DCamera::Release()
{
    RemoveAll3DRenderObjs();
    m_deque2DEffects.clear();
}

void SEASON3B::CNewUI3DCamera::UpdateDimensions(UINT uiWidth, UINT uiHeight)
{
    m_uiWidth = uiWidth;
    m_uiHeight = uiHeight;
}

bool SEASON3B::CNewUI3DCamera::IsEmpty()
{
    return m_list3DObjs.empty();
}

void SEASON3B::CNewUI3DCamera::Add3DRenderObj(INewUI3DRenderObj* pObj)
{
    if (std::find(m_list3DObjs.begin(), m_list3DObjs.end(), pObj) == m_list3DObjs.end())
    {
        m_list3DObjs.push_back(pObj);
    }
}

void SEASON3B::CNewUI3DCamera::Remove3DRenderObj(INewUI3DRenderObj* pObj)
{
    auto vi = std::find(m_list3DObjs.begin(), m_list3DObjs.end(), pObj);
    if (vi != m_list3DObjs.end())
    {
        m_list3DObjs.erase(vi);
    }
}

void SEASON3B::CNewUI3DCamera::RemoveAll3DRenderObjs()
{
    m_list3DObjs.clear();
}

void SEASON3B::CNewUI3DCamera::RenderUI2DEffect(UI_2DEFFECT_CALLBACK pCallbackFunc, LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
    UI_2DEFFECT_INFO UI2DEffectInfo;
    UI2DEffectInfo.pCallbackFunc = pCallbackFunc;
    UI2DEffectInfo.pClass = pClass;
    UI2DEffectInfo.dwParamA = dwParamA;
    UI2DEffectInfo.dwParamB = dwParamB;

    m_deque2DEffects.push_back(UI2DEffectInfo);
}

void SEASON3B::CNewUI3DCamera::DeleteUI2DEffectObject(UI_2DEFFECT_CALLBACK pCallbackFunc)
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

int SEASON3B::CNewUI3DCamera::GetCameraIndex() const
{
    return m_iCameraIndex;
}

float SEASON3B::CNewUI3DCamera::GetLayerDepth()
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

bool SEASON3B::CNewUI3DCamera::Render()
{
    if (m_list3DObjs.empty())
        return true;

    EndBitmap();
    SaveCameraPerspective();
    glViewport2(0, 0, m_uiWidth, m_uiHeight);
    gluPerspective2(1.f, (float)(m_uiWidth) / (float)(m_uiHeight), RENDER_ITEMVIEW_NEAR, RENDER_ITEMVIEW_FAR);
    // DXP-08a: the matching glMatrixMode/glPushMatrix/glLoadIdentity bracket and
    // CameraProjection::GetOpenGLMatrix(g_Camera.Matrix) read, plus the shadow-compare diagnostic
    // that used to validate this closed form, are deleted — DXP-07d already proved this exact
    // formula matches bit-for-bit across multiple soaks, and GlobalUBO is the only consumer
    // (same finding Category 3 already used to delete BeginOpengl/BeginBitmap's own
    // matrix-stack calls). Same closed-form shape as increment 1 (RenderInfomation3D) — plain
    // gluPerspective(1.f deg, aspect, near, far) and identity view, just with this camera's own
    // m_uiWidth/m_uiHeight and RENDER_ITEMVIEW_NEAR/FAR args.
    {
        float aspect = (float)m_uiWidth / (float)m_uiHeight;
        float fovRad = 1.f * 0.5f * Q_PI / 180.0f;
        float f = 1.0f / tanf(fovRad);
        float zNear = RENDER_ITEMVIEW_NEAR;
        float zFar  = RENDER_ITEMVIEW_FAR;
        float cpuProj[16];
        BuildPerspectiveProjection(f, aspect, zNear, zFar, cpuProj);
        float cpuView[16] = {
            1.f,0.f,0.f,0.f,  0.f,1.f,0.f,0.f,  0.f,0.f,1.f,0.f,  0.f,0.f,0.f,1.f
        };

        GlobalUBO::Instance().SetProj(cpuProj);
        GlobalUBO::Instance().SetView(cpuView);

        // g_Camera.Matrix (3x4 row-major, same layout CameraProjection::GetOpenGLMatrix() used to
        // produce from a GL_MODELVIEW_MATRIX read) is identity here, matching cpuView above.
        static const float s_IdentityCameraMatrix[3][4] = {
            {1.f,0.f,0.f,0.f}, {0.f,1.f,0.f,0.f}, {0.f,0.f,1.f,0.f}
        };
        memcpy(g_Camera.Matrix, s_IdentityCameraMatrix, sizeof(g_Camera.Matrix));
    }
    EnableDepthTest();
    EnableDepthMask();
    ClearDepthBuffer();

    auto li = m_list3DObjs.begin();
    for (; li != m_list3DObjs.end(); li++)
    {
        if ((*li)->IsVisible())
        {
            (*li)->Render3D();
        }
    }
    UpdateMousePositionn();

    // DXP-08a: the matching glMatrixMode/glPopMatrix pops and the post-restore shadow-compare are
    // deleted — BeginBitmap() (already fully CPU-sourced per DXP-07a) is GlobalUBO's real restore;
    // the deleted block only re-set GlobalUBO to its own just-restored value, a no-op.
    BeginBitmap();
    RestoreCameraPerspective();

    while (!m_deque2DEffects.empty())
    {
        UI_2DEFFECT_INFO& UI2DEffectInfo = m_deque2DEffects.front();
        if (UI2DEffectInfo.pCallbackFunc)
        {
            (*UI2DEffectInfo.pCallbackFunc)(UI2DEffectInfo.pClass, UI2DEffectInfo.dwParamA, UI2DEffectInfo.dwParamB);
        }
        m_deque2DEffects.pop_front();
    }

    return true;
}

bool SEASON3B::CNewUI3DCamera::Update()
{
    //. DOING NOTHING
    return true;
}

bool SEASON3B::CNewUI3DCamera::UpdateMouseEvent()
{
    //. DOING NOTHING
    return true;
}

bool SEASON3B::CNewUI3DCamera::UpdateKeyEvent()
{
    //. DOING NOTHING
    return true;
}

SEASON3B::CNewUI3DRenderMng::CNewUI3DRenderMng()
{
}

SEASON3B::CNewUI3DRenderMng::~CNewUI3DRenderMng()
{
    Release();
}

bool SEASON3B::CNewUI3DRenderMng::Create(CNewUIManager* pNewUIMng)
{
    m_pNewUIMng = pNewUIMng;
    return true;
}

void SEASON3B::CNewUI3DRenderMng::Release()
{
    RemoveAll3DRenderObjs();
}

void SEASON3B::CNewUI3DRenderMng::UpdateAllCameraDimensions(UINT uiWidth, UINT uiHeight)
{
    for (auto it = m_listCamera.begin(); it != m_listCamera.end(); ++it)
    {
        if (*it)
        {
            (*it)->UpdateDimensions(uiWidth, uiHeight);
        }
    }
}

void SEASON3B::CNewUI3DRenderMng::Add3DRenderObj(INewUI3DRenderObj* pObj, float fZOrder/* = INFORMATION_CAMERA_Z_ORDER*/)
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
void SEASON3B::CNewUI3DRenderMng::Remove3DRenderObj(INewUI3DRenderObj* pObj)
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

void SEASON3B::CNewUI3DRenderMng::RemoveAll3DRenderObjs()
{
    auto li = m_listCamera.begin();
    for (; li != m_listCamera.end(); li++)
    {
        delete (*li);
        m_pNewUIMng->RemoveUIObj(*li);
    }
    m_listCamera.clear();
}

void SEASON3B::CNewUI3DRenderMng::RenderUI2DEffect(float fZOrder, UI_2DEFFECT_CALLBACK pCallbackFunc, LPVOID pClass, DWORD dwParamA, DWORD dwParamB)
{
    CNewUI3DCamera* pCamera = FindCamera(fZOrder);
    if (pCamera)
        pCamera->RenderUI2DEffect(pCallbackFunc, pClass, dwParamA, dwParamB);
}

void SEASON3B::CNewUI3DRenderMng::DeleteUI2DEffectObject(UI_2DEFFECT_CALLBACK pCallbackFunc)
{
    auto li = m_listCamera.begin();
    for (; li != m_listCamera.end(); li++)
        (*li)->DeleteUI2DEffectObject(pCallbackFunc);
}

CNewUI3DCamera* SEASON3B::CNewUI3DRenderMng::FindCamera(float fZOrder)
{
    auto li = m_listCamera.begin();
    for (; li != m_listCamera.end(); li++)
        if ((*li)->GetLayerDepth() == fZOrder)
            return (*li);
    return NULL;
}

int SEASON3B::CNewUI3DRenderMng::FindAvailableCameraIndex()
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