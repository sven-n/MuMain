#include "Render/RmlUi/RmlAdditiveImageDecorator.h"

#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/Decorator.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Factory.h>
#include <RmlUi/Core/Geometry.h>
#include <RmlUi/Core/Mesh.h>
#include <RmlUi/Core/MeshUtilities.h>
#include <RmlUi/Core/PropertyDefinition.h>
#include <RmlUi/Core/PropertyDictionary.h>
#include <RmlUi/Core/RenderManager.h>

#include <cmath>

namespace
{
class AdditiveImageDecorator final : public Rml::Decorator
{
public:
    AdditiveImageDecorator(Rml::Colourb colour, Rml::Texture texture) : m_colour(colour)
    {
        AddTexture(texture);
    }

    Rml::DecoratorDataHandle GenerateElementData(Rml::Element* element, Rml::BoxArea paint_area) const override
    {
        const float opacity = element->GetComputedValues().opacity();
        const auto channel = [opacity](Rml::byte value)
        { return static_cast<Rml::byte>(std::lround(static_cast<float>(value) * opacity)); };
        const Rml::ColourbPremultiplied additive(channel(m_colour.red), channel(m_colour.green), channel(m_colour.blue),
                                                 0);

        const Rml::Box& box = element->GetBox();
        Rml::Mesh mesh;
        Rml::MeshUtilities::GenerateQuad(mesh, box.GetPosition(paint_area), box.GetSize(paint_area), additive,
                                         Rml::Vector2f(0.f, 0.f), Rml::Vector2f(1.f, 1.f));
        auto* geometry = new Rml::Geometry(element->GetRenderManager()->MakeGeometry(std::move(mesh)));
        return reinterpret_cast<Rml::DecoratorDataHandle>(geometry);
    }

    void ReleaseElementData(Rml::DecoratorDataHandle element_data) const override
    {
        delete reinterpret_cast<Rml::Geometry*>(element_data);
    }

    void RenderElement(Rml::Element* element, Rml::DecoratorDataHandle element_data) const override
    {
        reinterpret_cast<Rml::Geometry*>(element_data)
            ->Render(element->GetAbsoluteOffset(Rml::BoxArea::Border), GetTexture());
    }

private:
    Rml::Colourb m_colour;
};

class AdditiveImageDecoratorInstancer final : public Rml::DecoratorInstancer
{
public:
    AdditiveImageDecoratorInstancer()
    {
        m_colourId = RegisterProperty("color", "#ffffff").AddParser("color").GetId();
        m_imageId = RegisterProperty("image", "").AddParser("string").GetId();
        RegisterShorthand("decorator", "color, image", Rml::ShorthandType::FallThrough);
    }

    Rml::SharedPtr<Rml::Decorator>
    InstanceDecorator(const Rml::String&, const Rml::PropertyDictionary& properties,
                      const Rml::DecoratorInstancerInterface& instancer_interface) override
    {
        const Rml::Property* image = properties.GetProperty(m_imageId);
        if (image == nullptr)
            return nullptr;
        const Rml::Texture texture = instancer_interface.GetTexture(image->Get<Rml::String>());
        if (!texture)
            return nullptr;
        return Rml::MakeShared<AdditiveImageDecorator>(properties.GetProperty(m_colourId)->Get<Rml::Colourb>(),
                                                       texture);
    }

private:
    Rml::PropertyId m_colourId{};
    Rml::PropertyId m_imageId{};
};
} // namespace

void Render::RmlUi::RegisterAdditiveImageDecorator()
{
    static AdditiveImageDecoratorInstancer instancer;
    Rml::Factory::RegisterDecoratorInstancer("additive-image", &instancer);
}
