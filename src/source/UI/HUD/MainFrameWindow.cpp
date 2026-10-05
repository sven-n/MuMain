
#include "stdafx.h"
#include <algorithm>
#include "I18N/All.h"

#include "UI/HUD/MainFrameWindow.h"	// self
#include "UI/Options/OptionWindow.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Widgets/UIBaseDef.h"
#include "Audio/DSPlaySound.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"

#include "GameLogic/Items/CSItemOption.h"
#include "GameLogic/Events/CSChaosCastle.h"
#include "World/MapInfra/MapManager.h"
#include "Character/CharacterManager.h"
#include "GameLogic/Skills/SkillManager.h"
#include "UI/HUD/Skills/SkillIconAtlas.h"
#include "UI/HUD/Skills/SkillTooltip.h"
#include "UI/Scaling/UITransform.h"
#include "Core/Time/CTimCheck.h"
#include "GameLogic/Social/MonkSystem.h"

#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
#include "GameShop/InGameShopSystem.h"
#endif //PBG_ADD_INGAMESHOP_UI_MAINFRAME

// RmlUi migration -- see this class's header comment.
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Renderer/MuRenderer.h"
#include "Camera/CameraProjection.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlWorkspaceParticipant.h"
#include "UI/Placement/WindowPlacement.h"
#include "UI/RmlBridge/RmlTooltip.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>
#include "GameLogic/Quests/QuestMng.h"
#include "UI/Social/FriendWindow.h"

mu::ui::window::CMainFrameWindow::CMainFrameWindow()
{
    m_bExpEffect = false;
    m_dwExpEffectTime = 0;
    m_dwPreExp = 0;
    m_dwGetExp = 0;
    m_bButtonBlink = false;
}

mu::ui::window::CMainFrameWindow::~CMainFrameWindow()
{
    Release();
}

bool mu::ui::window::CMainFrameWindow::Create(CManager* pNewUIMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_MAINFRAME, this);

    UI::RmlBridge::RegisterWorkspaceDocument("main_hud", [this] { return m_pRmlDoc; }, "hud_layout");
    UI::RmlBridge::RegisterWorkspaceDocument("top_bar", [this] { return m_pRmlTopDoc; }, "buttons_top");

    // Guarded so the doc/model are created once, even though Create() re-runs on resolution change.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        BuildRmlUi();
        UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });
    }

    Show(true);

    return true;
}

namespace
{
constexpr float kSkillIconWidth = 20.f;
constexpr float kSkillIconHeight = 28.f;

struct SlotBox
{
    float left = 0.f, top = 0.f, width = 0.f, height = 0.f;
};

// The skill tooltips' frame: the screen in the original's reference units, i.e. scaled by the dp
// ratio main_frame.rcss is sized in, so the original's offsets (10 above a box, ...) keep their size.
UI::Scaling::Transform HudReferenceTransform(Rml::ElementDocument* document)
{
    const Rml::Context* context = document ? document->GetContext() : nullptr;
    const float ratio = context ? context->GetDensityIndependentPixelRatio() : 1.f;
    return {ratio, ratio, 0.f, 0.f, ratio};
}

// A hovered slot or cell's box in HudReferenceTransform()'s units, wherever the theme put it.
SlotBox SlotBoxInReference(Rml::Element* element)
{
    SlotBox box;
    if (element == nullptr)
        return box;
    const UI::Scaling::Transform bars = HudReferenceTransform(element->GetOwnerDocument());
    const Rml::Vector2f offset = element->GetAbsoluteOffset(Rml::BoxArea::Border);
    const Rml::Vector2f size = element->GetBox().GetSize(Rml::BoxArea::Border);
    box.left = UI::Scaling::LogicalX(bars, offset.x);
    box.top = UI::Scaling::LogicalY(bars, offset.y);
    box.width = size.x / bars.scaleX;
    box.height = size.y / bars.scaleY;
    return box;
}
} // namespace

void mu::ui::window::CMainFrameWindow::BuildRmlUi()
{
    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "main_frame",
            [this](Rml::DataModelConstructor& c, MainFrameRmlModel& model)
            {
                c.Bind("hint_px", &model.hintPx);

                c.Bind("hp_fraction", &model.hpFraction);
                c.Bind("mp_fraction", &model.mpFraction);
                c.Bind("ag_fraction", &model.agFraction);
                c.Bind("sd_fraction", &model.sdFraction);
                c.Bind("hp_text", &model.hpText);
                c.Bind("mp_text", &model.mpText);
                c.Bind("ag_text", &model.agText);
                c.Bind("sd_text", &model.sdText);
                c.Bind("hp_current_text", &model.hpCurrentText);
                c.Bind("mp_current_text", &model.mpCurrentText);
                c.Bind("ag_current_text", &model.agCurrentText);
                c.Bind("sd_current_text", &model.sdCurrentText);
                c.Bind("hp_tooltip", &model.hpTooltip);
                c.Bind("mp_tooltip", &model.mpTooltip);
                c.Bind("ag_tooltip", &model.agTooltip);
                c.Bind("sd_tooltip", &model.sdTooltip);
                c.Bind("poisoned", &model.poisoned);

                c.Bind("exp_fraction", &model.expFraction);
                c.Bind("exp_digit", &model.expDigit);
                c.Bind("exp_tooltip", &model.expTooltip);

                c.Bind("cshop_open", &model.cShopOpen);
                c.Bind("chainfo_open", &model.chaInfoOpen);
                c.Bind("myinven_open", &model.myInvenOpen);
                c.Bind("friend_open", &model.friendOpen);
                c.Bind("window_open", &model.windowOpen);
                c.Bind("cshop_tooltip", &model.cShopTooltip);
                c.Bind("chainfo_tooltip", &model.chaInfoTooltip);
                c.Bind("myinven_tooltip", &model.myInvenTooltip);
                c.Bind("friend_tooltip", &model.friendTooltip);
                c.Bind("window_tooltip", &model.windowTooltip);

                c.Bind("chainfo_alert", &model.chaInfoAlert);
                c.Bind("friend_alert", &model.friendAlert);

                c.Bind("skill_slot_0_selected", &model.skillSlot0Selected);
                c.Bind("skill_slot_1_selected", &model.skillSlot1Selected);
                c.Bind("skill_slot_2_selected", &model.skillSlot2Selected);
                c.Bind("skill_slot_3_selected", &model.skillSlot3Selected);
                c.Bind("skill_slot_4_selected", &model.skillSlot4Selected);

                c.Bind("skill_slot_0_hotkey", &model.skillSlot0Hotkey);
                c.Bind("skill_slot_1_hotkey", &model.skillSlot1Hotkey);
                c.Bind("skill_slot_2_hotkey", &model.skillSlot2Hotkey);
                c.Bind("skill_slot_3_hotkey", &model.skillSlot3Hotkey);
                c.Bind("skill_slot_4_hotkey", &model.skillSlot4Hotkey);

                c.Bind("skill_slot_0_icon", &model.skillSlot0Icon);
                c.Bind("skill_slot_1_icon", &model.skillSlot1Icon);
                c.Bind("skill_slot_2_icon", &model.skillSlot2Icon);
                c.Bind("skill_slot_3_icon", &model.skillSlot3Icon);
                c.Bind("skill_slot_4_icon", &model.skillSlot4Icon);
                c.Bind("current_skill_icon", &model.currentSkillIcon);
                c.Bind("current_skill_hotkey", &model.currentSkillHotkey);

                // Skill list cooldown bindings -- see MainFrameRmlModel::skillGridOpen's own
                // header comment.
                c.Bind("skill_slot_0_cooldown", &model.skillSlot0Cooldown);
                c.Bind("skill_slot_1_cooldown", &model.skillSlot1Cooldown);
                c.Bind("skill_slot_2_cooldown", &model.skillSlot2Cooldown);
                c.Bind("skill_slot_3_cooldown", &model.skillSlot3Cooldown);
                c.Bind("skill_slot_4_cooldown", &model.skillSlot4Cooldown);
                c.Bind("current_skill_cooldown", &model.currentSkillCooldown);

                // See CCharMakeWin::BuildRmlUi()'s comment on why this must re-run in full every
                // call, including from ReloadRmlTheme() -- no guard here.
                auto skillCell = c.RegisterStruct<SkillCellEntry>();
                skillCell.RegisterMember("left", &SkillCellEntry::left);
                skillCell.RegisterMember("top", &SkillCellEntry::top);
                skillCell.RegisterMember("skill_index", &SkillCellEntry::skillIndex);
                skillCell.RegisterMember("is_pet", &SkillCellEntry::isPet);
                skillCell.RegisterMember("is_current", &SkillCellEntry::isCurrent);
                skillCell.RegisterMember("cooldown_fraction", &SkillCellEntry::cooldownFraction);
                skillCell.RegisterMember("icon", &SkillCellEntry::icon);
                skillCell.RegisterMember("hotkey", &SkillCellEntry::hotkey);
                c.RegisterArray<std::vector<SkillCellEntry>>();

                c.Bind("skill_grid_open", &model.skillGridOpen);
                c.Bind("skill_list_up", &model.skillListUp);
                c.Bind("skill_grid_cells", &model.skillGridCells);
                c.Bind("pet_skill_cells", &model.petSkillCells);

                // Skill list click/hover bindings route into CSkillList (g_pSkillList has no RmlUi
                // doc of its own). Args are literal ints in RML (e.g. skill_hotkey_click(0)) or the
                // cell's skill_index for data-for'd lists; Variant::Get<int>() resolves either.
                c.BindEventCallback("skill_hotkey_click",
                    [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args) { g_pSkillList->OnHotkeySlotClick(args.empty() ? 0 : args[0].Get<int>()); });
                c.BindEventCallback("skill_hotkey_hover",
                                    [](Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList& args)
                                    {
                                        const SlotBox slot = SlotBoxInReference(event.GetCurrentElement());
                                        g_pSkillList->OnHotkeySlotHover(args.empty() ? 0 : args[0].Get<int>(),
                                                                        slot.left, slot.top);
                                    });
                c.BindEventCallback("skill_current_click",
                    [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { g_pSkillList->OnCurrentSkillClick(); });
                c.BindEventCallback("skill_current_hover",
                                    [](Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList&)
                                    {
                                        // The slot box is the icon plus the theme's even inset.
                                        const SlotBox slot = SlotBoxInReference(event.GetCurrentElement());
                                        g_pSkillList->OnCurrentSkillHover(
                                            slot.left + (slot.width - kSkillIconWidth) / 2.f,
                                            slot.top + (slot.height - kSkillIconHeight) / 2.f);
                                    });
                c.BindEventCallback("skill_grid_click",
                    [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args) { g_pSkillList->OnGridCellClick(args.empty() ? -1 : args[0].Get<int>()); });
                c.BindEventCallback("skill_grid_hover",
                                    [](Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList& args)
                                    {
                                        const SlotBox cell = SlotBoxInReference(event.GetCurrentElement());
                                        g_pSkillList->OnGridCellHover(args.empty() ? -1 : args[0].Get<int>(),
                                                                      cell.left, cell.top);
                                    });
                c.BindEventCallback("skill_pet_click",
                    [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args) { g_pSkillList->OnPetCellClick(args.empty() ? -1 : args[0].Get<int>()); });
                c.BindEventCallback("skill_pet_hover",
                                    [](Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList& args)
                                    {
                                        const SlotBox cell = SlotBoxInReference(event.GetCurrentElement());
                                        g_pSkillList->OnPetCellHover(args.empty() ? -1 : args[0].Get<int>(),
                                                                     cell.left, cell.top);
                                    });
                c.BindEventCallback("skill_unhover",
                    [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { g_pSkillList->OnUnhover(); });

                // Item hotkey chrome (#item_slots hover/stack-count/right-click). m_ItemHotKey is
                // a member, so these lambdas capture [this] directly.
                c.Bind("item_slot_0_hovered", &model.itemSlot0Hovered);
                c.Bind("item_slot_1_hovered", &model.itemSlot1Hovered);
                c.Bind("item_slot_2_hovered", &model.itemSlot2Hovered);
                c.Bind("item_slot_3_hovered", &model.itemSlot3Hovered);
                c.Bind("item_slot_0_count", &model.itemSlot0Count);
                c.Bind("item_slot_1_count", &model.itemSlot1Count);
                c.Bind("item_slot_2_count", &model.itemSlot2Count);
                c.Bind("item_slot_3_count", &model.itemSlot3Count);

                c.BindEventCallback("item_hotkey_hover",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args) { m_ItemHotKey.OnHotkeySlotHover(args.empty() ? 0 : args[0].Get<int>()); });
                c.BindEventCallback("item_hotkey_unhover",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { m_ItemHotKey.OnUnhover(); });
                // Uses data-event-mouseup, not data-event-click -- RmlUi's Context only dispatches
                // Click for the left button. Mouseup fires for any button; "button" param == 1 means right-click.
                c.BindEventCallback("item_hotkey_rightclick",
                    [this](Rml::DataModelHandle, Rml::Event& event, const Rml::VariantList& args)
                    {
                        if (event.GetParameter<int>("button", -1) != 1) return;
                        m_ItemHotKey.OnHotkeySlotRightClick(args.empty() ? 0 : args[0].Get<int>());
                    });

                c.BindEventCallback("mainframe_cshop_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickCShop(); });
                c.BindEventCallback("mainframe_chainfo_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickChaInfo(); });
                c.BindEventCallback("mainframe_myinven_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickMyInven(); });
                c.BindEventCallback("mainframe_friend_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickFriend(); });
                c.BindEventCallback("mainframe_window_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickWindow(); });
            });

        if (modelCreated)
        {
            m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/main_frame.rml");
            if (m_pRmlDoc)
                m_pRmlTopDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(),
                                                                 "Data/Interface/RmlUi/main_frame_top.rml");
        }

        // Not Show()n here -- Create() runs before SceneFlag reaches MAIN_SCENE; an eager Show()
        // here let the HUD flash once before the first scene gate check. Left hidden;
        // SyncDocVisibility() shows it once the gate allows it.
}

void mu::ui::window::CMainFrameWindow::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return; // never opened -- BuildRmlUi() will simply pick up the new theme whenever it first is

    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;
    if (m_pRmlTopDoc)
    {
        context->UnloadDocument(m_pRmlTopDoc);
        m_pRmlTopDoc = nullptr;
    }

    BuildRmlUi();
    UI::Placement::Invalidate();
    // Next frame's Update()/SyncDocVisibility() self-corrects live state/visibility for both docs.
}

void mu::ui::window::CMainFrameWindow::Release()
{
    UI::Placement::UnregisterParticipant("main_hud");
    UI::Placement::UnregisterParticipant("top_bar");
    m_ItemHotKey.SetSlotIconsShown(false);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        UI::RmlBridge::UnregisterForThemeReload(this);
        m_pNewUIMng = NULL;
    }

    // Hide directly since RmlUi renders last in the frame regardless of scene (see CMuHelperBar::Release()).
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
    if (m_pRmlTopDoc)
        m_pRmlTopDoc->Hide();
}

// Everything this window shows is in main_frame.rml; SyncRmlModel() feeds it from Update().
bool mu::ui::window::CMainFrameWindow::Render()
{
    return true;
}

bool mu::ui::window::CMainFrameWindow::IsVisible() const
{
    return CObject::IsVisible();
}

bool mu::ui::window::CMainFrameWindow::IsMouseOverHud() const
{
    if (m_pRmlDoc == nullptr || !m_pRmlDoc->IsVisible())
        return false;
    Rml::Context* context = m_pRmlDoc->GetContext();
    Rml::Element* hover = context ? context->GetHoverElement() : nullptr;
    return hover != nullptr && hover != m_pRmlDoc && hover->GetOwnerDocument() == m_pRmlDoc;
}

// RenderRightFrame()/RenderExperienceBackground()/RenderLifeMana()/RenderGuageAG()/RenderGuageSD()/
// RenderExperience() removed -- moved to RmlUi (see SyncRmlModel()).

// RenderButtons()/RenderCharInfoButton()/RenderFriendButton()/RenderFriendButtonState()/BtnProcess()
// removed -- the 5 corner buttons moved to RmlUi (see Create()).
bool mu::ui::window::CMainFrameWindow::UpdateMouseEvent()
{
    // RmlUi's own context does hit-testing now; never consumes the legacy mouse event.
    return true;
}

bool mu::ui::window::CMainFrameWindow::UpdateKeyEvent()
{
    if (m_ItemHotKey.UpdateKeyEvent() == false)
    {
        return false;
    }
    return true;
}

bool mu::ui::window::CMainFrameWindow::Update()
{
    if (m_bExpEffect == true)
    {
        if (timeGetTime() - m_dwExpEffectTime > 2000)
        {
            m_bExpEffect = false;
            m_dwExpEffectTime = 0;
            m_dwGetExp = 0;
        }
    }

    // Button clicks, polled-and-cleared like other windows' RmlClickX() pattern. Logic ported
    // verbatim from the legacy BtnProcess(), minus the CButton hit-test wrapper.
    if (m_bRmlMyInvenClicked)
    {
        m_bRmlMyInvenClicked = false;
        g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_INVENTORY);
        PlayBuffer(SOUND_CLICK01);
    }
    if (m_bRmlChaInfoClicked)
    {
        m_bRmlChaInfoClicked = false;
        g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_CHARACTER);
        PlayBuffer(SOUND_CLICK01);
        if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_CHARACTER))
            g_QuestMng.SendQuestIndexByEtcSelection();
    }
    if (m_bRmlFriendClicked)
    {
        m_bRmlFriendClicked = false;
        if (gMapManager.InChaosCastle() == true)
        {
            PlayBuffer(SOUND_CLICK01);
        }
        else
        {
            int iLevel = CharacterAttribute->Level;
            if (iLevel < 6)
            {
                if (g_pSystemLogBox->CheckChatRedundancy(I18N::Game::YouMustBeAtLeastLevel6ToUseTheMyFriendFunction) == FALSE)
                {
                    g_pSystemLogBox->AddText(I18N::Game::YouMustBeAtLeastLevel6ToUseTheMyFriendFunction, mu::ui::window::TYPE_SYSTEM_MESSAGE);
                }
            }
            else
            {
                g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_FRIEND);
            }
            PlayBuffer(SOUND_CLICK01);
        }
    }
    if (m_bRmlWindowClicked)
    {
        m_bRmlWindowClicked = false;
        g_pNewUISystem->Toggle(mu::ui::window::INTERFACE_WINDOW_MENU);
        PlayBuffer(SOUND_CLICK01);
    }
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
    if (m_bRmlCShopClicked)
    {
        m_bRmlCShopClicked = false;
        if (g_pInGameShop->IsInGameShopOpen() == false)
        {
            // Nothing to do until the server confirms the shop is open (matches legacy BtnProcess()).
        }
        else
        {
#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
            if (g_InGameShopSystem->IsScriptDownload() == true)
            {
                g_InGameShopSystem->ScriptDownload();
            }
            if (g_InGameShopSystem->IsBannerDownload() == true)
            {
                g_InGameShopSystem->BannerDownload();
            }
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
            if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_INGAMESHOP) == false)
            {
                if (g_InGameShopSystem->GetIsRequestShopOpenning() == false)
                {
                    SocketClient->ToGameServer()->SendCashShopOpenState(0);
                    g_InGameShopSystem->SetIsRequestShopOpenning(true);
#ifdef KJH_MOD_SHOP_SCRIPT_DOWNLOAD
                    SetBtnState(MAINFRAME_BTN_PARTCHARGE, true);
#endif // KJH_MOD_SHOP_SCRIPT_DOWNLOAD
                }
            }
            else
            {
                SocketClient->ToGameServer()->SendCashShopOpenState(1);
                g_pNewUISystem->Hide(mu::ui::window::INTERFACE_INGAMESHOP);
            }
        }
    }
#endif //defined PBG_ADD_INGAMESHOP_UI_MAINFRAME

    SyncRmlModel();

    return true;
}

void mu::ui::window::CMainFrameWindow::SyncRmlModel()
{
    if (!m_pRmlDoc) return;

    auto& model = m_RmlBinder.GetModel();

    // --- HP/MP/AG/SD/EXP: fraction/text/tooltip helper, mirrors CMuHelperBar's syncLabel() ---
    auto syncFloat = [this](float MainFrameRmlModel::* field, const char* boundName, float value)
    {
        if (m_RmlBinder.GetModel().*field != value)
        {
            m_RmlBinder.GetModel().*field = value;
            m_RmlBinder.MarkDirty(boundName);
        }
    };
    auto syncBool = [this](bool MainFrameRmlModel::* field, const char* boundName, bool value)
    {
        if (m_RmlBinder.GetModel().*field != value)
        {
            m_RmlBinder.GetModel().*field = value;
            m_RmlBinder.MarkDirty(boundName);
        }
    };
    auto syncText = [this](Rml::String MainFrameRmlModel::* field, const char* boundName, const Rml::String& value)
    {
        if (m_RmlBinder.GetModel().*field != value)
        {
            m_RmlBinder.GetModel().*field = value;
            m_RmlBinder.MarkDirty(boundName);
        }
    };
    auto syncWide = [&](Rml::String MainFrameRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        syncText(field, boundName, StringUtils::WideToNarrow(text));
    };

    {
        syncFloat(&MainFrameRmlModel::hintPx, "hint_px",
                  UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Normal, HudReferenceTransform(m_pRmlDoc)));

        m_ItemHotKey.SyncSlotIcons(m_pRmlDoc);
    }

    // HP/MP -- legacy RenderLifeMana(). fLife/fMana there are the EMPTY fraction; store filled.
    DWORD wLifeMax, wLife, wManaMax, wMana;
    if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
    {
        wLifeMax = Master_Level_Data.wMaxLife;
        wLife = std::min<int>(std::max<int>(0, CharacterAttribute->Life), wLifeMax);
        wManaMax = Master_Level_Data.wMaxMana;
        wMana = std::min<int>(std::max<int>(0, CharacterAttribute->Mana), wManaMax);
    }
    else
    {
        wLifeMax = CharacterAttribute->LifeMax;
        wLife = std::min<int>(std::max<int>(0, CharacterAttribute->Life), wLifeMax);
        wManaMax = CharacterAttribute->ManaMax;
        wMana = std::min<int>(std::max<int>(0, CharacterAttribute->Mana), wManaMax);
    }
    if (wLifeMax > 0 && wLife > 0 && (wLife / (float)wLifeMax) < 0.2f)
        PlayBuffer(SOUND_HEART);

    syncFloat(&MainFrameRmlModel::hpFraction, "hp_fraction", wLifeMax > 0 ? wLife / (float)wLifeMax : 0.f);
    syncFloat(&MainFrameRmlModel::mpFraction, "mp_fraction", wManaMax > 0 ? wMana / (float)wManaMax : 0.f);
    syncBool(&MainFrameRmlModel::poisoned, "poisoned", g_isCharacterBuff((&Hero->Object), eDeBuff_Poison));

    // "X / Y" (current/max), not just "X" -- shared model field, both themes.
    wchar_t szNum[32] = {};
    mu_swprintf(szNum, L"%d / %d", wLife, wLifeMax);
    syncWide(&MainFrameRmlModel::hpText, "hp_text", szNum);
    mu_swprintf(szNum, L"%d / %d", wMana, wManaMax);
    syncWide(&MainFrameRmlModel::mpText, "mp_text", szNum);
    mu_swprintf(szNum, L"%d", wLife);
    syncWide(&MainFrameRmlModel::hpCurrentText, "hp_current_text", szNum);
    mu_swprintf(szNum, L"%d", wMana);
    syncWide(&MainFrameRmlModel::mpCurrentText, "mp_current_text", szNum);

    wchar_t szTip[256] = {};
    mu_swprintf(szTip, I18N::Game::LifeDD, wLife, wLifeMax);
    syncWide(&MainFrameRmlModel::hpTooltip, "hp_tooltip", szTip);
    mu_swprintf(szTip, I18N::Game::ManaDD359, wMana, wManaMax);
    syncWide(&MainFrameRmlModel::mpTooltip, "mp_tooltip", szTip);

    // AG (stamina/skill-mana) -- legacy RenderGuageAG().
    DWORD dwMaxSkillMana, dwSkillMana;
    if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
    {
        dwMaxSkillMana = std::max<int>(1, Master_Level_Data.wMaxBP);
        dwSkillMana = std::min<int>(dwMaxSkillMana, CharacterAttribute->SkillMana);
    }
    else
    {
        dwMaxSkillMana = std::max<int>(1, CharacterAttribute->SkillManaMax);
        dwSkillMana = std::min<int>(dwMaxSkillMana, CharacterAttribute->SkillMana);
    }
    syncFloat(&MainFrameRmlModel::agFraction, "ag_fraction", dwSkillMana / (float)dwMaxSkillMana);
    mu_swprintf(szNum, L"%d / %d", dwSkillMana, dwMaxSkillMana);
    syncWide(&MainFrameRmlModel::agText, "ag_text", szNum);
    mu_swprintf(szNum, L"%d", dwSkillMana);
    syncWide(&MainFrameRmlModel::agCurrentText, "ag_current_text", szNum);
    mu_swprintf(szTip, I18N::Game::AGDD, dwSkillMana, dwMaxSkillMana);
    syncWide(&MainFrameRmlModel::agTooltip, "ag_tooltip", szTip);

    // SD (shield) -- legacy RenderGuageSD().
    DWORD wMaxShield, wShield;
    if (gCharacterManager.IsMasterLevel(Hero->Class) == true)
    {
        wMaxShield = std::max<int>(1, Master_Level_Data.wMaxShield);
        wShield = std::min<int>(wMaxShield, CharacterAttribute->Shield);
    }
    else
    {
        wMaxShield = std::max<int>(1, CharacterAttribute->ShieldMax);
        wShield = std::min<int>(wMaxShield, CharacterAttribute->Shield);
    }
    syncFloat(&MainFrameRmlModel::sdFraction, "sd_fraction", wShield / (float)wMaxShield);
    mu_swprintf(szNum, L"%d / %d", wShield, wMaxShield);
    syncWide(&MainFrameRmlModel::sdText, "sd_text", szNum);
    mu_swprintf(szNum, L"%d", wShield);
    syncWide(&MainFrameRmlModel::sdCurrentText, "sd_current_text", szNum);
    mu_swprintf(szTip, I18N::Game::SDDD, wShield, wMaxShield);
    syncWide(&MainFrameRmlModel::sdTooltip, "sd_tooltip", szTip);

    // EXP -- legacy RenderExperience(), flash-highlight overlay intentionally not reproduced (see
    // this class's header comment). expFraction is progress *within* the current decile, matching
    // buildExpSegment()'s original digit/fraction split.
    {
        __int64 wLevel, dwNexExperience, dwExperience;
        const bool masterActive = gCharacterManager.IsMasterExperienceActive(CharacterAttribute->Class, CharacterAttribute->Level);
        if (masterActive)
        {
            wLevel = (__int64)Master_Level_Data.nMLevel;
            dwNexExperience = (__int64)Master_Level_Data.lNext_MasterLevel_Experince;
            dwExperience = (__int64)Master_Level_Data.lMasterLevel_Experince;
        }
        else
        {
            wLevel = CharacterAttribute->Level;
            dwNexExperience = CharacterAttribute->NextExperience;
            dwExperience = CharacterAttribute->Experience;
        }

        __int64 lowerBound;
        if (masterActive)
        {
            const __int64 iTotalLevel = wLevel + 400;
            const __int64 iTOverLevel = iTotalLevel - 255;
            const __int64 iData_Master =
                (((__int64)9 + iTotalLevel) * iTotalLevel * iTotalLevel * (__int64)10)
                + (((__int64)9 + iTOverLevel) * iTOverLevel * iTOverLevel * (__int64)1000);
            lowerBound = (iData_Master - (__int64)3892250000) / (__int64)2;
        }
        else
        {
            const __int64 iPriorLevel = wLevel - 1;
            __int64 iPriorExperience = 0;
            if (iPriorLevel > 0)
            {
                iPriorExperience = (9 + iPriorLevel) * iPriorLevel * iPriorLevel * 10;
                if (iPriorLevel > 255)
                {
                    const __int64 iLevelOverN = iPriorLevel - 255;
                    iPriorExperience += (9 + iLevelOverN) * iLevelOverN * iLevelOverN * 1000;
                }
            }
            lowerBound = iPriorExperience;
        }

        __int64 upperBound = dwNexExperience;
        if (upperBound < lowerBound)
            upperBound = lowerBound;

        const double fNeedExp = static_cast<double>(upperBound - lowerBound);
        const double fClampedExp = std::clamp(static_cast<double>(dwExperience), static_cast<double>(lowerBound), static_cast<double>(upperBound));
        const double fRatio = (fNeedExp > 0.0) ? std::clamp((fClampedExp - static_cast<double>(lowerBound)) / fNeedExp, 0.0, 1.0) : 0.0;

        const double scaled = std::clamp(fRatio, 0.0, 1.0) * 10.0;
        int iExp = std::clamp(static_cast<int>(scaled), 0, 9);
        double fProgress = std::clamp(scaled - static_cast<double>(static_cast<long long>(scaled)), 0.0, 1.0);
        if (fRatio >= 1.0) { iExp = 9; fProgress = 1.0; }

        syncFloat(&MainFrameRmlModel::expFraction, "exp_fraction", static_cast<float>(fProgress));
        wchar_t szExp[8] = {};
        mu_swprintf(szExp, L"%d", iExp);
        syncWide(&MainFrameRmlModel::expDigit, "exp_digit", szExp);
        mu_swprintf(szTip, I18N::Game::EXPI64dI64d, static_cast<unsigned long long>(dwExperience),
                    static_cast<unsigned long long>(dwNexExperience));
        syncWide(&MainFrameRmlModel::expTooltip, "exp_tooltip", szTip);
    }

    // Button tooltips: static strings, re-checked every frame for consistency but effectively set-once.
    syncWide(&MainFrameRmlModel::chaInfoTooltip, "chainfo_tooltip", I18N::Game::CharacterC);
    syncWide(&MainFrameRmlModel::myInvenTooltip, "myinven_tooltip", I18N::Game::InventoryIV);
    syncWide(&MainFrameRmlModel::friendTooltip, "friend_tooltip", I18N::Game::FriendF);
    syncWide(&MainFrameRmlModel::windowTooltip, "window_tooltip", I18N::Game::MenuU);
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
    syncWide(&MainFrameRmlModel::cShopTooltip, "cshop_tooltip", I18N::Game::MUItemShopX);
#endif //defined PBG_ADD_INGAMESHOP_UI_MAINFRAME

    // Char-info quest-available blink -- legacy RenderCharInfoButton().
    bool chaInfoAlert = false;
    if (!g_QuestMng.IsQuestIndexByEtcListEmpty())
    {
        if (g_Time.GetTimeCheck(5, 500))
            m_bButtonBlink = !m_bButtonBlink;
        chaInfoAlert = m_bButtonBlink
            && !(g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_QUEST_PROGRESS_ETC)
                || g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_CHARACTER));
    }
    syncBool(&MainFrameRmlModel::chaInfoAlert, "chainfo_alert", chaInfoAlert);

    // Friend mail/chat blink -- legacy RenderFriendButton()/RenderFriendButtonState().
    const int iBlinkTemp = g_pFriendMenu->GetBlinkTemp();
    const bool bIsAlertTime = (iBlinkTemp % 24 < 12);
    bool friendAlert = false;
    if (g_pFriendMenu->IsNewChatAlert() && bIsAlertTime)
    {
        friendAlert = true;
    }
    if (g_pFriendMenu->IsNewMailAlert())
    {
        if (bIsAlertTime)
        {
            friendAlert = true;
            if (iBlinkTemp % 24 == 11)
                g_pFriendMenu->IncreaseLetterBlink();
        }
    }
    else if (g_pLetterList->CheckNoReadLetter())
    {
        friendAlert = true;
    }
    g_pFriendMenu->IncreaseBlinkTemp();
    syncBool(&MainFrameRmlModel::friendAlert, "friend_alert", friendAlert);

    // Skill-hotkey row selection highlight (legacy: the IMAGE_SKILLBOX_USE sprite, modern: an
    // outline; see MainFrameRmlModel::skillSlot0Selected and CSkillList::IsHotKeySlotCurrentSkill()).
    syncBool(&MainFrameRmlModel::skillSlot0Selected, "skill_slot_0_selected", g_pSkillList->IsHotKeySlotCurrentSkill(0));
    syncBool(&MainFrameRmlModel::skillSlot1Selected, "skill_slot_1_selected", g_pSkillList->IsHotKeySlotCurrentSkill(1));
    syncBool(&MainFrameRmlModel::skillSlot2Selected, "skill_slot_2_selected", g_pSkillList->IsHotKeySlotCurrentSkill(2));
    syncBool(&MainFrameRmlModel::skillSlot3Selected, "skill_slot_3_selected", g_pSkillList->IsHotKeySlotCurrentSkill(3));
    syncBool(&MainFrameRmlModel::skillSlot4Selected, "skill_slot_4_selected", g_pSkillList->IsHotKeySlotCurrentSkill(4));

    // Skill-hotkey number labels (see MainFrameRmlModel::skillSlot0Hotkey and
    // CSkillList::GetHotKeySlotNumber()). -1 (empty slot) becomes an empty string.
    auto hotkeyText = [](int hotkey) { return hotkey >= 0 ? std::to_string(hotkey) : Rml::String(); };
    syncText(&MainFrameRmlModel::skillSlot0Hotkey, "skill_slot_0_hotkey", hotkeyText(g_pSkillList->GetHotKeySlotNumber(0)));
    syncText(&MainFrameRmlModel::skillSlot1Hotkey, "skill_slot_1_hotkey", hotkeyText(g_pSkillList->GetHotKeySlotNumber(1)));
    syncText(&MainFrameRmlModel::skillSlot2Hotkey, "skill_slot_2_hotkey", hotkeyText(g_pSkillList->GetHotKeySlotNumber(2)));
    syncText(&MainFrameRmlModel::skillSlot3Hotkey, "skill_slot_3_hotkey", hotkeyText(g_pSkillList->GetHotKeySlotNumber(3)));
    syncText(&MainFrameRmlModel::skillSlot4Hotkey, "skill_slot_4_hotkey", hotkeyText(g_pSkillList->GetHotKeySlotNumber(4)));

    // Skill icons of the hotkey row and the current-skill slot, and the current skill's hotkey.
    syncText(&MainFrameRmlModel::skillSlot0Icon, "skill_slot_0_icon", g_pSkillList->GetHotKeySlotIconDecorator(0));
    syncText(&MainFrameRmlModel::skillSlot1Icon, "skill_slot_1_icon", g_pSkillList->GetHotKeySlotIconDecorator(1));
    syncText(&MainFrameRmlModel::skillSlot2Icon, "skill_slot_2_icon", g_pSkillList->GetHotKeySlotIconDecorator(2));
    syncText(&MainFrameRmlModel::skillSlot3Icon, "skill_slot_3_icon", g_pSkillList->GetHotKeySlotIconDecorator(3));
    syncText(&MainFrameRmlModel::skillSlot4Icon, "skill_slot_4_icon", g_pSkillList->GetHotKeySlotIconDecorator(4));
    syncText(&MainFrameRmlModel::currentSkillIcon, "current_skill_icon", g_pSkillList->GetCurrentSkillIconDecorator());
    syncText(&MainFrameRmlModel::currentSkillHotkey, "current_skill_hotkey",
             CharacterAttribute->SkillNumber > 0 ? hotkeyText(g_pSkillList->GetSkillHotKeyNumber(Hero->CurrentSkill))
                                                 : Rml::String());

    // Skill list cooldown fractions.
    syncFloat(&MainFrameRmlModel::skillSlot0Cooldown, "skill_slot_0_cooldown", g_pSkillList->GetHotKeySlotCooldownFraction(0));
    syncFloat(&MainFrameRmlModel::skillSlot1Cooldown, "skill_slot_1_cooldown", g_pSkillList->GetHotKeySlotCooldownFraction(1));
    syncFloat(&MainFrameRmlModel::skillSlot2Cooldown, "skill_slot_2_cooldown", g_pSkillList->GetHotKeySlotCooldownFraction(2));
    syncFloat(&MainFrameRmlModel::skillSlot3Cooldown, "skill_slot_3_cooldown", g_pSkillList->GetHotKeySlotCooldownFraction(3));
    syncFloat(&MainFrameRmlModel::skillSlot4Cooldown, "skill_slot_4_cooldown", g_pSkillList->GetHotKeySlotCooldownFraction(4));

    // Item hotkey chrome (#item_slots hover border + stack-count). m_ItemHotKey is a member, read directly.
    auto stackCountText = [](int count) { return count > 0 ? std::to_string(count) : Rml::String(); };
    syncBool(&MainFrameRmlModel::itemSlot0Hovered, "item_slot_0_hovered", m_ItemHotKey.GetHoveredSlot() == 0);
    syncBool(&MainFrameRmlModel::itemSlot1Hovered, "item_slot_1_hovered", m_ItemHotKey.GetHoveredSlot() == 1);
    syncBool(&MainFrameRmlModel::itemSlot2Hovered, "item_slot_2_hovered", m_ItemHotKey.GetHoveredSlot() == 2);
    syncBool(&MainFrameRmlModel::itemSlot3Hovered, "item_slot_3_hovered", m_ItemHotKey.GetHoveredSlot() == 3);
    syncText(&MainFrameRmlModel::itemSlot0Count, "item_slot_0_count", stackCountText(m_ItemHotKey.GetSlotItemCount(0)));
    syncText(&MainFrameRmlModel::itemSlot1Count, "item_slot_1_count", stackCountText(m_ItemHotKey.GetSlotItemCount(1)));
    syncText(&MainFrameRmlModel::itemSlot2Count, "item_slot_2_count", stackCountText(m_ItemHotKey.GetSlotItemCount(2)));
    syncText(&MainFrameRmlModel::itemSlot3Count, "item_slot_3_count", stackCountText(m_ItemHotKey.GetSlotItemCount(3)));
    syncFloat(&MainFrameRmlModel::currentSkillCooldown, "current_skill_cooldown", g_pSkillList->GetCurrentSkillCooldownFraction());

    syncBool(&MainFrameRmlModel::skillGridOpen, "skill_grid_open", g_pSkillList->IsSkillGridOpen());
    syncBool(&MainFrameRmlModel::skillListUp, "skill_list_up", g_pSkillList->IsSkillListUp());

    // Dynamic-count lists, copied unconditionally every frame while the grid is open (cooldown
    // fractions change every frame anyway, so a change-check would rarely help).
    if (model.skillGridOpen)
    {
        model.skillGridCells = g_pSkillList->GetGridSnapshot();
        model.petSkillCells = g_pSkillList->GetPetSnapshot();
        m_RmlBinder.MarkDirty("skill_grid_cells");
        m_RmlBinder.MarkDirty("pet_skill_cells");
    }

    // Shared skill tooltip: one hover target queued at a time (QueueTooltip()/OnUnhover()).
    // BuildModelForSlot() is the same content resolution SkillTooltip.cpp's Render() uses; the
    // destination is now UI::RmlBridge::Tooltip's own shared document, not a per-window RML block
    // -- consolidated onto it so this window doesn't carry its own separate tooltip styling/z-order
    // (see docs/rmlui-ui-system/component-catalog.md's "Tooltip" entry).
    if (g_pSkillList->IsTooltipPending())
    {
        UI::Skills::Tooltip::Model tooltipModel;
        if (UI::Skills::Tooltip::BuildModelForSlot(g_pSkillList->GetTooltipSkillIndex(), tooltipModel))
        {
            UI::RmlBridge::Tooltip::Config config;
            config.lines = UI::Skills::Tooltip::ToRmlBridgeLines(tooltipModel);
            // GetTooltipAnchorX/Y() are in HudReferenceTransform()'s units -- NOT the ambient
            // UI::Scaling::GetActiveTransform(), which during this window's Update() is
            // LayoutMode::Hud (ScreenOverlayTransform).
            const auto skillTooltipTransform = HudReferenceTransform(m_pRmlDoc);
            // The native box ends below its anchor (G2); measured with the native text renderer
            // under the tooltip's own transform.
            float bottomBelowAnchor = 0.f;
            {
                UI::Scaling::ScopedActiveTransform measure(skillTooltipTransform);
                bottomBelowAnchor = UI::Skills::Tooltip::NativeBoxBottomBelowAnchor(tooltipModel);
            }
            config.anchorX = UI::Scaling::PositionX(skillTooltipTransform, g_pSkillList->GetTooltipAnchorX());
            config.anchorY =
                UI::Scaling::PositionY(skillTooltipTransform, g_pSkillList->GetTooltipAnchorY() + bottomBelowAnchor);
            // The old #skill_tooltip's CSS (`transform: translateY(-100%)`) always grew upward,
            // unconditionally -- AboveLeft matches that; Show()'s own clamping now also covers the
            // horizontal/lower-edge cases that CSS-only transform never did.
            config.anchor = UI::RmlBridge::Tooltip::AnchorPoint::AboveLeft;
            // QueueTooltip()'s anchor is native's: the icon's centre, 10 above it; lines centred
            // (RenderTipTextList(..., RT3_SORT_CENTER, ...)).
            config.centerHorizontally = true;
            config.textAlign = UI::RmlBridge::Tooltip::Config::TextAlign::Center;
            config.transform = skillTooltipTransform;
            UI::RmlBridge::Tooltip::Show(config, g_pSkillList);
        }
        else
        {
            UI::RmlBridge::Tooltip::Hide(g_pSkillList);
        }
    }
    else
    {
        UI::RmlBridge::Tooltip::Hide(g_pSkillList);
    }
}

float mu::ui::window::CMainFrameWindow::GetLayerDepth()
{
    return 10.6f;
}

float mu::ui::window::CMainFrameWindow::GetKeyEventOrder()
{
    return 2.9f;
}

void mu::ui::window::CMainFrameWindow::SetItemHotKey(int iHotKey, int iItemType, int iItemLevel)
{
    m_ItemHotKey.SetHotKey(iHotKey, iItemType, iItemLevel);
}

int mu::ui::window::CMainFrameWindow::GetItemHotKey(int iHotKey)
{
    return m_ItemHotKey.GetHotKey(iHotKey);
}

int mu::ui::window::CMainFrameWindow::GetItemHotKeyLevel(int iHotKey)
{
    return m_ItemHotKey.GetHotKeyLevel(iHotKey);
}

void mu::ui::window::CMainFrameWindow::UpdateItemHotKey()
{
    m_ItemHotKey.UpdateKeyEvent();
}

void mu::ui::window::CMainFrameWindow::ResetSkillHotKey()
{
    g_pSkillList->Reset();
}

void mu::ui::window::CMainFrameWindow::SetSkillHotKey(int iHotKey, int iSkillType)
{
    g_pSkillList->SetHotKey(iHotKey, iSkillType);
}

int mu::ui::window::CMainFrameWindow::GetSkillHotKey(int iHotKey)
{
    return g_pSkillList->GetHotKey(iHotKey);
}

int mu::ui::window::CMainFrameWindow::GetSkillHotKeyIndex(int iSkillType)
{
    return g_pSkillList->GetSkillIndex(iSkillType);
}

void mu::ui::window::CMainFrameWindow::SetPreExp_Wide(__int64 dwPreExp)
{
    m_loPreExp = dwPreExp;
}

void mu::ui::window::CMainFrameWindow::SetGetExp_Wide(__int64 dwGetExp)
{
    m_loGetExp = dwGetExp;

    if (m_loGetExp > 0)
    {
        m_bExpEffect = true;
        m_dwExpEffectTime = timeGetTime();
    }
}

void mu::ui::window::CMainFrameWindow::SetPreExp(__int64 dwPreExp)
{
    m_dwPreExp = dwPreExp;
}

void mu::ui::window::CMainFrameWindow::SetGetExp(__int64 dwGetExp)
{
    m_dwGetExp = dwGetExp;

    if (m_dwGetExp > 0)
    {
        m_bExpEffect = true;
        m_dwExpEffectTime = timeGetTime();
    }
}


void mu::ui::window::CMainFrameWindow::SetBtnState(int iBtnType, bool bStateDown)
{
    // Sets a bound "open" model boolean; main_frame.rcss selects the panel-open sprite rect when true.
    if (!m_pRmlDoc) return;

    switch (iBtnType)
    {
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
    case MAINFRAME_BTN_PARTCHARGE:
        if (m_RmlBinder.GetModel().cShopOpen != bStateDown)
        {
            m_RmlBinder.GetModel().cShopOpen = bStateDown;
            m_RmlBinder.MarkDirty("cshop_open");
        }
        break;
#endif //defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
    case MAINFRAME_BTN_CHAINFO:
        if (m_RmlBinder.GetModel().chaInfoOpen != bStateDown)
        {
            m_RmlBinder.GetModel().chaInfoOpen = bStateDown;
            m_RmlBinder.MarkDirty("chainfo_open");
        }
        break;
    case MAINFRAME_BTN_MYINVEN:
        if (m_RmlBinder.GetModel().myInvenOpen != bStateDown)
        {
            m_RmlBinder.GetModel().myInvenOpen = bStateDown;
            m_RmlBinder.MarkDirty("myinven_open");
        }
        break;
    case MAINFRAME_BTN_FRIEND:
        if (m_RmlBinder.GetModel().friendOpen != bStateDown)
        {
            m_RmlBinder.GetModel().friendOpen = bStateDown;
            m_RmlBinder.MarkDirty("friend_open");
        }
        break;
    case MAINFRAME_BTN_WINDOW:
        if (m_RmlBinder.GetModel().windowOpen != bStateDown)
        {
            m_RmlBinder.GetModel().windowOpen = bStateDown;
            m_RmlBinder.MarkDirty("window_open");
        }
        break;
    }
}

void mu::ui::window::CMainFrameWindow::SyncDocVisibility(bool sceneAllowsShow)
{
    const bool show = IsVisible() && sceneAllowsShow;

    UI::RmlBridge::SyncDocumentVisibility(m_pRmlDoc, show);
    m_ItemHotKey.SetSlotIconsShown(show);
    UI::RmlBridge::SyncDocumentVisibility(m_pRmlTopDoc, show);
}
