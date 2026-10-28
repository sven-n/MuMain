//*****************************************************************************
// File: UIMapName.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UIMapName.h"
#include "World/MapInfra/MapManager.h"
#include "UI/HUD/MapNameOrnament.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "Core/Utilities/StringUtils.h"
#include "I18N/All.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>
#include <cmath>

#ifdef ASG_ADD_GENS_SYSTEM
#include "Engine/Object/ZzzInventory.h"
#endif // ASG_ADD_GENS_SYSTEM

#define UIMN_SHOW_TIME 5000
#define UIMN_ALPHA_VARIATION 0.015f

namespace
{
// How much of a long name's squeeze also makes it lower, as a power of the width's squeeze: the
// originals' long names (Crywolf Fortress, Illusion Temple) are about 0.6x as wide as the font
// writes them and 0.85x as high as their short names.
constexpr float ShareOfSqueezeInHeight = 1.f / 3.f;
} // namespace

CUIMapName::CUIMapName()
{
    I18N::RegisterLocaleObserver(&CUIMapName::OnLocaleChanged, this);
}

CUIMapName::~CUIMapName()
{
    I18N::UnregisterLocaleObserver(&CUIMapName::OnLocaleChanged, this);
}

void CUIMapName::OnLocaleChanged(void* context) noexcept
{
    static_cast<CUIMapName*>(context)->RefreshName();
}

void CUIMapName::RefreshName()
{
    using UI::MapName::MapNameRmlModel;
    const auto* name = gMapManager.FindMapName(gMapManager.WorldActive);
    SyncField(m_RmlView.Binder(), &MapNameRmlModel::mapName, "map_name",
              name != nullptr ? StringUtils::WideToNarrow(name) : Rml::String());
}

void CUIMapName::RefreshIcon()
{
    using UI::MapName::MapNameRmlModel;
    const auto* icon = UI::MapName::GetMapNameOrnament().FindIcon(gMapManager.WorldActive);
    auto& binder = m_RmlView.Binder();
    SyncField(binder, &MapNameRmlModel::icon, "icon", icon != nullptr);
    if (icon == nullptr)
        return;
    SyncField(binder, &MapNameRmlModel::iconSource, "icon_source",
              Rml::CreateString("/%s/%s.tga", UI::MapName::OrnamentDirectory, icon->image.c_str()));
    SyncField(binder, &MapNameRmlModel::iconStyle, "icon_style",
              Rml::CreateString("left: %gpx; top: %gpx; width: %gpx; height: %gpx;", icon->x, icon->y, icon->width,
                                icon->height));
}

void CUIMapName::CacheNameElements()
{
    auto* document = m_RmlView.Document();
    auto* text = document->GetElementById("name_text");
    auto* room = document->GetElementById("name_room");
    m_NameText = text ? text->GetObserverPtr() : Rml::ObserverPtr<Rml::Element>();
    m_NameRoom = room ? room->GetObserverPtr() : Rml::ObserverPtr<Rml::Element>();
    m_FittedRoomWidth = 0.f;
}

// The original images fitted a long name into the image's width, mostly with narrower letters and
// a little with smaller ones; the banner does the same with a name wider than its #name_room.
void CUIMapName::AfterRmlLayout(Rml::Context* context)
{
    auto* document = m_RmlView.Document();
    if (!document || document->GetContext() != context || !document->IsVisible() || !m_NameText || !m_NameRoom)
        return;

    const float roomWidth = m_NameRoom->GetOffsetWidth();
    const float width = m_NameText->GetOffsetWidth();
    if (roomWidth <= 0.f || (width == m_FittedTextWidth && roomWidth == m_FittedRoomWidth))
        return;
    m_FittedTextWidth = width;
    m_FittedRoomWidth = roomWidth;
    Rml::String transform = "none";
    if (width > roomWidth && roomWidth > 0.f)
    {
        const float squeeze = roomWidth / width;
        transform = Rml::CreateString("scale(%.4f, %.4f)", squeeze, std::pow(squeeze, ShareOfSqueezeInHeight));
    }
    for (const char* id : {"name", "name_shine"})
        if (auto* element = document->GetElementById(id))
            element->SetProperty("transform", transform);
    document->UpdateDocument(); // Fit before this frame is drawn, after bindings and layout settled.
}

void CUIMapName::Init()
{
    m_eState = HIDE;
    m_dwOldTime = ::timeGetTime();
    m_dwDeltaTickSum = 0;
    m_fAlpha = 1.0f;
#ifdef ASG_ADD_GENS_SYSTEM
    m_bStrife = false;
#endif // ASG_ADD_GENS_SYSTEM
}

void CUIMapName::ShowMapName()
{
    m_eState = FADEIN;
    m_fAlpha = 0.2f;
    m_dwDeltaTickSum = 0;

    RefreshName();
    RefreshIcon();
    if (gMapManager.WorldActive == WD_40AREA_FOR_GM || m_RmlView.GetModel().mapName.empty())
    {
        m_eState = HIDE;
        return;
    }

#ifdef ASG_ADD_GENS_SYSTEM
    m_bStrife = ::IsStrifeMap(gMapManager.WorldActive);
#endif // ASG_ADD_GENS_SYSTEM
}

void CUIMapName::Update()
{
    DWORD dwNowTime = ::timeGetTime();
    DWORD dwDeltaTick = dwNowTime - m_dwOldTime;

    switch (m_eState)
    {
    case FADEIN:
        m_fAlpha += UIMN_ALPHA_VARIATION;
        if (1.0f <= m_fAlpha)
        {
            m_eState = SHOW;
            m_fAlpha = 1.0f;
        }
        break;

    case SHOW:
        m_dwDeltaTickSum += dwDeltaTick;
        if (m_dwDeltaTickSum > UIMN_SHOW_TIME)
        {
            m_eState = FADEOUT;
            m_dwDeltaTickSum = 0;
        }
        break;

    case FADEOUT:
        m_fAlpha -= UIMN_ALPHA_VARIATION;
        if (0.0f >= m_fAlpha)
        {
            m_eState = HIDE;
            m_fAlpha = 0.0f;
        }
        break;
    }

    m_dwOldTime = dwNowTime;
}

void CUIMapName::Render()
{
    Update();
    BuildRmlUi();
    SyncView();
}

namespace
{
// The loaded Gens warning is shared by every battle map. Retry a missing bitmap
// without storing CGlobalBitmap's error sentinel as an image source.
Rml::String BitmapSource(GLuint bitmapIndex)
{
    BITMAP_t* bitmap = Bitmaps.FindTexture(bitmapIndex);
    if (bitmap == nullptr)
        return {};
    Rml::String source = "/";
    for (const wchar_t* c = bitmap->FileName; *c != L'\0'; ++c)
        source.push_back(*c == L'\\' ? '/' : static_cast<char>(*c));
    return source;
}
} // namespace

void CUIMapName::BindRmlModel(Rml::DataModelConstructor& c, UI::MapName::MapNameRmlModel& model)
{
    c.Bind("alpha", &model.alpha);
    c.Bind("map_name", &model.mapName);
    c.Bind("icon", &model.icon);
    c.Bind("icon_source", &model.iconSource);
    c.Bind("icon_style", &model.iconStyle);
    c.Bind("strife", &model.strife);
    c.Bind("strife_source", &model.strifeSource);
}

void CUIMapName::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void CUIMapName::SyncView()
{
    if (m_RmlView.Document() == nullptr)
        return;

    const bool visible = HIDE != m_eState;
    UI::RmlBridge::SyncDocumentVisibilityBehind(m_RmlView.Document(), visible);
    if (!visible)
        return;

    using UI::MapName::MapNameRmlModel;
    SyncField(m_RmlView.Binder(), &MapNameRmlModel::alpha, "alpha", std::clamp(m_fAlpha, 0.0f, 1.0f));
#ifdef ASG_ADD_GENS_SYSTEM
    if (m_RmlView.GetModel().strife != m_bStrife || (m_bStrife && m_RmlView.GetModel().strifeSource.empty()))
    {
        SyncField(m_RmlView.Binder(), &MapNameRmlModel::strife, "strife", m_bStrife);
        SyncField(m_RmlView.Binder(), &MapNameRmlModel::strifeSource, "strife_source",
                  m_bStrife ? BitmapSource(BITMAP_INTERFACE_EX + 47) : Rml::String());
    }
#endif // ASG_ADD_GENS_SYSTEM
}
