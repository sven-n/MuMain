#include "stdafx.h"
#include "UI/Inventory/ItemCameraTarget.h"
#include "UI/Inventory/ItemCamera.h"
#include "UI/RmlBridge/RmlElementBox.h"

#include <cmath>

#include <RmlUi/Core/Element.h>

UI::Items::ItemCameraTarget::ItemCameraTarget(Drawer drawer)
    : m_drawer(std::move(drawer)),
      m_target([this](std::uint32_t width, std::uint32_t height) { Render(width, height); })
{
}

void UI::Items::ItemCameraTarget::Sync(Rml::Element* image, bool enabled)
{
    if (image == nullptr)
    {
        m_target.SetEnabled(false);
        return;
    }
    const bool placed = UI::RmlBridge::DrawnContentBox(*image, m_offset, m_size);
    const bool drawing = enabled && placed && m_size.x >= 1.f && m_size.y >= 1.f;
    m_target.SetEnabled(drawing);
    // Drawn at the size it is shown, never upscaled.
    if (drawing)
        m_target.Resize(static_cast<std::uint32_t>(std::lround(m_size.x)), static_cast<std::uint32_t>(std::lround(m_size.y)));
    // An <img> without a texture still draws its quad, untextured: white. It stays laid out, since
    // the target takes its size from the box.
    const Rml::String source = drawing ? m_target.Source() : Rml::String();
    if (source.empty())
        image->SetProperty("visibility", "hidden");
    else
        image->RemoveProperty("visibility");
    if (!source.empty() && image->GetAttribute<Rml::String>("src", "") != source)
        image->SetAttribute("src", source);
}

void UI::Items::ItemCameraTarget::Render(std::uint32_t width, std::uint32_t height)
{
    if (width == 0 || height == 0 || !m_drawer)
        return;
    if (m_size.x < 1.f || m_size.y < 1.f)
        return;
    DrawUnderItemCamera(m_offset.x, m_offset.y, m_size.x, m_size.y, m_fieldOfView,
                        [this] { m_drawer(m_offset, m_size); });
}
