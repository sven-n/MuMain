#include "stdafx.h"

#ifdef _EDITOR

#include "LookFields.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "Data/GameData/Common/MeshLimits.h"
#include "I18N/All.h"
#include "Render/Items/LookDrawing.h"
#include "imgui.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace MuEditor::Looks::Fields
{
namespace
{
using namespace Data::Looks;

// Pixel widths before the editor UI scale.
constexpr float ComboWidth = 190.0f;
constexpr float NumberWidth = 110.0f;
constexpr float GraphWidth = 300.0f;
constexpr float GraphHeight = 60.0f;
constexpr int GraphSamples = 300;
// The meshes a look names, as the file reads them.
constexpr int LastMesh = Data::MaxModelMeshes - 1;
constexpr int MaxFrames = 1000;
// How far a drag field moves per pixel (design L42): a value of 0 to 1 by 1 in 200 pixels, seconds and counts faster.
constexpr float ValueSpeed = 0.005f;
constexpr float SecondsSpeed = 0.02f;
constexpr float CountSpeed = 0.1f;
// Seconds stay above 0: a shape divides by them. A random value's step too, which the file refuses at 0 or less.
constexpr double MinSeconds = 0.001;
constexpr double MinRandomStep = 0.001;
// The top of a drag field kept above a bound: as good as none.
constexpr double MaxNumber = 1.0e9;
// A tooltip's lines wrap at this many font heights.
constexpr float TipWidth = 28.0f;

// The preview's clock, in seconds, which the curves mark (design L43).
double s_curveTime = 0.0;

float Scaled(float width)
{
    return width * g_MuEditorCore.GetUIScale();
}

// What the item just drawn does (design L42), shown while the mouse rests on its name or on it; `drag` adds how a drag
// field is dragged and typed into.
void Tip(const char* text, bool drag = false)
{
    if (!ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip) || !ImGui::BeginTooltip())
    {
        return;
    }
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * TipWidth);
    ImGui::TextUnformatted(text);
    if (drag)
    {
        ImGui::TextDisabled("%s", I18N::Editor::DragFieldTooltip);
    }
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
}

template <typename T> using Names = std::vector<std::pair<T, const char* const*>>;

// The name, or the tip, a list gives the value; nullptr without one.
template <typename T> const char* const* Find(const Names<T>* names, T value)
{
    if (names != nullptr)
    {
        for (const auto& [choice, name] : *names)
        {
            if (choice == value)
            {
                return name;
            }
        }
    }
    return nullptr;
}

// A combo of named choices, translated when drawn, with what it does as its tooltip and, in `tips`, what each choice
// does.
template <typename T>
bool Choice(const char* label, T& value, const Names<T>& names, const char* tip, const Names<T>* tips = nullptr)
{
    const char* const* name = Find(&names, value);
    bool changed = false;
    ImGui::SetNextItemWidth(Scaled(ComboWidth));
    const bool open = ImGui::BeginCombo(label, name != nullptr ? *name : "");
    if (!open)
    {
        Tip(tip);
        return false;
    }
    for (const auto& [choice, choiceName] : names)
    {
        if (ImGui::Selectable(*choiceName, choice == value) && choice != value)
        {
            value = choice;
            changed = true;
        }
        if (const char* const* choiceTip = Find(tips, choice))
        {
            Tip(*choiceTip);
        }
    }
    ImGui::EndCombo();
    return changed;
}

// A number dragged sideways or typed (design L42); `atLeast` keeps it above a bound.
bool DragNumber(const char* label, double& value, float speed, const char* tip, std::optional<double> atLeast = {})
{
    ImGui::SetNextItemWidth(Scaled(NumberWidth));
    const double low = atLeast.value_or(0.0);
    const double high = MaxNumber;
    const bool changed = atLeast
                             ? ImGui::DragScalar(label, ImGuiDataType_Double, &value, speed, &low, &high, "%.7g",
                                                 ImGuiSliderFlags_AlwaysClamp)
                             : ImGui::DragScalar(label, ImGuiDataType_Double, &value, speed, nullptr, nullptr, "%.7g");
    Tip(tip, true);
    return changed;
}

const Names<LayerKind>& KindNames()
{
    static const Names<LayerKind> names{{LayerKind::Texture, &I18N::Editor::Texture},
                                        {LayerKind::Reflection, &I18N::Editor::Reflection},
                                        {LayerKind::FlatColor, &I18N::Editor::FlatColor}};
    return names;
}

const Names<Motion>& MotionNames()
{
    static const Names<Motion> names{
        {Motion::SlowDrift, &I18N::Editor::SlowDrift},          {Motion::FastSweep, &I18N::Editor::FastSweep},
        {Motion::FollowsLight, &I18N::Editor::FollowsTheLight}, {Motion::Swirl, &I18N::Editor::Swirl},
        {Motion::StrongSwirl, &I18N::Editor::StrongSwirl},      {Motion::Stripes, &I18N::Editor::Stripes},
        {Motion::VerySlowDrift, &I18N::Editor::VerySlowDrift},  {Motion::OilSheen, &I18N::Editor::OilSheen},
        {Motion::StillShine, &I18N::Editor::StillShine}};
    return names;
}

const Names<Where>& WhereNames()
{
    static const Names<Where> names{{Where::Over, &I18N::Editor::OverTheTexture},
                                    {Where::UnderHoles, &I18N::Editor::UnderTheHoles}};
    return names;
}

const Names<Blend>& BlendNames()
{
    static const Names<Blend> names{{Blend::Normal, &I18N::Editor::Normal},
                                    {Blend::Glow, &I18N::Editor::Glow},
                                    {Blend::AsDrawing, &I18N::Editor::AsTheDrawingAsks}};
    return names;
}

const Names<Blend>& BlendTips()
{
    static const Names<Blend> tips{{Blend::Normal, &I18N::Editor::BlendNormalTooltip},
                                   {Blend::Glow, &I18N::Editor::BlendGlowTooltip},
                                   {Blend::AsDrawing, &I18N::Editor::BlendAsTheDrawingAsksTooltip}};
    return tips;
}

// Where a value comes from, as its combo offers it.
enum class Source
{
    Item,
    Drawing,
    Number,
    Wave,
    Bounce,
    Scroll,
    Ramp,
    Frames,
    Random,
    Zigzag,
    Turn,
};

const Names<Source>& SourceNames()
{
    static const Names<Source> names{
        {Source::Item, &I18N::Editor::FromTheItem},   {Source::Drawing, &I18N::Editor::FromTheDrawing},
        {Source::Number, &I18N::Editor::ANumber},     {Source::Wave, &I18N::Editor::Wave},
        {Source::Bounce, &I18N::Editor::Bounce},      {Source::Scroll, &I18N::Editor::Scroll},
        {Source::Ramp, &I18N::Editor::Ramp},          {Source::Frames, &I18N::Editor::Frames},
        {Source::Random, &I18N::Editor::RandomValue}, {Source::Zigzag, &I18N::Editor::Zigzag},
        {Source::Turn, &I18N::Editor::TurningAngle}};
    return names;
}

const Names<Source>& SourceTips()
{
    static const Names<Source> tips{
        {Source::Item, &I18N::Editor::FromTheItemTooltip},   {Source::Drawing, &I18N::Editor::FromTheDrawingTooltip},
        {Source::Number, &I18N::Editor::ANumberTooltip},     {Source::Wave, &I18N::Editor::WaveTooltip},
        {Source::Bounce, &I18N::Editor::BounceTooltip},      {Source::Scroll, &I18N::Editor::ScrollTooltip},
        {Source::Ramp, &I18N::Editor::RampTooltip},          {Source::Frames, &I18N::Editor::FramesTooltip},
        {Source::Random, &I18N::Editor::RandomValueTooltip}, {Source::Zigzag, &I18N::Editor::ZigzagTooltip},
        {Source::Turn, &I18N::Editor::TurningAngleTooltip}};
    return tips;
}

Source SourceOf(ShapeKind kind)
{
    switch (kind)
    {
    case ShapeKind::Bounce:
        return Source::Bounce;
    case ShapeKind::Scroll:
        return Source::Scroll;
    case ShapeKind::Ramp:
        return Source::Ramp;
    case ShapeKind::Frames:
        return Source::Frames;
    case ShapeKind::Random:
        return Source::Random;
    case ShapeKind::Zigzag:
        return Source::Zigzag;
    case ShapeKind::Turn:
        return Source::Turn;
    case ShapeKind::Wave:
    default:
        return Source::Wave;
    }
}

// A new shape of the kind, from `from` to `to` where it has them.
Shape NewShape(Source source, double from, double to)
{
    switch (source)
    {
    case Source::Bounce:
        return {.kind = ShapeKind::Bounce, .from = from, .to = to, .seconds = 1.0};
    case Source::Scroll:
        return {.kind = ShapeKind::Scroll, .perSecond = 0.5};
    case Source::Ramp:
        return {.kind = ShapeKind::Ramp, .from = from, .to = to, .seconds = 1.0};
    case Source::Zigzag:
        return {.kind = ShapeKind::Zigzag, .from = from, .to = to, .seconds = 1.0};
    case Source::Turn:
        return {.kind = ShapeKind::Turn, .from = from, .to = to};
    case Source::Frames:
        return {.kind = ShapeKind::Frames, .seconds = 0.1, .step = 0.25, .count = 4};
    case Source::Random:
        return {.kind = ShapeKind::Random, .from = from, .to = std::max(from, to), .step = 0.1};
    case Source::Wave:
    default:
        return {.kind = ShapeKind::Wave, .from = from, .to = to, .seconds = 1.0};
    }
}

bool SecondsField(const char* label, double& seconds, const char* tip)
{
    return DragNumber(label, seconds, SecondsSpeed, tip, MinSeconds);
}

// The curve of a shape (design L43): ten seconds of the preview's clock with fixed axes, so a change of its numbers
// changes the curve, the numbers at its edges, a line at 0 and a mark at the preview's time.
void RenderCurve(const Shape& shape)
{
    if (shape.kind == ShapeKind::Random)
    {
        ImGui::TextDisabled("%s", I18N::Editor::ANewValueEachFrame);
        return;
    }
    const CurveWindow window = CurveWindowOf(shape, s_curveTime);
    std::array<float, GraphSamples> values{};
    for (int sample = 0; sample < GraphSamples; ++sample)
    {
        values[static_cast<std::size_t>(sample)] =
            static_cast<float>(CurveValueAt(shape, window, CurveSpan * sample / (GraphSamples - 1)));
    }
    ImGui::PlotLines("##Curve", values.data(), GraphSamples, 0, nullptr, static_cast<float>(window.low),
                     static_cast<float>(window.high), ImVec2(Scaled(GraphWidth), Scaled(GraphHeight)));
    // Inside the plot's frame, as PlotLines draws the line.
    const ImVec2 padding = ImGui::GetStyle().FramePadding;
    const ImVec2 min(ImGui::GetItemRectMin().x + padding.x, ImGui::GetItemRectMin().y + padding.y);
    const ImVec2 max(ImGui::GetItemRectMax().x - padding.x, ImGui::GetItemRectMax().y - padding.y);
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const auto yOf = [&](double value)
    { return max.y - (max.y - min.y) * static_cast<float>((value - window.low) / (window.high - window.low)); };
    if (window.low < 0.0 && window.high > 0.0)
    {
        draw->AddLine(ImVec2(min.x, yOf(0.0)), ImVec2(max.x, yOf(0.0)), ImGui::GetColorU32(ImGuiCol_Separator));
    }
    const float x = min.x + (max.x - min.x) * static_cast<float>(window.now / CurveSpan);
    draw->AddLine(ImVec2(x, min.y), ImVec2(x, max.y), ImGui::GetColorU32(ImGuiCol_PlotLinesHovered), 1.5f);
    const ImU32 color = ImGui::GetColorU32(ImGuiCol_TextDisabled);
    const float lineHeight = ImGui::GetFontSize();
    char text[64];
    std::snprintf(text, sizeof(text), "%.3g", window.high);
    draw->AddText(ImVec2(min.x + 2.0f, min.y), color, text);
    std::snprintf(text, sizeof(text), "%.3g", window.low);
    draw->AddText(ImVec2(min.x + 2.0f, max.y - lineHeight), color, text);
    std::snprintf(text, sizeof(text), "%.4g - %.4g s", window.start, window.start + CurveSpan);
    draw->AddText(ImVec2(max.x - ImGui::CalcTextSize(text).x - 2.0f, max.y - lineHeight), color, text);
}

// `item`: the shape is a layer's value, which can be the item's value times the shape (design L16).
bool ShapeFields(Shape& shape, bool item)
{
    bool changed = false;
    ImGui::Indent();
    switch (shape.kind)
    {
    case ShapeKind::Wave:
    case ShapeKind::Bounce:
    case ShapeKind::Ramp:
    case ShapeKind::Zigzag:
        changed |= DragNumber(I18N::Editor::From, shape.from, ValueSpeed, I18N::Editor::ShapeFromTooltip);
        ImGui::SameLine();
        changed |= DragNumber(I18N::Editor::To, shape.to, ValueSpeed, I18N::Editor::ShapeToTooltip);
        ImGui::SameLine();
        changed |= SecondsField(I18N::Editor::Seconds, shape.seconds, I18N::Editor::ShapeSecondsTooltip);
        break;
    case ShapeKind::Scroll:
        changed |=
            DragNumber(I18N::Editor::PerSecond, shape.perSecond, ValueSpeed, I18N::Editor::ShapePerSecondTooltip);
        break;
    case ShapeKind::Turn:
        changed |= DragNumber(I18N::Editor::From, shape.from, ValueSpeed, I18N::Editor::ShapeFromTooltip);
        ImGui::SameLine();
        changed |= DragNumber(I18N::Editor::To, shape.to, ValueSpeed, I18N::Editor::ShapeToTooltip);
        break;
    case ShapeKind::Frames:
        ImGui::SetNextItemWidth(Scaled(NumberWidth));
        changed |= ImGui::DragInt(I18N::Editor::Count, &shape.count, CountSpeed, 1, MaxFrames, "%d",
                                  ImGuiSliderFlags_AlwaysClamp);
        Tip(I18N::Editor::ShapeCountTooltip, true);
        ImGui::SameLine();
        changed |= DragNumber(I18N::Editor::Step, shape.step, ValueSpeed, I18N::Editor::ShapeStepTooltip);
        ImGui::SameLine();
        changed |= SecondsField(I18N::Editor::SecondsEach, shape.seconds, I18N::Editor::ShapeSecondsEachTooltip);
        break;
    case ShapeKind::Random:
        // The file refuses a step of 0 or less and To below From, so the fields keep them: From dragged past To
        // takes To along.
        if (DragNumber(I18N::Editor::From, shape.from, ValueSpeed, I18N::Editor::ShapeFromTooltip))
        {
            shape.to = std::max(shape.to, shape.from);
            changed = true;
        }
        ImGui::SameLine();
        changed |= DragNumber(I18N::Editor::To, shape.to, ValueSpeed, I18N::Editor::ShapeToTooltip, shape.from);
        ImGui::SameLine();
        changed |=
            DragNumber(I18N::Editor::Step, shape.step, ValueSpeed, I18N::Editor::ShapeStepTooltip, MinRandomStep);
        break;
    }
    if (item)
    {
        changed |= ImGui::Checkbox(I18N::Editor::TimesTheItemValue, &shape.timesItem);
        Tip(I18N::Editor::TimesTheItemValueTooltip);
    }
    RenderCurve(shape);
    ImGui::Unindent();
    return changed;
}

// The value's number or the shape's from and to, for a new shape.
std::pair<double, double> RangeOf(const Value& value)
{
    if (const float* number = std::get_if<float>(&value))
    {
        return {0.0, *number != 0.0f ? *number : 1.0};
    }
    if (const Shape* shape = std::get_if<Shape>(&value))
    {
        return {shape->from, shape->to};
    }
    return {0.0, 1.0};
}

Source SourceOf(const Value& value)
{
    if (std::holds_alternative<FromItem>(value))
    {
        return Source::Item;
    }
    if (std::holds_alternative<FromDrawing>(value))
    {
        return Source::Drawing;
    }
    if (const Shape* shape = std::get_if<Shape>(&value))
    {
        return SourceOf(shape->kind);
    }
    return Source::Number;
}

Value MakeValue(Source source, const Value& old)
{
    const auto [from, to] = RangeOf(old);
    switch (source)
    {
    case Source::Item:
        return FromItem{};
    case Source::Drawing:
        return FromDrawing{};
    case Source::Number:
        return static_cast<float>(to);
    default:
        return NewShape(source, from, to);
    }
}

// An opacity, glow strength or scroll: the item's, the drawing's (`drawing`: only an opacity), a number or a shape;
// `tip` says what it does.
bool ValueField(const char* label, Value& value, const char* tip, bool drawing, bool item = true)
{
    ImGui::PushID(label);
    bool changed = false;
    Names<Source> sources;
    for (const auto& entry : SourceNames())
    {
        if ((entry.first != Source::Drawing || drawing) && (entry.first != Source::Item || item))
        {
            sources.push_back(entry);
        }
    }
    Source source = SourceOf(value);
    if (Choice(label, source, sources, tip, &SourceTips()))
    {
        value = MakeValue(source, value);
        changed = true;
    }
    if (float* number = std::get_if<float>(&value))
    {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(Scaled(NumberWidth));
        changed |= ImGui::DragFloat("##Number", number, ValueSpeed, 0.0f, 0.0f, "%.7g");
        Tip(tip, true);
    }
    else if (Shape* shape = std::get_if<Shape>(&value))
    {
        changed |= ShapeFields(*shape, item);
    }
    ImGui::PopID();
    return changed;
}

Value ToValue(const Number& number)
{
    if (const Shape* shape = std::get_if<Shape>(&number))
    {
        return *shape;
    }
    return std::get<float>(number);
}

Number ToNumber(const Value& value)
{
    if (const Shape* shape = std::get_if<Shape>(&value))
    {
        return *shape;
    }
    if (const float* number = std::get_if<float>(&value))
    {
        return *number;
    }
    return 1.0f;
}

// A number or a shape: a light's channel, an item value.
bool NumberField(const char* label, Number& number, const char* tip)
{
    Value value = ToValue(number);
    if (!ValueField(label, value, tip, false, false))
    {
        return false;
    }
    number = ToNumber(value);
    return true;
}

std::string JoinMeshes(const std::vector<int>& meshes)
{
    std::string text;
    for (const int mesh : meshes)
    {
        text += (text.empty() ? "" : ", ") + std::to_string(mesh);
    }
    return text;
}

// "0, 2" as mesh numbers, each once; nothing when no number can be one.
std::vector<int> ParseMeshes(const std::string& text)
{
    std::vector<int> meshes;
    std::string cleaned = text;
    std::ranges::replace(cleaned, ',', ' ');
    std::istringstream stream(cleaned);
    std::string token;
    while (stream >> token)
    {
        char* end = nullptr;
        const long mesh = std::strtol(token.c_str(), &end, 10);
        if (end != nullptr && *end == '\0' && mesh >= 0 && mesh <= LastMesh &&
            std::ranges::find(meshes, static_cast<int>(mesh)) == meshes.end())
        {
            meshes.push_back(static_cast<int>(mesh));
        }
    }
    return meshes;
}

// The mesh numbers as text; the list changes when the field is left after typing.
bool MeshListField(const char* label, std::vector<int>& meshes)
{
    ImGui::SetNextItemWidth(Scaled(ComboWidth));
    std::string typed;
    const bool left = Items::InputStringOnLeave(label, I18N::Editor::MeshNumbersHint, JoinMeshes(meshes), typed);
    Tip(I18N::Editor::LayerMeshNumbersTooltip);
    if (!left)
    {
        return false;
    }
    std::vector<int> parsed = ParseMeshes(typed);
    if (parsed.empty() || parsed == meshes)
    {
        return false;
    }
    meshes = std::move(parsed);
    return true;
}

// The picked item's parts by name, a checkbox each (design L42); one stays ticked, since a layer without meshes would
// draw on every one. Without parts, the list of numbers.
bool MeshChecks(std::vector<int>& meshes, const std::vector<LookPart>& parts, bool allBut)
{
    if (parts.empty())
    {
        return MeshListField("##List", meshes);
    }
    bool changed = false;
    for (const MeshOption& option : MeshOptions(parts, meshes))
    {
        bool ticked = std::ranges::find(meshes, option.mesh) != meshes.end();
        ImGui::PushID(option.mesh);
        if (ImGui::Checkbox(option.label.c_str(), &ticked))
        {
            std::vector<int> next = WithMesh(meshes, option.mesh, ticked);
            if (!next.empty())
            {
                meshes = std::move(next);
                changed = true;
            }
        }
        // For every mesh but the ticked ones, a tick leaves the part out.
        Tip(allBut ? I18N::Editor::LayerPartLeftOutTooltip : I18N::Editor::LayerPartTooltip);
        ImGui::PopID();
    }
    return changed;
}

// A mesh picked from the item's parts by name (design L42), or without parts dragged as a number.
bool MeshField(const char* label, int& mesh, const std::vector<LookPart>& parts, const char* tip)
{
    ImGui::SameLine();
    ImGui::SetNextItemWidth(Scaled(ComboWidth));
    if (parts.empty())
    {
        const bool changed = ImGui::DragInt(label, &mesh, CountSpeed, 0, LastMesh, "%d", ImGuiSliderFlags_AlwaysClamp);
        Tip(tip, true);
        return changed;
    }
    const std::vector<MeshOption> options = MeshOptions(parts, {mesh});
    const auto shown = std::ranges::find(options, mesh, &MeshOption::mesh);
    bool changed = false;
    if (ImGui::BeginCombo(label, shown != options.end() ? shown->label.c_str() : ""))
    {
        for (const MeshOption& option : options)
        {
            if (ImGui::Selectable(option.label.c_str(), option.mesh == mesh) && option.mesh != mesh)
            {
                mesh = option.mesh;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    else
    {
        Tip(tip);
    }
    return changed;
}

// The texture changes when the field is left after typing: each key would be another file to look for.
bool TextureField(std::string& texture)
{
    ImGui::SetNextItemWidth(Scaled(GraphWidth));
    std::string typed;
    const bool left = Items::InputStringOnLeave(I18N::Editor::Texture, I18N::Editor::TextureHint, texture, typed);
    Tip(I18N::Editor::LayerTextureTooltip);
    if (!left || typed == texture)
    {
        return false;
    }
    texture = typed;
    return true;
}

enum class MeshChoice
{
    All,
    These,
    AllBut,
    None,
};

bool MeshesField(MeshSet& meshes, const std::vector<LookPart>& parts)
{
    ImGui::PushID("Meshes");
    bool changed = false;
    MeshChoice choice = meshes.none      ? MeshChoice::None
                        : meshes.allBut  ? MeshChoice::AllBut
                        : meshes.IsAll() ? MeshChoice::All
                                         : MeshChoice::These;
    static const Names<MeshChoice> names{{MeshChoice::All, &I18N::Editor::EveryMesh},
                                         {MeshChoice::These, &I18N::Editor::TheseMeshes},
                                         {MeshChoice::AllBut, &I18N::Editor::EveryMeshBut},
                                         {MeshChoice::None, &I18N::Editor::NoMesh}};
    if (Choice(I18N::Editor::Meshes, choice, names, I18N::Editor::LayerMeshesTooltip))
    {
        if (choice == MeshChoice::All)
        {
            meshes = {};
        }
        else if (choice == MeshChoice::None)
        {
            meshes = {.none = true};
        }
        else
        {
            const std::vector<int> listed = meshes.meshes.empty() ? std::vector<int>{0} : meshes.meshes;
            meshes = {listed, choice == MeshChoice::AllBut, false};
        }
        changed = true;
    }
    ImGui::Indent();
    if (choice == MeshChoice::All)
    {
        changed |= ImGui::Checkbox(I18N::Editor::AlsoTheHiddenMesh, &meshes.evenHidden);
        Tip(I18N::Editor::LayerHiddenMeshTooltip);
    }
    else if (choice != MeshChoice::None)
    {
        changed |= MeshChecks(meshes.meshes, parts, meshes.allBut);
    }
    ImGui::Unindent();
    ImGui::PopID();
    return changed;
}

enum class GlowMeshChoice
{
    Item,
    None,
    All,
    Mesh,
};

bool GlowMeshField(GlowMesh& mesh, const std::vector<LookPart>& parts)
{
    ImGui::PushID("GlowMesh");
    GlowMeshChoice choice = std::holds_alternative<int>(mesh)             ? GlowMeshChoice::Mesh
                            : std::holds_alternative<NoGlowMesh>(mesh)    ? GlowMeshChoice::None
                            : std::holds_alternative<AllGlowMeshes>(mesh) ? GlowMeshChoice::All
                                                                          : GlowMeshChoice::Item;
    static const Names<GlowMeshChoice> names{{GlowMeshChoice::Item, &I18N::Editor::FromTheItem},
                                             {GlowMeshChoice::None, &I18N::Editor::None},
                                             {GlowMeshChoice::All, &I18N::Editor::EveryMesh},
                                             {GlowMeshChoice::Mesh, &I18N::Editor::AMesh}};
    bool changed = false;
    if (Choice(I18N::Editor::GlowMesh, choice, names, I18N::Editor::LayerGlowMeshTooltip))
    {
        switch (choice)
        {
        case GlowMeshChoice::Item:
            mesh = FromItem{};
            break;
        case GlowMeshChoice::None:
            mesh = NoGlowMesh{};
            break;
        case GlowMeshChoice::All:
            mesh = AllGlowMeshes{};
            break;
        case GlowMeshChoice::Mesh:
            mesh = parts.empty() ? 0 : parts.front().mesh;
            break;
        }
        changed = true;
    }
    if (int* number = std::get_if<int>(&mesh))
    {
        changed |= MeshField("##Mesh", *number, parts, I18N::Editor::LayerGlowMeshTooltip);
    }
    ImGui::PopID();
    return changed;
}

// A mesh, or nothing: `empty` names what nothing is (the model's mesh, none); `tip` says what the mesh is for.
bool OptionalMeshField(const char* label, std::optional<int>& mesh, const char* const* empty,
                       const std::vector<LookPart>& parts, const char* tip)
{
    ImGui::PushID(label);
    bool set = mesh.has_value();
    const Names<bool> names{{false, empty}, {true, &I18N::Editor::AMesh}};
    bool changed = false;
    if (Choice(label, set, names, tip))
    {
        mesh = set ? std::optional<int>(parts.empty() ? 0 : parts.front().mesh) : std::nullopt;
        changed = true;
    }
    if (mesh)
    {
        changed |= MeshField("##Mesh", *mesh, parts, tip);
    }
    ImGui::PopID();
    return changed;
}

// A light's red, green and blue: a swatch with a color picker while all three are numbers (design L42), and with Each
// channel apart a field each, so one can change over time; it stays on while one does. Its state is kept in the ID
// scope the field is drawn in: the Style Editor gives each layer of each look its own.
bool ColorField(std::array<Number, 3>& color)
{
    const bool numbers =
        std::ranges::all_of(color, [](const Number& channel) { return std::holds_alternative<float>(channel); });
    bool changed = false;
    if (numbers)
    {
        std::array<float, 3> rgb{std::get<float>(color[0]), std::get<float>(color[1]), std::get<float>(color[2])};
        ImGui::SetNextItemWidth(Scaled(GraphWidth));
        if (ImGui::ColorEdit3(I18N::Editor::Color, rgb.data(), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR))
        {
            for (std::size_t channel = 0; channel < rgb.size(); ++channel)
            {
                color[channel] = rgb[channel];
            }
            changed = true;
        }
        Tip(I18N::Editor::LayerColorTooltip, true);
    }
    bool& apart = *ImGui::GetStateStorage()->GetBoolRef(ImGui::GetID("##ChannelsApart"), false);
    bool shown = apart || !numbers;
    ImGui::BeginDisabled(!numbers);
    if (ImGui::Checkbox(I18N::Editor::EachChannelApart, &shown) && numbers)
    {
        apart = shown;
    }
    ImGui::EndDisabled();
    Tip(I18N::Editor::EachChannelApartTooltip);
    if (shown)
    {
        const char* const* channels[] = {&I18N::Editor::Red, &I18N::Editor::Green, &I18N::Editor::Blue};
        for (std::size_t channel = 0; channel < color.size(); ++channel)
        {
            changed |= NumberField(*channels[channel], color[channel], I18N::Editor::LayerLightChannelTooltip);
        }
    }
    return changed;
}

enum class LightChoice
{
    Before,
    Color,
    Tint,
    Brightness,
};

bool LightField(std::optional<LayerLight>& light)
{
    ImGui::PushID("Light");
    LightChoice choice = !light              ? LightChoice::Before
                         : light->brightness ? LightChoice::Brightness
                         : light->tint       ? LightChoice::Tint
                                             : LightChoice::Color;
    static const Names<LightChoice> names{{LightChoice::Before, &I18N::Editor::TheLightBefore},
                                          {LightChoice::Color, &I18N::Editor::AColor},
                                          {LightChoice::Tint, &I18N::Editor::TintOfTheItemLight},
                                          {LightChoice::Brightness, &I18N::Editor::TheItemLightAtABrightness}};
    bool changed = false;
    if (Choice(I18N::Editor::Light, choice, names, I18N::Editor::LayerLightTooltip))
    {
        if (choice == LightChoice::Before)
        {
            light.reset();
        }
        else if (choice == LightChoice::Brightness)
        {
            light = LayerLight{.brightness = Number{1.f}};
        }
        else
        {
            const LayerLight kept = light.value_or(LayerLight{});
            light = LayerLight{.tint = choice == LightChoice::Tint, .color = kept.color};
        }
        changed = true;
    }
    if (light && light->brightness)
    {
        ImGui::Indent();
        changed |= NumberField(I18N::Editor::Brightness, *light->brightness, I18N::Editor::LightBrightnessTooltip);
        ImGui::Unindent();
    }
    else if (light)
    {
        ImGui::Indent();
        changed |= ColorField(light->color);
        ImGui::Unindent();
    }
    ImGui::PopID();
    return changed;
}

// The light a look leaves: a color, or a tint of the item's light.
bool LeftLight(LayerLight& light)
{
    ImGui::PushID("Light");
    LightChoice choice = light.tint ? LightChoice::Tint : LightChoice::Color;
    static const Names<LightChoice> names{{LightChoice::Color, &I18N::Editor::AColor},
                                          {LightChoice::Tint, &I18N::Editor::TintOfTheItemLight}};
    bool changed = false;
    if (Choice(I18N::Editor::Light, choice, names, I18N::Editor::LeftLightTooltip))
    {
        light.tint = choice == LightChoice::Tint;
        changed = true;
    }
    ImGui::Indent();
    changed |= ColorField(light.color);
    ImGui::Unindent();
    ImGui::PopID();
    return changed;
}

// An item value a look may leave: a box for whether it leaves one, then its field.
template <typename T, typename Field>
bool LeftValue(const char* label, std::optional<T>& value, const T& fresh, Field&& field)
{
    ImGui::PushID(label);
    bool set = value.has_value();
    bool changed = false;
    if (ImGui::Checkbox("##Leaves", &set))
    {
        value = set ? std::optional<T>(fresh) : std::nullopt;
        changed = true;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::LeavesIt);
    }
    ImGui::SameLine();
    if (value)
    {
        changed |= field(*value);
    }
    else
    {
        ImGui::TextDisabled("%s", label);
    }
    ImGui::PopID();
    return changed;
}

// A mesh of the item values, or none (-1).
bool MeshOrNone(const char* label, int& mesh, const std::vector<LookPart>& parts, const char* tip)
{
    std::optional<int> set = mesh >= 0 ? std::optional<int>(mesh) : std::nullopt;
    if (!OptionalMeshField(label, set, &I18N::Editor::None, parts, tip))
    {
        return false;
    }
    mesh = set.value_or(-1);
    return true;
}

void AddPart(std::string& text, const std::string& part, const char* separator = ", ")
{
    text += (text.empty() ? "" : separator) + part;
}

template <typename T> std::string NameIn(const Names<T>& names, T value)
{
    for (const auto& [choice, name] : names)
    {
        if (choice == value)
        {
            return *name;
        }
    }
    return "";
}

// The fields an effect shows, as bits.
namespace EffectField
{
enum : unsigned
{
    Motion = 1u << 0,
    Blend = 1u << 1,
    Texture = 1u << 2,
    Light = 1u << 3,
    Opacity = 1u << 4,
    GlowMesh = 1u << 5,
    GlowStrength = 1u << 6,
    Scroll = 1u << 7,
};
} // namespace EffectField

// A glow is its glow mesh's strength, or the opacity of a layer added over the rest.
unsigned GlowFields(const Data::Looks::Layer& layer)
{
    if (DrawsGlowing(layer.glowMesh) || std::holds_alternative<Shape>(layer.glowStrength))
    {
        return EffectField::GlowMesh | EffectField::GlowStrength;
    }
    return EffectField::Opacity | EffectField::Blend;
}

unsigned FieldsOf(const Data::Looks::Layer& layer, const LayerEffect& effect)
{
    unsigned fields = 0;
    switch (effect.kind)
    {
    case EffectKind::Base:
        fields = EffectField::Opacity;
        break;
    case EffectKind::Glow:
    case EffectKind::Pulse:
        fields = GlowFields(layer);
        break;
    case EffectKind::Shine:
        fields = EffectField::Motion | EffectField::Blend | EffectField::Opacity;
        break;
    case EffectKind::Overlay:
        fields = EffectField::Texture | EffectField::Blend | EffectField::Opacity;
        break;
    case EffectKind::Flowing:
    case EffectKind::Frames:
        fields = EffectField::Scroll;
        break;
    case EffectKind::Color:
        fields = EffectField::Light;
        break;
    }
    if (effect.glows)
    {
        fields |= EffectField::GlowMesh | EffectField::GlowStrength;
    }
    if (effect.pulses)
    {
        fields |= EffectField::GlowStrength | EffectField::Opacity;
    }
    if (effect.flows || effect.frames)
    {
        fields |= EffectField::Scroll;
    }
    if (effect.colored)
    {
        fields |= EffectField::Light;
    }
    return fields;
}
} // namespace

std::vector<MeshOption> MeshOptions(const std::vector<LookPart>& parts, const std::vector<int>& listed)
{
    std::vector<MeshOption> options;
    for (const LookPart& part : parts)
    {
        options.push_back(
            {part.mesh, part.name.empty() ? PartLabel(part) : std::to_string(part.mesh) + " \xc2\xb7 " + part.name});
    }
    std::vector<int> others;
    for (const int mesh : listed)
    {
        if (std::ranges::find(options, mesh, &MeshOption::mesh) == options.end() &&
            std::ranges::find(others, mesh) == others.end())
        {
            others.push_back(mesh);
        }
    }
    std::ranges::sort(others);
    for (const int mesh : others)
    {
        options.push_back({mesh, I18N::Format(I18N::Editor::MeshNotInTheModel, {std::to_string(mesh)})});
    }
    return options;
}

void SetCurveTime(double seconds)
{
    s_curveTime = seconds;
}

double CurveValueAt(const Shape& shape, const CurveWindow& window, double seconds)
{
    return shape.kind == ShapeKind::Scroll ? ShapeValueAt(shape, seconds) : ShapeValueAt(shape, window.start + seconds);
}

CurveWindow CurveWindowOf(const Shape& shape, double now)
{
    CurveWindow window;
    const double clock = std::max(0.0, now);
    window.start = std::floor(clock / CurveSpan) * CurveSpan;
    window.now = clock - window.start;
    for (int sample = 0; sample < GraphSamples; ++sample)
    {
        const double value = CurveValueAt(shape, window, CurveSpan * sample / (GraphSamples - 1));
        window.low = std::min(window.low, value);
        window.high = std::max(window.high, value);
    }
    return window;
}

std::vector<int> WithMesh(std::vector<int> meshes, int mesh, bool in)
{
    const auto found = std::ranges::find(meshes, mesh);
    if (in && found == meshes.end())
    {
        meshes.push_back(mesh);
    }
    else if (!in && found != meshes.end())
    {
        meshes.erase(found);
    }
    return meshes;
}

double ShapeValueAt(const Shape& shape, double seconds)
{
    switch (shape.kind)
    {
    case ShapeKind::Scroll:
        break;
    case ShapeKind::Frames:
        if (shape.seconds <= 0.0 || shape.count <= 0)
        {
            return 0.0;
        }
        break;
    case ShapeKind::Random:
        return (shape.from + shape.to) / 2.0;
    case ShapeKind::Turn:
        // Follows no clock: the item at no turn.
        return shape.from;
    case ShapeKind::Wave:
    case ShapeKind::Bounce:
    case ShapeKind::Ramp:
    case ShapeKind::Zigzag:
        // A draft's shape can have no time yet, which the game's looks never have.
        if (shape.seconds <= 0.0)
        {
            return shape.from;
        }
        break;
    }
    // The game's own shape math, so the curve shows what the game draws.
    return Render::Items::Looks::ShapeAtClock(shape, seconds * 1000.0);
}

namespace
{
// A choice that can also be unset, `unset` naming that choice.
template <typename T>
bool OptionalChoice(const char* label, std::optional<T>& value, const Names<T>& names, const char* const* unset,
                    const char* tip)
{
    Names<int> options{{-1, unset}};
    int current = -1;
    for (std::size_t index = 0; index < names.size(); ++index)
    {
        options.emplace_back(static_cast<int>(index), names[index].second);
        if (value && names[index].first == *value)
        {
            current = static_cast<int>(index);
        }
    }
    if (!Choice(label, current, options, tip))
    {
        return false;
    }
    value = current < 0 ? std::nullopt : std::optional<T>(names[static_cast<std::size_t>(current)].first);
    return true;
}

const Names<Place>& PlaceNames()
{
    static const Names<Place> names{{Place::Worn, &I18N::Editor::Worn},
                                    {Place::InventoryAndGround, &I18N::Editor::InTheInventoryOrOnTheGround},
                                    {Place::Inventory, &I18N::Editor::InTheInventory},
                                    {Place::Ground, &I18N::Editor::OnTheGround}};
    return names;
}

const Names<LookBuff>& BuffNames()
{
    static const Names<LookBuff> names{{LookBuff::InfinityArrow, &I18N::Editor::InfinityArrow}};
    return names;
}

// The levels the item glows like the layer draws at, from and to; unset: every level.
bool LevelRangeField(std::optional<std::pair<int, int>>& level)
{
    constexpr int MaxLevel = 15;
    ImGui::PushID("Level");
    bool set = level.has_value();
    bool changed = ImGui::Checkbox("##Set", &set);
    if (changed)
    {
        level = set ? std::optional<std::pair<int, int>>(std::pair{0, MaxLevel}) : std::nullopt;
    }
    Tip(I18N::Editor::LevelFromTooltip);
    ImGui::SameLine();
    if (level)
    {
        ImGui::SetNextItemWidth(Scaled(NumberWidth));
        changed |= ImGui::DragInt(I18N::Editor::LevelFrom, &level->first, CountSpeed, 0, level->second, "+%d",
                                  ImGuiSliderFlags_AlwaysClamp);
        Tip(I18N::Editor::LevelFromTooltip, true);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(Scaled(NumberWidth));
        changed |= ImGui::DragInt(I18N::Editor::To, &level->second, CountSpeed, level->first, MaxLevel, "+%d",
                                  ImGuiSliderFlags_AlwaysClamp);
        Tip(I18N::Editor::LevelFromTooltip, true);
    }
    else
    {
        ImGui::TextDisabled("%s", I18N::Editor::LevelFrom);
    }
    ImGui::PopID();
    return changed;
}

// The item's hidden mesh the layer draws with: unset (any), none (-1) or a mesh.
bool HiddenMeshCondition(std::optional<int>& hidden, const std::vector<LookPart>& parts)
{
    constexpr int NoMesh = -1;
    enum class Kind
    {
        Any,
        None,
        Mesh,
    };
    const Names<Kind> names{
        {Kind::Any, &I18N::Editor::Any}, {Kind::None, &I18N::Editor::NoHiddenMesh}, {Kind::Mesh, &I18N::Editor::AMesh}};
    Kind kind = !hidden ? Kind::Any : *hidden == NoMesh ? Kind::None : Kind::Mesh;
    ImGui::PushID("HiddenMesh");
    bool changed = false;
    if (Choice(I18N::Editor::HiddenMesh, kind, names, I18N::Editor::OnlyWhenHiddenMeshTooltip))
    {
        hidden = kind == Kind::Any    ? std::nullopt
                 : kind == Kind::None ? std::optional<int>(NoMesh)
                                      : std::optional<int>(parts.empty() ? 0 : parts.front().mesh);
        changed = true;
    }
    if (kind == Kind::Mesh)
    {
        changed |= MeshField("##Mesh", *hidden, parts, I18N::Editor::OnlyWhenHiddenMeshTooltip);
    }
    ImGui::PopID();
    return changed;
}

// A field of a look that each item can set: its key in the file and its label.
struct EachItemField
{
    const char* key;
    const char* const* label;
};

// "Each item sets" (design L12): a checkbox for each field that an item can set itself, with the name of the item's
// value for a checked one; the field's value in the look stays the default.
bool EachItemBoxes(Data::Looks::EachItemFields& eachItem, std::initializer_list<EachItemField> fields)
{
    namespace Text = I18N::Editor;
    const bool open = ImGui::TreeNodeEx(Text::EachItemSets, eachItem.empty() ? 0 : ImGuiTreeNodeFlags_DefaultOpen);
    Tip(Text::EachItemSetsTooltip);
    if (!open)
    {
        return false;
    }
    bool changed = false;
    for (const EachItemField& field : fields)
    {
        ImGui::PushID(field.key);
        const auto set = eachItem.find(field.key);
        bool on = set != eachItem.end();
        if (ImGui::Checkbox(*field.label, &on))
        {
            if (on)
            {
                eachItem[field.key] = field.key;
            }
            else
            {
                eachItem.erase(field.key);
            }
            changed = true;
        }
        if (on && eachItem.contains(field.key))
        {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(Scaled(ComboWidth));
            std::string typed;
            if (Items::InputStringOnLeave(Text::ValueName, "", eachItem[field.key], typed))
            {
                eachItem[field.key] = typed;
                changed = true;
            }
            Tip(Text::ValueNameTooltip);
        }
        ImGui::PopID();
    }
    ImGui::TreePop();
    return changed;
}

bool OnlyWhenFields(OnlyWhen& when, const std::vector<LookPart>& parts)
{
    namespace Text = I18N::Editor;
    const bool open = ImGui::TreeNodeEx(Text::OnlyWhen, when.IsEmpty() ? 0 : ImGuiTreeNodeFlags_DefaultOpen);
    Tip(Text::OnlyWhenTooltip);
    if (!open)
    {
        return false;
    }
    bool changed = OptionalChoice(Text::Place, when.place, PlaceNames(), &Text::Anywhere, Text::PlaceTooltip);
    changed |= LevelRangeField(when.level);
    changed |= HiddenMeshCondition(when.hiddenMesh, parts);
    changed |= OptionalChoice(Text::Buff, when.buff, BuffNames(), &Text::None, Text::BuffTooltip);
    changed |= OptionalChoice(Text::WithoutBuff, when.withoutBuff, BuffNames(), &Text::None, Text::WithoutBuffTooltip);
    ImGui::TreePop();
    return changed;
}
} // namespace

bool LookOptions(Data::Looks::LookDefinition& look)
{
    namespace Text = I18N::Editor;
    bool changed = ImGui::Checkbox(Text::NotForDoppelgangers, &look.notFor.doppelganger);
    Tip(Text::NotForDoppelgangersTooltip);
    ImGui::SameLine();
    changed |= ImGui::Checkbox(Text::OnlyInMonsterHands, &look.notFor.outsideMonsterHands);
    Tip(Text::OnlyInMonsterHandsTooltip);
    ImGui::SameLine();
    changed |= ImGui::Checkbox(Text::DrawsAlone, &look.drawsAlone);
    Tip(Text::DrawsAloneTooltip);
    return changed;
}

bool Layer(Data::Looks::Layer& layer, const std::vector<LookPart>& parts)
{
    namespace Text = I18N::Editor;
    bool changed = false;
    if (Choice(Text::Kind, layer.kind, KindNames(), Text::LayerKindTooltip))
    {
        // A texture layer has no motion and no place under the holes; the file leaves them out.
        layer.motion = Motion::SlowDrift;
        layer.where = Where::Over;
        changed = true;
    }
    if (layer.kind == LayerKind::Reflection)
    {
        changed |= Choice(Text::Motion, layer.motion, MotionNames(), Text::LayerMotionTooltip);
        changed |= Choice(Text::Where, layer.where, WhereNames(), Text::LayerWhereTooltip);
    }
    changed |= Choice(Text::Blend, layer.blend, BlendNames(), Text::LayerBlendTooltip, &BlendTips());
    changed |= MeshesField(layer.meshes, parts);
    changed |= TextureField(layer.texture);
    changed |= LightField(layer.light);
    changed |= ValueField(Text::Opacity, layer.opacity, Text::LayerOpacityTooltip, true);
    changed |= GlowMeshField(layer.glowMesh, parts);
    changed |= ValueField(Text::GlowStrength, layer.glowStrength, Text::LayerGlowStrengthTooltip, false);
    changed |= ValueField(Text::ScrollU, layer.scrollU, Text::LayerScrollTooltip, false);
    changed |= ValueField(Text::ScrollV, layer.scrollV, Text::LayerScrollTooltip, false);
    changed |= OptionalMeshField(Text::ScrollingMesh, layer.scrollingMesh, &Text::KeptFromTheModel, parts,
                                 Text::LayerScrollingMeshTooltip);
    changed |= OnlyWhenFields(layer.onlyWhen, parts);
    changed |= EachItemBoxes(layer.eachItem, {{"meshes", &Text::Meshes},
                                              {"texture", &Text::Texture},
                                              {"light", &Text::Light},
                                              {"opacity", &Text::Opacity},
                                              {"glowMesh", &Text::GlowMesh},
                                              {"glowStrength", &Text::GlowStrength},
                                              {"scrollU", &Text::ScrollU},
                                              {"scrollV", &Text::ScrollV},
                                              {"scrollingMesh", &Text::ScrollingMesh}});
    return changed;
}

bool Effect(Data::Looks::Layer& layer, const LayerEffect& effect, const std::vector<LookPart>& parts)
{
    namespace Text = I18N::Editor;
    const unsigned fields = FieldsOf(layer, effect);
    bool changed = MeshesField(layer.meshes, parts);
    if ((fields & EffectField::Motion) != 0 && layer.kind == LayerKind::Reflection)
    {
        changed |= Choice(Text::Motion, layer.motion, MotionNames(), Text::LayerMotionTooltip);
    }
    if ((fields & EffectField::Blend) != 0)
    {
        changed |= Choice(Text::Blend, layer.blend, BlendNames(), Text::LayerBlendTooltip, &BlendTips());
    }
    if ((fields & EffectField::Texture) != 0)
    {
        changed |= TextureField(layer.texture);
    }
    if ((fields & EffectField::Light) != 0)
    {
        changed |= LightField(layer.light);
    }
    if ((fields & EffectField::Opacity) != 0)
    {
        changed |= ValueField(Text::Opacity, layer.opacity, Text::LayerOpacityTooltip, true);
    }
    if ((fields & EffectField::GlowMesh) != 0)
    {
        changed |= GlowMeshField(layer.glowMesh, parts);
    }
    if ((fields & EffectField::GlowStrength) != 0)
    {
        changed |= ValueField(Text::GlowStrength, layer.glowStrength, Text::LayerGlowStrengthTooltip, false);
    }
    if ((fields & EffectField::Scroll) != 0)
    {
        changed |= ValueField(Text::ScrollU, layer.scrollU, Text::LayerScrollTooltip, false);
        changed |= ValueField(Text::ScrollV, layer.scrollV, Text::LayerScrollTooltip, false);
    }
    return changed;
}

const char* EffectName(EffectKind kind)
{
    switch (kind)
    {
    case EffectKind::Base:
        return I18N::Editor::EffectBase;
    case EffectKind::Glow:
        return I18N::Editor::EffectGlow;
    case EffectKind::Pulse:
        return I18N::Editor::EffectPulse;
    case EffectKind::Shine:
        return I18N::Editor::EffectShine;
    case EffectKind::Overlay:
        return I18N::Editor::EffectOverlayTexture;
    case EffectKind::Flowing:
        return I18N::Editor::EffectFlowingTexture;
    case EffectKind::Color:
        return I18N::Editor::EffectColor;
    case EffectKind::Frames:
        return I18N::Editor::EffectFramesAndFlicker;
    }
    return "";
}

const char* EffectNote(EffectKind kind)
{
    switch (kind)
    {
    case EffectKind::Glow:
        return I18N::Editor::EffectGlowNote;
    case EffectKind::Pulse:
        return I18N::Editor::EffectPulseNote;
    case EffectKind::Shine:
        return I18N::Editor::EffectShineNote;
    case EffectKind::Overlay:
        return I18N::Editor::EffectOverlayTextureNote;
    case EffectKind::Flowing:
        return I18N::Editor::EffectFlowingTextureNote;
    case EffectKind::Color:
        return I18N::Editor::EffectColorNote;
    case EffectKind::Frames:
        return I18N::Editor::EffectFramesAndFlickerNote;
    case EffectKind::Base:
        break;
    }
    return "";
}

std::string EffectLabel(const LayerEffect& effect)
{
    std::string also;
    const auto add = [&also](bool set, const char* label)
    {
        if (set)
        {
            AddPart(also, label);
        }
    };
    add(effect.glows, I18N::Editor::AlsoGlows);
    add(effect.pulses, I18N::Editor::AlsoPulses);
    add(effect.flows, I18N::Editor::AlsoFlows);
    add(effect.frames, I18N::Editor::AlsoStepsThroughFrames);
    add(effect.colored, I18N::Editor::AlsoColored);
    const std::string name = EffectName(effect.kind);
    return also.empty() ? name : name + " (" + also + ")";
}

std::string DescribeMeshes(const MeshSet& meshes)
{
    if (meshes.none)
    {
        return I18N::Editor::NoMesh;
    }
    if (meshes.IsAll())
    {
        return meshes.evenHidden ? std::string(I18N::Editor::EveryMesh) + " + " + I18N::Editor::HiddenMesh
                                 : std::string(I18N::Editor::EveryMesh);
    }
    if (meshes.allBut)
    {
        return std::string(I18N::Editor::EveryMeshBut) + " " + JoinMeshes(meshes.meshes);
    }
    return std::string(I18N::Editor::Meshes) + " " + JoinMeshes(meshes.meshes);
}

bool ItemValues(Data::Looks::ItemValues& values, const std::vector<LookPart>& parts, bool light)
{
    namespace Text = I18N::Editor;
    bool changed = false;
    const auto number = [](const char* label, const char* tip)
    { return [label, tip](Number& value) { return NumberField(label, value, tip); }; };
    const auto mesh = [&parts](const char* label, const char* tip)
    { return [label, tip, &parts](int& value) { return MeshOrNone(label, value, parts, tip); }; };
    changed |= LeftValue<Number>(Text::Opacity, values.opacity, 1.0f, number(Text::Opacity, Text::LayerOpacityTooltip));
    changed |= LeftValue<int>(Text::GlowMesh, values.glowMesh, 0, mesh(Text::GlowMesh, Text::LayerGlowMeshTooltip));
    changed |= LeftValue<Number>(Text::GlowStrength, values.glowStrength, 1.0f,
                                 number(Text::GlowStrength, Text::LayerGlowStrengthTooltip));
    changed |= LeftValue<Number>(Text::ScrollU, values.scrollU, 0.0f, number(Text::ScrollU, Text::LayerScrollTooltip));
    changed |= LeftValue<Number>(Text::ScrollV, values.scrollV, 0.0f, number(Text::ScrollV, Text::LayerScrollTooltip));
    changed |=
        LeftValue<int>(Text::HiddenMesh, values.hiddenMesh, 0, mesh(Text::HiddenMesh, Text::ItemHiddenMeshTooltip));
    changed |= LeftValue<int>(Text::ScrollingMesh, values.scrollingMesh, 0,
                              mesh(Text::ScrollingMesh, Text::LayerScrollingMeshTooltip));
    if (light)
    {
        changed |= LeftValue<LayerLight>(Text::Light, values.light, LayerLight{}, LeftLight);
        changed |= EachItemBoxes(values.eachItem, {{"opacity", &Text::Opacity},
                                                   {"glowMesh", &Text::GlowMesh},
                                                   {"glowStrength", &Text::GlowStrength},
                                                   {"scrollU", &Text::ScrollU},
                                                   {"scrollV", &Text::ScrollV},
                                                   {"hiddenMesh", &Text::HiddenMesh},
                                                   {"scrollingMesh", &Text::ScrollingMesh},
                                                   {"light", &Text::Light}});
    }
    return changed;
}

bool ItemValuesBeforeEntry(Data::Looks::ItemValuesBefore& entry, const std::vector<LookPart>& parts)
{
    namespace Text = I18N::Editor;
    constexpr int GlowLevel = 7;
    constexpr int MaxLevel = 15;
    bool changed = OnlyWhenFields(entry.onlyWhen, parts);
    // The drawing sets the model's light after the item effect: an entry has none.
    changed |= ItemValues(entry.values, parts, false);
    changed |= LeftValue<Number>(Text::DrawingOpacity, entry.drawingOpacity, 1.0f, [](Number& value)
                                 { return NumberField(Text::DrawingOpacity, value, Text::DrawingOpacityTooltip); });
    changed |= LeftValue<int>(Text::GlowsLikeLevel, entry.glowsLikeLevel, GlowLevel,
                              [](int& level)
                              {
                                  ImGui::SetNextItemWidth(Scaled(NumberWidth));
                                  const bool dragged = ImGui::DragInt(Text::GlowsLikeLevel, &level, CountSpeed, 0,
                                                                      MaxLevel, "+%d", ImGuiSliderFlags_AlwaysClamp);
                                  Tip(Text::GlowsLikeLevelTooltip, true);
                                  return dragged;
                              });
    changed |= EachItemBoxes(entry.values.eachItem, {{"opacity", &Text::Opacity},
                                                     {"glowMesh", &Text::GlowMesh},
                                                     {"glowStrength", &Text::GlowStrength},
                                                     {"scrollU", &Text::ScrollU},
                                                     {"scrollV", &Text::ScrollV},
                                                     {"hiddenMesh", &Text::HiddenMesh},
                                                     {"scrollingMesh", &Text::ScrollingMesh},
                                                     {"drawingOpacity", &Text::DrawingOpacity},
                                                     {"glowsLikeLevel", &Text::GlowsLikeLevel}});
    return changed;
}

bool ObjectLight(std::optional<std::array<Number, 3>>& light)
{
    namespace Text = I18N::Editor;
    ImGui::PushID("ObjectLight");
    bool set = light.has_value();
    bool changed = ImGui::Checkbox(Text::ObjectLight, &set);
    Tip(Text::ObjectLightTooltip);
    if (changed)
    {
        light = set ? std::optional<std::array<Number, 3>>(std::array<Number, 3>{1.0f, 1.0f, 1.0f}) : std::nullopt;
    }
    if (light)
    {
        changed |= ColorField(*light);
    }
    ImGui::PopID();
    return changed;
}

std::string DescribeItemValuesBefore(const Data::Looks::ItemValuesBefore& entry)
{
    std::string text = DescribeItemValues(entry.values);
    if (entry.drawingOpacity)
    {
        AddPart(text, I18N::Editor::DrawingOpacity);
    }
    if (entry.glowsLikeLevel)
    {
        AddPart(text, std::string(I18N::Editor::GlowsLikeLevel) + " +" + std::to_string(*entry.glowsLikeLevel));
    }
    if (!entry.onlyWhen.IsEmpty())
    {
        text = std::string(I18N::Editor::OnlyWhen) + ": " + text;
    }
    return text.empty() ? std::string(I18N::Editor::None) : text;
}

std::string DescribeLayer(const Data::Looks::Layer& layer)
{
    std::string text = NameIn(KindNames(), layer.kind);
    if (layer.kind == LayerKind::Reflection)
    {
        text += ": " + NameIn(MotionNames(), layer.motion);
        if (layer.where != Where::Over)
        {
            AddPart(text, NameIn(WhereNames(), layer.where));
        }
    }
    if (layer.blend != Blend::Normal)
    {
        AddPart(text, NameIn(BlendNames(), layer.blend));
    }
    AddPart(text, DescribeMeshes(layer.meshes));
    std::string fields;
    if (!layer.texture.empty())
    {
        AddPart(fields, layer.texture);
    }
    if (layer.light)
    {
        AddPart(fields, I18N::Editor::Light);
    }
    if (!std::holds_alternative<FromItem>(layer.opacity))
    {
        AddPart(fields, I18N::Editor::Opacity);
    }
    if (!std::holds_alternative<FromItem>(layer.glowMesh))
    {
        AddPart(fields, I18N::Editor::GlowMesh);
    }
    if (!std::holds_alternative<FromItem>(layer.glowStrength))
    {
        AddPart(fields, I18N::Editor::GlowStrength);
    }
    if (!std::holds_alternative<FromItem>(layer.scrollU))
    {
        AddPart(fields, I18N::Editor::ScrollU);
    }
    if (!std::holds_alternative<FromItem>(layer.scrollV))
    {
        AddPart(fields, I18N::Editor::ScrollV);
    }
    if (layer.scrollingMesh)
    {
        AddPart(fields, I18N::Editor::ScrollingMesh);
    }
    if (!fields.empty())
    {
        AddPart(text, fields, "; ");
    }
    return text;
}

std::string DescribeItemValues(const Data::Looks::ItemValues& values)
{
    std::string text;
    const auto add = [&text](bool set, const char* label)
    {
        if (set)
        {
            AddPart(text, label);
        }
    };
    add(values.opacity.has_value(), I18N::Editor::Opacity);
    add(values.glowMesh.has_value(), I18N::Editor::GlowMesh);
    add(values.glowStrength.has_value(), I18N::Editor::GlowStrength);
    add(values.scrollU.has_value(), I18N::Editor::ScrollU);
    add(values.scrollV.has_value(), I18N::Editor::ScrollV);
    add(values.hiddenMesh.has_value(), I18N::Editor::HiddenMesh);
    add(values.scrollingMesh.has_value(), I18N::Editor::ScrollingMesh);
    add(values.light.has_value(), I18N::Editor::Light);
    return text;
}
} // namespace MuEditor::Looks::Fields

#endif // _EDITOR
