#include "stdafx.h"
#include "UI/RmlBridge/RmlThemedView.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Plugin.h>

#include <algorithm>
#include <unordered_set>

namespace UI::RmlBridge
{
namespace
{
// Follows which contexts RmlUi still owns. Rml::Shutdown() frees the context list itself, so a
// teardown running after it (a window released late, a static owner at exit) cannot ask RmlUi; it
// asks this instead. Registered on first use and again after each Rml::Initialise(), since
// Rml::Shutdown() unregisters every plugin.
class ContextTracker final : public Rml::Plugin
{
public:
    void Follow()
    {
        if (m_Following)
            return;
        m_Following = true;
        m_Alive.clear();
        for (int i = 0; i < Rml::GetNumContexts(); ++i)
            m_Alive.insert(Rml::GetContext(i));
        Rml::RegisterPlugin(this);
    }

    bool IsAlive(const Rml::Context* context) const { return m_Following && m_Alive.contains(context); }

    int GetEventClasses() override { return EVT_BASIC; }
    void OnContextCreate(Rml::Context* context) override { m_Alive.insert(context); }
    void OnContextDestroy(Rml::Context* context) override { m_Alive.erase(context); }
    void OnShutdown() override
    {
        m_Alive.clear();
        m_Following = false;
    }

private:
    std::unordered_set<const Rml::Context*> m_Alive;
    bool m_Following = false;
};

// Never destroyed: a static owner's teardown at exit may still ask it.
ContextTracker& Tracker()
{
    static auto* tracker = new ContextTracker();
    return *tracker;
}
} // namespace

bool IsContextAlive(const Rml::Context* context)
{
    return context != nullptr && Tracker().IsAlive(context);
}

ThemedDocuments::ThemedDocuments(std::string modelName, std::vector<ThemedDocumentSpec> documents,
                                 ThemedViewOptions options)
    : m_ModelName(std::move(modelName)), m_Options(std::move(options))
{
    m_Slots.reserve(documents.size());
    for (ThemedDocumentSpec& spec : documents)
        m_Slots.push_back({std::move(spec)});
}

ThemedDocuments::~ThemedDocuments()
{
    Release();
}

Rml::Context* ThemedDocuments::ResolveContext(const Slot& slot) const
{
    if (slot.spec.context)
        return slot.spec.context();
    RmlUiRuntime& runtime = RmlUiRuntime::Instance();
    return runtime.IsCreated() ? runtime.GetContext() : nullptr;
}

bool ThemedDocuments::Ensure()
{
    if (m_Built)
        return true;

    if (!m_Registered)
    {
        RegisterForThemeReload(this, [this] { Reload(); });
        m_Registered = true;
    }

    std::vector<Rml::Context*> contexts;
    contexts.reserve(m_Slots.size());
    for (const Slot& slot : m_Slots)
    {
        Rml::Context* context = slot.document != nullptr ? slot.context : ResolveContext(slot);
        if (context == nullptr)
            return false;
        contexts.push_back(context);
    }
    if (contexts.empty())
        return false;

    Tracker().Follow();

    // The model lives in the first document's context; the documents that show it load there too.
    if (!m_ModelCreated)
    {
        if (!CreateModel(contexts.front()))
            return false;
        m_ModelCreated = true;
        m_ModelContext = contexts.front();
    }

    for (size_t i = 0; i < m_Slots.size(); ++i)
    {
        Slot& slot = m_Slots[i];
        if (slot.document != nullptr)
            continue;
        slot.document = m_Options.modelPlaceholder.empty()
                            ? LoadThemedDocument(contexts[i], slot.spec.path.c_str())
                            : LoadThemedDocument(contexts[i], slot.spec.path.c_str(),
                                                 "data-model=\"" + m_Options.modelPlaceholder + "\"",
                                                 "data-model=\"" + m_ModelName + "\"");
        if (slot.document == nullptr)
            return false;
        slot.context = contexts[i];
    }

    m_Built = true;
    if (m_Options.afterBuild)
        m_Options.afterBuild();
    return true;
}

void ThemedDocuments::Reload()
{
    const bool anything = m_ModelCreated ||
                          std::any_of(m_Slots.begin(), m_Slots.end(), [](const Slot& slot) { return slot.document; });
    if (!anything)
        return;

    std::vector<bool> wasVisible;
    wasVisible.reserve(m_Slots.size());
    for (const Slot& slot : m_Slots)
        wasVisible.push_back(slot.document != nullptr && slot.document->IsVisible());

    UnloadDocuments();
    ForgetModel(true);
    Ensure();

    for (size_t i = 0; i < m_Slots.size(); ++i)
    {
        Rml::ElementDocument* document = m_Slots[i].document;
        if (!wasVisible[i] || document == nullptr)
            continue;
        document->Show(m_Options.modal, m_Options.focus);
        if (m_Options.stacking == ThemedStacking::Front)
            document->PullToFront();
        else if (m_Options.stacking == ThemedStacking::Back)
            document->PushToBack();
    }
    if (m_Built && m_Options.afterReload)
        m_Options.afterReload();
}

void ThemedDocuments::Release()
{
    UnloadDocuments();
    ForgetModel(false);
    if (m_Registered)
    {
        UnregisterForThemeReload(this);
        m_Registered = false;
    }
}

void ThemedDocuments::Hide()
{
    for (const Slot& slot : m_Slots)
    {
        if (slot.document != nullptr && IsContextAlive(slot.context) && slot.document->IsVisible())
            slot.document->Hide();
    }
}

void ThemedDocuments::SetModelName(std::string modelName)
{
    if (!m_ModelCreated)
        m_ModelName = std::move(modelName);
}

Rml::ElementDocument* ThemedDocuments::Document(size_t index) const
{
    if (index >= m_Slots.size() || !IsContextAlive(m_Slots[index].context))
        return nullptr;
    return m_Slots[index].document;
}

void ThemedDocuments::UnloadDocuments()
{
    const bool anyLoaded = std::any_of(m_Slots.begin(), m_Slots.end(), [](const Slot& slot)
                                       { return slot.document != nullptr && IsContextAlive(slot.context); });
    if (anyLoaded && m_Options.beforeUnload)
        m_Options.beforeUnload();
    for (Slot& slot : m_Slots)
    {
        if (slot.document != nullptr && IsContextAlive(slot.context))
        {
            // UnloadDocument() drops the focus without a blur, so a focused field never releases
            // the keyboard and the client keeps believing text is being typed: no hotkeys.
            Rml::Element* focused = slot.context->GetFocusElement();
            if (focused != nullptr && focused->GetOwnerDocument() == slot.document)
                focused->Blur();
            slot.context->UnloadDocument(slot.document);
        }
        slot.document = nullptr;
        slot.context = nullptr;
    }
    m_Built = false;
}

void ThemedDocuments::ForgetModel(bool keepValues)
{
    if (!m_ModelCreated)
        return;
    DestroyModel(IsContextAlive(m_ModelContext) ? m_ModelContext : nullptr, keepValues);
    m_ModelCreated = false;
    m_ModelContext = nullptr;
}
} // namespace UI::RmlBridge
