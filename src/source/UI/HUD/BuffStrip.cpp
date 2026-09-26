
#include "stdafx.h"
#include "UI/HUD/BuffStrip.h"
#include "I18N/All.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInventory.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>
#include <algorithm>
#include <iterator>

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
    const float BUFF_IMG_WIDTH = 20.0f;
    const float BUFF_IMG_HEIGHT = 28.0f;
    const int BUFF_MAX_LINE_COUNT = 8;
    const int BUFF_IMG_SPACE = 5;

    // Picks the highest tier of each buff family; buffs sort to the front, debuffs to the back.
    eBuffState NormalizeBuffState(eBuffState raw)
    {
        switch (raw)
        {
        case EFFECT_GREATER_LIFE_ENHANCED:
        case EFFECT_GREATER_LIFE_MASTERED:
            return eBuff_Life;
        case EFFECT_MAGIC_CIRCLE_IMPROVED:
        case EFFECT_MAGIC_CIRCLE_ENHANCED:
            return eBuff_SwellOfMagicPower;
        case EFFECT_GREATER_CRITICAL_DAMAGE_MASTERED:
        case EFFECT_GREATER_CRITICAL_DAMAGE_EXTENDED:
            return eBuff_AddCriticalDamage;
        case EFFECT_INFINITY_ARROW_IMPROVED:
            return eBuff_InfinityArrow;
        case EFFECT_BLIND_IMPROVED:
            return eDeBuff_Blind;
        case EFFECT_POISON_ARROW_IMPROVED:
            return EFFECT_POISON_ARROW;
        case EFFECT_BLESS_IMPROVED:
            return EFFECT_BLESS;
        case EFFECT_IRON_DEFENSE_IMPROVED:
            return EFFECT_IRON_DEFENSE;
        case EFFECT_BLOOD_HOWLING_IMPROVED:
            return EFFECT_BLOOD_HOWLING;
        default:
            return raw;
        }
    }

    int BuffTier(eBuffState buf)
    {
        switch (buf)
        {
        case EFFECT_GREATER_LIFE_ENHANCED:
        case EFFECT_MAGIC_CIRCLE_IMPROVED:
        case EFFECT_GREATER_CRITICAL_DAMAGE_EXTENDED:
        case EFFECT_INFINITY_ARROW_IMPROVED:
        case EFFECT_BLIND_IMPROVED:
        case EFFECT_POISON_ARROW_IMPROVED:
        case EFFECT_BLESS_IMPROVED:
        case EFFECT_IRON_DEFENSE_IMPROVED:
        case EFFECT_BLOOD_HOWLING_IMPROVED:
            return 1;
        case EFFECT_GREATER_LIFE_MASTERED:
        case EFFECT_MAGIC_CIRCLE_ENHANCED:
        case EFFECT_GREATER_CRITICAL_DAMAGE_MASTERED:
            return 2;
        default:
            return 0;
        }
    }

    bool SetDisableRenderBuff(const eBuffState& _BuffState)
    {
        switch (_BuffState)
        {
#ifdef PBG_ADD_PKSYSTEM_INGAMESHOP
        case eDeBuff_MoveCommandWin:
#endif //PBG_ADD_PKSYSTEM_INGAMESHOP
        case eDeBuff_FlameStrikeDamage:
        case eDeBuff_GiganticStormDamage:
        case eDeBuff_LightningShockDamage:
        case eDeBuff_Discharge_Stamina:
            return true;
        default:
            return false;
        }
        return false;
    }

    void BuffSort(std::list<eBuffState>& buffstate)
    {
        OBJECT* pHeroObject = &Hero->Object;
        int iBuffSize = g_CharacterBuffSize(pHeroObject);

        eBuffState top[eBuff_Count] = {};

        for (int i = 0; i < iBuffSize; ++i)
        {
            eBuffState buf = g_CharacterBuff(pHeroObject, i);
            if (buf == eBuffNone)
                continue;
            if (SetDisableRenderBuff(buf))
                continue;

            eBuffState base = NormalizeBuffState(buf);
            if (top[base] == eBuffNone || BuffTier(buf) > BuffTier(top[base]))
                top[base] = buf;
        }

        for (int i = 0; i < iBuffSize; ++i)
        {
            eBuffState buf = g_CharacterBuff(pHeroObject, i);
            if (buf == eBuffNone)
                continue;
            if (SetDisableRenderBuff(buf))
                continue;

            eBuffState base = NormalizeBuffState(buf);
            if (buf != top[base])
                continue;

            eBuffClass eBuffClassType = g_IsBuffClass(buf);
            if (eBuffClassType == eBuffClass_Buff)
                buffstate.push_front(buf);
            else if (eBuffClassType == eBuffClass_DeBuff)
                buffstate.push_back(buf);
        }
    }

    // One @spritesheet rect per 20x28 tile, 10 columns wide (generated in buff_strip.rcss). The
    // real art is 200x224 (10x8 tiles) padded to a 256x256 texture, so tile indices are clamped to
    // 89 (rows 0-8) -- ids beyond that already go out of UV [0,1] range in the original too.
    const int kMaxTileIndex = 89; // rows 0-8, 10 cols
    Rml::String BuildIconDecorator(eBuffState buff)
    {
        const int buffId = static_cast<int>(buff);
        const bool isAtlas1 = (buffId < 81); // eBuff_Berserker, matches RenderBuffIcon()'s own branch
        const int rawIndex = isAtlas1 ? (buffId - 1) : (buffId - 81);
        const int tileIndex = std::min(std::max(rawIndex, 0), kMaxTileIndex);
        return "image(" + Rml::String(isAtlas1 ? "atlas1-" : "atlas2-") + std::to_string(tileIndex) + ")";
    }

    std::wstring JoinLines(std::list<std::wstring>::const_iterator first, std::list<std::wstring>::const_iterator last)
    {
        std::wstring combined;
        for (auto it = first; it != last; ++it)
        {
            if (!combined.empty())
                combined += L"\n";
            combined += *it;
        }
        return combined;
    }

    // The original's tooltip rows (RenderBuffTooltip()): the first line is the buff's name, then
    // its description lines, then the remaining duration if it has one.
    struct TooltipTexts
    {
        Rml::String title, body, duration, combined;
    };

    TooltipTexts BuildTooltip(eBuffState buff)
    {
        TooltipTexts entry;
        std::list<std::wstring> tooltipinfo;
        g_BuffToolTipString(tooltipinfo, buff);

        std::wstring bufftime;
        g_BuffStringTime(buff, bufftime);
        std::wstring duration;
        if (!bufftime.empty())
        {
            wchar_t durLine[128] = {};
            mu_swprintf(durLine, I18N::Game::DurationPeriodS, bufftime.c_str());
            duration = durLine;
        }

        const auto body = tooltipinfo.empty() ? tooltipinfo.cend() : std::next(tooltipinfo.cbegin());
        entry.title = tooltipinfo.empty() ? Rml::String() : StringUtils::WideToNarrow(tooltipinfo.front().c_str());
        entry.body = StringUtils::WideToNarrow(JoinLines(body, tooltipinfo.cend()).c_str());
        entry.duration = StringUtils::WideToNarrow(duration.c_str());

        // One plain newline-joined block, for a theme that draws it as one.
        std::wstring combined = JoinLines(tooltipinfo.cbegin(), tooltipinfo.cend());
        if (!duration.empty())
            combined += (combined.empty() ? L"" : L"\n") + duration;
        entry.combined = StringUtils::WideToNarrow(combined.c_str());
        return entry;
    }
}

CBuffStrip::CBuffStrip()
{
}

CBuffStrip::~CBuffStrip()
{
    Release();
}

bool CBuffStrip::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_BUFF_WINDOW, this);

    // Guarded so the doc/model are created once, even though Create() re-runs on resolution change.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        BuildRmlUi();
        UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });
    }
        // Not Show()n here -- Create() runs before SceneFlag reaches MAIN_SCENE; SyncDocVisibility()
        // (called every frame) shows it once the scene gate allows it.

    Show(true);

    return true;
}

void CBuffStrip::BuildRmlUi()
{
    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "buff_strip",
        [this](Rml::DataModelConstructor& c, BuffStripRmlModel& model)
        {
            // See CCharMakeWin::BuildRmlUi()'s comment on why this must re-run in full every
            // call, including from ReloadRmlTheme() -- no guard here.
            auto buff = c.RegisterStruct<BuffEntry>();
            buff.RegisterMember("slot_left", &BuffEntry::slotLeft);
            buff.RegisterMember("slot_top", &BuffEntry::slotTop);
            buff.RegisterMember("decorator", &BuffEntry::decorator);
            buff.RegisterMember("tooltip", &BuffEntry::tooltip);
            buff.RegisterMember("tooltip_title", &BuffEntry::tooltipTitle);
            buff.RegisterMember("tooltip_body", &BuffEntry::tooltipBody);
            buff.RegisterMember("tooltip_duration", &BuffEntry::tooltipDuration);
            c.RegisterArray<std::vector<BuffEntry>>();

            c.Bind("buffs", &model.buffs);
            c.Bind("strip_center", &model.stripCenter);
            c.Bind("tooltip_line_px", &model.tooltipLinePx);
        });

    if (modelCreated)
        m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/buff_strip.rml");
}

void CBuffStrip::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return;

    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
    // Next frame's Update()/SyncDocVisibility() self-corrects live state/visibility.
}

void CBuffStrip::Release()
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        UI::RmlBridge::UnregisterForThemeReload(this);
        m_pNewUIMng = NULL;
    }

    // Hide the doc directly since RmlUi renders last in the frame regardless of scene (see CLoginWin::PreRelease()).
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
}

bool CBuffStrip::UpdateMouseEvent()
{
    // RmlUi's own context does hit-testing now; never consumes the legacy mouse event.
    // (Right-click-to-cancel isn't reproduced -- see this class's header comment.)
    return true;
}

bool CBuffStrip::UpdateKeyEvent()
{
    return true;
}

bool CBuffStrip::Update()
{
    SyncRmlModel();
    return true;
}

bool CBuffStrip::Render()
{
    // RmlUi's #panel owns all visuals now; SyncRmlModel() (from Update()) keeps it current.
    return true;
}

void CBuffStrip::SyncRmlModel()
{
    if (!m_pRmlDoc) return;

    std::list<eBuffState> buffstate;
    BuffSort(buffstate);

    // Rebuilt and marked dirty unconditionally every frame -- deliberate, since this verifies
    // RmlModelBinder handling a bound std::vector whose size changes at runtime. Revisit if this
    // proves too expensive in practice.
    auto& model = m_RmlBinder.GetModel();
    model.buffs.clear();
    model.buffs.reserve(buffstate.size());

    int buffwidthcount = 0, buffheightcount = 0;
    for (eBuffState buff : buffstate)
    {
        BuffEntry entry;

        entry.slotLeft = static_cast<float>(buffwidthcount) * (BUFF_IMG_WIDTH + BUFF_IMG_SPACE);
        entry.slotTop = static_cast<float>(buffheightcount) * (BUFF_IMG_HEIGHT + BUFF_IMG_SPACE);

        entry.decorator = BuildIconDecorator(buff);
        TooltipTexts tooltip = BuildTooltip(buff);
        entry.tooltip = std::move(tooltip.combined);
        entry.tooltipTitle = std::move(tooltip.title);
        entry.tooltipBody = std::move(tooltip.body);
        entry.tooltipDuration = std::move(tooltip.duration);

        model.buffs.push_back(entry);

        if (++buffwidthcount >= BUFF_MAX_LINE_COUNT)
        {
            buffwidthcount = 0;
            ++buffheightcount;
        }
    }

    m_RmlBinder.MarkDirty("buffs");
    SyncStripCenter();
    SyncTooltipLineHeight();
}

void CBuffStrip::SyncTooltipLineHeight()
{
    constexpr float kNativeRowAdvance = 1.1f;
    const auto transform = UI::Scaling::TransformForLayout(GetLayoutMode(), WindowWidth, WindowHeight);
    float lineHeight = 0.0f;
    {
        const UI::Scaling::ScopedActiveTransform measureScope(transform);
        lineHeight = static_cast<float>(CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal));
    }
    const float advance = lineHeight * transform.scaleY * kNativeRowAdvance;

    auto& model = m_RmlBinder.GetModel();
    if (model.tooltipLinePx == advance)
        return;
    model.tooltipLinePx = advance;
    m_RmlBinder.MarkDirty("tooltip_line_px");
}

void CBuffStrip::SyncStripCenter()
{
    // Docked panels sit at the right edge through the dock transform, so the free area ends where
    // the leftmost of them begins.
    const auto dock = UI::Scaling::TransformForLayout(UI::Scaling::LayoutMode::DockRight, WindowWidth, WindowHeight);
    const float freeWidth = UI::Scaling::PositionX(dock, static_cast<float>(m_iFreeScreenWidth));
    const float center = freeWidth * 0.5f;

    auto& model = m_RmlBinder.GetModel();
    if (model.stripCenter == center)
        return;
    model.stripCenter = center;
    m_RmlBinder.MarkDirty("strip_center");
}

float CBuffStrip::GetLayerDepth()
{
    return 0.95f;
}

void CBuffStrip::SyncDocVisibility(bool sceneAllowsShow)
{
    UI::RmlBridge::SyncDocumentVisibility(m_pRmlDoc, IsVisible() && sceneAllowsShow);
}

void CBuffStrip::OpenningProcess()
{
}

void CBuffStrip::ClosingProcess()
{
}
