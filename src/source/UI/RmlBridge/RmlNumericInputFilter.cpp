#include "stdafx.h"
#include "UI/RmlBridge/RmlNumericInputFilter.h"

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>
#include <RmlUi/Core/EventListener.h>

namespace UI::RmlBridge
{
    namespace
    {
        class NumericInputFilter final : public Rml::EventListener
        {
        public:
            void ProcessEvent(Rml::Event& event) override
            {
                Rml::Element* target = event.GetTargetElement();
                if (target == nullptr || !target->IsClassSet(NumericFieldClass))
                    return;

                const Rml::String text = event.GetParameter<Rml::String>("text", Rml::String());
                for (const char c : text)
                {
                    if (c < '0' || c > '9')
                    {
                        event.StopPropagation();
                        return;
                    }
                }
            }
        };

        NumericInputFilter& SharedFilter()
        {
            static NumericInputFilter filter;
            return filter;
        }
    }

    void AttachNumericInputFilter(Rml::ElementDocument* doc)
    {
        if (doc != nullptr)
            doc->AddEventListener(Rml::EventId::Textinput, &SharedFilter(), true);
    }

    Rml::String KeepDigitsOnly(const Rml::String& value)
    {
        Rml::String digits;
        digits.reserve(value.size());
        for (const char c : value)
        {
            if (c >= '0' && c <= '9')
                digits += c;
        }
        return digits;
    }
}
