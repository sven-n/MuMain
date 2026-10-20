#include "stdafx.h"
#include "UI/RmlBridge/RmlWindowClose.h"

#include "UI/Core/WindowSystem.h"

#include <RmlUi/Core/DataModelHandle.h>

void UI::RmlBridge::BindWindowClose(Rml::DataModelConstructor& constructor, std::uint32_t windowId)
{
    BindWindowClose(constructor,
                    [windowId]
                    {
                        if (g_pNewUISystem != nullptr)
                            g_pNewUISystem->Hide(windowId);
                    });
}

void UI::RmlBridge::BindWindowClose(Rml::DataModelConstructor& constructor, std::function<void()> close)
{
    constructor.BindEventCallback("window_close",
                                  [close = std::move(close)](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                                  { close(); });
}
