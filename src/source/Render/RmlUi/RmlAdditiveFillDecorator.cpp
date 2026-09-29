#include "Render/RmlUi/RmlAdditiveFillDecorator.h"

#include <RmlUi/Core/Decorator.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Factory.h>
#include <RmlUi/Core/Geometry.h>
#include <RmlUi/Core/Mesh.h>
#include <RmlUi/Core/MeshUtilities.h>
#include <RmlUi/Core/PropertyDefinition.h>
#include <RmlUi/Core/PropertyDictionary.h>
#include <RmlUi/Core/RenderBox.h>
#include <RmlUi/Core/RenderManager.h>

namespace
{
class AdditiveFillDecorator final : public Rml::Decorator
{
public:
    explicit AdditiveFillDecorator(Rml::Colourb colour) : m_colour(colour) {}

    Rml::DecoratorDataHandle GenerateElementData(Rml::Element* element, Rml::BoxArea paint_area) const override
    {
        // GenerateBackground() skips a fully transparent colour, so the fill is generated opaque and
        // its alpha cleared afterwards.
        Rml::Mesh mesh;
        const Rml::ColourbPremultiplied additive(m_colour.red, m_colour.green, m_colour.blue, 0);
        Rml::MeshUtilities::GenerateBackground(
            mesh, element->GetRenderBox(paint_area),
            Rml::ColourbPremultiplied(m_colour.red, m_colour.green, m_colour.blue, 255));
        for (Rml::Vertex& vertex : mesh.vertices)
            vertex.colour = additive;
        auto* geometry = new Rml::Geometry(element->GetRenderManager()->MakeGeometry(std::move(mesh)));
        return reinterpret_cast<Rml::DecoratorDataHandle>(geometry);
    }

    void ReleaseElementData(Rml::DecoratorDataHandle element_data) const override
    {
        delete reinterpret_cast<Rml::Geometry*>(element_data);
    }

    void RenderElement(Rml::Element* element, Rml::DecoratorDataHandle element_data) const override
    {
        reinterpret_cast<Rml::Geometry*>(element_data)->Render(element->GetAbsoluteOffset(Rml::BoxArea::Border));
    }

private:
    Rml::Colourb m_colour;
};

class AdditiveFillDecoratorInstancer final : public Rml::DecoratorInstancer
{
public:
    AdditiveFillDecoratorInstancer()
    {
        m_colourId = RegisterProperty("color", "#000000").AddParser("color").GetId();
        RegisterShorthand("decorator", "color", Rml::ShorthandType::FallThrough);
    }

    Rml::SharedPtr<Rml::Decorator> InstanceDecorator(const Rml::String&, const Rml::PropertyDictionary& properties,
                                                     const Rml::DecoratorInstancerInterface&) override
    {
        return Rml::MakeShared<AdditiveFillDecorator>(properties.GetProperty(m_colourId)->Get<Rml::Colourb>());
    }

private:
    Rml::PropertyId m_colourId{};
};
} // namespace

void Render::RmlUi::RegisterAdditiveFillDecorator()
{
    static AdditiveFillDecoratorInstancer instancer;
    Rml::Factory::RegisterDecoratorInstancer("additive-fill", &instancer);
}
