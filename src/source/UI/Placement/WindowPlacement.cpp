#include "stdafx.h"

#include "UI/Placement/WindowPlacement.h"

#include "Data/GameConfig/GameConfig.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "UI/Core/UILayoutPolicy.h"
#include "UI/Core/WindowSystem.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>
#include <unordered_map>

namespace UI::Placement
{
namespace
{
// The docked panels' size in their own layout units; a content-fit slot takes this times the
// region's scale.
constexpr float PanelWidth = 190.f;
constexpr float PanelHeight = 429.f;

struct Entry
{
    std::uint32_t windowId;
    SetPosition setPosition;
    bool placed = false;
    POINT lastPosition{};
};

std::unordered_map<std::string, Entry> g_windows;
Rml::ElementDocument* g_workspace = nullptr;
struct Reserve
{
    float top = 0.f;
    float bottom = 0.f;
};

UI::Scaling::Transform g_lastDock{};
Reserve g_lastReserve{};
unsigned int g_lastWidth = 0;
unsigned int g_lastHeight = 0;
float g_uncoveredRight = 0.f;
const int g_themeReloadToken = 0;

void ReloadWorkspace()
{
    if (g_workspace != nullptr)
        g_workspace->Close();
    g_workspace = nullptr;
    for (auto& [name, entry] : g_windows)
        entry.placed = false;
    Arrange();
}

Rml::ElementDocument* Workspace()
{
    if (g_workspace != nullptr || !RmlUiRuntime::Instance().IsCreated())
        return g_workspace;
    // Never shown, so it is never drawn or hit; Arrange() lays it out itself.
    g_workspace =
        UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/workspace.rml");
    UI::RmlBridge::RegisterForThemeReload(&g_themeReloadToken, ReloadWorkspace);
    return g_workspace;
}

bool IsOpen(const Entry& entry)
{
    return g_pNewUISystem != nullptr && g_pNewUISystem->IsVisible(entry.windowId);
}

// A saved user position keeps a dragged window where the player put it while it is the first
// open window of its region; behind another window it rejoins the region's flow.
bool SavedPosition(Rml::Element* slot, POINT& position)
{
    const std::string key = slot->GetAttribute<Rml::String>("data-saved-position", "");
    if (key.empty())
        return false;
    int x = 0;
    int y = 0;
    if (!GameConfig::GetInstance().GetWindowPosition(std::wstring(key.begin(), key.end()), x, y))
        return false;
    position = {x, y};
    return true;
}

bool IsFirstOpenSlot(Rml::Element* slot)
{
    Rml::Element* region = slot->GetParentNode();
    for (int i = 0; region != nullptr && i < region->GetNumChildren(); ++i)
    {
        Rml::Element* sibling = region->GetChild(i);
        if (sibling->IsClassSet("open"))
            return sibling == slot;
    }
    return false;
}

Entry* EntryFor(Rml::Element* slot)
{
    const auto it = g_windows.find(slot->GetAttribute<Rml::String>("data-window", ""));
    return it == g_windows.end() ? nullptr : &it->second;
}

void SetLength(Rml::Element* element, Rml::PropertyId property, float px)
{
    element->SetProperty(property, Rml::Property(px, Rml::Unit::PX));
}

// The screen edge the HUD strip sits on is kept free, so docks follow a theme that moves the HUD.
Reserve HudReserve(const UI::Scaling::Transform& dock)
{
    const float height = static_cast<float>(WindowHeight);
    float left = 0.f, top = 0.f, right = 0.f, bottom = 0.f;
    if (g_pMainFrame != nullptr && g_pMainFrame->GetStripRect(left, top, right, bottom))
    {
        if (bottom >= height - 2.f)
            return {0.f, height - top};
        if (top <= 2.f)
            return {bottom, 0.f};
    }
    // HUD not on screen: keep the original strip's place, as before.
    return {0.f, height - (dock.offsetY + UI::Scaling::DockLogicalBottom * dock.scaleY)};
}

bool SameReserve(const Reserve& a, const Reserve& b)
{
    return a.top == b.top && a.bottom == b.bottom;
}
}

void RegisterWindow(std::uint32_t windowId, std::string_view slotName, SetPosition setPosition)
{
    g_windows[std::string(slotName)] = {windowId, std::move(setPosition)};
}

void CloseForOpening(std::uint32_t windowId)
{
    Rml::ElementDocument* workspace = Workspace();
    if (workspace == nullptr || g_pNewUISystem == nullptr)
        return;

    Rml::ElementList slots;
    workspace->QuerySelectorAll(slots, ".slot");
    for (Rml::Element* slot : slots)
    {
        const Entry* opening = EntryFor(slot);
        if (opening == nullptr || opening->windowId != windowId)
            continue;

        std::istringstream names(slot->GetAttribute<Rml::String>("data-closes", ""));
        for (std::string name; names >> name;)
        {
            const auto it = g_windows.find(name);
            if (it != g_windows.end() && g_pNewUISystem->IsVisible(it->second.windowId))
                g_pNewUISystem->Hide(it->second.windowId);
        }
        return;
    }
}

void Arrange()
{
    Rml::ElementDocument* workspace = Workspace();
    if (workspace == nullptr)
        return;

    const auto dock = UI::Scaling::DockRightTransform(WindowWidth, WindowHeight);
    const Reserve reserve = HudReserve(dock);
    g_lastDock = dock;
    g_lastReserve = reserve;
    g_lastWidth = WindowWidth;
    g_lastHeight = WindowHeight;

    // The area the HUD leaves, and region lengths authored in the docked windows' units.
    if (Rml::Element* safeArea = workspace->GetElementById("safe_area"))
    {
        SetLength(safeArea, Rml::PropertyId::Top, reserve.top);
        SetLength(safeArea, Rml::PropertyId::Bottom, reserve.bottom);
    }
    Rml::ElementList regions;
    workspace->QuerySelectorAll(regions, ".region");
    for (Rml::Element* region : regions)
    {
        const float referenceHeight = region->GetAttribute<float>("data-ref-height", 0.f);
        if (referenceHeight > 0.f)
            SetLength(region, Rml::PropertyId::Height, referenceHeight * dock.scaleY);
    }

    Rml::ElementList slots;
    workspace->QuerySelectorAll(slots, ".slot");
    for (Rml::Element* slot : slots)
    {
        const Entry* entry = EntryFor(slot);
        const bool open = entry != nullptr && IsOpen(*entry);
        slot->SetClass("open", open);
        if (open)
        {
            SetLength(slot, Rml::PropertyId::Width, PanelWidth * dock.scaleX);
            SetLength(slot, Rml::PropertyId::Height, PanelHeight * dock.scaleY);
        }
    }

    workspace->UpdateDocument();

    g_uncoveredRight = static_cast<float>(WindowWidth);
    for (Rml::Element* slot : slots)
    {
        Rml::Element* region = slot->GetParentNode();
        if (slot->IsClassSet("open") && region != nullptr && region->HasAttribute("data-covers-world"))
            g_uncoveredRight = std::min(g_uncoveredRight, slot->GetAbsoluteOffset(Rml::BoxArea::Border).x);
    }

    for (Rml::Element* slot : slots)
    {
        Entry* entry = EntryFor(slot);
        if (entry == nullptr)
            continue;
        if (!slot->IsClassSet("open"))
        {
            entry->placed = false;
            continue;
        }

        POINT position{};
        if (!(IsFirstOpenSlot(slot) && SavedPosition(slot, position)))
        {
            const auto layout = UI::Scaling::TransformForLayout(UI::Layout::ForInterface(entry->windowId), WindowWidth,
                                                                WindowHeight);
            const Rml::Vector2f offset = slot->GetAbsoluteOffset(Rml::BoxArea::Border);
            position.x = std::lround((offset.x - layout.offsetX) / layout.scaleX);
            position.y = std::lround((offset.y - layout.offsetY) / layout.scaleY);
        }

        // Only on change: a window dragged while behind another stays where it was dropped
        // until the arrangement itself changes.
        if (entry->placed && entry->lastPosition.x == position.x && entry->lastPosition.y == position.y)
            continue;
        entry->placed = true;
        entry->lastPosition = position;
        if (entry->setPosition)
            entry->setPosition(position.x, position.y);
    }
}

void Update()
{
    const auto dock = UI::Scaling::DockRightTransform(WindowWidth, WindowHeight);
    if (WindowWidth != g_lastWidth || WindowHeight != g_lastHeight || dock.scaleX != g_lastDock.scaleX ||
        dock.offsetY != g_lastDock.offsetY || !SameReserve(HudReserve(dock), g_lastReserve))
        Arrange();
}

float UncoveredWorldRight()
{
    return g_workspace != nullptr ? g_uncoveredRight : static_cast<float>(WindowWidth);
}

void Release()
{
    UI::RmlBridge::UnregisterForThemeReload(&g_themeReloadToken);
    if (g_workspace != nullptr)
        g_workspace->Close();
    g_workspace = nullptr;
    g_windows.clear();
}
}
