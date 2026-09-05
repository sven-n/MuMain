// NewUIMessageBoxWindow.cpp: implementation of the CNewUIMessageBoxWindow class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/Dialogs/NewUIMessageBox.h"	// self
#include "UI/Core/NewUIManager.h"
#include "UI/Widgets/UIControls.h"
#include "UI/Scaling/UITransform.h"
#include "Render/Textures/ZzzOpenglUtil.h"

using namespace SEASON3B;
using namespace mu::ui::window;

//////////////////////////////////////////////////////////////////////
// CNewUIMessageBoxBase
//////////////////////////////////////////////////////////////////////

mu::ui::window::CNewUIMessageBoxBase::CNewUIMessageBoxBase()
{
    Release();
}

mu::ui::window::CNewUIMessageBoxBase::~CNewUIMessageBoxBase()
{
    Release();
}

bool mu::ui::window::CNewUIMessageBoxBase::Create(int x, int y, int width, int height, float fPriority/* = 3.f*/)
{
    SetPos(x, y);
    SetSize(width, height);
    m_fPriority = fPriority;
    m_fOpacityAlpha = 0.5f;
    Vector(0.0f, 0.0f, 0.0f, m_vColor);
    return true;
}

void mu::ui::window::CNewUIMessageBoxBase::Release()
{
    m_Pos.x = m_Pos.y = 0;
    m_Size.cx = m_Size.cy = 0;
    m_fPriority = 0.f;
    m_bCanMove = false;

    RemoveAllCallbackFuncs();
}

void mu::ui::window::CNewUIMessageBoxBase::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

void mu::ui::window::CNewUIMessageBoxBase::SetSize(int width, int height)
{
    m_Size.cx = width;
    m_Size.cy = height;
}

const POINT& mu::ui::window::CNewUIMessageBoxBase::GetPos()
{
    return m_Pos;
}

const SIZE& mu::ui::window::CNewUIMessageBoxBase::GetSize()
{
    return m_Size;
}

float mu::ui::window::CNewUIMessageBoxBase::GetPriority() const
{
    return 8.f;
}

void mu::ui::window::CNewUIMessageBoxBase::SetCanMove(bool bCanMove)
{
    m_bCanMove = bCanMove;
}

bool mu::ui::window::CNewUIMessageBoxBase::CanMove()
{
    return m_bCanMove;
}

void mu::ui::window::CNewUIMessageBoxBase::AddCallbackFunc(EVENT_CALLBACK pFunc, DWORD dwEvent)
{
    auto mi = m_mapCallbacks.find(dwEvent);
    if (mi != m_mapCallbacks.end())
        m_mapCallbacks.erase(mi);
    m_mapCallbacks.insert(type_map_callback::value_type(dwEvent, pFunc));
}

void mu::ui::window::CNewUIMessageBoxBase::RemoveCallbackFunc(DWORD dwEvent)
{
    auto mi = m_mapCallbacks.find(dwEvent);
    if (mi != m_mapCallbacks.end())
        m_mapCallbacks.erase(mi);
}

void mu::ui::window::CNewUIMessageBoxBase::RemoveAllCallbackFuncs()
{
    m_mapCallbacks.clear();
}

EVENT_CALLBACK mu::ui::window::CNewUIMessageBoxBase::GetCallbackFunc(DWORD dwEvent)
{
    auto mi = m_mapCallbacks.find(dwEvent);
    if (mi != m_mapCallbacks.end())
        return (*mi).second;
    return NULL;
}

void mu::ui::window::CNewUIMessageBoxBase::SendEvent(CNewUIMessageBoxBase* pOwner, DWORD dwEvent)
{
    CNewUIMessageBoxMng::GetInstance()->SendEvent(pOwner, dwEvent);
}

void mu::ui::window::CNewUIMessageBoxBase::SendEvent(CNewUIMessageBoxBase* pOwner, DWORD dwEvent, const leaf::xstreambuf& xParam)
{
    CNewUIMessageBoxMng::GetInstance()->SendEvent(pOwner, dwEvent, xParam);
}

void mu::ui::window::CNewUIMessageBoxBase::RenderMsgBackColor(bool _bRender)
{
    if (_bRender)
    {
        UI::Scaling::ScopedActiveTransform layout(
            UI::Scaling::ScreenOverlayTransform(WindowWidth, WindowHeight));
        EnableAlphaTest();
        const auto toByte = [](float value)
        {
            return static_cast<unsigned int>(std::clamp(value, 0.f, 1.f) * 255.f);
        };
        const unsigned int color = (toByte(m_fOpacityAlpha) << 24)
            | (toByte(m_vColor[0]) << 16)
            | (toByte(m_vColor[1]) << 8)
            | toByte(m_vColor[2]);
        RenderColorQuadARGB(0.0f, 0.0f, static_cast<float>(REFERENCE_WIDTH),
                            UI::Scaling::ScreenOverlayContentHeight(WindowWidth, WindowHeight), color);

        DisableAlphaBlend();
        EnableAlphaTest();
    }
}

void mu::ui::window::CNewUIMessageBoxBase::SetMsgBackOpacity(float _fAlpha)
{
    m_fOpacityAlpha = _fAlpha;
}

void mu::ui::window::CNewUIMessageBoxBase::SetMsgBackColor(vec3_t _vColor)
{
    if (!_vColor)
    {
        Vector(0.0f, 0.0f, 0.0f, m_vColor);
    }
    else
    {
        VectorCopy(_vColor, m_vColor);
    }
}

mu::ui::window::CNewUIMessageBoxMng::CNewUIMessageBoxMng() : m_pNewUIMng(NULL), m_pMsgBoxFactory(NULL)
{
}

mu::ui::window::CNewUIMessageBoxMng::~CNewUIMessageBoxMng()
{
    Release();
}

bool mu::ui::window::CNewUIMessageBoxMng::Create(CNewUIManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_MESSAGEBOX, this);

    m_pMsgBoxFactory = new CNewUIMessageBoxFactory;

    LoadImages();

    return true;
}

void mu::ui::window::CNewUIMessageBoxMng::Release()
{
    if (m_pNewUIMng == nullptr && m_pMsgBoxFactory == nullptr)
    {
        return;
    }

    UnloadImages();

    PopAllEvents();
    PopAllMessageBoxes();

    SAFE_DELETE(m_pMsgBoxFactory);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

bool mu::ui::window::CNewUIMessageBoxMng::UpdateMouseEvent()
{
    std::sort(m_vecMsgBoxes.begin(), m_vecMsgBoxes.end(), ComparePriority);
    auto vi = m_vecMsgBoxes.begin();
    if (vi == m_vecMsgBoxes.end())
        return true;

    CNewUIMessageBoxBase* pCurMsgBox = (*vi);

    if (m_EventState == EVENT_NONE && false == MouseLButtonPush &&
        mu::ui::window::CheckMouseIn(pCurMsgBox->GetPos().x, pCurMsgBox->GetPos().y,
            pCurMsgBox->GetSize().cx, pCurMsgBox->GetSize().cy))
    {
        SendEvent(pCurMsgBox, MSGBOX_EVENT_MOUSE_HOVER);
        m_EventState = EVENT_WND_MOUSE_HOVER;
    }
    else if (m_EventState == EVENT_WND_MOUSE_HOVER && false == MouseLButtonPush &&
        false == mu::ui::window::CheckMouseIn(pCurMsgBox->GetPos().x, pCurMsgBox->GetPos().y,
            pCurMsgBox->GetSize().cx, pCurMsgBox->GetSize().cy))
    {
        m_EventState = EVENT_NONE;
    }
    else if (m_EventState == EVENT_WND_MOUSE_HOVER && MouseLButtonPush &&
        mu::ui::window::CheckMouseIn(pCurMsgBox->GetPos().x, pCurMsgBox->GetPos().y,
            pCurMsgBox->GetSize().cx, pCurMsgBox->GetSize().cy))
    {
        SendEvent(pCurMsgBox, MSGBOX_EVENT_MOUSE_LBUTTON_DOWN);
        m_EventState = EVENT_WND_MOUSE_LBUTTON_DOWN;

        return false;
    }
    else if (m_EventState == EVENT_WND_MOUSE_LBUTTON_DOWN)
    {
        if (false == MouseLButtonPush && mu::ui::window::CheckMouseIn(pCurMsgBox->GetPos().x, pCurMsgBox->GetPos().y,
            pCurMsgBox->GetSize().cx, pCurMsgBox->GetSize().cy))
        {
            SendEvent(pCurMsgBox, MSGBOX_EVENT_MOUSE_LBUTTON_UP);
            m_EventState = EVENT_NONE;

            return false;
        }
        else if (false == MouseLButtonPush || true == MouseLButtonPop)
        {
            m_EventState = EVENT_NONE;
        }
    }
    else if (m_EventState == EVENT_WND_MOUSE_HOVER && MouseRButtonPush &&
        mu::ui::window::CheckMouseIn(pCurMsgBox->GetPos().x, pCurMsgBox->GetPos().y,
            pCurMsgBox->GetSize().cx, pCurMsgBox->GetSize().cy))
    {
        SendEvent(pCurMsgBox, MSGBOX_EVENT_MOUSE_RBUTTON_DOWN);
        m_EventState = EVENT_WND_MOUSE_RBUTTON_DOWN;

        return false;
    }
    else if (m_EventState == EVENT_WND_MOUSE_RBUTTON_DOWN)
    {
        if (false == MouseRButtonPush && mu::ui::window::CheckMouseIn(pCurMsgBox->GetPos().x, pCurMsgBox->GetPos().y,
            pCurMsgBox->GetSize().cx, pCurMsgBox->GetSize().cy))
        {
            SendEvent(pCurMsgBox, MSGBOX_EVENT_MOUSE_RBUTTON_UP);
            m_EventState = EVENT_NONE;

            return false;
        }
        else if (false == MouseRButtonPush || true == MouseRButtonPop)
        {
            m_EventState = EVENT_NONE;
        }
    }

    if (pCurMsgBox->CanMove() == false)
    {
        return false;
    }

    return true;
}

bool mu::ui::window::CNewUIMessageBoxMng::UpdateKeyEvent()
{
    std::sort(m_vecMsgBoxes.begin(), m_vecMsgBoxes.end(), ComparePriority);
    auto vi = m_vecMsgBoxes.begin();
    if (vi == m_vecMsgBoxes.end())
        return true;

    CNewUIMessageBoxBase* pCurMsgBox = (*vi);
    if (mu::ui::window::IsPress(VK_ESCAPE))
    {
        SendEvent(pCurMsgBox, MSGBOX_EVENT_PRESSKEY_ESC);
    }
    if (mu::ui::window::IsPress(VK_RETURN))
    {
        SendEvent(pCurMsgBox, MSGBOX_EVENT_PRESSKEY_RETURN);
    }

    if (!IsEmpty())
    {
        return false;
    }

    return true;
}

bool mu::ui::window::CNewUIMessageBoxMng::Update()
{
    //. Update
    std::sort(m_vecMsgBoxes.begin(), m_vecMsgBoxes.end(), ComparePriority);
    auto vi = m_vecMsgBoxes.begin();
    if (vi == m_vecMsgBoxes.end())
    {
        return true;
    }
    CNewUIMessageBoxBase* pCurMsgBox = (*vi);
    bool bResult = pCurMsgBox->Update();

    //. Event Processing
    while (!m_queueEvents.empty())
    {
        CNewUIEvent* pEvent = m_queueEvents.front();
        if (pEvent->GetOwner() == pCurMsgBox)
        {
            //. function call
            EVENT_CALLBACK pCallback = pCurMsgBox->GetCallbackFunc(pEvent->GetEvent());
            if (pCallback)
            {
                CALLBACK_RESULT Result = (*pCallback)(pCurMsgBox, pEvent->GetParam());
                if (CALLBACK_BREAK == Result)
                {
                    PopEvent(); break;
                }
                if (CALLBACK_POP_ALL_EVENTS == Result)
                {
                    PopAllEvents(); break;
                }
            }
            if (pEvent->GetEvent() == MSGBOX_EVENT_DESTROY)
            {
                DeleteMessageBox(pCurMsgBox);
                PopAllEvents();
                break;
            }
        }
        PopEvent();
    }
    return bResult;
}

bool mu::ui::window::CNewUIMessageBoxMng::Render()
{
    std::sort(m_vecMsgBoxes.begin(), m_vecMsgBoxes.end(), ComparePriority);
    auto vi = m_vecMsgBoxes.begin();
    if (vi == m_vecMsgBoxes.end())
    {
        return true;
    }
    return (*vi)->Render();
}

float mu::ui::window::CNewUIMessageBoxMng::GetLayerDepth()
{
    return 10.7f;
}

float mu::ui::window::CNewUIMessageBoxMng::GetKeyEventOrder()
{
    return 10.f;
}

CNewUIMessageBoxMng* mu::ui::window::CNewUIMessageBoxMng::GetInstance()
{
    static CNewUIMessageBoxMng s_Instance;
    return &s_Instance;
}

bool mu::ui::window::CNewUIMessageBoxMng::ComparePriority(CNewUIMessageBoxBase* pObj1, CNewUIMessageBoxBase* pObj2)
{
    return pObj1->GetPriority() < pObj2->GetPriority();
}

void mu::ui::window::CNewUIMessageBoxMng::DeleteMessageBox(const CNewUIMessageBoxBase* pObj)
{
    if (m_pMsgBoxFactory)
        m_pMsgBoxFactory->DeleteMessageBox(pObj);
    auto vi = m_vecMsgBoxes.begin();
    for (; vi != m_vecMsgBoxes.end(); vi++)
    {
        if ((*vi) == pObj)
        {
            m_vecMsgBoxes.erase(vi);
            break;
        }
    }
}

void mu::ui::window::CNewUIMessageBoxMng::PopMessageBox()
{
    if (m_vecMsgBoxes.empty() == false)
    {
        std::sort(m_vecMsgBoxes.begin(), m_vecMsgBoxes.end(), ComparePriority);
        auto vi = m_vecMsgBoxes.begin();
        m_pMsgBoxFactory->DeleteMessageBox((*vi));
        m_vecMsgBoxes.erase(vi);
    }
}

void mu::ui::window::CNewUIMessageBoxMng::PopAllMessageBoxes()
{
    m_pMsgBoxFactory->DeleteAllMessageBoxes();
    m_vecMsgBoxes.clear();
}

bool mu::ui::window::CNewUIMessageBoxMng::IsEmpty()
{
    return m_vecMsgBoxes.empty();
}

void mu::ui::window::CNewUIMessageBoxMng::SendEvent(CNewUIMessageBoxBase* pOwner, DWORD dwEvent)
{
    auto* pEvent = new CNewUIEvent(pOwner, dwEvent);
    m_queueEvents.push(pEvent);
}

void mu::ui::window::CNewUIMessageBoxMng::SendEvent(CNewUIMessageBoxBase* pOwner, DWORD dwEvent, const leaf::xstreambuf& xParam)
{
    auto* pEvent = new CNewUIEvent(pOwner, dwEvent, xParam);
    m_queueEvents.push(pEvent);
}

void mu::ui::window::CNewUIMessageBoxMng::PopEvent()
{
    if (!m_queueEvents.empty())
    {
        delete m_queueEvents.front();
        m_queueEvents.pop();
    }
}

void mu::ui::window::CNewUIMessageBoxMng::PopAllEvents()
{
    while (!m_queueEvents.empty())
    {
        delete m_queueEvents.front();
        m_queueEvents.pop();
    }
}

void mu::ui::window::CNewUIMessageBoxMng::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_top.tga", IMAGE_MSGBOX_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_msgbox_middle.tga", IMAGE_MSGBOX_MIDDLE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_msgbox_bottom.tga", IMAGE_MSGBOX_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_MSGBOX_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_Message_Line.tga", IMAGE_MSGBOX_LINE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_Message_03.tga", IMAGE_MSGBOX_TOP_TITLEBAR, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_separate_line.jpg", IMAGE_MSGBOX_SEPARATE_LINE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_button_ok.tga", IMAGE_MSGBOX_BTN_OK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_button_cancel.tga", IMAGE_MSGBOX_BTN_CANCEL, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_button_close.tga", IMAGE_MSGBOX_BTN_CLOSE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_btn_empty.tga", IMAGE_MSGBOX_BTN_EMPTY, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_btn_empty_small.tga", IMAGE_MSGBOX_BTN_EMPTY_SMALL, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_btn_empty_big.tga", IMAGE_MSGBOX_BTN_EMPTY_BIG, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_btn_empty_very_small.tga", IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL, GL_LINEAR);

    LoadBitmap(L"Interface\\newui_Bar_switch01.jpg", IMAGE_MSGBOX_PROGRESS_BG, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_Bar_switch02.jpg", IMAGE_MSGBOX_PROGRESS_BAR, GL_LINEAR);

    LoadBitmap(L"Interface\\newui_DuelWindow.tga", IMAGE_MSGBOX_DUEL_BACK, GL_LINEAR);
}

void mu::ui::window::CNewUIMessageBoxMng::UnloadImages()
{
    DeleteBitmap(IMAGE_MSGBOX_DUEL_BACK);

    DeleteBitmap(IMAGE_MSGBOX_PROGRESS_BAR);
    DeleteBitmap(IMAGE_MSGBOX_PROGRESS_BG);

    DeleteBitmap(IMAGE_MSGBOX_TOP);
    DeleteBitmap(IMAGE_MSGBOX_MIDDLE);
    DeleteBitmap(IMAGE_MSGBOX_BOTTOM);
    DeleteBitmap(IMAGE_MSGBOX_BACK);
    DeleteBitmap(IMAGE_MSGBOX_LINE);
    DeleteBitmap(IMAGE_MSGBOX_TOP_TITLEBAR);
    DeleteBitmap(IMAGE_MSGBOX_SEPARATE_LINE);

    DeleteBitmap(IMAGE_MSGBOX_BTN_OK);
    DeleteBitmap(IMAGE_MSGBOX_BTN_CANCEL);
    DeleteBitmap(IMAGE_MSGBOX_BTN_CLOSE);
    DeleteBitmap(IMAGE_MSGBOX_BTN_EMPTY);
    DeleteBitmap(IMAGE_MSGBOX_BTN_EMPTY_SMALL);
    DeleteBitmap(IMAGE_MSGBOX_BTN_EMPTY_BIG);
    DeleteBitmap(IMAGE_MSGBOX_BTN_EMPTY_VERY_SMALL);
}
