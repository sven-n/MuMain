
#include "stdafx.h"
#include "UI/Dialogs/MessageBox.h"	// self
#include "UI/Core/WindowManager.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/Scaling/UITransform.h"
#include "Render/Textures/ZzzOpenglUtil.h"

using namespace SEASON3B;
using namespace mu::ui::window;

//////////////////////////////////////////////////////////////////////
// CMessageBoxBase
//////////////////////////////////////////////////////////////////////

mu::ui::window::CMessageBoxBase::CMessageBoxBase()
{
    Release();
}

mu::ui::window::CMessageBoxBase::~CMessageBoxBase()
{
    Release();
}

bool mu::ui::window::CMessageBoxBase::Create(int x, int y, int width, int height, float fPriority/* = 3.f*/)
{
    SetPos(x, y);
    SetSize(width, height);
    m_fPriority = fPriority;
    return true;
}

void mu::ui::window::CMessageBoxBase::Release()
{
    m_Pos.x = m_Pos.y = 0;
    m_Size.cx = m_Size.cy = 0;
    m_fPriority = 0.f;
    m_bCanMove = false;

    RemoveAllCallbackFuncs();
}

void mu::ui::window::CMessageBoxBase::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

void mu::ui::window::CMessageBoxBase::SetSize(int width, int height)
{
    m_Size.cx = width;
    m_Size.cy = height;
}

const POINT& mu::ui::window::CMessageBoxBase::GetPos()
{
    return m_Pos;
}

const SIZE& mu::ui::window::CMessageBoxBase::GetSize()
{
    return m_Size;
}

float mu::ui::window::CMessageBoxBase::GetPriority() const
{
    return 8.f;
}

void mu::ui::window::CMessageBoxBase::SetCanMove(bool bCanMove)
{
    m_bCanMove = bCanMove;
}

bool mu::ui::window::CMessageBoxBase::CanMove()
{
    return m_bCanMove;
}

void mu::ui::window::CMessageBoxBase::AddCallbackFunc(EVENT_CALLBACK pFunc, DWORD dwEvent)
{
    auto mi = m_mapCallbacks.find(dwEvent);
    if (mi != m_mapCallbacks.end())
        m_mapCallbacks.erase(mi);
    m_mapCallbacks.insert(type_map_callback::value_type(dwEvent, pFunc));
}

void mu::ui::window::CMessageBoxBase::RemoveCallbackFunc(DWORD dwEvent)
{
    auto mi = m_mapCallbacks.find(dwEvent);
    if (mi != m_mapCallbacks.end())
        m_mapCallbacks.erase(mi);
}

void mu::ui::window::CMessageBoxBase::RemoveAllCallbackFuncs()
{
    m_mapCallbacks.clear();
}

EVENT_CALLBACK mu::ui::window::CMessageBoxBase::GetCallbackFunc(DWORD dwEvent)
{
    auto mi = m_mapCallbacks.find(dwEvent);
    if (mi != m_mapCallbacks.end())
        return (*mi).second;
    return NULL;
}

void mu::ui::window::CMessageBoxBase::SendEvent(CMessageBoxBase* pOwner, DWORD dwEvent)
{
    CMessageBoxMng::GetInstance()->SendEvent(pOwner, dwEvent);
}

void mu::ui::window::CMessageBoxBase::SendEvent(CMessageBoxBase* pOwner, DWORD dwEvent, const leaf::xstreambuf& xParam)
{
    CMessageBoxMng::GetInstance()->SendEvent(pOwner, dwEvent, xParam);
}

mu::ui::window::CMessageBoxMng::CMessageBoxMng() : m_pNewUIMng(NULL), m_pMsgBoxFactory(NULL)
{
}

mu::ui::window::CMessageBoxMng::~CMessageBoxMng()
{
    Release();
}

bool mu::ui::window::CMessageBoxMng::Create(CManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_MESSAGEBOX, this);

    m_pMsgBoxFactory = new CMessageBoxFactory;

    LoadImages();

    return true;
}

void mu::ui::window::CMessageBoxMng::Release()
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

bool mu::ui::window::CMessageBoxMng::UpdateMouseEvent()
{
    std::sort(m_vecMsgBoxes.begin(), m_vecMsgBoxes.end(), ComparePriority);
    auto vi = m_vecMsgBoxes.begin();
    if (vi == m_vecMsgBoxes.end())
        return true;

    // The top box's document reports its buttons. A box that cannot move takes every press; one
    // that can takes the presses over its drawn panel, so they never reach the world.
    CMessageBoxBase* pCurMsgBox = (*vi);
    if (pCurMsgBox->CanMove() == false)
        return false;
    if ((MouseLButtonPush || MouseRButtonPush) && UI::RmlBridge::IsPointerWithin(pCurMsgBox->GetPanel()))
        return false;
    return true;
}

bool mu::ui::window::CMessageBoxMng::UpdateKeyEvent()
{
    std::sort(m_vecMsgBoxes.begin(), m_vecMsgBoxes.end(), ComparePriority);
    auto vi = m_vecMsgBoxes.begin();
    if (vi == m_vecMsgBoxes.end())
        return true;

    CMessageBoxBase* pCurMsgBox = (*vi);
    if (mu::ui::window::IsPress(VK_ESCAPE))
    {
        SendEvent(pCurMsgBox, MSGBOX_EVENT_PRESSKEY_ESC);
    }
    if (mu::ui::window::IsPress(VK_RETURN))
    {
        SendEvent(pCurMsgBox, MSGBOX_EVENT_PRESSKEY_RETURN);
    }

    return IsEmpty();
}

bool mu::ui::window::CMessageBoxMng::Update()
{
    //. Update
    std::sort(m_vecMsgBoxes.begin(), m_vecMsgBoxes.end(), ComparePriority);
    auto vi = m_vecMsgBoxes.begin();
    if (vi == m_vecMsgBoxes.end())
    {
        return true;
    }
    CMessageBoxBase* pCurMsgBox = (*vi);
    bool bResult = pCurMsgBox->Update();

    //. Event Processing
    while (!m_queueEvents.empty())
    {
        CEvent* pEvent = m_queueEvents.front();
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

bool mu::ui::window::CMessageBoxMng::Render()
{
    std::sort(m_vecMsgBoxes.begin(), m_vecMsgBoxes.end(), ComparePriority);
    auto vi = m_vecMsgBoxes.begin();
    if (vi == m_vecMsgBoxes.end())
    {
        return true;
    }
    return (*vi)->Render();
}

float mu::ui::window::CMessageBoxMng::GetLayerDepth()
{
    return 10.7f;
}

float mu::ui::window::CMessageBoxMng::GetKeyEventOrder()
{
    return 10.f;
}

CMessageBoxMng* mu::ui::window::CMessageBoxMng::GetInstance()
{
    static CMessageBoxMng s_Instance;
    return &s_Instance;
}

bool mu::ui::window::CMessageBoxMng::ComparePriority(CMessageBoxBase* pObj1, CMessageBoxBase* pObj2)
{
    return pObj1->GetPriority() < pObj2->GetPriority();
}

void mu::ui::window::CMessageBoxMng::DeleteMessageBox(const CMessageBoxBase* pObj)
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

void mu::ui::window::CMessageBoxMng::PopMessageBox()
{
    if (m_vecMsgBoxes.empty() == false)
    {
        std::sort(m_vecMsgBoxes.begin(), m_vecMsgBoxes.end(), ComparePriority);
        auto vi = m_vecMsgBoxes.begin();
        m_pMsgBoxFactory->DeleteMessageBox((*vi));
        m_vecMsgBoxes.erase(vi);
    }
}

void mu::ui::window::CMessageBoxMng::PopAllMessageBoxes()
{
    m_pMsgBoxFactory->DeleteAllMessageBoxes();
    m_vecMsgBoxes.clear();
}

bool mu::ui::window::CMessageBoxMng::IsEmpty()
{
    return m_vecMsgBoxes.empty();
}

void mu::ui::window::CMessageBoxMng::SendEvent(CMessageBoxBase* pOwner, DWORD dwEvent)
{
    auto* pEvent = new CEvent(pOwner, dwEvent);
    m_queueEvents.push(pEvent);
}

void mu::ui::window::CMessageBoxMng::SendEvent(CMessageBoxBase* pOwner, DWORD dwEvent, const leaf::xstreambuf& xParam)
{
    auto* pEvent = new CEvent(pOwner, dwEvent, xParam);
    m_queueEvents.push(pEvent);
}

void mu::ui::window::CMessageBoxMng::PopEvent()
{
    if (!m_queueEvents.empty())
    {
        delete m_queueEvents.front();
        m_queueEvents.pop();
    }
}

void mu::ui::window::CMessageBoxMng::PopAllEvents()
{
    while (!m_queueEvents.empty())
    {
        delete m_queueEvents.front();
        m_queueEvents.pop();
    }
}

void mu::ui::window::CMessageBoxMng::LoadImages()
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

void mu::ui::window::CMessageBoxMng::UnloadImages()
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
