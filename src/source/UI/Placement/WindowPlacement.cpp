#include "stdafx.h"

#include "UI/Placement/WindowPlacement.h"

#include "Data/GameConfig/GameConfig.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "UI/Core/UILayoutPolicy.h"
#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowSystem.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
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
// The docked panels' size in their own layout units: a content slot's size when its window has
// no document to read a #panel from.
constexpr float DefaultPanelWidth = 190.f;
constexpr float DefaultPanelHeight = 429.f;

struct Entry
{
    std::uint32_t windowId = 0;
    GetWindow getWindow;
    SetPosition setPosition;
    std::string document;
    PlacementParticipant participant;
    PlacementParticipant::Box box;
    bool hasBox = false;
    bool lastVisible = false;
    bool placed = false;
    bool warnedUnsupportedFill = false;
    POINT lastPosition{};
};

std::unordered_map<std::string, Entry> g_windows;
Rml::ElementDocument* g_workspace = nullptr;
bool g_namesChecked = false;
UI::Scaling::Transform g_lastDock{};
bool g_dirty = true;
// Regions the theme capped below their reference height, and the factor that makes them fit.
std::unordered_map<Rml::Element*, float> g_regionFit;
unsigned int g_lastWidth = 0;
unsigned int g_lastHeight = 0;
float g_uncoveredLeft = 0.f;
float g_uncoveredRight = 0.f;
const int g_themeReloadToken = 0;

void ReloadWorkspace()
{
    if (g_workspace != nullptr)
        g_workspace->Close();
    g_workspace = nullptr;
    g_namesChecked = false;
    for (auto& [name, entry] : g_windows)
    {
        entry.placed = false;
        entry.warnedUnsupportedFill = false;
    }
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

// A name the workspace uses but no window answers to does nothing, so say so once per workspace.
void CheckNames(const Rml::ElementList& slots)
{
    if (g_namesChecked || g_windows.empty())
        return;
    g_namesChecked = true;
    const auto check = [](const std::string& name, const char* attribute)
    {
        if (!name.empty() && g_windows.find(name) == g_windows.end())
            g_ErrorReport.Write(L"> [Placement] workspace.rml %hs names unknown window '%hs'.\r\n", attribute,
                                name.c_str());
    };
    for (Rml::Element* slot : slots)
    {
        check(slot->GetAttribute<Rml::String>("data-window", ""), "data-window");
        std::istringstream closes(slot->GetAttribute<Rml::String>("data-closes", ""));
        for (std::string name; closes >> name;)
            check(name, "data-closes");
    }
}

bool IsOpen(const Entry& entry)
{
    return entry.participant.visible ? entry.participant.visible()
        : g_pNewUISystem != nullptr && g_pNewUISystem->IsVisible(entry.windowId);
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

// The scale a region's windows are drawn at, chosen by its data-scale.
float BaseRegionScale(Rml::Element* region, const UI::Scaling::Transform& dock)
{
    const std::string scale = region != nullptr ? region->GetAttribute<Rml::String>("data-scale", "") : "";
    if (scale == "panel")
        return UI::Scaling::PanelTransform(WindowWidth, WindowHeight).scaleX;
    if (scale == "hud")
        return UI::Scaling::BottomHudScale(WindowWidth, WindowHeight);
    return dock.scaleX;
}

// The base scale, smaller where the theme caps the region below its reference height.
float RegionScale(Rml::Element* region, const UI::Scaling::Transform& dock)
{
    const auto fit = g_regionFit.find(region);
    return BaseRegionScale(region, dock) * (fit != g_regionFit.end() ? fit->second : 1.f);
}

// The window's own #panel size, in its layout units; the docked windows' size without one.
Rml::Vector2f PanelSize(const Entry& entry)
{
    if (entry.participant.measure)
    {
        const auto size = entry.participant.measure();
        return {size.width, size.height};
    }
    const Rml::Vector2f fallback{DefaultPanelWidth, DefaultPanelHeight};
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    if (entry.document.empty() || context == nullptr)
        return fallback;
    const std::string suffix = "/" + entry.document;
    for (int i = 0; i < context->GetNumDocuments(); ++i)
    {
        Rml::ElementDocument* document = context->GetDocument(i);
        const std::string& url = document->GetSourceURL();
        if (url.size() < suffix.size() || url.compare(url.size() - suffix.size(), suffix.size(), suffix) != 0)
            continue;
        Rml::Vector2f size = fallback;
        if (!UI::RmlBridge::RefreshLogicalPanelSize(document, "panel", size.x, size.y))
        {
            // Never shown yet: lay it out once to learn its size.
            document->UpdateDocument();
            UI::RmlBridge::RefreshLogicalPanelSize(document, "panel", size.x, size.y);
        }
        return size;
    }
    return fallback;
}

// A saved user position keeps a dragged window where the player put it while it is the first
// open window of its region; behind another window it rejoins the region's flow. Saved positions
// are in the original docked windows' space.
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

}

void Invalidate()
{
    g_dirty = true;
}

void RegisterParticipant(std::string_view name, PlacementParticipant participant)
{
    Entry& entry = g_windows[std::string(name)];
    entry = {};
    entry.participant = std::move(participant);
    g_namesChecked = false;
    Invalidate();
}

void UnregisterParticipant(std::string_view name)
{
    g_windows.erase(std::string(name));
    Invalidate();
}

bool SlotBox(std::string_view name, PlacementParticipant::Box& box)
{
    const auto found = g_windows.find(std::string(name));
    if (found == g_windows.end() || !found->second.hasBox)
        return false;
    box = found->second.box;
    return true;
}

void RegisterWindow(std::uint32_t windowId, std::string_view slotName, GetWindow getWindow, SetPosition setPosition,
                    const char* document)
{
    Entry& entry = g_windows[std::string(slotName)];
    entry = {};
    entry.windowId = windowId;
    entry.getWindow = std::move(getWindow);
    entry.setPosition = std::move(setPosition);
    entry.document = document != nullptr ? document : "";
}

void RegisterName(std::uint32_t windowId, std::string_view name)
{
    Entry& entry = g_windows[std::string(name)];
    entry = {};
    entry.windowId = windowId;
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

static void PrepareRegions(Rml::ElementDocument* workspace, const UI::Scaling::Transform& dock)
{
    // RCSS lays out the shell; only component sizes and region reference units enter from C++.
    for (auto& [name, entry] : g_windows)
        entry.hasBox = false;
    Rml::ElementList regions;
    workspace->QuerySelectorAll(regions, ".region");
    for (Rml::Element* region : regions)
    {
        const float referenceHeight = region->GetAttribute<float>("data-ref-height", 0.f);
        if (referenceHeight > 0.f)
            SetLength(region, Rml::PropertyId::Height, referenceHeight * RegionScale(region, dock));
    }

}

// A region whose theme caps it (max-height) below its reference height draws its windows at the
// scale that fits. True when a region changed, so the slots need sizing again.
static bool FitRegions(Rml::ElementDocument* workspace, const UI::Scaling::Transform& dock)
{
    bool changed = false;
    Rml::ElementList regions;
    workspace->QuerySelectorAll(regions, ".region");
    for (Rml::Element* region : regions)
    {
        const float referenceHeight = region->GetAttribute<float>("data-ref-height", 0.f);
        if (referenceHeight <= 0.f)
            continue;
        const float wanted = referenceHeight * BaseRegionScale(region, dock);
        const float resolved = region->GetBox().GetSize(Rml::BoxArea::Border).y;
        if (resolved > 0.f && resolved < wanted - 0.5f)
        {
            g_regionFit[region] = resolved / wanted;
            changed = true;
        }
    }
    return changed;
}

static Rml::ElementList PrepareSlots(Rml::ElementDocument* workspace, const UI::Scaling::Transform& dock)
{
    Rml::ElementList slots;
    workspace->QuerySelectorAll(slots, ".slot");
    CheckNames(slots);
    for (Rml::Element* slot : slots)
    {
        Entry* entry = EntryFor(slot);
        mu::ui::window::CObject* window = entry != nullptr && entry->getWindow ? entry->getWindow() : nullptr;
        const bool open = entry != nullptr && (window != nullptr || entry->participant.visible) && IsOpen(*entry);
        const bool wantsFill = slot->GetAttribute<Rml::String>("data-fit", "") == "fill";
        const bool fill = open && wantsFill && window != nullptr && window->SupportsFillPlacement();
        slot->SetClass("open", open);
        slot->SetClass("fill", fill);
        if (wantsFill && open && !fill && !entry->warnedUnsupportedFill)
        {
            g_ErrorReport.Write(L"> [Placement] window '%hs' does not support data-fit=fill.\r\n",
                                slot->GetAttribute<Rml::String>("data-window", "").c_str());
            entry->warnedUnsupportedFill = true;
        }
        if (window != nullptr)
            window->SetFillPlacementSize(0.f, 0.f);
        slot->RemoveProperty(Rml::PropertyId::MinWidth);
        slot->RemoveProperty(Rml::PropertyId::MinHeight);
        if (fill)
        {
            // The theme sizes a fill slot, but never below the window's content size.
            const float scale = RegionScale(slot->GetParentNode(), dock);
            Rml::Vector2f content = PanelSize(*entry);
            window->GetFillMinimumSize(content.x, content.y);
            slot->RemoveProperty(Rml::PropertyId::Width);
            slot->RemoveProperty(Rml::PropertyId::Height);
            SetLength(slot, Rml::PropertyId::MinWidth, content.x * scale);
            SetLength(slot, Rml::PropertyId::MinHeight, content.y * scale);
        }
        else if (open)
        {
            const float scale = RegionScale(slot->GetParentNode(), dock);
            const Rml::Vector2f size = PanelSize(*entry);
            SetLength(slot, Rml::PropertyId::Width, size.x * scale);
            SetLength(slot, Rml::PropertyId::Height, size.y * scale);
        }
    }

    return slots;
}

static void UpdateUncoveredArea(Rml::ElementDocument* workspace, const Rml::ElementList& slots)
{
    // Open slots in covering regions narrow the world from the side of the screen they are on.
    const float screenWidth = static_cast<float>(WindowWidth);
    Rml::Element* content = workspace->GetElementById("safe_area");
    g_uncoveredLeft = content != nullptr ? content->GetAbsoluteOffset(Rml::BoxArea::Border).x : 0.f;
    g_uncoveredRight = content != nullptr
        ? g_uncoveredLeft + content->GetBox().GetSize(Rml::BoxArea::Border).x : screenWidth;
    for (Rml::Element* slot : slots)
    {
        Rml::Element* region = slot->GetParentNode();
        if (!slot->IsClassSet("open") || region == nullptr || !region->HasAttribute("data-covers-world"))
            continue;
        const float left = slot->GetAbsoluteOffset(Rml::BoxArea::Border).x;
        const float right = left + slot->GetBox().GetSize(Rml::BoxArea::Border).x;
        if (left + right >= screenWidth)
            g_uncoveredRight = std::min(g_uncoveredRight, left);
        else
            g_uncoveredLeft = std::max(g_uncoveredLeft, right);
    }

}

static void PlaceSlots(const Rml::ElementList& slots, const UI::Scaling::Transform& dock)
{
    // Each open window's logical space becomes its slot: (0, 0) at the slot's top-left, at the
    // region's scale.
    for (Rml::Element* slot : slots)
    {
        Entry* entry = EntryFor(slot);
        mu::ui::window::CObject* window = entry != nullptr && entry->getWindow ? entry->getWindow() : nullptr;
        if (entry == nullptr)
            continue;
        if (!slot->IsClassSet("open"))
        {
            entry->placed = false;
            continue;
        }

        const float scale = RegionScale(slot->GetParentNode(), dock);
        const Rml::Vector2f offset = slot->GetAbsoluteOffset(Rml::BoxArea::Border);
        const Rml::Vector2f size = slot->GetBox().GetSize(Rml::BoxArea::Border);
        entry->box = {offset.x, offset.y, size.x, size.y, scale};
        entry->hasBox = true;
        if (entry->participant.place)
        {
            entry->participant.place(&entry->box);
            continue;
        }
        if (window == nullptr)
            continue;
        const UI::Scaling::Transform transform{scale, scale, offset.x, offset.y, scale};
        window->PlaceInSlot(transform);
        if (slot->IsClassSet("fill"))
        {
            const Rml::Vector2f size = slot->GetBox().GetSize(Rml::BoxArea::Border);
            window->SetFillPlacementSize(size.x / scale, size.y / scale);
        }

        POINT position{};
        POINT saved{};
        if (IsFirstOpenSlot(slot) && SavedPosition(slot, saved))
        {
            position.x = std::lround(
                UI::Scaling::LogicalX(transform, UI::Scaling::PositionX(dock, static_cast<float>(saved.x))));
            position.y = std::lround(
                UI::Scaling::LogicalY(transform, UI::Scaling::PositionY(dock, static_cast<float>(saved.y))));
        }

        // Only on change: a window dragged while behind another keeps its offset in the slot
        // until the arrangement itself changes.
        if (entry->placed && entry->lastPosition.x == position.x && entry->lastPosition.y == position.y)
            continue;
        entry->placed = true;
        entry->lastPosition = position;
        if (entry->setPosition)
            entry->setPosition(position.x, position.y);
    }

}

static void RestoreUnslotted(const Rml::ElementList& slots)
{
    // A registered component the workspace gives no slot keeps its own placement.
    for (auto& [name, entry] : g_windows)
    {
        if (entry.participant.place && !entry.hasBox)
            entry.participant.place(nullptr);
        mu::ui::window::CObject* window = entry.getWindow ? entry.getWindow() : nullptr;
        if (window == nullptr || window->GetLayoutMode() != UI::Scaling::LayoutMode::Slot)
            continue;
        const bool hasSlot =
            std::any_of(slots.begin(), slots.end(), [&](Rml::Element* slot) { return EntryFor(slot) == &entry; });
        if (!hasSlot)
        {
            window->SetLayoutMode(UI::Layout::ForInterface(entry.windowId));
            entry.placed = false;
        }
    }
}

static void ArrangeNow()
{
    Rml::ElementDocument* workspace = Workspace();
    if (workspace == nullptr)
        return;

    const auto dock = UI::Scaling::DockRightTransform(WindowWidth, WindowHeight);
    g_lastDock = dock;
    g_dirty = false;
    g_lastWidth = WindowWidth;
    g_lastHeight = WindowHeight;
    for (auto& [name, entry] : g_windows)
        entry.lastVisible = IsOpen(entry);

    g_regionFit.clear();
    PrepareRegions(workspace, dock);
    Rml::ElementList slots = PrepareSlots(workspace, dock);
    workspace->UpdateDocument();
    // The content area depends only on the shell, so one more pass sizes capped regions' windows.
    if (FitRegions(workspace, dock))
    {
        PrepareRegions(workspace, dock);
        slots = PrepareSlots(workspace, dock);
        workspace->UpdateDocument();
    }
    UpdateUncoveredArea(workspace, slots);
    PlaceSlots(slots, dock);
    RestoreUnslotted(slots);
}

void Arrange()
{
    // Immediate, so a window opened during this frame's input is drawn and hit-tested in place.
    ArrangeNow();
}

void Update()
{
    const auto dock = UI::Scaling::DockRightTransform(WindowWidth, WindowHeight);
    for (auto& [name, entry] : g_windows)
    {
        const bool visible = IsOpen(entry);
        if (visible != entry.lastVisible)
        {
            entry.lastVisible = visible;
            Invalidate();
        }
    }
    if (g_dirty || WindowWidth != g_lastWidth || WindowHeight != g_lastHeight || dock.scaleX != g_lastDock.scaleX ||
        dock.offsetY != g_lastDock.offsetY)
        ArrangeNow();
    for (const auto& [name, entry] : g_windows)
    {
        if (const mu::ui::window::CObject* window = entry.getWindow ? entry.getWindow() : nullptr)
            window->SyncFillPlacement();
    }
}

bool PanelSizeOf(std::uint32_t windowId, float& width, float& height)
{
    for (const auto& [name, entry] : g_windows)
    {
        if (entry.windowId != windowId || entry.document.empty())
            continue;
        const Rml::Vector2f size = PanelSize(entry);
        width = size.x;
        height = size.y;
        return true;
    }
    return false;
}

float UncoveredWorldLeft()
{
    return g_workspace != nullptr ? g_uncoveredLeft : 0.f;
}

float UncoveredWorldRight()
{
    return g_workspace != nullptr ? g_uncoveredRight : static_cast<float>(WindowWidth);
}

bool InitialPosition(std::string_view slotName, float width, float height, float& x, float& y)
{
    Rml::ElementDocument* workspace = Workspace();
    if (workspace == nullptr)
        return false;

    Rml::ElementList slots;
    workspace->QuerySelectorAll(slots, ".slot");
    for (Rml::Element* slot : slots)
    {
        if (slot->GetAttribute<Rml::String>("data-window", "") != slotName)
            continue;
        // Laid out only to be read: the window floats, so it takes no room in its region.
        slot->SetClass("open", true);
        SetLength(slot, Rml::PropertyId::Width, width);
        SetLength(slot, Rml::PropertyId::Height, height);
        workspace->UpdateDocument();
        const Rml::Vector2f offset = slot->GetAbsoluteOffset(Rml::BoxArea::Border);
        x = offset.x;
        y = offset.y;
        slot->SetClass("open", false);
        slot->RemoveProperty(Rml::PropertyId::Width);
        slot->RemoveProperty(Rml::PropertyId::Height);
        workspace->UpdateDocument();
        return true;
    }
    return false;
}

float UncoveredWorldLeftIn(const UI::Scaling::Transform& transform)
{
    return (UncoveredWorldLeft() - transform.offsetX) / transform.scaleX;
}

float UncoveredWorldRightIn(const UI::Scaling::Transform& transform)
{
    return (UncoveredWorldRight() - transform.offsetX) / transform.scaleX;
}

void Release()
{
    UI::RmlBridge::UnregisterForThemeReload(&g_themeReloadToken);
    if (g_workspace != nullptr)
        g_workspace->Close();
    g_workspace = nullptr;
    g_windows.clear();
    g_dirty = true;
}
}
