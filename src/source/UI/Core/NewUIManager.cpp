#include "stdafx.h"
#include "UI/Core/NewUIManager.h"
#include "UI/Core/UILayoutPolicy.h"
#include "UI/Widgets/UIControls.h"  // CUITextInputBox::GetFocusedPortable (issue #447)
#include "UI/Scaling/UITransform.h"

using namespace SEASON3B;
using namespace mu::ui::window;

mu::ui::window::CManager::CManager()
{
    m_pActiveMouseUIObj = NULL;
    m_pActiveKeyUIObj = NULL;
#ifdef PBG_MOD_STAMINA_UI
    m_nShowUICnt = 0;
#endif // PBG_MOD_STAMINA_UI
}

mu::ui::window::CManager::~CManager()
{
    RemoveAllUIObjs();
}

void mu::ui::window::CManager::AddUIObj(DWORD dwKey, CObject* pUIObj)
{
    auto mi = m_mapUI.find(dwKey);
    if (mi == m_mapUI.end())
    {
        pUIObj->SetLayoutMode(UI::Layout::ForInterface(dwKey));
        m_vecUI.push_back(pUIObj);
        m_mapUI.insert(type_map_uibase::value_type(dwKey, pUIObj));
    }
}

void mu::ui::window::CManager::RemoveUIObj(DWORD dwKey)
{
    auto mi = m_mapUI.find(dwKey);
    if (mi != m_mapUI.end())
    {
        auto vi = std::find(m_vecUI.begin(), m_vecUI.end(), (*mi).second);
        if (vi != m_vecUI.end())
        {
            m_vecUI.erase(vi);
        }
        m_mapUI.erase(mi);
    }
}

void mu::ui::window::CManager::RemoveUIObj(CObject* pUIObj)
{
    auto mi = m_mapUI.begin();
    for (; mi != m_mapUI.end(); mi++)
    {
        if ((*mi).second == pUIObj)
        {
            m_mapUI.erase(mi);
            break;
        }
    }

    auto vi = std::find(m_vecUI.begin(), m_vecUI.end(), pUIObj);
    if (vi != m_vecUI.end())
    {
        m_vecUI.erase(vi);
    }
}

void mu::ui::window::CManager::RemoveAllUIObjs()
{
#if defined(_DEBUG)

    {
        unsigned int uiUIManageCNT = m_mapUI.size();

        type_map_uibase::iterator mi = m_mapUI.begin();
        for (; mi != m_mapUI.end(); ++mi)
        {
            DWORD dwKey = (*mi).first;
            CObject* pUIObj = (*mi).second;
            if (pUIObj != NULL)
            {
                __TraceF(TEXT("UIKEY(%d) : mapUI \n"), uiUIManageCNT, dwKey);
            }
        }

        type_vector_uibase::iterator vi = m_vecUI.begin();
        for (; vi < m_vecUI.end(); ++vi)
        {
            CObject* pUIObj = (*vi);
            if (pUIObj != NULL)
            {
                __TraceF(TEXT("vecUI \n"), uiUIManageCNT);
            }
        }
    }

#endif // defined(_DEBUG)
    m_vecUI.clear();
    m_mapUI.clear();
}

CObject* mu::ui::window::CManager::FindUIObj(DWORD dwKey)
{
    auto mi = m_mapUI.find(dwKey);
    if (mi != m_mapUI.end())
        return (*mi).second;
    return NULL;
}

CObject* mu::ui::window::CManager::FindUIObjByRelatedWnd(HWND hWnd) const
{
    const auto result = std::find_if(m_vecUI.begin(), m_vecUI.end(),
                                     [hWnd](const CObject* object) { return object->GetRelatedWnd() == hWnd; });
    return result != m_vecUI.end() ? *result : nullptr;
}

bool mu::ui::window::CManager::UpdateMouseEvent()
{
    m_pActiveMouseUIObj = NULL;

    std::sort(m_vecUI.begin(), m_vecUI.end(), CompareLayerDepthReverse);
    auto vecUI = m_vecUI;

    auto vi = vecUI.begin();
    vi = vecUI.begin();
    for (; vi != vecUI.end(); vi++)
    {
        if ((*vi)->IsVisible())
        {
            CObject* obj_backup = (*vi);
            bool bResult;
            {
                const auto transform =
                    UI::Scaling::TransformForLayout((*vi)->GetLayoutMode(), WindowWidth, WindowHeight);
                UI::Scaling::ScopedActiveTransform layout(transform, true);
                bResult = (*vi)->UpdateMouseEvent();
            }

            auto vi2 = std::find(vecUI.begin(), vecUI.end(), obj_backup);
            if (vi2 != vecUI.end())
            {
                vi = vi2;
            }
            else
            {
                break;
            }

            if (bResult == false)
            {
                m_pActiveMouseUIObj = *vi;
                return false;
            }
        }
    }

    return true;
}

bool mu::ui::window::CManager::UpdateKeyEvent()
{
    m_pActiveKeyUIObj = NULL;
    std::sort(m_vecUI.begin(), m_vecUI.end(), CompareKeyEventOrder);

    auto vecUI = m_vecUI;

    // Portable text fields (issue #447) don't take Win32 focus, so GetFocus()
    // can't tell which widget owns the keyboard. When a field is focused, use
    // its stable identity (matches the owning widget's GetRelatedWnd(), which it
    // set to the same handle); otherwise fall back to the window focus. This
    // keeps game hotkeys from firing while the player is typing.
    CUITextInputBox* pFocusedField = CUITextInputBox::GetFocusedPortable();
    const HWND hFocus = pFocusedField ? reinterpret_cast<HWND>(pFocusedField) : GetFocus();

    auto vi = vecUI.begin();
    for (; vi != vecUI.end(); vi++)
    {
        HWND hRelatedWnd = (*vi)->GetRelatedWnd();
        if (NULL == hRelatedWnd)
        {
            hRelatedWnd = g_hWnd;
        }

        HWND hWnd = hFocus;

        if ((*vi)->IsEnabled() && hWnd == hRelatedWnd)
        {
            bool result;
            {
                const auto transform =
                    UI::Scaling::TransformForLayout((*vi)->GetLayoutMode(), WindowWidth, WindowHeight);
                UI::Scaling::ScopedActiveTransform layout(transform, true);
                result = (*vi)->UpdateKeyEvent();
            }
            if (false == result)
            {
                m_pActiveKeyUIObj = (*vi);
                return false; //. stop calling UpdateKeyEvent functions
            }
        }
    }
    return true;
}

bool mu::ui::window::CManager::Update()
{
    std::sort(m_vecUI.begin(), m_vecUI.end(), CompareLayerDepth);

    auto vi = m_vecUI.begin();
    for (; vi != m_vecUI.end(); vi++)
    {
        if ((*vi)->IsEnabled())
        {
            bool result;
            {
                const auto transform =
                    UI::Scaling::TransformForLayout((*vi)->GetLayoutMode(), WindowWidth, WindowHeight);
                UI::Scaling::ScopedActiveTransform layout(transform, true);
                result = (*vi)->Update();
            }
            if (false == result)
            {
                return false; //. stop calling Update functions
            }
        }
    }

    return true;
}

bool mu::ui::window::CManager::Render()
{
    std::sort(m_vecUI.begin(), m_vecUI.end(), CompareLayerDepth);
    auto vecUI = m_vecUI;

    auto vi = vecUI.begin();
    for (; vi != vecUI.end(); vi++)
    {
        if ((*vi)->IsVisible())
        {
            const auto transform = UI::Scaling::TransformForLayout((*vi)->GetLayoutMode(), WindowWidth, WindowHeight);
            UI::Scaling::ScopedActiveTransform layout(transform, true);
            (*vi)->Render();
        }
    }

    return true;
}

CObject* mu::ui::window::CManager::GetActiveMouseUIObj()
{
    return m_pActiveMouseUIObj;
}

CObject* mu::ui::window::CManager::GetActiveKeyUIObj()
{
    return m_pActiveKeyUIObj;
}

void mu::ui::window::CManager::ResetActiveUIObj()
{
    m_pActiveMouseUIObj = NULL;
    m_pActiveKeyUIObj = NULL;
}

bool mu::ui::window::CManager::IsInterfaceVisible(DWORD dwKey)
{
    CObject* pObj = FindUIObj(dwKey);
    if (NULL == pObj)
    {
        return false;
    }
    return pObj->IsVisible();
}

bool mu::ui::window::CManager::IsInterfaceEnabled(DWORD dwKey)
{
    CObject* pObj = FindUIObj(dwKey);
    if (NULL == pObj)
        return false;
    return pObj->IsEnabled();
}

void mu::ui::window::CManager::ShowInterface(DWORD dwKey, bool bShow /* = true*/)
{
    CObject* pObj = FindUIObj(dwKey);
    if (NULL != pObj)
        pObj->Show(bShow);
}

void mu::ui::window::CManager::EnableInterface(DWORD dwKey, bool bEnable /* = true*/)
{
    CObject* pObj = FindUIObj(dwKey);
    if (NULL != pObj)
        pObj->Enable(bEnable);
}

void mu::ui::window::CManager::ShowAllInterfaces(bool bShow /* = true*/)
{
    auto mi = m_mapUI.begin();
    for (; mi != m_mapUI.end(); mi++)
        (*mi).second->Show(bShow);
}

void mu::ui::window::CManager::EnableAllInterfaces(bool bEnable /* = true*/)
{
    auto mi = m_mapUI.begin();
    for (; mi != m_mapUI.end(); mi++)
        (*mi).second->Show(bEnable);
}

bool mu::ui::window::CManager::CompareLayerDepth(IObject* pObj1, IObject* pObj2)
{
    return pObj1->GetLayerDepth() < pObj2->GetLayerDepth();
}

bool mu::ui::window::CManager::CompareLayerDepthReverse(IObject* pObj1, IObject* pObj2)
{
    return pObj1->GetLayerDepth() > pObj2->GetLayerDepth();
}

bool mu::ui::window::CManager::CompareKeyEventOrder(IObject* pObj1, IObject* pObj2)
{
    return pObj1->GetKeyEventOrder() > pObj2->GetKeyEventOrder();
}

#ifdef PBG_MOD_STAMINA_UI
int mu::ui::window::CManager::GetShowUICnt()
{
    int m_nShowUICnt = 0;
    // How many of certain interfaces are open
    for (int i = INTERFACE_PARTY; i < INTERFACE_CHARACTER + 1; ++i)
    {
        if (IsInterfaceVisible(i))
            m_nShowUICnt++;
    }
    return m_nShowUICnt;
}
#endif // PBG_MOD_STAMINA_UI
