#pragma once

#include "UI/RmlBridge/RmlModelBinder.h"

#include <RmlUi/Core/ElementDocument.h>

#include <functional>
#include <string>
#include <utility>
#include <vector>

// The owner of a window's RmlUi documents and the data model they show: building them against the
// active theme, rebuilding them on a theme switch, and tearing them down. A window declares its
// model's fields, its documents and any setup that follows a build; it calls Ensure() where it used
// to build, and Release() (or lets the destructor run) where it used to unload.
//
// A theme switch keeps the model's values and rebuilds the documents over them, and shows again
// each document that was visible. A failed build retries on the next Ensure(), and the view is
// registered for theme switches from its first Ensure() whether or not the build succeeded.
namespace UI::RmlBridge
{
struct ThemedDocumentSpec
{
    std::string path; // as LoadThemedDocument() takes it
    // The context the document loads into; RmlUiRuntime's main context when empty. Null means not
    // yet: Ensure() waits.
    std::function<Rml::Context*()> context;
};

// Where a document shown again after a theme switch goes among the documents of its depth: as
// SyncDocumentVisibilityInFront() or Behind() put it, or wherever Show() leaves it.
enum class ThemedStacking
{
    AsShown,
    Front,
    Back,
};

struct ThemedViewOptions
{
    // Per-instance documents: the markup's data-model="<this>" binds the view's model name instead,
    // so each instance binds its own model.
    std::string modelPlaceholder;
    // How a document that was visible is shown again after a theme switch.
    Rml::ModalFlag modal = Rml::ModalFlag::None;
    Rml::FocusFlag focus = Rml::FocusFlag::None;
    ThemedStacking stacking = ThemedStacking::AsShown;
    // After every build, once all the documents are loaded: dragging, input filters, layout that
    // depends on the theme, caches to invalidate.
    std::function<void()> afterBuild;
    // After a theme switch's rebuild, once what was visible is shown again: whatever the owner sized
    // or placed from the previous theme's metrics.
    std::function<void()> afterReload;
    // Before the documents unload, on a theme switch and on Release(): listeners and controls the
    // owner attached to them.
    std::function<void()> beforeUnload;
};

// True while RmlUi still owns `context`: false once it was removed or RmlUi shut down.
bool IsContextAlive(const Rml::Context* context);

// For a ThemedDocumentSpec: the background context, which draws before native 3D, or the main one
// when there is none.
Rml::Context* BackgroundOrMainContext();

// The documents and the theme-switch registration; ThemedView<Model> adds the model.
class ThemedDocuments
{
public:
    ThemedDocuments(std::string modelName, std::vector<ThemedDocumentSpec> documents, ThemedViewOptions options);
    virtual ~ThemedDocuments();
    ThemedDocuments(const ThemedDocuments&) = delete;
    ThemedDocuments& operator=(const ThemedDocuments&) = delete;

    // Builds whatever is missing, if the documents' contexts exist. Cheap once built.
    bool Ensure();
    // Unloads the documents, removes the model and stops following theme switches.
    void Release();
    // Hides every document, keeping them for the next Show().
    void Hide();
    // What a theme switch runs.
    void Reload();
    // Replaces the options' afterBuild, for an owner that sets it after constructing the view.
    void SetAfterBuild(std::function<void()> afterBuild) { m_Options.afterBuild = std::move(afterBuild); }
    void SetAfterReload(std::function<void()> afterReload) { m_Options.afterReload = std::move(afterReload); }
    // For an owner that knows its per-instance model name only once it is built; ignored once the
    // model exists.
    void SetModelName(std::string modelName);

    bool IsBuilt() const { return m_Built; }
    // Null until built, and once RmlUi no longer owns the document's context.
    Rml::ElementDocument* Document(size_t index = 0) const;
    const std::string& ModelName() const { return m_ModelName; }

protected:
    virtual bool CreateModel(Rml::Context*) { return true; }
    // `context` is null once RmlUi no longer owns it.
    virtual void DestroyModel(Rml::Context*, bool /*keepValues*/) {}

private:
    struct Slot
    {
        ThemedDocumentSpec spec;
        Rml::Context* context = nullptr;
        Rml::ElementDocument* document = nullptr;
    };

    Rml::Context* ResolveContext(const Slot& slot) const;
    void UnloadDocuments();
    void ForgetModel(bool keepValues);

    std::string m_ModelName;
    std::vector<Slot> m_Slots;
    ThemedViewOptions m_Options;
    Rml::Context* m_ModelContext = nullptr;
    bool m_ModelCreated = false;
    bool m_Built = false;
    bool m_Registered = false;
};

template <typename Model = void> class ThemedView : public ThemedDocuments
{
public:
    using RegisterFields = std::function<void(Rml::DataModelConstructor&, Model&)>;

    ThemedView(std::string modelName, RegisterFields registerFields, std::vector<ThemedDocumentSpec> documents,
               ThemedViewOptions options = {})
        : ThemedDocuments(std::move(modelName), std::move(documents), std::move(options)),
          m_RegisterFields(std::move(registerFields))
    {
    }

    // The base destructor can no longer reach DestroyModel().
    ~ThemedView() override { Release(); }

    Model& GetModel() { return m_Binder.GetModel(); }
    const Model& GetModel() const { return m_Binder.GetModel(); }
    RmlModelBinder<Model>& Binder() { return m_Binder; }
    void MarkDirty(const Rml::String& field) { m_Binder.MarkDirty(field); }

protected:
    bool CreateModel(Rml::Context* context) override
    {
        return m_Binder.Create(context, ModelName(), m_RegisterFields);
    }

    void DestroyModel(Rml::Context* context, bool keepValues) override
    {
        if (!keepValues)
        {
            m_Binder.Destroy(context);
            return;
        }
        Model kept = std::move(m_Binder.GetModel());
        m_Binder.Destroy(context);
        m_Binder.GetModel() = std::move(kept);
    }

private:
    RegisterFields m_RegisterFields;
    RmlModelBinder<Model> m_Binder;
};

// Documents with no data model.
template <> class ThemedView<void> : public ThemedDocuments
{
public:
    explicit ThemedView(std::vector<ThemedDocumentSpec> documents, ThemedViewOptions options = {})
        : ThemedDocuments(std::string(), std::move(documents), std::move(options))
    {
    }
};
} // namespace UI::RmlBridge
