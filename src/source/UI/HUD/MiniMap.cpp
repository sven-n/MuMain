
#include "stdafx.h"
#include "UI/Placement/WindowPlacement.h"
#include "I18N/All.h"

#include "UI/HUD/MiniMap.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "Audio/DSPlaySound.h"

#include "Guild/GuildInfoWindow.h"
#include "UI/Inventory/MyInventory.h"
#include "GameLogic/Items/CSItemOption.h"
#include "World/MapInfra/MapManager.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/HUD/MiniMapLayout.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Render/Text/CUIRenderText.h"

#include <RmlUi/Core/ElementDocument.h>

#include <algorithm>
#include <cstdio>

extern BYTE m_OccupationState;

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
// The part of the screen the original's mouse handler kept to itself.
constexpr int kMapAreaHeight = 430;
// Marker sizes, physical px (RenderPointRotate() never scaled them).
constexpr float kNpcSize = 15.f;
constexpr float kPortalSize = 30.f;
// The side border tiles: 35 x 6 reference units, 20 down each side, turned a quarter.
constexpr float kTileWidth = 35.f;
constexpr float kTileHeight = 6.f;
constexpr int kSideTiles = 20;
// .edge-side's element size in mini_map.rcss (the tile's texel rectangle).
constexpr float kSideElementWidth = 41.7f;
constexpr float kSideElementHeight = 8.f;

template <typename Model, typename T>
void Sync(RmlModelBinder<Model>& binder, T Model::* field, const char* name, T value)
{
    Model& model = binder.GetModel();
    if (model.*field == value)
        return;
    model.*field = std::move(value);
    binder.MarkDirty(name);
}

Rml::String MatrixText(const UI::MiniMap::CssMatrix& m)
{
    char buffer[192];
    std::snprintf(buffer, sizeof(buffer), "matrix(%.5f, %.5f, %.5f, %.5f, %.3f, %.3f)", m.a, m.b, m.c, m.d, m.e, m.f);
    return buffer;
}

// The original drew the map under the screen overlay's W/640 x H/480 stretch.
UI::Scaling::Transform OverlayTransform()
{
    return UI::Scaling::ScreenOverlayTransform(static_cast<int>(WindowWidth), static_cast<int>(WindowHeight));
}

UI::MiniMap::Screen CurrentScreen()
{
    const UI::Scaling::Transform transform = OverlayTransform();
    return {static_cast<float>(WindowWidth),
            static_cast<float>(WindowHeight),
            transform.scaleX,
            transform.scaleY,
            transform.offsetX,
            transform.offsetY};
}

// The marker the original drew: Kind 1 an NPC (hidden in Crywolf while it is occupied, but the
// one at 228/48), Kind 2 a gate.
bool MarkerDrawn(const MINI_MAP& data)
{
    if (data.Kind == 2)
        return true;
    return !(gMapManager.WorldActive == WD_34CRYWOLF_1ST && m_OccupationState > 0) ||
           (data.Location[0] == 228 && data.Location[1] == 48 && gMapManager.WorldActive == WD_34CRYWOLF_1ST);
}
} // namespace

mu::ui::window::CMiniMap::CMiniMap()
{
    m_pNewUIMng = NULL;
    m_bSuccess = false;
    for (auto& data : m_Mini_Map_Data)
        data.Kind = 0;
    for (auto& box : m_Btn_Loc)
        box[0] = box[1] = box[2] = box[3] = 0.f;
}

mu::ui::window::CMiniMap::~CMiniMap()
{
    Release();
}

bool mu::ui::window::CMiniMap::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_MINI_MAP, this);

    SetPos(x, y);
    m_bSuccess = false;

    BuildRmlUi();
    return true;
}

void mu::ui::window::CMiniMap::ClosingProcess()
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();
}

float mu::ui::window::CMiniMap::GetLayerDepth()
{
    return 8.1f;
}

void mu::ui::window::CMiniMap::OpenningProcess()
{
    m_PendingClose = false;
}

void mu::ui::window::CMiniMap::Release()
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    m_RmlView.Release();
}

void mu::ui::window::CMiniMap::SetPos(int x, int y)
{
    // Full screen; mini_map.rcss places the close button at the original's (640 - 27, 3).
}

void mu::ui::window::CMiniMap::SetBtnPos(int Num, float x, float y, float nx, float ny)
{
    m_Btn_Loc[Num][0] = x;
    m_Btn_Loc[Num][1] = y;
    m_Btn_Loc[Num][2] = nx;
    m_Btn_Loc[Num][3] = ny;
}

bool mu::ui::window::CMiniMap::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_MINI_MAP))
    {
        if (IsPress(VK_ESCAPE) == true || IsPress(VK_TAB) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MINI_MAP);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    return true;
}

bool mu::ui::window::CMiniMap::Render()
{
    // Nothing native left: the map, its markers, the frame and the hint are RmlUi. Kept because
    // CObject requires the override.
    return m_bSuccess;
}

bool mu::ui::window::CMiniMap::Update()
{
    SyncRmlModel();

    if (m_PendingClose)
    {
        m_PendingClose = false;
        if (IsVisible())
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MINI_MAP);
    }
    return true;
}

void mu::ui::window::CMiniMap::LoadImages(const wchar_t* Filename)
{
    wchar_t Fname[300];
    int i = 0;
    mu_swprintf(Fname, L"Data\\%ls\\mini_map.ozt", Filename);
    FILE* pFile = _wfopen(Fname, L"rb");

    if (pFile == NULL)
    {
        m_bSuccess = false;
        m_WorldName.clear();
        return;
    }
    else
    {
        m_bSuccess = true;
        fclose(pFile);
        // The texture itself is loaded by the document (mini_map.rml's .map).
        m_WorldName = Filename;
    }

    mu_swprintf(Fname, L"Data\\Local\\%ls\\Minimap\\Minimap_%ls_%ls.bmd", g_strSelectedML.c_str(), Filename, g_strSelectedML.c_str());

    for (i = 0; i < MAX_MINI_MAP_DATA; i++)
    {
        m_Mini_Map_Data[i].Kind = 0;
    }

    FILE* fp = _wfopen(Fname, L"rb");

    if (fp != NULL)
    {
        int Size = sizeof(MINI_MAP_FILE);
        BYTE* Buffer = new BYTE[Size * MAX_MINI_MAP_DATA + 45];
        fread(Buffer, (Size * MAX_MINI_MAP_DATA) + 45, 1, fp);

        DWORD dwCheckSum;
        fread(&dwCheckSum, sizeof(DWORD), 1, fp);
        fclose(fp);

        if (dwCheckSum != GenerateCheckSum2(Buffer, (Size * MAX_MINI_MAP_DATA) + 45, 0x2BC1))
        {
            wchar_t Text[256];
            mu_swprintf(Text, L"%ls - File corrupted.", Fname);
            g_ErrorReport.Write(Text);
            MessageBox(g_hWnd, Text, NULL, MB_OK);
            SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        }
        else
        {
            BYTE* pSeek = Buffer;

            for (i = 0; i < MAX_MINI_MAP_DATA; i++)
            {
                BuxConvert(pSeek, Size);

                MINI_MAP_FILE current{ };
                auto target = &(m_Mini_Map_Data[i]);
                memcpy(&current, pSeek, Size);
                memcpy(target, pSeek, Size);

                CMultiLanguage::ConvertFromUtf8(target->Name, current.Name);
                pSeek += Size;
            }
        }

        delete[] Buffer;
    }
}

void mu::ui::window::CMiniMap::UnloadImages()
{
    m_WorldName.clear();
}

bool mu::ui::window::CMiniMap::UpdateMouseEvent()
{
    // The close button's click is RmlUi's (minimap_close); like the original, the pointer over the
    // top 430 rows goes to nothing behind the map.
    const UI::Scaling::Transform overlay = OverlayTransform();
    const float pointerY = UI::Scaling::LogicalY(overlay, g_fWindowMouseY);
    return !(pointerY >= 0.f && pointerY < static_cast<float>(kMapAreaHeight));
}

void mu::ui::window::CMiniMap::BindRmlModel(Rml::DataModelConstructor& c, MiniMapRmlModel& model)
{
    c.Bind("text_px", &model.textPx);
    c.Bind("close_hint", &model.closeHint);

    auto clip = c.RegisterStruct<MiniMapClipEntry>();
    clip.RegisterMember("left", &MiniMapClipEntry::left);
    clip.RegisterMember("top", &MiniMapClipEntry::top);
    clip.RegisterMember("width", &MiniMapClipEntry::width);
    clip.RegisterMember("height", &MiniMapClipEntry::height);
    clip.RegisterMember("world_left", &MiniMapClipEntry::worldLeft);
    clip.RegisterMember("world_top", &MiniMapClipEntry::worldTop);
    c.RegisterArray<std::vector<MiniMapClipEntry>>();
    c.Bind("clips", &model.clips);

    c.Bind("map_source", &model.mapSource);
    c.Bind("map_transform", &model.mapTransform);

    auto marker = c.RegisterStruct<MiniMapMarkerEntry>();
    marker.RegisterMember("portal", &MiniMapMarkerEntry::portal);
    marker.RegisterMember("size", &MiniMapMarkerEntry::size);
    marker.RegisterMember("transform", &MiniMapMarkerEntry::transform);
    c.RegisterArray<std::vector<MiniMapMarkerEntry>>();
    c.Bind("markers", &model.markers);

    c.RegisterArray<std::vector<Rml::String>>();
    c.Bind("side_lines", &model.sideLines);
    c.RegisterArray<std::vector<float>>();

    c.Bind("hint_visible", &model.hintVisible);
    c.Bind("hint_text", &model.hintText);
    c.Bind("hint_left", &model.hintLeft);
    c.Bind("hint_top", &model.hintTop);
    c.Bind("hint_width", &model.hintWidth);
    c.Bind("hint_height", &model.hintHeight);

    c.BindEventCallback("minimap_close", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { m_PendingClose = true; });
}

void mu::ui::window::CMiniMap::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void mu::ui::window::CMiniMap::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    const bool visible = IsVisible() && m_bSuccess && Hero != nullptr;
    // Over the location bar, the logs and the buff strip, under the bottom HUD: the stacking table
    // (UI/RmlBridge/RmlStackingOrder.cpp) orders it as the original's layer depths did.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_RmlView.Document(), visible);

    if (!visible)
        return;

    SyncScreen();
    SyncClips();
    SyncMap();
    SyncHint();
}

void mu::ui::window::CMiniMap::SyncScreen()
{
    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());
    Sync(m_RmlView.Binder(), &MiniMapRmlModel::closeHint, "close_hint", StringUtils::WideToNarrow(I18N::Game::Close388));

    // The border tiles depend on the screen alone: rebuilt when it changes, or when a theme reload
    // left the model empty.
    const UI::MiniMap::Screen screen = CurrentScreen();
    if (screen == m_SideLinesScreen && !m_RmlView.GetModel().sideLines.empty())
        return;
    m_SideLinesScreen = screen;
    std::vector<Rml::String> sideLines;
    sideLines.reserve(kSideTiles * 2);
    for (int i = 0; i < kSideTiles; ++i)
    {
        const float y = static_cast<float>(i) * (kTileWidth - 3.f);
        sideLines.push_back(MatrixText(UI::MiniMap::ElementToQuad(
            UI::MiniMap::RotatedQuad(screen, kTileHeight / 2.f, y, kTileWidth, kTileHeight, -90.f), kSideElementWidth,
            kSideElementHeight)));
        sideLines.push_back(MatrixText(UI::MiniMap::ElementToQuad(
            UI::MiniMap::RotatedQuad(screen, REFERENCE_WIDTH - kTileHeight / 2.f, y, kTileWidth, kTileHeight, 90.f),
            kSideElementWidth, kSideElementHeight)));
    }
    Sync(m_RmlView.Binder(), &MiniMapRmlModel::sideLines, "side_lines", std::move(sideLines));
}

void mu::ui::window::CMiniMap::SyncClips()
{
    // The original drew the bottom HUD strip over the map, so the map paints only outside it:
    // above the strip, and beside it down to the screen's bottom edge.
    const float width = static_cast<float>(WindowWidth);
    const float height = static_cast<float>(WindowHeight);
    UI::Placement::PlacementParticipant::Box strip;
    const bool hasStrip = UI::Placement::SlotBox("main_hud", strip);
    const float bandTop = hasStrip ? std::clamp(strip.top, 0.f, height) : height;
    const float bandBottom = hasStrip ? std::clamp(strip.top + strip.height, bandTop, height) : height;
    const float bandLeft = std::clamp(strip.left, 0.f, width);
    const float bandRight = std::clamp(strip.left + strip.width, bandLeft, width);

    std::vector<MiniMapClipEntry> clips;
    clips.push_back({0.f, 0.f, width, bandTop, 0.f, 0.f});
    if (bandLeft > 0.f && bandBottom > bandTop)
        clips.push_back({0.f, bandTop, bandLeft, bandBottom - bandTop, 0.f, -bandTop});
    if (bandRight < width && bandBottom > bandTop)
        clips.push_back({bandRight, bandTop, width - bandRight, bandBottom - bandTop, -bandRight, -bandTop});
    if (bandBottom < height)
        clips.push_back({0.f, bandBottom, width, height - bandBottom, 0.f, -bandBottom});

    MiniMapRmlModel& model = m_RmlView.GetModel();
    const bool same =
        model.clips.size() == clips.size() &&
        std::equal(clips.begin(), clips.end(), model.clips.begin(),
                   [](const MiniMapClipEntry& a, const MiniMapClipEntry& b)
                   { return a.left == b.left && a.top == b.top && a.width == b.width && a.height == b.height; });
    if (same)
        return;
    model.clips = std::move(clips);
    m_RmlView.MarkDirty("clips");
}

void mu::ui::window::CMiniMap::SyncMap()
{
    const UI::MiniMap::Screen screen = CurrentScreen();
    const float length = UI::MiniMap::MapLength;
    const float heroY = (static_cast<float>(Hero->PositionX) / 256.f) * length;
    const float heroX = (static_cast<float>(Hero->PositionY) / 256.f) * length;

    // Relative to the document: themes/<theme>/mini_map.rml -> Data/<World>/mini_map.tga.
    Rml::String source;
    if (!m_WorldName.empty())
        source = "../../../../" + StringUtils::WideToNarrow(m_WorldName.c_str()) + "/mini_map.tga";
    Sync(m_RmlView.Binder(), &MiniMapRmlModel::mapSource, "map_source", std::move(source));

    Sync(m_RmlView.Binder(), &MiniMapRmlModel::mapTransform, "map_transform",
         MatrixText(UI::MiniMap::ElementToQuad(
             UI::MiniMap::MapQuad(screen, heroX, heroY, length, UI::MiniMap::MapRotation), length, length)));

    std::vector<MiniMapMarkerEntry> markers;
    for (int i = 0; i < MAX_MINI_MAP_DATA; i++)
    {
        const MINI_MAP& data = m_Mini_Map_Data[i];
        if (data.Kind <= 0)
            break;
        if (data.Kind != 1 && data.Kind != 2)
            continue;

        const bool portal = data.Kind == 2;
        const float size = portal ? kPortalSize : kNpcSize;
        const float pointY = (static_cast<float>(data.Location[0]) / 256.f) * length;
        const float pointX = (static_cast<float>(data.Location[1]) / 256.f) * length;
        const UI::MiniMap::Marker marker =
            UI::MiniMap::MarkerQuad(screen, heroX, heroY, pointX, pointY, size, static_cast<float>(data.Rotation),
                                    length, UI::MiniMap::MapRotation, portal);
        if (!MarkerDrawn(data))
            continue;
        // RenderPointRotate() stored the name hint's box of every marker it drew.
        SetBtnPos(i, marker.hitBox[0], marker.hitBox[1], marker.hitBox[2], marker.hitBox[3]);
        markers.push_back({portal, size, MatrixText(UI::MiniMap::ElementToQuad(marker.quad, size, size))});
    }

    MiniMapRmlModel& model = m_RmlView.GetModel();
    const bool same = model.markers.size() == markers.size() &&
                      std::equal(markers.begin(), markers.end(), model.markers.begin(),
                                 [](const MiniMapMarkerEntry& a, const MiniMapMarkerEntry& b)
                                 { return a.portal == b.portal && a.size == b.size && a.transform == b.transform; });
    if (!same)
    {
        model.markers = std::move(markers);
        m_RmlView.MarkDirty("markers");
    }
}

void mu::ui::window::CMiniMap::SyncHint()
{
    // Check_Btn(): the first marker whose stored box holds the pointer shows its name above that
    // box, white on RGBA(0, 0, 0, 180), in a box 6 units wider than the text, starting where the
    // text would be centred on the box (so the text sits 3 units right of centre).
    bool found = false;
    std::wstring name;
    float left = 0.f, top = 0.f, width = 0.f, height = 0.f;
    const UI::Scaling::Transform transform = OverlayTransform();
    const float pointerX = UI::Scaling::LogicalX(transform, g_fWindowMouseX);
    const float pointerY = UI::Scaling::LogicalY(transform, g_fWindowMouseY);
    for (int i = 0; i < MAX_MINI_MAP_DATA && !found; i++)
    {
        if (m_Mini_Map_Data[i].Kind <= 0)
            break;
        const float* box = m_Btn_Loc[i];
        if (pointerX > box[0] && pointerX < (box[0] + box[2]) && pointerY > box[1] && pointerY < (box[1] + box[3]))
        {
            found = true;
            name = m_Mini_Map_Data[i].Name;
            g_pRenderText->SetFont(g_hFont);
            const SIZE size = g_pRenderText->MeasureText(name.c_str(), static_cast<int>(name.size()));
            // The original's int / float mix: the text half-width in whole units.
            const int x = static_cast<int>(box[0] + ((box[2] / 2) - static_cast<float>(size.cx / 2)));
            const int y = static_cast<int>(box[1] - static_cast<float>(size.cy + 2));
            left = UI::Scaling::PositionX(transform, static_cast<float>(x));
            top = UI::Scaling::PositionY(transform, static_cast<float>(y));
            width = UI::Scaling::SizeX(transform, static_cast<float>(size.cx + 6));
            height = UI::Scaling::SizeY(transform, static_cast<float>(size.cy));
        }
    }

    Sync(m_RmlView.Binder(), &MiniMapRmlModel::hintVisible, "hint_visible", found);
    if (!found)
        return;
    Sync(m_RmlView.Binder(), &MiniMapRmlModel::hintText, "hint_text", StringUtils::WideToNarrow(name.c_str()));
    Sync(m_RmlView.Binder(), &MiniMapRmlModel::hintLeft, "hint_left", left);
    Sync(m_RmlView.Binder(), &MiniMapRmlModel::hintTop, "hint_top", top);
    Sync(m_RmlView.Binder(), &MiniMapRmlModel::hintWidth, "hint_width", width);
    Sync(m_RmlView.Binder(), &MiniMapRmlModel::hintHeight, "hint_height", height);
}
