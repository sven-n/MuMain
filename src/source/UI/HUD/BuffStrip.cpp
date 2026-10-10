
#include "stdafx.h"
#include "UI/HUD/BuffStrip.h"
#include "I18N/All.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInventory.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/Placement/WindowPlacement.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"
#include "UI/Dialogs/ConfirmRequest.h"
#include "Network/Server/WSclient.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>
#include <algorithm>
#include <iterator>

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
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

    // The skill whose buff a right-click offers to cancel; 0 if it can't be cancelled.
    int CancellableSkill(eBuffState buff)
    {
        switch (buff)
        {
        case eBuff_InfinityArrow:
            return AT_SKILL_INFINITY_ARROW;
        case eBuff_SwellOfMagicPower:
            return AT_SKILL_EXPANSION_OF_WIZARDRY;
        default:
            return 0;
        }
    }

    // The original's tooltip rows (RenderBuffTooltip()): the buff's name in bold blue, its
    // description lines, then the remaining duration in purple if it has one.
    std::vector<UI::RmlBridge::Tooltip::Line> BuildTooltip(eBuffState buff)
    {
        using UI::RmlBridge::Tooltip::LineColor;
        std::vector<UI::RmlBridge::Tooltip::Line> lines;
        std::list<std::wstring> tooltipinfo;
        g_BuffToolTipString(tooltipinfo, buff);
        bool first = true;
        for (const std::wstring& text : tooltipinfo)
        {
            UI::RmlBridge::Tooltip::Line line;
            line.text = StringUtils::WideToNarrow(text.c_str());
            if (first)
            {
                line.color = LineColor::Blue;
                line.bold = true;
            }
            first = false;
            lines.push_back(std::move(line));
        }

        std::wstring bufftime;
        g_BuffStringTime(buff, bufftime);
        if (!bufftime.empty())
        {
            wchar_t durLine[128] = {};
            mu_swprintf(durLine, I18N::Game::DurationPeriodS, bufftime.c_str());
            UI::RmlBridge::Tooltip::Line line;
            line.text = StringUtils::WideToNarrow(durLine);
            line.color = LineColor::Purple;
            lines.push_back(std::move(line));
        }
        return lines;
    }
}

CBuffStrip::CBuffStrip()
{
}

CBuffStrip::~CBuffStrip()
{
    Release();
}

bool CBuffStrip::Create(CManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_BUFF_WINDOW, this);

    // Guarded so the doc/model are created once, even though Create() re-runs on resolution change.
    if (!m_RmlView.Document() && RmlUiRuntime::Instance().IsCreated())
    {
        BuildRmlUi();
        
    }
        // Not Show()n here -- Create() runs before SceneFlag reaches MAIN_SCENE; SyncDocVisibility()
        // (called every frame) shows it once the scene gate allows it.

    Show(true);

    return true;
}

void CBuffStrip::BindRmlModel(Rml::DataModelConstructor& c, BuffStripRmlModel& model)
{
    // See CCharMakeWin::BindRmlModel()'s comment on why this must re-run in full every
    // call, including on a theme switch -- no guard here.
    auto buff = c.RegisterStruct<BuffEntry>();
    buff.RegisterMember("decorator", &BuffEntry::decorator);
    c.RegisterArray<std::vector<BuffEntry>>();

    c.BindEventCallback("buff_cancel",
        [this](Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList& args)
        {
            if (event.GetParameter<int>("button", -1) != 1 || args.empty())
                return;
            OnBuffRightClick(args[0].Get<int>());
        });
    c.BindEventCallback("buff_hover",
        [this](Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList& args)
        {
            if (!args.empty())
                m_Tooltip.Enter(event, args[0].Get<int>(-1));
        });
    c.BindEventCallback("buff_leave",
        [this](Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList&) { m_Tooltip.Leave(event); });

    c.Bind("buffs", &model.buffs);
}

void CBuffStrip::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void CBuffStrip::Release()
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    // Hide the doc directly since RmlUi renders last in the frame regardless of scene (see CLoginWin::PreRelease()).
    if (m_RmlView.Document())
        m_RmlView.Document()->Hide();

    m_RmlView.Release();
}

bool CBuffStrip::UpdateMouseEvent()
{
    // RmlUi's own context does hit-testing now; never consumes the legacy mouse event.
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
    if (!m_RmlView.Document()) return;

    std::list<eBuffState> buffstate;
    BuffSort(buffstate);

    // Rebuilt and marked dirty unconditionally every frame -- deliberate, since this verifies
    // RmlModelBinder handling a bound std::vector whose size changes at runtime. Revisit if this
    // proves too expensive in practice.
    auto& model = m_RmlView.GetModel();
    model.buffs.clear();
    model.buffs.reserve(buffstate.size());
    m_ShownBuffs.assign(buffstate.begin(), buffstate.end());

    // Slot positions are the stylesheets' own: #panel wraps its 20x28 slots at 8 per row, which is
    // what native's own counters produced. Order here is the order they appear.
    for (eBuffState buff : buffstate)
    {
        BuffEntry entry;

        entry.decorator = BuildIconDecorator(buff);
        model.buffs.push_back(entry);
    }

    m_RmlView.MarkDirty("buffs");
    SyncTooltip();
}

void CBuffStrip::SyncTooltip()
{
    const int slot = m_Tooltip.Hovered();
    if (slot < 0 || slot >= static_cast<int>(m_ShownBuffs.size()))
    {
        m_Tooltip.Hide();
        return;
    }
    // Native RenderTipTextList() hung the box from the icon's centre, 20 of its 28 units down.
    UI::RmlBridge::ElementTooltip::Placement placement;
    placement.anchorAt = 20.f / 28.f;
    m_Tooltip.Show(BuildTooltip(static_cast<eBuffState>(m_ShownBuffs[slot])), placement);
}

void CBuffStrip::OnBuffRightClick(int slot)
{
    if (slot < 0 || slot >= static_cast<int>(m_ShownBuffs.size()))
        return;
    const int skill = CancellableSkill(static_cast<eBuffState>(m_ShownBuffs[slot]));
    if (skill == 0)
        return;

    wchar_t text[MAX_GLOBAL_TEXT_STRING];
    mu_swprintf(text, I18N::Game::CancelSkillQuestion, SkillAttribute[skill].Name);

    UI::Dialogs::ConfirmRequest request;
    request.cancellable = true;
    request.lines = {{text, true, RGBA(255, 255, 0, 255)}};
    request.onAccept = [skill] { SocketClient->ToGameServer()->SendMagicEffectCancelRequest(skill, HeroKey); };
    UI::Dialogs::ShowConfirm(std::move(request));
}

float CBuffStrip::GetLayerDepth()
{
    return 0.95f;
}

void CBuffStrip::SyncDocVisibility(bool sceneAllowsShow)
{
    UI::RmlBridge::SyncDocumentVisibility(m_RmlView.Document(), IsVisible() && sceneAllowsShow);
}

void CBuffStrip::OpenningProcess()
{
}

void CBuffStrip::ClosingProcess()
{
}
