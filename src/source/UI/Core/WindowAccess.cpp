#include "stdafx.h"

#include "UI/Core/WindowAccess.h"

#include "UI/Core/UIManager.h"
#include "UI/Core/WindowSystem.h"

namespace UI::Windows
{
void Show(WindowId id)
{
    g_pNewUISystem->Show(id);
}

void Hide(WindowId id)
{
    g_pNewUISystem->Hide(id);
}

bool IsVisible(WindowId id)
{
    return g_pNewUISystem->IsVisible(id);
}

void HideAll()
{
    g_pNewUISystem->HideAll();
}

void ResetLegacyPanels()
{
    if (g_pUIManager)
        g_pUIManager->Init();
}

void OpenServerDivision()
{
    g_pUIManager->Open(::MUTEX_SERVERDIVISION);
}
}
