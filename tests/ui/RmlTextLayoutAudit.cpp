#include "stdafx.h"
#include "RmlTextLayoutAudit.h"
#include "UI/RmlBridge/RmlElementBox.h"
#include "UI/Scaling/UITransform.h"
#include <doctest.h>
#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/FontEngineInterface.h>
#include <RmlUi/Core/Mesh.h>
#include <RmlUi/Core/TextShapingContext.h>
#include <algorithm>
#include <limits>
#include <iostream>

namespace
{
constexpr float PixelTolerance = 1.f;
constexpr const char* Controls = "input, .checkbox-box, .radio-box, .mh-slot, .mh-tab, .mh-btn-setting, "
    ".mh-btn-igs, .mh-btn-plus, .mh-btn-minus, .entry-btn, .shop-btn, .shop-btn-exit, #btn_exit, #frame_corner_close";

struct Bounds
{
    float left = 0.f, top = 0.f, right = 0.f, bottom = 0.f;
};

Bounds Box(Rml::Element& element)
{
    Rml::Vector2f point, size;
    UI::RmlBridge::DrawnBox(element, Rml::BoxArea::Content, point, size);
    return {point.x, point.y, point.x + size.x, point.y + size.y};
}

bool Overlaps(const Bounds& a, const Bounds& b)
{
    if (a.right <= a.left || a.bottom <= a.top || b.right <= b.left || b.bottom <= b.top)
        return false;
    return a.right > b.left + PixelTolerance && b.right > a.left + PixelTolerance &&
           a.bottom > b.top + PixelTolerance && b.bottom > a.top + PixelTolerance;
}

bool Contains(const Bounds& box, const Bounds& ink)
{
    return ink.left >= box.left - PixelTolerance && ink.right <= box.right + PixelTolerance &&
        ink.top >= box.top - PixelTolerance && ink.bottom <= box.bottom + PixelTolerance;
}

bool Visible(Rml::Element& element)
{
    for (auto* current = &element; current != nullptr; current = current->GetParentNode())
        if (current->GetComputedValues().display() == Rml::Style::Display::None)
            return false;
    return true;
}

std::string Name(Rml::Element& element)
{
    if (!element.GetId().empty())
        return "#" + element.GetId();
    auto* parent = element.GetParentNode();
    const std::string prefix = parent ? Name(*parent) + "/" : "";
    int index = 0;
    while (parent && parent->GetChild(index) != &element)
        ++index;
    return prefix + element.GetTagName() + "[" + std::to_string(index) + "]";
}

std::string Csv(const std::string& text)
{
    std::string escaped = "\"";
    for (char c : text)
        escaped += c == '"' ? "\"\"" : std::string(1, c);
    return escaped + "\"";
}

void WriteBounds(std::ostream& output, const Bounds& box)
{
    output << box.left << ',' << box.top << ',' << box.right - box.left << ',' << box.bottom - box.top;
}

void WriteScenario(std::ostream& output, const UI::Tests::RmlTextLayoutAudit::Scenario& s)
{
    output << s.theme << ',' << s.window << ',' << s.locale << ',' << s.width << ',' << s.height << ','
           << s.percent << ',' << s.contentScale << ',' << s.tab << ',' << s.character;
}

Bounds Ink(Rml::ElementText& text, const Rml::ElementText::Line& line)
{
    const auto& style = text.GetComputedValues();
    const Rml::TextShapingContext shaping{style.language(), style.direction(), style.font_kerning(), style.letter_spacing()};
    Rml::TexturedMeshList meshes;
    Rml::GetFontEngineInterface()->GenerateString(text.GetContext()->GetRenderManager(), text.GetFontFaceHandle(), 0,
        line.text, line.position, {255, 255, 255, 255}, 1.f, shaping, meshes);
    Bounds bounds{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                  std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};
    Rml::Vector2f origin;
    REQUIRE(UI::RmlBridge::DrawnTopLeft(text, origin));
    const float scale = UI::RmlBridge::DrawnScale(text);
    for (const auto& mesh : meshes)
    for (const auto& vertex : mesh.mesh.vertices)
    {
        const auto point = origin + vertex.position * scale;
        bounds.left = std::min(bounds.left, point.x);
        bounds.top = std::min(bounds.top, point.y);
        bounds.right = std::max(bounds.right, point.x);
        bounds.bottom = std::max(bounds.bottom, point.y);
    }
    return bounds;
}

bool OwnControl(Rml::Element& label, Rml::Element& control)
{
    if (&control == label.GetParentNode())
        return true;
    return !control.GetId().empty() && label.GetId() == control.GetId() + "_label";
}

// The box RmlUi clips an overflowing element's children to (its padding box unless changed).
Bounds ClipBox(Rml::Element& element)
{
    Rml::Vector2f point, size;
    UI::RmlBridge::DrawnBox(element, element.GetClipArea(), point, size);
    return {point.x, point.y, point.x + size.x, point.y + size.y};
}

std::string ClipReason(Rml::Element& text, const Bounds& ink)
{
    for (auto* parent = text.GetParentNode(); parent != nullptr; parent = parent->GetParentNode())
    {
        const Bounds clip = ClipBox(*parent);
        const auto& style = parent->GetComputedValues();
        if (style.overflow_x() != Rml::Style::Overflow::Visible &&
            (ink.left < clip.left - PixelTolerance || ink.right > clip.right + PixelTolerance))
            return "clip-x:" + Name(*parent);
        // Vertical scrolling intentionally clips offscreen lines; reachability is checked separately.
        if (style.overflow_y() == Rml::Style::Overflow::Hidden &&
            (ink.top < clip.top - PixelTolerance || ink.bottom > clip.bottom + PixelTolerance))
            return "clip-y:" + Name(*parent);
    }
    return "";
}

Bounds PaintedBounds(Rml::Element& element, Bounds bounds)
{
    for (auto* parent = element.GetParentNode(); parent != nullptr; parent = parent->GetParentNode())
    {
        const Bounds clip = ClipBox(*parent);
        const auto& style = parent->GetComputedValues();
        if (style.overflow_x() != Rml::Style::Overflow::Visible)
        {
            bounds.left = std::max(bounds.left, clip.left);
            bounds.right = std::min(bounds.right, clip.right);
        }
        if (style.overflow_y() != Rml::Style::Overflow::Visible)
        {
            bounds.top = std::max(bounds.top, clip.top);
            bounds.bottom = std::min(bounds.bottom, clip.bottom);
        }
    }
    return bounds;
}

bool ControlCollision(Rml::Element& label, const Bounds& ink, const Rml::ElementList& controls,
    float& right, std::string& neighbor, Bounds& neighborBox)
{
    for (auto* control : controls)
    {
        if (!Visible(*control) || OwnControl(label, *control))
            continue;
        const Bounds box = PaintedBounds(*control, Box(*control));
        if (box.left >= ink.left && box.bottom > ink.top && box.top < ink.bottom)
            right = std::min(right, box.left);
        if (!Overlaps(ink, box))
            continue;
        neighbor = Name(*control);
        neighborBox = box;
        return true;
    }
    return false;
}

bool TextCollision(Rml::ElementText& text, const Bounds& ink, const Rml::ElementList& texts,
    std::string& neighbor, Bounds& neighborBox)
{
    for (auto* element : texts)
    {
        auto* other = rmlui_dynamic_cast<Rml::ElementText*>(element);
        if (other == &text)
            continue;
        for (const auto& line : other->GetLines())
        {
            if (line.text.empty() || line.text.find("___") != std::string::npos)
                continue;
            const Bounds box = PaintedBounds(*other, Ink(*other, line));
            if (!Overlaps(ink, box))
                continue;
            neighbor = Name(*other->GetParentNode());
            neighborBox = box;
            return true;
        }
    }
    return false;
}

void AddReason(std::string& reasons, const char* reason)
{
    if (!reasons.empty()) reasons += ';';
    reasons += reason;
}

bool ExplicitOverflow(Rml::Element& label, const Bounds& box, const Bounds& ink)
{
    const auto& style = label.GetComputedValues();
    // Auto-sized, overflow-visible spans can legitimately have a zero-width layout box.
    const bool horizontal = style.width().type != Rml::Style::LengthPercentageAuto::Auto &&
        (ink.left < box.left - PixelTolerance || ink.right > box.right + PixelTolerance);
    const bool vertical = style.height().type != Rml::Style::LengthPercentageAuto::Auto &&
        (ink.top < box.top - PixelTolerance || ink.bottom > box.bottom + PixelTolerance);
    return horizontal || vertical;
}
} // namespace

UI::Tests::RmlTextLayoutAudit::RmlTextLayoutAudit(const std::filesystem::path& directory)
    : m_Lines(directory / "text-lines.csv"), m_Scenarios(directory / "scenarios.csv")
{
    REQUIRE(m_Lines.is_open());
    REQUIRE(m_Scenarios.is_open());
    const char* scenario = "theme,window,locale,width,height,ui_percent,os_scale,tab,class";
    m_Lines << scenario << ",element,line,font_px,drawn_font_px,ink_x,ink_y,ink_w,ink_h,"
        "element_x,element_y,element_w,element_h,available_w,available_h,reason,neighbor,"
        "neighbor_x,neighbor_y,neighbor_w,neighbor_h,text\n";
    m_Scenarios << scenario << ",native_normal_px,normal_floor_px,source_complete,failures\n";
}

void UI::Tests::RmlTextLayoutAudit::InspectText(Rml::ElementText& text, Rml::Element& panel,
    const Rml::ElementList& controls, const Rml::ElementList& texts, const Scenario& scenario)
{
    auto* label = text.GetParentNode();
    REQUIRE(label != nullptr);
    const Bounds available = Box(*label), panelBox = Box(panel);
    int index = 0;
    for (const auto& line : text.GetLines())
    {
        if (line.text.empty() || line.text.find("___") != std::string::npos)
            continue;
        const Bounds ink = Ink(text, line);
        const Bounds painted = PaintedBounds(text, ink);
        std::string reason = ClipReason(text, ink), neighbor;
        Bounds neighborBox;
        float right = label->GetComputedValues().width().type == Rml::Style::LengthPercentageAuto::Auto
            ? panelBox.right : std::min(available.right, panelBox.right);
        if (ControlCollision(*label, painted, controls, right, neighbor, neighborBox))
            AddReason(reason, "control-overlap");
        else if (TextCollision(text, painted, texts, neighbor, neighborBox))
            AddReason(reason, "text-overlap");
        if (ExplicitOverflow(*label, available, ink)) AddReason(reason, "element-overflow");
        if (painted.right > painted.left && painted.bottom > painted.top && !Contains(panelBox, painted))
            AddReason(reason, "panel-overflow");
        if (!reason.empty())
        {
            ++m_Failures[scenario.window];
            ++m_ElementFailures[{scenario.window, Name(*label)}];
        }
        WriteScenario(m_Lines, scenario);
        m_Lines << ',' << Csv(Name(*label)) << ',' << index++ << ',' << text.GetComputedValues().font_size()
            << ',' << text.GetComputedValues().font_size() * UI::RmlBridge::DrawnScale(text) << ',';
        WriteBounds(m_Lines, ink); m_Lines << ','; WriteBounds(m_Lines, available);
        m_Lines << ',' << std::max(0.f, right - available.left) << ',' << available.bottom - available.top
            << ',' << Csv(reason) << ',' << Csv(neighbor) << ',';
        WriteBounds(m_Lines, neighborBox); m_Lines << ',' << Csv(line.text) << '\n';
    }
}

void UI::Tests::RmlTextLayoutAudit::Inspect(Rml::ElementDocument& document, const Scenario& scenario)
{
    auto* panel = document.GetElementById("panel");
    REQUIRE(panel != nullptr);
    const size_t before = FailureCount(scenario.window);
    const auto* description = document.QuerySelector("[data-audit-complete]");
    const bool complete = description == nullptr || description->GetAttribute<bool>("data-audit-complete", false);
    if (!complete)
        ++m_Failures[scenario.window];
    Rml::ElementList controls;
    panel->QuerySelectorAll(controls, Controls);
    Rml::ElementList texts;
    const auto visit = [&](auto&& self, Rml::Element& element) -> void
    {
        if (!Visible(element))
            return;
        if (auto* text = rmlui_dynamic_cast<Rml::ElementText*>(&element))
            texts.push_back(text);
        for (int i = 0; i < element.GetNumChildren(); ++i)
            self(self, *element.GetChild(i));
    };
    visit(visit, *panel);
    for (auto* element : texts)
        InspectText(*rmlui_dynamic_cast<Rml::ElementText*>(element), *panel, controls, texts, scenario);
    WriteScenario(m_Scenarios, scenario);
    m_Scenarios << ',' << UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Normal, scenario.width, scenario.height)
        << ',' << UI::Scaling::MinimumTextPixelSize(UI::Scaling::FontRole::Normal)
        << ',' << complete << ',' << FailureCount(scenario.window) - before << '\n';
    ++m_ScenarioCount;
}

size_t UI::Tests::RmlTextLayoutAudit::FailureCount(const std::string& window) const
{
    const auto it = m_Failures.find(window);
    return it == m_Failures.end() ? 0 : it->second;
}

size_t UI::Tests::RmlTextLayoutAudit::FailureCount(const std::string& window, const std::string& element) const
{
    const auto it = m_ElementFailures.find({window, element});
    return it == m_ElementFailures.end() ? 0 : it->second;
}

void UI::Tests::RmlTextLayoutAudit::Finish(size_t expectedScenarios)
{
    CHECK(m_ScenarioCount == expectedScenarios);
    m_Lines.flush();
    m_Scenarios.flush();
    REQUIRE(m_Lines.good());
    REQUIRE(m_Scenarios.good());
    std::cout << "Text layout diagnostic: " << m_ScenarioCount << " scenarios\n";
    for (const auto& [window, failures] : m_Failures)
        std::cout << window << ": " << failures << " failing text lines\n";
}
