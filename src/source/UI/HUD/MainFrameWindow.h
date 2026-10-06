
#if !defined(AFX_NEWUIMAINFRAMEWINDOW_H__46A029CA_44A5_4050_9216_FA8A25EC4629__INCLUDED_)
#define AFX_NEWUIMAINFRAMEWINDOW_H__46A029CA_44A5_4050_9216_FA8A25EC4629__INCLUDED_

#pragma once

#include <vector>

#include "UI/Core/WindowObject.h"
#include "Render/Textures/ZzzTexture.h"
#include "UI/Core/Window3DRenderMng.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "UI/HUD/ItemHotKey.h"
#include "UI/HUD/SkillList.h"

#include <memory>

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    enum
    {
#ifdef PBG_ADD_INGAMESHOP_UI_MAINFRAME
        MAINFRAME_BTN_PARTCHARGE = 0,
#endif //defined PBG_ADD_INGAMESHOP_UI_MAINFRAME
        MAINFRAME_BTN_CHAINFO,
        MAINFRAME_BTN_MYINVEN,
        MAINFRAME_BTN_FRIEND,
        MAINFRAME_BTN_WINDOW,
    };

    // The frame chrome, HP/MP/AG/SD/EXP bars and corner buttons, together with CSkillList (hotkey
    // row, grid, pet commands) and CItemHotKey (Q/W/E/R slots), all drawn by main_frame.rml.
    //
    // UpdateMouseEvent() always reports "not consumed": RmlUi hit-tests the HUD. UpdateKeyEvent()
    // still handles the Q/W/E/R item hotkeys.
    //
    // Known simplifications: no "gained EXP" flash overlay; readouts are plain text instead of the
    // digit-sprite atlas.
    class CMainFrameWindow : public CObject
    {
    public:
        CMainFrameWindow();
        virtual ~CMainFrameWindow();

        bool Create(CManager* pNewUIMng);
        void Release();

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        bool IsVisible() const;

        // Whether the cursor is on a part of the HUD that takes the pointer, wherever the theme
        // placed it -- so world clicks there don't go through.
        bool IsMouseOverHud() const;


        float GetLayerDepth();		// 10.2f
        float GetKeyEventOrder();	// 7.f

        void SetItemHotKey(int iHotKey, int iItemType, int iItemLevel);
        int GetItemHotKey(int iHotKey);
        int GetItemHotKeyLevel(int iHotKey);
        //void RenderHotKeyItems();
        void UpdateItemHotKey();

        void ResetSkillHotKey();
        void SetSkillHotKey(int iHotKey, int iSkillType);
        int GetSkillHotKey(int iHotKey);
        int GetSkillHotKeyIndex(int iSkillType);

        void SetPreExp_Wide(__int64 dwPreExp);
        void SetGetExp_Wide(__int64 dwGetExp);

        void SetPreExp(__int64 dwPreExp);
        void SetGetExp(__int64 dwGetExp);

        // Called externally when a button's target panel opens/closes, to sync its bound "open" model boolean.
        void SetBtnState(int iBtnType, bool bStateDown);

        // Set by the RmlUi document's data-event-click bindings; polled and cleared like other windows' RmlClickX().
        void RmlClickCShop() { m_bRmlCShopClicked = true; }
        void RmlClickChaInfo() { m_bRmlChaInfoClicked = true; }
        void RmlClickMyInven() { m_bRmlMyInvenClicked = true; }
        void RmlClickFriend() { m_bRmlFriendClicked = true; }
        void RmlClickWindow() { m_bRmlWindowClicked = true; }

    private:
        void SyncRmlModel();
        void BuildRmlUi();

    public:
        __int64	m_loPreExp;
        __int64	m_loGetExp;

    private:
        CManager* m_pNewUIMng;

        CItemHotKey m_ItemHotKey;

        bool m_bExpEffect;
        DWORD m_dwExpEffectTime;

        __int64 m_dwPreExp;
        __int64 m_dwGetExp;

        bool m_bButtonBlink;

        struct MainFrameRmlModel
        {
            // The original's gauge and button hint text size in real pixels: RenderTipText() under
            // the HUD's scale, which grows about half as fast as the bars. The HUD itself is sized
            // in dp by main_frame.rcss.
            float hintPx = 0.f;

            float hpFraction = 0.f, mpFraction = 0.f, agFraction = 0.f, sdFraction = 0.f;
            Rml::String hpText, mpText, agText, sdText;

            // Current-value-only readout ("935", not "935 / 935") -- legacy theme's gauge numbers
            // bind this instead of hpText/etc; both computed unconditionally, themes just
            // reference different fields.
            Rml::String hpCurrentText, mpCurrentText, agCurrentText, sdCurrentText;
            Rml::String hpTooltip, mpTooltip, agTooltip, sdTooltip;
            bool poisoned = false; // true -> HP fill swaps red to green (eDeBuff_Poison)

            // expFraction is progress within the current 10%-of-level decile (0..1), not overall
            // level progress; expDigit (0-9) is that decile number. Position comes from the shared
            // bars transform group, spanning x=0 to x=640 (not the full window width the legacy
            // band used).
            float expFraction = 0.f;
            Rml::String expDigit;
            Rml::String expTooltip;

            // Corner buttons: "Open" mirrors SetBtnState()'s bStateDown; tooltips are static strings pushed once.
            bool cShopOpen = false, chaInfoOpen = false, myInvenOpen = false, friendOpen = false, windowOpen = false;
            Rml::String cShopTooltip, chaInfoTooltip, myInvenTooltip, friendTooltip, windowTooltip;

            // Alert-blink dots: bound boolean + CSS dot, computed each frame from the same state
            // the legacy blink read.
            bool chaInfoAlert = false, friendAlert = false;

            // Which of #skill_slot_0..4 shows the active skill (see
            // CSkillList::IsHotKeySlotCurrentSkill()). 5 named fields, not an array -- RmlUi's
            // DataModelConstructor binds pointer-to-member scalars only.
            bool skillSlot0Selected = false, skillSlot1Selected = false, skillSlot2Selected = false,
                 skillSlot3Selected = false, skillSlot4Selected = false;

            // Hotkey number label per slot (see CSkillList::GetHotKeySlotNumber()); empty string
            // for an unbound slot renders nothing.
            Rml::String skillSlot0Hotkey, skillSlot1Hotkey, skillSlot2Hotkey, skillSlot3Hotkey, skillSlot4Hotkey;

            // Icon decorator per hotkey slot and for the current-skill slot (see
            // CSkillList::GetSkillIconDecorator()), and the current skill's hotkey number.
            Rml::String skillSlot0Icon = "none", skillSlot1Icon = "none", skillSlot2Icon = "none",
                        skillSlot3Icon = "none", skillSlot4Icon = "none";
            Rml::String currentSkillIcon = "none";
            Rml::String currentSkillHotkey;

            // Cooldown wipe for the compact row + current-skill slot; same per-field convention as skillSlot0..4Hotkey.
            float skillSlot0Cooldown = 0.f, skillSlot1Cooldown = 0.f, skillSlot2Cooldown = 0.f,
                  skillSlot3Cooldown = 0.f, skillSlot4Cooldown = 0.f;
            float currentSkillCooldown = 0.f;

            // Mirrors CSkillList::IsSkillGridOpen() (NOT the similarly-named IsSkillListUp() --
            // see that method's comment). Gates #skill_grid/#pet_skill_row visibility; cell arrays
            // are rebuilt each frame while open, left stale (harmless, hidden) while closed.
            bool skillGridOpen = false;
            // CSkillList::IsSkillListUp(): the compact row shows its upper set, which the theme
            // highlights (#skill_list_highlight).
            bool skillListUp = false;
            std::vector<SkillCellEntry> skillGridCells;
            std::vector<SkillCellEntry> petSkillCells;

            // Item hotkey chrome (#item_slots): hover border + stack-count for the 4 Q/W/E/R
            // slots. The 3D-rendered potion icon itself is untouched (permanent native content).
            bool itemSlot0Hovered = false, itemSlot1Hovered = false, itemSlot2Hovered = false, itemSlot3Hovered = false;
            Rml::String itemSlot0Count, itemSlot1Count, itemSlot2Count, itemSlot3Count;
        };
        void BindRmlModel(Rml::DataModelConstructor& c, MainFrameRmlModel& model);
        void OnRmlReloaded();
        // The HUD, and its top-right button row on the same model: a document of its own so it
        // stacks under the windows docked over that corner while the bars stay over them
        // (RmlStackingOrder.cpp).
        UI::RmlBridge::ThemedView<MainFrameRmlModel> m_RmlView{"main_frame",
            [this](Rml::DataModelConstructor& c, MainFrameRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/main_frame.rml"}, {"Data/Interface/RmlUi/main_frame_top.rml"}},
            {.afterReload = [this] { OnRmlReloaded(); }}};

        bool m_bRmlCShopClicked = false;
        bool m_bRmlChaInfoClicked = false;
        bool m_bRmlMyInvenClicked = false;
        bool m_bRmlFriendClicked = false;
        bool m_bRmlWindowClicked = false;

    public:
        // Gates the RmlUi doc's Show()/Hide() on IsVisible() && sceneAllowsShow, same as CMuHelperBar/CBuffStrip.
        void SyncDocVisibility(bool sceneAllowsShow);
    };
}

#endif // !defined(AFX_NEWUIMAINFRAMEWINDOW_H__46A029CA_44A5_4050_9216_FA8A25EC4629__INCLUDED_)
