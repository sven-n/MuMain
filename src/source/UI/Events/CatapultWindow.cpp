
#include "stdafx.h"
#include "UI/Events/CatapultWindow.h"
#include "UI/Core/WindowSystem.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/AI/ZzzAI.h"
#include "Render/Effects/ZzzEffect.h"
#include "Audio/DSPlaySound.h"
#include "I18N/All.h"
#include "UI/Core/WindowGeometry.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/ElementDocument.h>

#include <span>

using namespace SEASON3B;
using namespace mu::ui::window;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace
{
// CCatapultGroupButton::Create(): each side's target areas, in reference px in the window.
struct CatapultArea
{
    const wchar_t* const* label;
    int left, top;
    bool big; // newui_Btn_round 77 x 47, else newui_Btn_gate 46 x 36
};
const CatapultArea kAttackAreas[] = {
    {&I18N::Game::CastleGate1, 22, 135, false},
    {&I18N::Game::CastleGate2, 74, 135, false},
    {&I18N::Game::CastleGate3, 126, 135, false},
    {&I18N::Game::FrontYard, 59, 182, true},
};
const CatapultArea kDefenseAreas[] = {
    {&I18N::Game::FrontYard1, 18, 125, true},
    {&I18N::Game::FrontYard2, 97, 125, true},
    {&I18N::Game::Bridge, 56, 179, true},
};
} // namespace

mu::ui::window::CCatapultWindow::CCatapultWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;

    OpenningProcess();
}

mu::ui::window::CCatapultWindow::~CCatapultWindow()
{
    Release();
}

bool mu::ui::window::CCatapultWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_CATAPULT, this);

    SetPos(x, y);

    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}

void mu::ui::window::CCatapultWindow::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CCatapultWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool mu::ui::window::CCatapultWindow::UpdateMouseEvent()
{
    if (BtnProcess() == true)
    {
        return false;
    }

    return true;
}

bool mu::ui::window::CCatapultWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_CATAPULT) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_CATAPULT);
            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }
    return true;
}

bool mu::ui::window::CCatapultWindow::Update()
{
    // Clicks RmlUi reported, in the original's BtnProcess() order: exit, an area, Shoot.
    const int area = m_PendingArea;
    const bool fire = m_PendingFire;
    const bool exit = m_PendingExit;
    m_PendingArea = -1;
    m_PendingFire = m_PendingExit = false;
    if (IsVisible())
    {
        const int areaCount = m_iType == CATAPULT_ATTACK    ? static_cast<int>(std::size(kAttackAreas))
                              : m_iType == CATAPULT_DEFENSE ? static_cast<int>(std::size(kDefenseAreas))
                                                            : 0;
        if (exit)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_CATAPULT);
        }
        else if (area >= 0 && area < areaCount && area != m_iAreaIndex)
        {
            // CCatapultGroupButton::BtnSelected(): the chosen area locks, Shoot unlocks.
            m_iAreaIndex = area;
            m_bFireLocked = false;
        }
        else if (fire && !m_bFireLocked && m_iAreaIndex > -1)
        {
            SocketClient->ToGameServer()->SendFireCatapultRequest(m_iNpcKey, m_iAreaIndex + 1);
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_CATAPULT);
        }
    }

    SyncRmlModel();
    return true;
}

bool mu::ui::window::CCatapultWindow::Render()
{
    // Nothing native left: the frame, the texts and the buttons are RmlUi. Kept because CObject
    // requires the override.
    return true;
}

float mu::ui::window::CCatapultWindow::GetLayerDepth()
{
    return 5.0f;
}

void mu::ui::window::CCatapultWindow::OpenningProcess()
{
    m_iType = 0;
    m_iNpcKey = 0;
    Vector(0.f, 0.f, 0.f, m_vCameraPos);

    // Shoot locked until an area is chosen.
    m_bFireLocked = true;
}

void mu::ui::window::CCatapultWindow::ClosingProcess()
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();
}

void mu::ui::window::CCatapultWindow::Init(int iKey, int iType)
{
    m_iNpcKey = iKey;
    m_iType = iType;

    // CCatapultGroupButton::Create(): the side's areas, none chosen.
    m_iAreaIndex = -1;
}

void mu::ui::window::CCatapultWindow::DoFire(int iKey, int iResult, int iType, int iPositionX, int iPositionY)
{
    int iIndex = FindCharacterIndex(iKey);
    CHARACTER* c = &CharactersClient[iIndex];
    OBJECT* o = &c->Object;

    SetAction(o, 1);

    BYTE bySubType = 0;
    BYTE byKeyH = 0;
    BYTE byKeyL = 0;
    if (iResult == 1)
    {
        bySubType = 1;
        byKeyH = (BYTE)(iKey & 0xff);
        byKeyL = (BYTE)(iKey >> 8);
    }
    else if (iResult == 2)
    {
        bySubType = 0;
    }

    vec3_t vPos, vTargetPos;
    Vector(o->Position[0], o->Position[1], 500.f, vPos);
    Vector(iPositionX * TERRAIN_SCALE, iPositionY * TERRAIN_SCALE, 100.f, vTargetPos);
    switch (iType)
    {
    case 1:
        CreateEffect(MODEL_FLY_BIG_STONE1, vPos, vTargetPos, o->Light, bySubType, &Hero->Object, 1, byKeyH, byKeyL);
        break;

    case 2:
        CreateEffect(MODEL_FLY_BIG_STONE2, vPos, vTargetPos, o->Light, bySubType, &Hero->Object, 1, byKeyH, byKeyL);
        break;
    }

    PlayBuffer(SOUND_BC_CATAPULT_ATTACK);
}

void mu::ui::window::CCatapultWindow::DoFireFixStartPosition(int iType, int iPositionX, int iPositionY)
{
    vec3_t vPos, vTargetPos;

    Vector(iPositionX * TERRAIN_SCALE, iPositionY * TERRAIN_SCALE, 100.f, vTargetPos);

    switch (iType)
    {
    case 1:
        Vector(9200, 3000, 500.f, vPos);
        CreateEffect(MODEL_FLY_BIG_STONE1, vPos, vTargetPos, Hero->Object.Light, 0, &Hero->Object, 1);
        break;

    case 2:
        Vector(9400, 19000, 500.f, vPos);
        CreateEffect(MODEL_FLY_BIG_STONE2, vPos, vTargetPos, Hero->Object.Light, 0, &Hero->Object, 1);
        break;
    }

    vec3_t vLight = { 1.f, 0.3f, 0.1f };
    CreateEffect(BITMAP_SHOCK_WAVE, vTargetPos, Hero->Object.Angle, vLight, 6);

    PlayBuffer(SOUND_BC_CATAPULT_ATTACK);
}

void mu::ui::window::CCatapultWindow::SetCameraPos(float x, float y, float z)
{
    Vector(x, y, z, m_vCameraPos);
}

void mu::ui::window::CCatapultWindow::GetCameraPos(vec3_t& vPos)
{
    if (m_vCameraPos[0] != 0.f || m_vCameraPos[1] != 0.f || m_vCameraPos[2] != 0.f)
    {
        VectorCopy(m_vCameraPos, vPos);
    }
    else
    {
        VectorCopy(Hero->Object.Position, vPos);
    }
}

bool mu::ui::window::CCatapultWindow::BtnProcess()
{
    // Top-right corner close "X" (shared frame). Hides + swallows the click. The area, Shoot and
    // exit buttons are RmlUi's (see Update()).
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_CATAPULT))
        return true;

    return false;
}

void mu::ui::window::CCatapultWindow::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(
        RmlUiRuntime::Instance().GetContext(), "catapult",
        [this](Rml::DataModelConstructor& c, CatapultRmlModel& model)
        {
            c.Bind("mode", &model.mode);
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);
            c.Bind("root_scale", &model.rootScale);
            c.Bind("text_px", &model.textPx);
            c.Bind("line_height_px", &model.lineHeightPx);
            c.Bind("title", &model.title);
            c.Bind("title_px", &model.titlePx);
            auto line = c.RegisterStruct<CatapultLineEntry>();
            line.RegisterMember("text", &CatapultLineEntry::text);
            line.RegisterMember("text_px", &CatapultLineEntry::textPx);
            c.RegisterArray<std::vector<CatapultLineEntry>>();
            c.Bind("lines", &model.lines);
            auto area = c.RegisterStruct<CatapultAreaEntry>();
            area.RegisterMember("label", &CatapultAreaEntry::label);
            area.RegisterMember("index", &CatapultAreaEntry::index);
            area.RegisterMember("big", &CatapultAreaEntry::big);
            area.RegisterMember("locked", &CatapultAreaEntry::locked);
            area.RegisterMember("label_top", &CatapultAreaEntry::labelTop);
            area.RegisterMember("label_px", &CatapultAreaEntry::labelPx);
            c.RegisterArray<std::vector<CatapultAreaEntry>>();
            c.Bind("areas", &model.areas);
            c.Bind("fire_text", &model.fireText);
            c.Bind("fire_locked", &model.fireLocked);
            c.Bind("fire_label_top", &model.fireLabelTop);
            c.Bind("exit_tooltip", &model.exitTooltip);
            c.BindEventCallback("catapult_area",
                                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                                {
                                    if (arguments.size() == 1)
                                        m_PendingArea = arguments[0].Get<int>(-1);
                                });
            c.BindEventCallback("catapult_fire", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                                { m_PendingFire = true; });
            c.BindEventCallback("catapult_exit", [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                                { m_PendingExit = true; });
        });
    if (!modelCreated)
        return;

    m_pRmlDoc =
        UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/catapult.rml");
}

void mu::ui::window::CCatapultWindow::ReloadRmlTheme()
{
    if (!m_pRmlDoc)
        return;
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
}

void mu::ui::window::CCatapultWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_pRmlDoc)
        return;

    // Layer depth 5: over the HUD like every panel the original opened.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_pRmlDoc, IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncRootTransform(m_RmlBinder, m_Pos);
    UI::RmlBridge::SyncNativeTextSize(m_RmlBinder);

    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    CatapultRmlModel& model = m_RmlBinder.GetModel();
    // RenderText() in a 190-unit box, bold: shrunk to it when wider.
    auto boldPxIn = [&](const wchar_t* text)
    {
        g_pRenderText->SetFont(g_hFontBold);
        const int width = g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text))).cx;
        return UI::Scaling::NativeTextPixelSizeInBox(UI::Scaling::FontRole::Bold, transform, static_cast<float>(width),
                                                     190.f);
    };

    // The original's RenderTexts(): the side's title and the three lines, bold (220, 220, 220).
    const wchar_t* title = m_iType == CATAPULT_ATTACK    ? I18N::Game::WeaponForInvadingTeam
                           : m_iType == CATAPULT_DEFENSE ? I18N::Game::WeaponForDefendingTeam
                                                         : L"";
    SyncField(m_RmlBinder, &CatapultRmlModel::title, "title", StringUtils::WideToNarrow(title));
    SyncField(m_RmlBinder, &CatapultRmlModel::titlePx, "title_px", title[0] != L'\0' ? boldPxIn(title) : 0.f);
    const wchar_t* const texts[] = {I18N::Game::DesiredAttackingLocation, I18N::Game::SelectTheButtonAndPress,
                                    I18N::Game::ToShoot};
    std::vector<CatapultLineEntry> lines;
    for (const wchar_t* text : texts)
        lines.push_back({StringUtils::WideToNarrow(text), boldPxIn(text)});
    const bool sameLines =
        model.lines.size() == lines.size() && std::equal(model.lines.begin(), model.lines.end(), lines.begin(),
                                                         [](const CatapultLineEntry& x, const CatapultLineEntry& y)
                                                         { return x.text == y.text && x.textPx == y.textPx; });
    if (!sameLines)
    {
        model.lines = std::move(lines);
        m_RmlBinder.MarkDirty("lines");
    }

    // The buttons' labels in the normal font at CButton::Render()'s whole-unit centre.
    const int lineHeight = CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal);
    SyncField(m_RmlBinder, &CatapultRmlModel::lineHeightPx, "line_height_px", static_cast<float>(lineHeight) * transform.scaleY);
    std::vector<CatapultAreaEntry> areas;
    const std::span<const CatapultArea> sideAreas =
        m_iType == CATAPULT_ATTACK    ? std::span<const CatapultArea>(kAttackAreas)
        : m_iType == CATAPULT_DEFENSE ? std::span<const CatapultArea>(kDefenseAreas)
                                      : std::span<const CatapultArea>();
    for (std::size_t i = 0; i < sideAreas.size(); ++i)
    {
        const CatapultArea& a = sideAreas[i];
        const int width = a.big ? 77 : 46;
        const int height = a.big ? 47 : 36;
        g_pRenderText->SetFont(g_hFont);
        const int measured = g_pRenderText->MeasureText(*a.label, static_cast<int>(wcslen(*a.label))).cx;
        areas.push_back(
            {StringUtils::WideToNarrow(*a.label), static_cast<int>(i), a.big,
             static_cast<int>(i) == m_iAreaIndex, static_cast<float>(height / 2 - lineHeight / 2),
             UI::Scaling::NativeTextPixelSizeInBox(UI::Scaling::FontRole::Normal, transform,
                                                   static_cast<float>(measured), static_cast<float>(width))});
    }
    SyncField(m_RmlBinder, &CatapultRmlModel::mode, "mode", static_cast<int>(m_iType));
    const bool sameAreas = model.areas.size() == areas.size() &&
                           std::equal(model.areas.begin(), model.areas.end(), areas.begin(),
                                      [](const CatapultAreaEntry& x, const CatapultAreaEntry& y)
                                      {
                                          return x.label == y.label && x.big == y.big && x.locked == y.locked &&
                                                 x.labelTop == y.labelTop && x.labelPx == y.labelPx;
                                      });
    if (!sameAreas)
    {
        model.areas = std::move(areas);
        m_RmlBinder.MarkDirty("areas");
    }
    SyncField(m_RmlBinder, &CatapultRmlModel::fireText, "fire_text", StringUtils::WideToNarrow(I18N::Game::Shoot));
    SyncField(m_RmlBinder, &CatapultRmlModel::fireLocked, "fire_locked", m_bFireLocked);
    SyncField(m_RmlBinder, &CatapultRmlModel::fireLabelTop, "fire_label_top", static_cast<float>(29 / 2 - lineHeight / 2));
    SyncField(m_RmlBinder, &CatapultRmlModel::exitTooltip, "exit_tooltip", StringUtils::WideToNarrow(I18N::Game::Close388));
}
