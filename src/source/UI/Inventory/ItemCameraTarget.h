#pragma once

#include <cstdint>
#include <functional>

#include <RmlUi/Core/Types.h>

#include "UI/RmlBridge/RmlRenderTarget.h"
#include "UI/Scaling/UITransform.h"

namespace Rml
{
class Element;
}

// Live 3D items shown through an <img>: the drawer runs under the item camera the original drew
// every UI item with (an identity view at a 1-degree field of view over the whole window), its
// projection cropped to the image's box, so each RenderItem3D() call lands in the target exactly
// where it would have landed on screen; native 2D the drawer draws (with the depth test off) maps
// the same way. The drawer keeps its own coordinates: whatever space its RenderItem3D() rectangles
// are in, through the transform it was built with.
namespace UI::Items
{
class ItemCameraTarget
{
public:
    // Gets the image's box in window pixels, for a drawer that frames one item to fill it.
    using Drawer = std::function<void(const Rml::Vector2f& offset, const Rml::Vector2f& size)>;
    using TransformSource = std::function<UI::Scaling::Transform()>;

    // `transform` gives the space the drawer's rectangles are in; window pixels when empty.
    explicit ItemCameraTarget(Drawer drawer, TransformSource transform = {});

    // Once a frame: sizes the target to `image`'s box, shows it there and points its src at the
    // target. Null or disabled stops the drawing; the image is invisible while it has no frame.
    void Sync(Rml::Element* image, bool enabled);
    void Disable() { m_target.SetEnabled(false); }
    // The camera's vertical field of view, in degrees: the item camera's 1 unless a window drew
    // its items through another.
    void SetFieldOfView(float degrees) { m_fieldOfView = degrees; }

private:
    void Render(std::uint32_t width, std::uint32_t height);

    Drawer m_drawer;
    TransformSource m_transform;
    float m_fieldOfView = 1.f;
    // The image's box in window pixels as of the last Sync().
    Rml::Vector2f m_offset{0.f, 0.f};
    Rml::Vector2f m_size{0.f, 0.f};
    UI::RmlBridge::RenderTarget m_target;
};
} // namespace UI::Items
