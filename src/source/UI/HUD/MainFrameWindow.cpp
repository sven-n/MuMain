
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

mu::ui::window::CItemHotKey::CItemHotKey()
{
    for (int i = 0; i < HOTKEY_COUNT; ++i)
    {
        m_iHotKeyItemType[i] = -1;
        m_iHotKeyItemLevel[i] = 0;
        m_SlotTargets[i] = std::make_unique<UI::RmlBridge::RenderTarget>(
            [this, i](std::uint32_t width, std::uint32_t height) { RenderSlot(i, width, height); });
    }
}

mu::ui::window::CItemHotKey::~CItemHotKey()
{
}

bool mu::ui::window::CItemHotKey::UpdateKeyEvent()
{
    int iIndex = -1;

    if (mu::ui::window::IsPress('Q') == true)
    {
        iIndex = GetHotKeyItemIndex(HOTKEY_Q);
    }
    else if (mu::ui::window::IsPress('W') == true)
    {
        iIndex = GetHotKeyItemIndex(HOTKEY_W);
    }
    else if (mu::ui::window::IsPress('E') == true)
    {
        iIndex = GetHotKeyItemIndex(HOTKEY_E);
    }
    else if (mu::ui::window::IsPress('R') == true)
    {
        iIndex = GetHotKeyItemIndex(HOTKEY_R);
    }

    if (iIndex != -1)
    {
        ITEM* pItem = NULL;
        pItem = g_pMyInventory->FindItem(iIndex);
        if ((pItem->Type >= ITEM_POTION + 78 && pItem->Type <= ITEM_POTION + 82))
        {
            std::list<eBuffState> secretPotionbufflist;
            secretPotionbufflist.push_back(eBuff_SecretPotion1);
            secretPotionbufflist.push_back(eBuff_SecretPotion2);
            secretPotionbufflist.push_back(eBuff_SecretPotion3);
            secretPotionbufflist.push_back(eBuff_SecretPotion4);
            secretPotionbufflist.push_back(eBuff_SecretPotion5);

            if (g_isCharacterBufflist((&Hero->Object), secretPotionbufflist) != eBuffNone) {
                mu::ui::window::CreateOkMessageBox(I18N::Game::YouCannotUseThisItemWhileThePotionEffectsRemainActive, RGBA(255, 30, 0, 255));
            }
            else {
                SendRequestUse(iIndex, 0);
            }
        }
        else

        {
            SendRequestUse(iIndex, 0);
        }
        return false;
    }

    return true;
}

int mu::ui::window::CItemHotKey::GetHotKeyItemIndex(int iType, bool bItemCount)
{
    int iStartItemType = 0, iEndItemType = 0;
    int i, j;

    switch (iType)
    {
    case HOTKEY_Q:
        if (GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
        {
            if (m_iHotKeyItemType[iType] >= ITEM_SMALL_MANA_POTION && m_iHotKeyItemType[iType] <= ITEM_LARGE_MANA_POTION)
            {
                iStartItemType = ITEM_LARGE_MANA_POTION; iEndItemType = ITEM_SMALL_MANA_POTION;
            }
            else
            {
                iStartItemType = ITEM_LARGE_HEALING_POTION; iEndItemType = ITEM_APPLE;
            }
        }
        break;
    case HOTKEY_W:
        if (GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
        {
            if (m_iHotKeyItemType[iType] >= ITEM_APPLE && m_iHotKeyItemType[iType] <= ITEM_LARGE_HEALING_POTION)
            {
                iStartItemType = ITEM_LARGE_HEALING_POTION; iEndItemType = ITEM_APPLE;
            }
            else
            {
                iStartItemType = ITEM_LARGE_MANA_POTION; iEndItemType = ITEM_SMALL_MANA_POTION;
            }
        }
        break;
    case HOTKEY_E:
        if (GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
        {
            if (m_iHotKeyItemType[iType] >= ITEM_APPLE && m_iHotKeyItemType[iType] <= ITEM_LARGE_HEALING_POTION)
            {
                iStartItemType = ITEM_LARGE_HEALING_POTION; iEndItemType = ITEM_APPLE;
            }
            else if (m_iHotKeyItemType[iType] >= ITEM_SMALL_MANA_POTION && m_iHotKeyItemType[iType] <= ITEM_LARGE_MANA_POTION)
            {
                iStartItemType = ITEM_LARGE_MANA_POTION; iEndItemType = ITEM_SMALL_MANA_POTION;
            }
            else
            {
                iStartItemType = ITEM_ANTIDOTE; iEndItemType = ITEM_ANTIDOTE;
            }
        }
        break;
    case HOTKEY_R:
        if (GetHotKeyCommonItem(iType, iStartItemType, iEndItemType) == false)
        {
            if (m_iHotKeyItemType[iType] >= ITEM_APPLE && m_iHotKeyItemType[iType] <= ITEM_LARGE_HEALING_POTION)
            {
                iStartItemType = ITEM_LARGE_HEALING_POTION; iEndItemType = ITEM_APPLE;
            }
            else if (m_iHotKeyItemType[iType] >= ITEM_SMALL_MANA_POTION && m_iHotKeyItemType[iType] <= ITEM_LARGE_MANA_POTION)
            {
                iStartItemType = ITEM_LARGE_MANA_POTION; iEndItemType = ITEM_SMALL_MANA_POTION;
            }
            else
            {
                iStartItemType = ITEM_LARGE_SHIELD_POTION; iEndItemType = ITEM_SMALL_SHIELD_POTION;
            }
        }
        break;
    }

    int iItemCount = 0;
    ITEM* pItem = NULL;

    int iNumberofItems = g_pMyInventory->GetInventoryCtrl()->GetNumberOfItems();
    for (i = iStartItemType; i >= iEndItemType; --i)
    {
        if (bItemCount)
        {
            for (j = 0; j < iNumberofItems; ++j)
            {
                pItem = g_pMyInventory->GetInventoryCtrl()->GetItem(j);
                if (pItem == NULL)
                {
                    continue;
                }

                if (
                    (pItem->Type == i && pItem->Level == m_iHotKeyItemLevel[iType])
                    || (pItem->Type == i && (pItem->Type >= ITEM_APPLE && pItem->Type <= ITEM_LARGE_HEALING_POTION))
                    )
                {
                    if (pItem->Type == ITEM_ALE
                        || pItem->Type == ITEM_TOWN_PORTAL_SCROLL
                        || pItem->Type == ITEM_POTION + 20
                        )
                    {
                        iItemCount++;
                    }
                    else
                    {
                        iItemCount += pItem->Durability;
                    }
                }
            }
        }
        else
        {
            int iIndex = -1;
            if (i >= ITEM_APPLE && i <= ITEM_LARGE_HEALING_POTION)
            {
                iIndex = g_pMyInventory->FindItemReverseIndex(i);
            }
            else
            {
                iIndex = g_pMyInventory->FindItemReverseIndex(i, m_iHotKeyItemLevel[iType]);
            }

            if (-1 != iIndex)
            {
                pItem = g_pMyInventory->FindItem(iIndex);
                if ((pItem->Type != ITEM_SIEGE_POTION
                    && pItem->Type != ITEM_TOWN_PORTAL_SCROLL
                    && pItem->Type != ITEM_POTION + 20)
                    || pItem->Level == m_iHotKeyItemLevel[iType]
                    )
                {
                    return iIndex;
                }
            }
        }
    }

    if (bItemCount == true)
    {
        return iItemCount;
    }

    return -1;
}

bool mu::ui::window::CItemHotKey::GetHotKeyCommonItem(IN int iHotKey, OUT int& iStart, OUT int& iEnd)
{
    switch (m_iHotKeyItemType[iHotKey])
    {
    case ITEM_SIEGE_POTION:
    case ITEM_ANTIDOTE:
    case ITEM_ALE:
    case ITEM_TOWN_PORTAL_SCROLL:
    case ITEM_POTION + 20:
    case ITEM_JACK_OLANTERN_BLESSINGS:
    case ITEM_JACK_OLANTERN_WRATH:
    case ITEM_JACK_OLANTERN_CRY:
    case ITEM_JACK_OLANTERN_FOOD:
    case ITEM_JACK_OLANTERN_DRINK:
    case ITEM_POTION + 70:
    case ITEM_POTION + 71:
    case ITEM_POTION + 78:
    case ITEM_POTION + 79:
    case ITEM_POTION + 80:
    case ITEM_POTION + 81:
    case ITEM_POTION + 82:
    case ITEM_POTION + 94:
    case ITEM_CHERRY_BLOSSOM_WINE:
    case ITEM_CHERRY_BLOSSOM_RICE_CAKE:
    case ITEM_CHERRY_BLOSSOM_FLOWER_PETAL:
    case ITEM_POTION + 133:
        if (m_iHotKeyItemType[iHotKey] != ITEM_POTION + 20 || m_iHotKeyItemLevel[iHotKey] == 0)
        {
            iStart = iEnd = m_iHotKeyItemType[iHotKey];
            return true;
        }
        break;
    default:
        if (m_iHotKeyItemType[iHotKey] >= ITEM_SMALL_SHIELD_POTION && m_iHotKeyItemType[iHotKey] <= ITEM_LARGE_SHIELD_POTION)
        {
            iStart = ITEM_LARGE_SHIELD_POTION; iEnd = ITEM_SMALL_SHIELD_POTION;
            return true;
        }
        else if (m_iHotKeyItemType[iHotKey] >= ITEM_SMALL_COMPLEX_POTION && m_iHotKeyItemType[iHotKey] <= ITEM_LARGE_COMPLEX_POTION)
        {
            iStart = ITEM_LARGE_COMPLEX_POTION; iEnd = ITEM_SMALL_COMPLEX_POTION;
            return true;
        }
        break;
    }
    return false;
}

int mu::ui::window::CItemHotKey::GetHotKeyItemCount(int iType)
{
    return 0;
}

void mu::ui::window::CItemHotKey::SetHotKey(int iHotKey, int iItemType, int iItemLevel)
{
    if (iHotKey != -1 && CMyInventory::CanRegisterItemHotKey(iItemType) == true
        )
    {
        m_iHotKeyItemType[iHotKey] = iItemType;
        m_iHotKeyItemLevel[iHotKey] = iItemLevel;
    }
    else
    {
        m_iHotKeyItemType[iHotKey] = -1;
        m_iHotKeyItemLevel[iHotKey] = 0;
    }
}

int mu::ui::window::CItemHotKey::GetHotKey(int iHotKey)
{
    if (iHotKey != -1)
    {
        return m_iHotKeyItemType[iHotKey];
    }

    return -1;
}

int mu::ui::window::CItemHotKey::GetHotKeyLevel(int iHotKey)
{
    if (iHotKey != -1)
    {
        return m_iHotKeyItemLevel[iHotKey];
    }

    return 0;
}

ITEM* mu::ui::window::CItemHotKey::GetSlotItem(int iSlotIndex)
{
    const int iIndex = GetHotKeyItemIndex(iSlotIndex);
    return iIndex != -1 ? g_pMyInventory->FindItem(iIndex) : nullptr;
}

void mu::ui::window::CItemHotKey::SyncSlotIcons(Rml::ElementDocument* document)
{
    if (document == nullptr)
        return;
    for (int i = 0; i < HOTKEY_COUNT; ++i)
    {
        Rml::Element* icon = document->GetElementById("item_icon_" + std::to_string(i));
        if (icon == nullptr)
            continue;
        auto& target = *m_SlotTargets[i];
        const bool filled = GetSlotItem(i) != nullptr;
        target.SetEnabled(m_bSlotIconsShown && filled);
        if (filled)
        {
            // The box is in screen pixels: the item is drawn at the size it is shown, never upscaled.
            const auto size = icon->GetBox().GetSize(Rml::BoxArea::Content);
            target.Resize(static_cast<std::uint32_t>(std::lround(size.x)),
                          static_cast<std::uint32_t>(std::lround(size.y)));
        }
        const Rml::String source = filled ? target.Source() : Rml::String();
        if (icon->GetAttribute<Rml::String>("src", "") != source)
            icon->SetAttribute("src", source);
    }
}

void mu::ui::window::CItemHotKey::SetSlotIconsShown(bool shown)
{
    m_bSlotIconsShown = shown;
    if (shown)
        return;
    for (auto& target : m_SlotTargets)
        target->SetEnabled(false);
}

// The item camera C3DCamera::Render() sets up -- an identity view at a 1-degree field of view --
// with its projection cropped to one slot-sized rectangle, so that rectangle fills the target. At
// that field of view where the rectangle sits barely matters, so it is centred, on-axis; only its
// size frames the item, and every per-item offset in RenderItem3D() applies exactly as before.
void mu::ui::window::CItemHotKey::RenderSlot(int iSlotIndex, std::uint32_t width, std::uint32_t height)
{
    ITEM* pItem = GetSlotItem(iSlotIndex);
    if (pItem == nullptr || width == 0 || height == 0)
        return;

    const float windowWidth = static_cast<float>(WindowWidth);
    const float windowHeight = static_cast<float>(WindowHeight);
    const float w = static_cast<float>(width);
    const float h = static_cast<float>(height);
    const float x = (windowWidth - w) * 0.5f;
    const float y = (windowHeight - h) * 0.5f;

    // gluPerspective2() and the identity view overwrite g_Camera, which picking reads.
    SaveCameraPerspective();
    // The rectangle is in window pixels, which is what ScreenToWorldRay() turns it into.
    const UI::Scaling::ScopedActiveTransform pixels({1.f, 1.f, 0.f, 0.f, 1.f});

    auto& renderer = mu::GetRenderer();
    renderer.SetMatrixMode(GL_PROJECTION);
    renderer.PushMatrix();
    renderer.LoadIdentity();
    const float scaleX = windowWidth / w;
    const float scaleY = windowHeight / h;
    const float centerX = (2.f * x + w) / windowWidth - 1.f;
    const float centerY = 1.f - (2.f * y + h) / windowHeight;
    renderer.Translate(-centerX * scaleX, -centerY * scaleY, 0.f);
    renderer.Scale(scaleX, scaleY, 1.f);
    // gluPerspective2() takes the camera's screen centre from the viewport; the capture brings its own.
    SetRenderViewport(0, 0, WindowWidth, WindowHeight);
    gluPerspective2(1.f, windowWidth / windowHeight, RENDER_ITEMVIEW_NEAR, RENDER_ITEMVIEW_FAR);
    renderer.SetMatrixMode(GL_MODELVIEW);
    renderer.PushMatrix();
    renderer.LoadIdentity();
    CameraProjection::GetOpenGLMatrix(g_Camera.Matrix);
    EnableDepthTest();
    EnableDepthMask();

    RenderItem3DWithHover(x, y, w, h, pItem->Type, pItem->Level, 0, 0, m_iHoveredSlot == iSlotIndex);

    renderer.SetMatrixMode(GL_MODELVIEW);
    renderer.PopMatrix();
    renderer.SetMatrixMode(GL_PROJECTION);
    renderer.PopMatrix();
    RestoreCameraPerspective();
}

void mu::ui::window::CItemHotKey::OnHotkeySlotRightClick(int iSlotIndex)
{
    // RmlUi's Context now does hit-testing for these 4 slots (data-event-mouseup). `Hero->Dead !=
    // 0` reproduces the old call site's own alive guard.
    if (Hero->Dead != 0)
    {
        return;
    }
    int iIndex = GetHotKeyItemIndex(iSlotIndex);
    if (iIndex != -1)
    {
        SendRequestUse(iIndex, 0);
    }
}

int mu::ui::window::CItemHotKey::GetSlotItemCount(int iSlotIndex)
{
    return GetHotKeyItemIndex(iSlotIndex, true);
}

mu::ui::window::CSkillList::CSkillList()
{
    m_pNewUIMng = NULL;
    Reset();
}

mu::ui::window::CSkillList::~CSkillList()
{
    Release();
}

bool mu::ui::window::CSkillList::Create(CManager* pNewUIMng, C3DRenderMng* pNewUI3DRenderMng)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_SKILL_LIST, this);

    m_pNewUI3DRenderMng = pNewUI3DRenderMng;

    LoadImages();

    Show(true);

    return true;
}

void mu::ui::window::CSkillList::Release()
{
    // The tooltip is a plain RmlUi element now, so nothing needs unregistering via
    // UI2DEffectObject/DeleteUI2DEffectObject() here.
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CSkillList::Reset()
{
    m_bSkillList = false;
    m_bHotKeySkillListUp = false;

    for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
    {
        m_iHotKeySkillType[i] = -1;
    }

    m_GridSnapshot.clear();
    m_PetSnapshot.clear();
    m_bTooltipPending = false;
    m_iTooltipSkillIndex = -1;
    m_fTooltipAnchorX = 0.f;
    m_fTooltipAnchorY = 0.f;
    m_iHoveredGridSkillIndex = -1;
}

void mu::ui::window::CSkillList::LoadImages()
{
    // The HUD draws none of these any more (main_frame.rml loads the atlases itself), but the
    // texture slots are shared: CUIMuHelper draws skill icons and boxes from them natively.
    LoadBitmap(L"Interface\\newui_skill.jpg", IMAGE_SKILL1, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_skill2.jpg", IMAGE_SKILL2, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_command.jpg", IMAGE_COMMAND, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_skillbox.jpg", IMAGE_SKILLBOX, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_skillbox2.jpg", IMAGE_SKILLBOX_USE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_non_skill.jpg", IMAGE_NON_SKILL1, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_non_skill2.jpg", IMAGE_NON_SKILL2, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_non_command.jpg", IMAGE_NON_COMMAND, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_skill3.jpg", IMAGE_SKILL3, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_non_skill3.jpg", IMAGE_NON_SKILL3, GL_LINEAR);
}

void mu::ui::window::CSkillList::UnloadImages()
{
    DeleteBitmap(IMAGE_SKILL1);
    DeleteBitmap(IMAGE_SKILL2);
    DeleteBitmap(IMAGE_COMMAND);
    DeleteBitmap(IMAGE_SKILLBOX);
    DeleteBitmap(IMAGE_SKILLBOX_USE);
    DeleteBitmap(IMAGE_NON_SKILL1);
    DeleteBitmap(IMAGE_NON_SKILL2);
    DeleteBitmap(IMAGE_NON_COMMAND);
    DeleteBitmap(IMAGE_SKILL3);
    DeleteBitmap(IMAGE_NON_SKILL3);
}

bool mu::ui::window::CSkillList::UpdateMouseEvent()
{
    // RmlUi's own Context now does hit-testing for the current-skill icon, hotkey row, and
    // grid/pet row (see OnHotkeySlotClick()/OnCurrentSkillClick()/OnGridCellClick()/
    // OnPetCellClick() and their *Hover() counterparts).
    return true;
}

bool mu::ui::window::CSkillList::UpdateKeyEvent()
{
    for (int i = 0; i < 9; ++i)
    {
        if (mu::ui::window::IsPress('1' + i))
        {
            UseHotKey(i + 1);
        }
    }

    if (mu::ui::window::IsPress('0'))
    {
        UseHotKey(0);
    }

    // m_iHoveredGridSkillIndex arms Ctrl+digit assignment; set by OnGridCellHover()/OnPetCellHover(), cleared by OnUnhover().
    if (m_iHoveredGridSkillIndex != -1)
    {
        if (mu::ui::window::IsRepeat(VK_CONTROL))
        {
            for (int i = 0; i < 9; ++i)
            {
                if (mu::ui::window::IsPress('1' + i))
                {
                    SetHotKey(i + 1, m_iHoveredGridSkillIndex);

                    return false;
                }
            }

            if (mu::ui::window::IsPress('0'))
            {
                SetHotKey(0, m_iHoveredGridSkillIndex);

                return false;
            }
        }
    }

    if (mu::ui::window::IsRepeat(VK_SHIFT))
    {
        for (int i = 0; i < 4; ++i)
        {
            if (mu::ui::window::IsPress('1' + i))
            {
                Hero->CurrentSkill = AT_PET_COMMAND_DEFAULT + i;
                return false;
            }
        }
    }

    return true;
}

bool mu::ui::window::CSkillList::IsArrayUp(BYTE bySkill)
{
    for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
    {
        if (m_iHotKeySkillType[i] == bySkill)
        {
            if (i == 0 || i > 5)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    return false;
}

bool mu::ui::window::CSkillList::IsArrayIn(BYTE bySkill)
{
    for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
    {
        if (m_iHotKeySkillType[i] == bySkill)
        {
            return true;
        }
    }

    return false;
}

void mu::ui::window::CSkillList::SetHotKey(int iHotKey, int iSkillType)
{
    for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
    {
        if (m_iHotKeySkillType[i] == iSkillType)
        {
            m_iHotKeySkillType[i] = -1;
            break;
        }
    }

    m_iHotKeySkillType[iHotKey] = iSkillType;
}

int mu::ui::window::CSkillList::GetHotKey(int iHotKey)
{
    return m_iHotKeySkillType[iHotKey];
}

int mu::ui::window::CSkillList::GetSkillIndex(int iSkillType)
{
    // special handling for skills with different skill id for the trigger
    if (iSkillType == AT_SKILL_NOVA_BEGIN)
    {
        iSkillType = AT_SKILL_NOVA;
    }

    int iReturn = -1;
    for (int i = 0; i < MAX_MAGIC; ++i)
    {
        if (CharacterAttribute->Skill[i] == iSkillType)
        {
            iReturn = i;
            break;
        }
    }

    return iReturn;
}

void mu::ui::window::CSkillList::UseHotKey(int iHotKey)
{
    if (m_iHotKeySkillType[iHotKey] != -1)
    {
        if (m_iHotKeySkillType[iHotKey] >= AT_PET_COMMAND_DEFAULT && m_iHotKeySkillType[iHotKey] < AT_PET_COMMAND_END)
        {
            if (Hero->m_pPet == NULL)
            {
                return;
            }
        }

        auto wHotKeySkill = CharacterAttribute->Skill[m_iHotKeySkillType[iHotKey]];

        if (wHotKeySkill == 0)
        {
            return;
        }

        m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];

        Hero->CurrentSkill = m_iHotKeySkillType[iHotKey];

        auto bySkill = CharacterAttribute->Skill[Hero->CurrentSkill];

        if (
            g_pOption->IsAutoAttack() == true
            && gMapManager.WorldActive != WD_6STADIUM
            && gMapManager.InChaosCastle() == false
            && (bySkill == AT_SKILL_TELEPORT || bySkill == AT_SKILL_TELEPORT_ALLY))
        {
            SelectedCharacter = -1;
            Attacking = -1;
        }
    }
}

bool mu::ui::window::CSkillList::Update()
{
    if (IsArrayIn(Hero->CurrentSkill) == true)
    {
        if (IsArrayUp(Hero->CurrentSkill) == true)
        {
            m_bHotKeySkillListUp = true;
        }
        else
        {
            m_bHotKeySkillListUp = false;
        }
    }

    if (Hero->m_pPet == NULL)
    {
        if (Hero->CurrentSkill >= AT_PET_COMMAND_DEFAULT && Hero->CurrentSkill < AT_PET_COMMAND_END)
        {
            Hero->CurrentSkill = 0;
        }
    }

    // Refreshes the RmlUi-facing overlay snapshot while the grid is open; left stale (harmless,
    // hidden) while closed.
    if (m_bSkillList)
    {
        RebuildGridSnapshot();
    }

    return true;
}

bool mu::ui::window::CSkillList::IsHotKeySlotCurrentSkill(int iSlotIndex)
{
    // The hotkey row shows hotkeys 1-5 or 6-9,0 (wraparound), skipping empty slots and pet
    // commands without a pet.
    if (iSlotIndex < 0 || iSlotIndex >= 5)
        return false;

    if (CharacterAttribute->SkillNumber == 0)
        return false;

    int iStartSkillIndex = m_bHotKeySkillListUp ? 6 : 1;
    int iIndex = iStartSkillIndex + iSlotIndex;
    if (iIndex == 10)
        iIndex = 0;

    if (m_iHotKeySkillType[iIndex] == -1)
        return false;

    if (m_iHotKeySkillType[iIndex] >= AT_PET_COMMAND_DEFAULT && m_iHotKeySkillType[iIndex] < AT_PET_COMMAND_END)
    {
        if (Hero->m_pPet == NULL)
            return false;
    }

    return Hero->CurrentSkill == m_iHotKeySkillType[iIndex];
}

int mu::ui::window::CSkillList::GetHotKeySlotNumber(int iSlotIndex)
{
    // Same loop as IsHotKeySlotCurrentSkill(), but doesn't check Hero->CurrentSkill -- the number
    // shows for every occupied slot, not just the active one.
    if (iSlotIndex < 0 || iSlotIndex >= 5)
        return -1;

    if (CharacterAttribute->SkillNumber == 0)
        return -1;

    int iStartSkillIndex = m_bHotKeySkillListUp ? 6 : 1;
    int iIndex = iStartSkillIndex + iSlotIndex;
    if (iIndex == 10)
        iIndex = 0;

    if (m_iHotKeySkillType[iIndex] == -1)
        return -1;

    if (m_iHotKeySkillType[iIndex] >= AT_PET_COMMAND_DEFAULT && m_iHotKeySkillType[iIndex] < AT_PET_COMMAND_END)
    {
        if (Hero->m_pPet == NULL)
            return -1;
    }

    return iIndex;
}

bool mu::ui::window::CSkillList::Render()
{
    // Nothing native: the hotkey row, current-skill slot, grid and pet row are main_frame.rml
    // (icons, legacy box art, hotkey numbers, cooldown wipes), fed by SyncRmlModel().
    return true;
}

float mu::ui::window::CSkillList::GetLayerDepth()
{
    return 5.2f;
}

WORD mu::ui::window::CSkillList::GetHeroPriorSkill()
{
    return m_wHeroPriorSkill;
}

void mu::ui::window::CSkillList::SetHeroPriorSkill(BYTE bySkill)
{
    m_wHeroPriorSkill = bySkill;
}

namespace
{
bool HasOneHandedOrMeleeWeapon()
{
    const int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
    const int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;
    return iTypeR != -1 && (iTypeR < ITEM_STAFF || iTypeR >= ITEM_STAFF + MAX_ITEM_INDEX) &&
           (iTypeL < ITEM_STAFF || iTypeL >= ITEM_STAFF + MAX_ITEM_INDEX);
}

// The original RenderSkillIcon()'s checks that chose the grey icon: equipment, mount, buffs,
// party, map and stats. The atlas cell itself is UI::Skills::Icon::ResolveSkillIcon().
bool IsHudSkillUsable(ActionSkillType bySkillType)
{
    bool bCantSkill = false;

    const BYTE bySkillUseType = SkillAttribute[bySkillType].SkillUseType;

    if (!gSkillManager.AreSkillAttributeRequirementsMet(bySkillType))
    {
        bCantSkill = true;
    }

    if (IsCanBCSkill(bySkillType) == false)
    {
        bCantSkill = true;
    }
    if (g_isCharacterBuff((&Hero->Object), eBuff_AddSkill) && bySkillUseType == SKILL_USE_TYPE_BRAND)
    {
        bCantSkill = true;
    }
    auto isSittingOnPet = (Hero->Helper.Type == MODEL_HORN_OF_UNIRIA || Hero->Helper.Type == MODEL_HORN_OF_DINORANT ||
                           Hero->Helper.Type == MODEL_HORN_OF_FENRIR);
    if (bySkillType == AT_SKILL_IMPALE && !isSittingOnPet)
    {
        bCantSkill = true;
    }

    if (bySkillType == AT_SKILL_IMPALE && isSittingOnPet)
    {
        int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
        int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;
        if ((iTypeL < ITEM_SPEAR || iTypeL >= ITEM_BOW) && (iTypeR < ITEM_SPEAR || iTypeR >= ITEM_BOW))
        {
            bCantSkill = true;
        }
    }

    if (isSittingOnPet && ((bySkillType >= AT_SKILL_BLOCKING && bySkillType <= AT_SKILL_SLASH) ||
                           bySkillType == AT_SKILL_FALLING_SLASH_STR || bySkillType == AT_SKILL_LUNGE_STR ||
                           bySkillType == AT_SKILL_CYCLONE_STR || bySkillType == AT_SKILL_CYCLONE_STR_MG ||
                           bySkillType == AT_SKILL_SLASH_STR))
    {
        bCantSkill = true;
    }

    if ((bySkillType == AT_SKILL_POWER_SLASH || bySkillType == AT_SKILL_POWER_SLASH_STR) && isSittingOnPet)
    {
        bCantSkill = true;
    }

    if (bySkillType == AT_SKILL_PARTY_TELEPORT && PartyNumber <= 0)
    {
        bCantSkill = true;
    }

    if (bySkillType == AT_SKILL_PARTY_TELEPORT &&
        (IsDoppelGanger1() || IsDoppelGanger2() || IsDoppelGanger3() || IsDoppelGanger4()))
    {
        bCantSkill = true;
    }

    if (bySkillType == AT_SKILL_EARTHSHAKE || bySkillType == AT_SKILL_EARTHSHAKE_STR ||
        bySkillType == AT_SKILL_EARTHSHAKE_MASTERY)
    {
        BYTE byDarkHorseLife = 0;
        byDarkHorseLife = CharacterMachine->Equipment[EQUIPMENT_HELPER].Durability;
        if (byDarkHorseLife == 0 || Hero->Helper.Type != MODEL_DARK_HORSE_ITEM)
        {
            bCantSkill = true;
        }
    }
#ifdef PJH_FIX_SPRIT
    /*박종훈*/
    if (bySkillType >= AT_PET_COMMAND_DEFAULT && bySkillType < AT_PET_COMMAND_END)
    {
        int iCharisma = CharacterAttribute->Charisma + CharacterAttribute->AddCharisma;
        PET_INFO PetInfo;
        giPetManager::GetPetInfo(PetInfo, 421 - PET_TYPE_DARK_SPIRIT);
        int RequireCharisma = (185 + (PetInfo.m_wLevel * 15));
        if (RequireCharisma > iCharisma)
        {
            bCantSkill = true;
        }
    }
#endif // PJH_FIX_SPRIT
    if ((bySkillType == AT_SKILL_INFINITY_ARROW) || (bySkillType == AT_SKILL_INFINITY_ARROW_STR) ||
        (bySkillType == AT_SKILL_EXPANSION_OF_WIZARDRY) || (bySkillType == AT_SKILL_EXPANSION_OF_WIZARDRY_STR) ||
        (bySkillType == AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY))
    {
        if ((g_isCharacterBuff((&Hero->Object), eBuff_InfinityArrow)) ||
            (g_isCharacterBuff((&Hero->Object), eBuff_SwellOfMagicPower)))
        {
            bCantSkill = true;
        }
    }

    if (bySkillType == AT_SKILL_FIRE_SLASH || bySkillType == AT_SKILL_FIRE_SLASH_STR)
    {
        WORD Strength;
        const WORD wRequireStrength = 596;
        Strength = CharacterAttribute->Strength + CharacterAttribute->AddStrength;
        if (Strength < wRequireStrength)
        {
            bCantSkill = true;
        }
        int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
        int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;

        if (!(iTypeR != -1 && (iTypeR < ITEM_STAFF || iTypeR >= ITEM_STAFF + MAX_ITEM_INDEX) &&
              (iTypeL < ITEM_STAFF || iTypeL >= ITEM_STAFF + MAX_ITEM_INDEX)))
        {
            bCantSkill = true;
        }
    }

    switch (bySkillType)
    {
        // case AT_SKILL_PIERCING:
    case AT_SKILL_ICE_ARROW:
    case AT_SKILL_ICE_ARROW_STR:
    {
        WORD Dexterity;
        const WORD wRequireDexterity = 646;
        Dexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
        if (Dexterity < wRequireDexterity)
        {
            bCantSkill = true;
        }
    }
    break;
    }

    if (bySkillType == AT_SKILL_TWISTING_SLASH || bySkillType == AT_SKILL_TWISTING_SLASH_STR ||
        bySkillType == AT_SKILL_TWISTING_SLASH_STR_MG || bySkillType == AT_SKILL_TWISTING_SLASH_MASTERY ||
        bySkillType == AT_SKILL_RAGEFUL_BLOW || bySkillType == AT_SKILL_RAGEFUL_BLOW_STR ||
        bySkillType == AT_SKILL_RAGEFUL_BLOW_MASTERY || bySkillType == AT_SKILL_DEATHSTAB ||
        bySkillType == AT_SKILL_DEATHSTAB_STR)
    {
        int iTypeL = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;
        int iTypeR = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;

        if (!(iTypeR != -1 && (iTypeR < ITEM_STAFF || iTypeR >= ITEM_STAFF + MAX_ITEM_INDEX) &&
              (iTypeL < ITEM_STAFF || iTypeL >= ITEM_STAFF + MAX_ITEM_INDEX)))
        {
            bCantSkill = true;
        }
    }

    if (gMapManager.InChaosCastle() == true)
    {
        if (bySkillType == AT_SKILL_EARTHSHAKE || bySkillType == AT_SKILL_EARTHSHAKE_STR ||
            bySkillType == AT_SKILL_EARTHSHAKE_MASTERY || bySkillType == AT_SKILL_RIDER ||
            (static_cast<int>(bySkillType) >= static_cast<int>(AT_PET_COMMAND_DEFAULT) &&
             static_cast<int>(bySkillType) <= static_cast<int>(AT_PET_COMMAND_TARGET)))
        {
            bCantSkill = true;
        }
    }
    else
    {
        if (bySkillType == AT_SKILL_EARTHSHAKE || bySkillType == AT_SKILL_EARTHSHAKE_STR ||
            bySkillType == AT_SKILL_EARTHSHAKE_MASTERY)
        {
            BYTE byDarkHorseLife = 0;
            byDarkHorseLife = CharacterMachine->Equipment[EQUIPMENT_HELPER].Durability;
            if (byDarkHorseLife == 0)
            {
                bCantSkill = true;
            }
        }
    }

    if (!g_CMonkSystem.IsSwordformGlovesUseSkill(bySkillType))
    {
        bCantSkill = true;
    }
    if (g_CMonkSystem.IsRideNotUseSkill(bySkillType, Hero->Helper.Type))
    {
        bCantSkill = true;
    }

    ITEM* pLeftRing = &CharacterMachine->Equipment[EQUIPMENT_RING_LEFT];
    ITEM* pRightRing = &CharacterMachine->Equipment[EQUIPMENT_RING_RIGHT];

    if (g_CMonkSystem.IsChangeringNotUseSkill(pLeftRing->Type, pRightRing->Type, pLeftRing->Level, pRightRing->Level) &&
        (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER))
    {
        bCantSkill = true;
    }

    if (!g_csItemOption.IsNonWeaponSkillOrIsSkillEquipped(bySkillType))
    {
        bCantSkill = true;
    }

    if (bySkillType == AT_SKILL_MULTI_SHOT && gCharacterManager.GetEquipedBowType_Skill() == BOWTYPE_NONE)
    {
        bCantSkill = true;
    }

    if (bySkillType == AT_SKILL_FLAME_STRIKE && !HasOneHandedOrMeleeWeapon())
    {
        bCantSkill = true;
    }

    return !bCantSkill;
}
} // namespace

Rml::String mu::ui::window::CSkillList::GetSkillIconDecorator(int iIndex)
{
    auto bySkillType = CharacterAttribute->Skill[iIndex];

    if (bySkillType == 0)
    {
        return "none";
    }

    if (iIndex >= AT_PET_COMMAND_DEFAULT)
    {
        bySkillType = (ActionSkillType)iIndex;
    }

    const UI::Skills::Icon::SkillIcon icon =
        UI::Skills::Icon::ResolveSkillIcon({.skillType = bySkillType,
                                            .skillUseType = SkillAttribute[bySkillType].SkillUseType,
                                            .magicIcon = SkillAttribute[bySkillType].Magic_Icon,
                                            .usable = IsHudSkillUsable(bySkillType)});
    const std::string sprite = UI::Skills::Icon::IconSpriteName(icon);
    return sprite.empty() ? Rml::String("none") : "image(" + sprite + ")";
}

Rml::String mu::ui::window::CSkillList::GetHotKeySlotIconDecorator(int iSlotIndex)
{
    const int iHotKey = GetHotKeySlotNumber(iSlotIndex);
    return iHotKey >= 0 ? GetSkillIconDecorator(m_iHotKeySkillType[iHotKey]) : Rml::String("none");
}

Rml::String mu::ui::window::CSkillList::GetCurrentSkillIconDecorator()
{
    return CharacterAttribute->SkillNumber > 0 ? GetSkillIconDecorator(Hero->CurrentSkill) : Rml::String("none");
}

int mu::ui::window::CSkillList::GetSkillHotKeyNumber(int iIndex)
{
    for (int i = 0; i < SKILLHOTKEY_COUNT; ++i)
    {
        if (m_iHotKeySkillType[i] == iIndex)
        {
            return i;
        }
    }
    return -1;
}

namespace
{
    // RenderSkillDelay()'s own fraction math (iSkillDelay/iSkillMaxDelay), draw call replaced with
    // a plain return. Feeds SkillCellEntry::cooldownFraction and
    // GetHotKeySlotCooldownFraction()/GetCurrentSkillCooldownFraction().
    //
    // Known gap: unlike the original, this doesn't suppress the cooldown wipe for
    // AT_SKILL_CHAIN_DRIVE/_STR, AT_SKILL_DRAGON_KICK/_ROAR/_STR when bCantSkill is also true --
    // a minor double-signal for those 4 skills, not a functional bug.
    float ComputeSkillCooldownFraction(int iIndex)
    {
        // Mirrors RenderSkillIcon()'s bySkillType resolution, used only for the 5-skill exclusion
        // gate below. RenderSkillDelay()'s own resolution (further down) is NOT pet-aware -- a
        // pre-existing inconsistency between the two, preserved rather than fixed.
        WORD bySkillTypeForGate = CharacterAttribute->Skill[iIndex];
        if (iIndex >= AT_PET_COMMAND_DEFAULT)
            bySkillTypeForGate = (WORD)iIndex;

        if (bySkillTypeForGate == AT_SKILL_INFINITY_ARROW || bySkillTypeForGate == AT_SKILL_INFINITY_ARROW_STR
            || bySkillTypeForGate == AT_SKILL_EXPANSION_OF_WIZARDRY || bySkillTypeForGate == AT_SKILL_EXPANSION_OF_WIZARDRY_STR
            || bySkillTypeForGate == AT_SKILL_EXPANSION_OF_WIZARDRY_MASTERY)
            return 0.f;

        // From here down: RenderSkillDelay()'s own original body, draw call replaced with a
        // fraction return.
        int iSkillDelay = CharacterAttribute->SkillDelay[iIndex];
        if (iSkillDelay <= 0)
            return 0.f;

        int iSkillType = CharacterAttribute->Skill[iIndex];

        if (iSkillType == AT_SKILL_PLASMA_STORM_FENRIR && !CheckAttack())
            return 0.f;

        int iSkillMaxDelay = SkillAttribute[iSkillType].Delay;
        if (iSkillMaxDelay == 0)
            return 0.f; // avoid a divide-by-zero the original's own float division would also hit

        return iSkillDelay / (float)iSkillMaxDelay;
    }
}

namespace
{
Rml::String HotKeyText(int hotkey)
{
    return hotkey >= 0 ? std::to_string(hotkey) : Rml::String();
}
} // namespace

void mu::ui::window::CSkillList::RebuildGridSnapshot()
{
    // Same iteration/filter/zig-zag-position math the legacy grid loop used, now producing data
    // for main_frame.rml's .skill-cell list instead of drawing.
    m_GridSnapshot.clear();
    m_PetSnapshot.clear();

    if (CharacterAttribute->SkillNumber == 0)
        return;

    // The original's zig-zag around its first cell at 385/390, as offsets from that cell: the
    // theme places #skill_list where the first cell goes.
    float x = 0.f, y = 0.f;
    constexpr float width = 32.f, height = 38.f;
    const float fOrigX = 0.f;
    int iSkillCount = 0;

    for (int i = 0; i < MAX_MAGIC; ++i)
    {
        int iSkillType = CharacterAttribute->Skill[i];

        if (iSkillType == 0 || (iSkillType >= AT_SKILL_STUN && iSkillType <= AT_SKILL_REMOVAL_BUFF))
            continue;

        BYTE bySkillUseType = SkillAttribute[iSkillType].SkillUseType;
        if (bySkillUseType == SKILL_USE_TYPE_MASTER || bySkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
            continue;

        if (iSkillCount == 18)
        {
            y -= height;
        }

        if (iSkillCount < 14)
        {
            int iRemainder = iSkillCount % 2;
            int iQuotient = iSkillCount / 2;
            x = (iRemainder == 0) ? (fOrigX + iQuotient * width) : (fOrigX - (iQuotient + 1) * width);
        }
        else if (iSkillCount < 18)
        {
            x = fOrigX - (8 * width) - ((iSkillCount - 14) * width);
        }
        else
        {
            x = fOrigX - (12 * width) + ((iSkillCount - 17) * width);
        }

        iSkillCount++;

        SkillCellEntry entry;
        entry.left = x;
        entry.top = y;
        entry.skillIndex = i;
        entry.isPet = false;
        entry.isCurrent = (i == Hero->CurrentSkill);
        entry.cooldownFraction = ComputeSkillCooldownFraction(i);
        entry.icon = GetSkillIconDecorator(i);
        entry.hotkey = HotKeyText(GetSkillHotKeyNumber(i));
        m_GridSnapshot.push_back(entry);
    }

    if (Hero->m_pPet != NULL)
    {
        // The four commands are always all four, so the theme puts each on its own cell; only the
        // zig-zag grid above needs a computed position.
        for (int i = AT_PET_COMMAND_DEFAULT; i < AT_PET_COMMAND_END; ++i)
        {
            SkillCellEntry entry;
            entry.skillIndex = i;
            entry.isPet = true;
            entry.isCurrent = (i == Hero->CurrentSkill);
            entry.cooldownFraction = ComputeSkillCooldownFraction(i);
            entry.icon = GetSkillIconDecorator(i);
            entry.hotkey = HotKeyText(GetSkillHotKeyNumber(i));
            m_PetSnapshot.push_back(entry);
        }
    }
}

// Native CNewUISkillList::RenderSkillInfo() centres the skill tooltip at box x + 10 for the current
// skill and hotkeys, cell x + 15 in the expanded list, anchored 10 above the box.
namespace
{
constexpr float kSlotTooltipOffsetX = 10.f;
constexpr float kGridTooltipOffsetX = 15.f;
constexpr float kTooltipGapAbove = 10.f;
// The original hit-tested the current skill as a 32x38 box around its 20x28 icon (385/431 around
// 392/437) and anchored the hint on that box like a hotkey slot.
constexpr float kCurrentSkillBoxInsetX = 7.f;
constexpr float kCurrentSkillBoxInsetY = 6.f;
} // namespace

void mu::ui::window::CSkillList::QueueTooltip(int iSkillIndex, float x, float y)
{
    m_bTooltipPending = true;
    m_iTooltipSkillIndex = iSkillIndex;
    m_fTooltipAnchorX = x;
    m_fTooltipAnchorY = y;
}

float mu::ui::window::CSkillList::GetHotKeySlotCooldownFraction(int iSlotIndex)
{
    if (iSlotIndex < 0 || iSlotIndex >= 5)
        return 0.f;
    if (CharacterAttribute->SkillNumber == 0)
        return 0.f;

    int iStartSkillIndex = m_bHotKeySkillListUp ? 6 : 1;
    int iIndex = iStartSkillIndex + iSlotIndex;
    if (iIndex == 10)
        iIndex = 0;

    if (m_iHotKeySkillType[iIndex] == -1)
        return 0.f;

    if (m_iHotKeySkillType[iIndex] >= AT_PET_COMMAND_DEFAULT && m_iHotKeySkillType[iIndex] < AT_PET_COMMAND_END)
    {
        if (Hero->m_pPet == NULL)
            return 0.f;
    }

    return ComputeSkillCooldownFraction(m_iHotKeySkillType[iIndex]);
}

float mu::ui::window::CSkillList::GetCurrentSkillCooldownFraction()
{
    return ComputeSkillCooldownFraction(Hero->CurrentSkill);
}

// Click/hover entry points bound from main_frame.rml's data-event-click/mouseover/mouseout (see Create(), CMainFrameWindow.cpp).
void mu::ui::window::CSkillList::OnHotkeySlotClick(int iSlotIndex)
{
    if (iSlotIndex < 0 || iSlotIndex >= 5)
        return;
    if (CharacterAttribute->SkillNumber == 0)
        return;

    int iStartSkillIndex = m_bHotKeySkillListUp ? 6 : 1;
    int iIndex = iStartSkillIndex + iSlotIndex;
    if (iIndex == 10)
        iIndex = 0;

    if (m_iHotKeySkillType[iIndex] == -1)
        return;

    // Mirrors the legacy mouse-click branch, deliberately NOT UseHotKey() -- the original mouse
    // click never went through it either (only the keyboard 0-9 press does), so its pet-check/
    // auto-attack-cancel rule doesn't apply here.
    WORD bySkillType = CharacterAttribute->Skill[m_iHotKeySkillType[iIndex]];
    if (bySkillType == 0 || (bySkillType >= AT_SKILL_STUN && bySkillType <= AT_SKILL_REMOVAL_BUFF))
        return;
    if (SkillAttribute[bySkillType].SkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
        return;

    m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
    Hero->CurrentSkill = m_iHotKeySkillType[iIndex];
    PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::CSkillList::OnHotkeySlotHover(int iSlotIndex, float slotLeft, float slotTop)
{
    if (iSlotIndex < 0 || iSlotIndex >= 5)
        return;
    if (CharacterAttribute->SkillNumber == 0)
        return;

    int iStartSkillIndex = m_bHotKeySkillListUp ? 6 : 1;
    int iIndex = iStartSkillIndex + iSlotIndex;
    if (iIndex == 10)
        iIndex = 0;

    if (m_iHotKeySkillType[iIndex] == -1)
        return;

    WORD bySkillType = CharacterAttribute->Skill[m_iHotKeySkillType[iIndex]];
    if (bySkillType == 0 || (bySkillType >= AT_SKILL_STUN && bySkillType <= AT_SKILL_REMOVAL_BUFF))
        return;
    if (SkillAttribute[bySkillType].SkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
        return;

    QueueTooltip(m_iHotKeySkillType[iIndex], slotLeft + kSlotTooltipOffsetX, slotTop - kTooltipGapAbove);
}

void mu::ui::window::CSkillList::OnCurrentSkillClick()
{
    m_bSkillList = !m_bSkillList;
    PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::CSkillList::OnCurrentSkillHover(float iconLeft, float iconTop)
{
    QueueTooltip(Hero->CurrentSkill, iconLeft - kCurrentSkillBoxInsetX + kSlotTooltipOffsetX,
                 iconTop - kCurrentSkillBoxInsetY - kTooltipGapAbove);
}

void mu::ui::window::CSkillList::OnGridCellClick(int iSkillIndex)
{
    m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
    Hero->CurrentSkill = iSkillIndex;
    m_bSkillList = false;
    PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::CSkillList::OnGridCellHover(int iSkillIndex, float cellLeft, float cellTop)
{
    m_iHoveredGridSkillIndex = iSkillIndex;
    QueueTooltip(iSkillIndex, cellLeft + kGridTooltipOffsetX, cellTop - kTooltipGapAbove);
}

void mu::ui::window::CSkillList::OnPetCellClick(int iSkillIndex)
{
    m_wHeroPriorSkill = CharacterAttribute->Skill[Hero->CurrentSkill];
    Hero->CurrentSkill = iSkillIndex;
    m_bSkillList = false;
    PlayBuffer(SOUND_CLICK01);
}

void mu::ui::window::CSkillList::OnPetCellHover(int iSkillIndex, float cellLeft, float cellTop)
{
    // Pet-row entries arm Ctrl+digit assignment the same way grid entries do (legacy behavior, preserved).
    m_iHoveredGridSkillIndex = iSkillIndex;
    QueueTooltip(iSkillIndex, cellLeft + kGridTooltipOffsetX, cellTop - kTooltipGapAbove);
}

void mu::ui::window::CSkillList::OnUnhover()
{
    m_bTooltipPending = false;
    m_iTooltipSkillIndex = -1;
    m_iHoveredGridSkillIndex = -1;
}

bool mu::ui::window::CSkillList::IsSkillListUp()
{
    return m_bHotKeySkillListUp;
}

void mu::ui::window::CSkillList::ResetMouseLButton()
{
    MouseLButton = false;
    MouseLButtonPop = false;
    MouseLButtonPush = false;
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
