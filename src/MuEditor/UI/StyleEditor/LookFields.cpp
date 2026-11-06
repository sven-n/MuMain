#include "stdafx.h"

#ifdef _EDITOR

#include "LookFields.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/ItemTools/ItemTextInput.h"
#include "I18N/All.h"
#include "imgui.h"

#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <utility>
#include <variant>
#include <vector>

namespace MuEditor::Looks::Fields
{
namespace
{
using namespace Data::Looks;

constexpr double Pi = 3.14159265358979323846;
// Pixel widths before the editor UI scale.
constexpr float ComboWidth = 190.0f;
constexpr float NumberWidth = 110.0f;
constexpr float GraphWidth = 300.0f;
constexpr float GraphHeight = 44.0f;
constexpr int GraphSamples = 160;
// The meshes a look names, as the file reads them.
constexpr int LastMesh = 49;
constexpr int MaxFrames = 1000;

float Scaled(float width)
{
    return width * g_MuEditorCore.GetUIScale();
}

template <typename T> using Names = std::vector<std::pair<T, const char* const*>>;

// A combo of named choices, translated when drawn.
template <typename T> bool Choice(const char* label, T& value, const Names<T>& names)
{
    const char* shown = "";
    for (const auto& [choice, name] : names)
    {
        if (choice == value)
        {
            shown = *name;
        }
    }
    bool changed = false;
    ImGui::SetNextItemWidth(Scaled(ComboWidth));
    if (ImGui::BeginCombo(label, shown))
    {
        for (const auto& [choice, name] : names)
        {
            if (ImGui::Selectable(*name, choice == value) && choice != value)
            {
                value = choice;
                changed = true;
            }
        }
        ImGui::EndCombo();
    }
    return changed;
}

const Names<LayerKind>& KindNames()
{
    static const Names<LayerKind> names{{LayerKind::Texture, &I18N::Editor::Texture},
                                        {LayerKind::Reflection, &I18N::Editor::Reflection}};
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

const Names<Blend>& BlendNames()
{
    static const Names<Blend> names{{Blend::Normal, &I18N::Editor::Normal},
                                    {Blend::Glow, &I18N::Editor::Glow},
                                    {Blend::AsDrawing, &I18N::Editor::AsTheDrawingAsks}};
    return names;
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
};

const Names<Source>& SourceNames()
{
    static const Names<Source> names{
        {Source::Item, &I18N::Editor::FromTheItem},  {Source::Drawing, &I18N::Editor::FromTheDrawing},
        {Source::Number, &I18N::Editor::ANumber},    {Source::Wave, &I18N::Editor::Wave},
        {Source::Bounce, &I18N::Editor::Bounce},     {Source::Scroll, &I18N::Editor::Scroll},
        {Source::Ramp, &I18N::Editor::Ramp},         {Source::Frames, &I18N::Editor::Frames},
        {Source::Random, &I18N::Editor::RandomValue}};
    return names;
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
    case Source::Frames:
        return {.kind = ShapeKind::Frames, .seconds = 0.1, .step = 0.25, .count = 4};
    case Source::Random:
        return {.kind = ShapeKind::Random, .from = from, .to = std::max(from, to), .step = 0.1};
    case Source::Wave:
    default:
        return {.kind = ShapeKind::Wave, .from = from, .to = to, .seconds = 1.0};
    }
}

bool InputNumber(const char* label, double& value)
{
    ImGui::SetNextItemWidth(Scaled(NumberWidth));
    return ImGui::InputDouble(label, &value, 0.0, 0.0, "%.7g");
}

// The span of the curve: two cycles, or ten seconds of a scroll.
double CurveSeconds(const Shape& shape)
{
    double seconds = 10.0;
    switch (shape.kind)
    {
    case ShapeKind::Wave:
    case ShapeKind::Bounce:
    case ShapeKind::Ramp:
        seconds = 2.0 * shape.seconds;
        break;
    case ShapeKind::Frames:
        seconds = 2.0 * shape.seconds * shape.count;
        break;
    default:
        break;
    }
    return std::clamp(seconds, 0.001, 600.0);
}

void RenderCurve(const Shape& shape)
{
    if (shape.kind == ShapeKind::Random)
    {
        ImGui::TextDisabled("%s", I18N::Editor::ANewValueEachFrame);
        return;
    }
    const double span = CurveSeconds(shape);
    std::array<float, GraphSamples> values{};
    for (int sample = 0; sample < GraphSamples; ++sample)
    {
        values[static_cast<std::size_t>(sample)] =
            static_cast<float>(ShapeValueAt(shape, span * sample / (GraphSamples - 1)));
    }
    char overlay[64];
    std::snprintf(overlay, sizeof(overlay), "0 - %.3g s", span);
    ImGui::PlotLines("##Curve", values.data(), GraphSamples, 0, overlay, FLT_MAX, FLT_MAX,
                     ImVec2(Scaled(GraphWidth), Scaled(GraphHeight)));
}

bool ShapeFields(Shape& shape)
{
    bool changed = false;
    ImGui::Indent();
    switch (shape.kind)
    {
    case ShapeKind::Wave:
    case ShapeKind::Bounce:
    case ShapeKind::Ramp:
        changed |= InputNumber(I18N::Editor::From, shape.from);
        ImGui::SameLine();
        changed |= InputNumber(I18N::Editor::To, shape.to);
        ImGui::SameLine();
        changed |= InputNumber(I18N::Editor::Seconds, shape.seconds);
        break;
    case ShapeKind::Scroll:
        changed |= InputNumber(I18N::Editor::PerSecond, shape.perSecond);
        break;
    case ShapeKind::Frames:
        ImGui::SetNextItemWidth(Scaled(NumberWidth));
        if (ImGui::InputInt(I18N::Editor::Count, &shape.count))
        {
            shape.count = std::clamp(shape.count, 1, MaxFrames);
            changed = true;
        }
        ImGui::SameLine();
        changed |= InputNumber(I18N::Editor::Step, shape.step);
        ImGui::SameLine();
        changed |= InputNumber(I18N::Editor::SecondsEach, shape.seconds);
        break;
    case ShapeKind::Random:
        changed |= InputNumber(I18N::Editor::From, shape.from);
        ImGui::SameLine();
        changed |= InputNumber(I18N::Editor::To, shape.to);
        ImGui::SameLine();
        changed |= InputNumber(I18N::Editor::Step, shape.step);
        break;
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

// An opacity, glow strength or scroll: the item's, the drawing's (`drawing`: only an opacity), a number or a shape.
bool ValueField(const char* label, Value& value, bool drawing, bool item = true)
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
    if (Choice(label, source, sources))
    {
        value = MakeValue(source, value);
        changed = true;
    }
    if (float* number = std::get_if<float>(&value))
    {
        ImGui::SameLine();
        ImGui::SetNextItemWidth(Scaled(NumberWidth));
        changed |= ImGui::InputFloat("##Number", number, 0.0f, 0.0f, "%.7g");
    }
    else if (Shape* shape = std::get_if<Shape>(&value))
    {
        changed |= ShapeFields(*shape);
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
bool NumberField(const char* label, Number& number)
{
    Value value = ToValue(number);
    if (!ValueField(label, value, false, false))
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
    static ImGuiID editing = 0;
    static std::string text;
    const ImGuiID id = ImGui::GetID(label);
    if (editing != id)
    {
        text = JoinMeshes(meshes);
    }
    ImGui::SetNextItemWidth(Scaled(ComboWidth));
    Items::InputStringWithHint(label, I18N::Editor::MeshNumbersHint, text);
    bool changed = false;
    if (ImGui::IsItemActive())
    {
        editing = id;
    }
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        std::vector<int> parsed = ParseMeshes(text);
        if (!parsed.empty() && parsed != meshes)
        {
            meshes = std::move(parsed);
            changed = true;
        }
    }
    if (!ImGui::IsItemActive() && editing == id)
    {
        editing = 0;
    }
    return changed;
}

bool MeshField(const char* label, int& mesh)
{
    ImGui::SameLine();
    ImGui::SetNextItemWidth(Scaled(NumberWidth));
    if (!ImGui::InputInt(label, &mesh))
    {
        return false;
    }
    mesh = std::clamp(mesh, 0, LastMesh);
    return true;
}

enum class MeshChoice
{
    All,
    These,
    AllBut,
};

bool MeshesField(MeshSet& meshes)
{
    ImGui::PushID("Meshes");
    bool changed = false;
    MeshChoice choice = meshes.allBut ? MeshChoice::AllBut : meshes.IsAll() ? MeshChoice::All : MeshChoice::These;
    static const Names<MeshChoice> names{{MeshChoice::All, &I18N::Editor::EveryMesh},
                                         {MeshChoice::These, &I18N::Editor::TheseMeshes},
                                         {MeshChoice::AllBut, &I18N::Editor::EveryMeshBut}};
    if (Choice(I18N::Editor::Meshes, choice, names))
    {
        if (choice == MeshChoice::All)
        {
            meshes = {};
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
    }
    else
    {
        changed |= MeshListField("##List", meshes.meshes);
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

bool GlowMeshField(GlowMesh& mesh)
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
    if (Choice(I18N::Editor::GlowMesh, choice, names))
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
            mesh = 0;
            break;
        }
        changed = true;
    }
    if (int* number = std::get_if<int>(&mesh))
    {
        changed |= MeshField("##Mesh", *number);
    }
    ImGui::PopID();
    return changed;
}

// A mesh, or nothing: `empty` names what nothing is (the model's mesh, none).
bool OptionalMeshField(const char* label, std::optional<int>& mesh, const char* const* empty)
{
    ImGui::PushID(label);
    bool set = mesh.has_value();
    const Names<bool> names{{false, empty}, {true, &I18N::Editor::AMesh}};
    bool changed = false;
    if (Choice(label, set, names))
    {
        mesh = set ? std::optional<int>(0) : std::nullopt;
        changed = true;
    }
    if (mesh)
    {
        changed |= MeshField("##Mesh", *mesh);
    }
    ImGui::PopID();
    return changed;
}

enum class LightChoice
{
    Before,
    Color,
    Tint,
};

bool LightField(std::optional<LayerLight>& light)
{
    ImGui::PushID("Light");
    LightChoice choice = !light ? LightChoice::Before : light->tint ? LightChoice::Tint : LightChoice::Color;
    static const Names<LightChoice> names{{LightChoice::Before, &I18N::Editor::TheLightBefore},
                                          {LightChoice::Color, &I18N::Editor::AColor},
                                          {LightChoice::Tint, &I18N::Editor::TintOfTheItemLight}};
    bool changed = false;
    if (Choice(I18N::Editor::Light, choice, names))
    {
        if (choice == LightChoice::Before)
        {
            light.reset();
        }
        else
        {
            const LayerLight kept = light.value_or(LayerLight{});
            light = LayerLight{.tint = choice == LightChoice::Tint, .color = kept.color};
        }
        changed = true;
    }
    if (light)
    {
        ImGui::Indent();
        const char* const* channels[] = {&I18N::Editor::Red, &I18N::Editor::Green, &I18N::Editor::Blue};
        for (std::size_t channel = 0; channel < 3; ++channel)
        {
            changed |= NumberField(*channels[channel], light->color[channel]);
        }
        ImGui::Unindent();
    }
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
bool MeshOrNone(const char* label, int& mesh)
{
    std::optional<int> set = mesh >= 0 ? std::optional<int>(mesh) : std::nullopt;
    if (!OptionalMeshField(label, set, &I18N::Editor::None))
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

bool DrawsGlowing(const GlowMesh& mesh)
{
    return std::holds_alternative<int>(mesh) || std::holds_alternative<AllGlowMeshes>(mesh);
}

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

double ShapeValueAt(const Shape& shape, double seconds)
{
    switch (shape.kind)
    {
    case ShapeKind::Wave:
        return shape.seconds > 0.0 ? (shape.from + shape.to) / 2.0 +
                                         (shape.to - shape.from) / 2.0 * std::sin(2.0 * Pi * seconds / shape.seconds)
                                   : shape.from;
    case ShapeKind::Bounce:
        return shape.seconds > 0.0
                   ? shape.from + std::abs(std::sin(Pi * seconds / shape.seconds)) * (shape.to - shape.from)
                   : shape.from;
    case ShapeKind::Scroll:
        return shape.perSecond * seconds;
    case ShapeKind::Ramp:
        return shape.seconds > 0.0
                   ? shape.from + std::fmod(seconds, shape.seconds) / shape.seconds * (shape.to - shape.from)
                   : shape.from;
    case ShapeKind::Frames:
    {
        if (shape.seconds <= 0.0 || shape.count <= 0)
        {
            return 0.0;
        }
        const auto frame = static_cast<long long>(std::floor(seconds / shape.seconds)) % shape.count;
        return static_cast<double>(frame) * shape.step;
    }
    case ShapeKind::Random:
        return (shape.from + shape.to) / 2.0;
    }
    return shape.from;
}

bool Layer(Data::Looks::Layer& layer)
{
    bool changed = false;
    if (Choice(I18N::Editor::Kind, layer.kind, KindNames()))
    {
        // A texture layer has no motion; the file leaves it out.
        layer.motion = Motion::SlowDrift;
        changed = true;
    }
    if (layer.kind == LayerKind::Reflection)
    {
        changed |= Choice(I18N::Editor::Motion, layer.motion, MotionNames());
    }
    changed |= Choice(I18N::Editor::Blend, layer.blend, BlendNames());
    changed |= MeshesField(layer.meshes);
    ImGui::SetNextItemWidth(Scaled(GraphWidth));
    changed |= Items::InputStringWithHint(I18N::Editor::Texture, I18N::Editor::TextureHint, layer.texture);
    changed |= LightField(layer.light);
    changed |= ValueField(I18N::Editor::Opacity, layer.opacity, true);
    changed |= GlowMeshField(layer.glowMesh);
    changed |= ValueField(I18N::Editor::GlowStrength, layer.glowStrength, false);
    changed |= ValueField(I18N::Editor::ScrollU, layer.scrollU, false);
    changed |= ValueField(I18N::Editor::ScrollV, layer.scrollV, false);
    changed |= OptionalMeshField(I18N::Editor::ScrollingMesh, layer.scrollingMesh, &I18N::Editor::KeptFromTheModel);
    return changed;
}

bool Effect(Data::Looks::Layer& layer, const LayerEffect& effect)
{
    const unsigned fields = FieldsOf(layer, effect);
    bool changed = MeshesField(layer.meshes);
    if ((fields & EffectField::Motion) != 0 && layer.kind == LayerKind::Reflection)
    {
        changed |= Choice(I18N::Editor::Motion, layer.motion, MotionNames());
    }
    if ((fields & EffectField::Blend) != 0)
    {
        changed |= Choice(I18N::Editor::Blend, layer.blend, BlendNames());
    }
    if ((fields & EffectField::Texture) != 0)
    {
        ImGui::SetNextItemWidth(Scaled(GraphWidth));
        changed |= Items::InputStringWithHint(I18N::Editor::Texture, I18N::Editor::TextureHint, layer.texture);
    }
    if ((fields & EffectField::Light) != 0)
    {
        changed |= LightField(layer.light);
    }
    if ((fields & EffectField::Opacity) != 0)
    {
        changed |= ValueField(I18N::Editor::Opacity, layer.opacity, true);
    }
    if ((fields & EffectField::GlowMesh) != 0)
    {
        changed |= GlowMeshField(layer.glowMesh);
    }
    if ((fields & EffectField::GlowStrength) != 0)
    {
        changed |= ValueField(I18N::Editor::GlowStrength, layer.glowStrength, false);
    }
    if ((fields & EffectField::Scroll) != 0)
    {
        changed |= ValueField(I18N::Editor::ScrollU, layer.scrollU, false);
        changed |= ValueField(I18N::Editor::ScrollV, layer.scrollV, false);
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

bool ItemValues(Data::Looks::ItemValues& values)
{
    bool changed = false;
    const auto number = [](const char* label) { return [label](Number& value) { return NumberField(label, value); }; };
    const auto mesh = [](const char* label) { return [label](int& value) { return MeshOrNone(label, value); }; };
    changed |= LeftValue<Number>(I18N::Editor::Opacity, values.opacity, 1.0f, number(I18N::Editor::Opacity));
    changed |= LeftValue<int>(I18N::Editor::GlowMesh, values.glowMesh, 0, mesh(I18N::Editor::GlowMesh));
    changed |=
        LeftValue<Number>(I18N::Editor::GlowStrength, values.glowStrength, 1.0f, number(I18N::Editor::GlowStrength));
    changed |= LeftValue<Number>(I18N::Editor::ScrollU, values.scrollU, 0.0f, number(I18N::Editor::ScrollU));
    changed |= LeftValue<Number>(I18N::Editor::ScrollV, values.scrollV, 0.0f, number(I18N::Editor::ScrollV));
    changed |= LeftValue<int>(I18N::Editor::HiddenMesh, values.hiddenMesh, 0, mesh(I18N::Editor::HiddenMesh));
    changed |= LeftValue<int>(I18N::Editor::ScrollingMesh, values.scrollingMesh, 0, mesh(I18N::Editor::ScrollingMesh));
    return changed;
}

std::string DescribeLayer(const Data::Looks::Layer& layer)
{
    std::string text = NameIn(KindNames(), layer.kind);
    if (layer.kind == LayerKind::Reflection)
    {
        text += ": " + NameIn(MotionNames(), layer.motion);
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
    return text;
}
} // namespace MuEditor::Looks::Fields

#endif // _EDITOR
