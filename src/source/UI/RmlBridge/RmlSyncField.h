#pragma once

#include "UI/RmlBridge/RmlModelBinder.h"

#include <utility>

// Write a model field and dirty it only if it changed.
//
// RmlUi's contract is "assign the field, then DirtyVariable() it by name" -- manual, per field, and
// the one operation every window performs dozens of times. Before this existed, 28 windows had each
// written their own version, in fifteen signatures and two mutually inverted argument orders
// ((field, name, value) and (field, value, name)). Both orders take a string in the same two
// positions, so a line copied between two neighbouring windows compiled and silently wrote the
// field's *name* into the model -- and nothing caught it, because DataModel::DirtyVariable()'s only
// guard against an unknown name is an RMLUI_ASSERTMSG that compiles out under NDEBUG.
//
// Deliberately at global scope, like RmlModelBinder itself, so a call needs no qualification.
//
// The single `T` is the point, not an accident: the member's type and the value's type must deduce
// to the same thing, so passing the name where the value belongs is a deduction conflict at compile
// time rather than a silent wrong value at runtime. That costs an explicit cast when a caller
// genuinely wants a conversion (an int into a float field) -- worth it, and the cast documents the
// conversion at the call site.
//
// Comparison, not assignment, is what makes this correct: an unconditional MarkDirty() re-runs every
// view bound to that name on a value that did not change. See UI::RmlBridge::SyncNativeTextSize()
// for the same shape applied to one field.
template <typename Model, typename T>
void SyncField(RmlModelBinder<Model>& binder, T Model::* field, const char* name, T value)
{
    Model& model = binder.GetModel();
    if (model.*field == value)
        return;
    model.*field = std::move(value);
    binder.MarkDirty(name);
}

// Copy one field across from a staged model, if it changed.
//
// Some windows build a whole `updated` model each frame and then reconcile it against the live one
// field by field -- a different operation from SyncField() above, which takes a value. Worth its own
// overload rather than writing SyncField(binder, &Model::x, "x", updated.x): that names the field
// twice, and the two names are free to disagree while still compiling, which is the hazard this
// header exists to remove. Here the field is named once and the value cannot come from elsewhere.
template <typename Model, typename T>
void SyncFieldFrom(RmlModelBinder<Model>& binder, T Model::* field, const char* name, Model& source)
{
    Model& model = binder.GetModel();
    if (model.*field == source.*field)
        return;
    model.*field = std::move(source.*field);
    binder.MarkDirty(name);
}
