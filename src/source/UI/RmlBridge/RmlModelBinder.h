#pragma once

#include "stdafx.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>

// Generalizes the model/renderer split already proven by UI::Skills::Tooltip's
// SkillTooltipModel.h/.cpp (a plain-data Model, consumed today by two independent renderers)
// into a reusable per-window wrapper for RmlUi's Rml::DataModel binding. See the RmlUi
// migration plan's Phase 0.6/0.7.
//
// This does NOT do reflection-based auto-binding -- RmlUi's own DataModelConstructor::Bind()
// requires each field to be registered by name explicitly (its actual API, not a limitation of
// this wrapper), so each window still writes its own small "register my fields" function. What
// this wrapper standardizes is the surrounding lifecycle: owning the Model instance, creating
// the Rml::DataModelHandle once, and exposing a single MarkDirty() call so packet-handler/action-
// controller call sites that mutate Model fields (per the adapter pattern, e.g.
// CNewUIMyInventoryRmlAdapter::InsertItem()) don't need to know RmlUi's binding API directly --
// that dependency is isolated to this one header and each window's registration function.
//
// Usage (illustrative -- the actual Model type and registration function are written per
// migrated window, starting with the Phase 1 pilot):
//
//   struct PilotModel { Rml::String title; int secondsRemaining = 0; };
//
//   RmlModelBinder<PilotModel> binder;
//   binder.Create(context, "pilot_dialog", [](Rml::DataModelConstructor& c, PilotModel& model) {
//       c.Bind("title", &model.title);
//       c.Bind("seconds_remaining", &model.secondsRemaining);
//   });
//   ...
//   binder.Model().secondsRemaining = 30;
//   binder.MarkDirty("seconds_remaining");
template <typename Model>
class RmlModelBinder
{
public:
    // RegisterFn: void(Rml::DataModelConstructor&, Model&) -- binds each field the RML template
    // needs, exactly once, at creation time.
    template <typename RegisterFn>
    bool Create(Rml::Context* context, const Rml::String& modelName, RegisterFn&& registerFields)
    {
        Rml::DataModelConstructor constructor = context->CreateDataModel(modelName);
        if (!constructor) return false;

        registerFields(constructor, m_Model);
        m_Handle = constructor.GetModelHandle();
        return true;
    }

    Model& GetModel() { return m_Model; }
    const Model& GetModel() const { return m_Model; }

    // "all" is not a magic wildcard here -- per-field DirtyVariable() calls are RmlUi's real
    // contract (DataModelHandle.h). A window with many fields that all change together (e.g. a
    // full item-list refresh) should call MarkDirty() once per bound field name it registered,
    // not rely on this wrapper to enumerate them -- there is no reflection to enumerate from.
    void MarkDirty(const Rml::String& fieldName)
    {
        m_Handle.DirtyVariable(fieldName);
    }

private:
    Model m_Model{};
    Rml::DataModelHandle m_Handle;
};
