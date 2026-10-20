#pragma once

#include "stdafx.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/DataTypeRegister.h>

#include <memory>

// Generalizes the model/renderer split already proven by UI::Skills::Tooltip's
// SkillTooltipModel.h/.cpp (a plain-data Model, consumed today by two independent renderers)
// into a reusable per-window wrapper for RmlUi's Rml::DataModel binding.
//
// This does NOT do reflection-based auto-binding -- RmlUi's own DataModelConstructor::Bind()
// requires each field to be registered by name explicitly (its actual API, not a limitation of
// this wrapper), so each window still writes its own small "register my fields" function. What
// this wrapper standardizes is the surrounding lifecycle: owning the Model instance, creating
// the Rml::DataModelHandle once, and exposing a single MarkDirty() call so packet-handler/action-
// controller call sites that mutate Model fields don't need to know RmlUi's binding API directly
// -- that dependency is isolated to this one header and each window's registration function.
//
// Usage (illustrative -- the actual Model type and registration function are written per
// migrated window):
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
    //
    // Idempotent: while this binder still holds the model `modelName` in `context` (its document
    // failed to load, so the window's build step runs again), Create() keeps that model and returns
    // true, so the caller retries only its document load -- a later frame, a restored asset or a
    // theme switch can still bring the window up.
    template <typename RegisterFn>
    bool Create(Rml::Context* context, const Rml::String& modelName, RegisterFn&& registerFields)
    {
        if (m_Handle && m_Context == context && m_ModelName == modelName)
            return true;

        // Own type register per model, not the context's shared default one: the shared
        // register keeps every RegisterStruct<T>()/RegisterArray<C>() for the context's lifetime,
        // so a second Create() after Destroy() (ReloadRmlTheme()) would get "Struct type already
        // declared" and a null StructHandle whose RegisterMember() dereferences it. A fresh
        // register per Create() makes the registration function re-runnable; struct types are
        // then per model, which is how every window here uses them anyway. It replaces the held
        // register only once its model exists: a failed Create() must not free the register a
        // model created earlier still points to.
        auto typeRegister = std::make_unique<Rml::DataTypeRegister>();
        Rml::DataModelConstructor constructor = context->CreateDataModel(modelName, typeRegister.get());
        if (!constructor)
            return false;

        m_TypeRegister = std::move(typeRegister);
        registerFields(constructor, m_Model);
        m_Handle = constructor.GetModelHandle();
        m_Context = context;
        m_ModelName = modelName;
        return true;
    }

    // Undoes Create(): removes the named model from `context` and resets this binder to its
    // pre-Create() state so Create() can be called again (e.g. rebuilding against a new RmlUi
    // theme -- see UI::RmlBridge::ThemedView). No-op if Create() was never called or
    // already undone. `context` must be the same context Create() was given; RmlUi's data models
    // are owned per-Context, not globally. A null `context` (RmlUi already released it) only
    // resets the binder.
    void Destroy(Rml::Context* context)
    {
        if (m_ModelName.empty()) return;
        if (context)
            context->RemoveDataModel(m_ModelName);
        m_TypeRegister.reset(); // after the model, which holds a raw pointer to it
        m_Model = Model{};
        m_Handle = Rml::DataModelHandle{};
        m_Context = nullptr;
        m_ModelName.clear();
    }

    Model& GetModel() { return m_Model; }
    const Model& GetModel() const { return m_Model; }

    // "all" is not a magic wildcard here -- per-field DirtyVariable() calls are RmlUi's real
    // contract (DataModelHandle.h). A window with many fields that all change together (e.g. a
    // full item-list refresh) should call MarkDirty() once per bound field name it registered,
    // not rely on this wrapper to enumerate them -- there is no reflection to enumerate from.
    void MarkDirty(const Rml::String& fieldName)
    {
        // DataModelHandle::DirtyVariable() dereferences its model unchecked, so a MarkDirty() before
        // Create() -- or after Destroy(), or when Create() failed -- would crash. A window that stages
        // model state from game code (a config applied before its document exists) hits exactly that.
        if (m_Handle)
            m_Handle.DirtyVariable(fieldName);
    }

private:
    Model m_Model{};
    std::unique_ptr<Rml::DataTypeRegister> m_TypeRegister;
    Rml::DataModelHandle m_Handle;
    Rml::Context* m_Context = nullptr;
    Rml::String m_ModelName;
};
