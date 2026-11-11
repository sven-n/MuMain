#include "stdafx.h"

#include "UI/Core/WindowAccess.h"

#include "UI/Core/UIManager.h"
#include "UI/Core/WindowSystem.h"

#include <RmlUi/Core/ElementDocument.h>

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

Rml::Element* FindElement(WindowId id, const char* selector)
{
    if (!g_pNewUISystem->IsVisible(id))
        return nullptr;
    mu::ui::window::CObject* window = g_pNewUIMng->FindUIObj(id);
    Rml::ElementDocument* document = window != nullptr ? window->GetPlacedDocument() : nullptr;
    return document != nullptr ? document->QuerySelector(selector) : nullptr;
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
