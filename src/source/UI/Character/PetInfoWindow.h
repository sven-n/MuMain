
#if !defined(AFX_NEWUIPETINFORMATION_H__A1B6EB0C_4F9F_4285_B89A_7BB680BF84D6__INCLUDED_)
#define AFX_NEWUIPETINFORMATION_H__A1B6EB0C_4F9F_4285_B89A_7BB680BF84D6__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    // Fully RmlUi-based (#panel, pet_info.rml/.rcss) -- no live 3D content, same shape as the
    // already-ported CCharacterInfoWindow it docks alongside.
    class CPetInfoWindow : public CObject
    {
    private:
        enum
        {
            PETINFOWINDOW_WIDTH = 190,
            PETINFOWINDOW_HEIGHT = 429,
        };

        enum CUR_OPEN_TAB_BUTTON
        {
            TAB_TYPE_DARKHORSE = 0,
            TAB_TYPE_DARKSPIRIT
        };

    public:
        CPetInfoWindow();
        virtual ~CPetInfoWindow();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);
        void Show(bool bShow) override;

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 2.3f

        void OpenningProcess();
        void ClosingProcess();

        // Invoked directly from RmlUi data-event-click bindings (see Create()), not polled.
        void RmlClickSelectTab(int tab);
        void RmlClickExit();

        void ReloadRmlTheme();

    private:
        void BuildRmlUi();
        void SyncRmlModel();
        void CalcDamage(int iNumTapButton);

        struct PetInfoRmlModel
        {
            // Movable window (SetPos(), collision-shuffled by PanelColumnX), not HUD-anchored --
            // sourced from UI::Scaling::GetActiveTransform(), same convention as character_info.
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
            float textPx = 0.f; // native text size in physical px (RmlRootTransform.h)

            int activeTab = TAB_TYPE_DARKHORSE;
            Rml::String windowTitle;
            Rml::String tabDarkHorseLabel, tabDarkSpiritLabel;
            Rml::String exitTooltip;

            // Dark Horse tab
            bool dhHasPet = false;
            Rml::String dhNoPetText;
            Rml::String dhLevelText, dhLifeText, dhExpText, dhDmgText, dhAtkSpeedText;
            float dhHpPercent = 0.f;

            // Dark Spirit tab
            bool dsHasPet = false;
            Rml::String dsNoPetText;
            Rml::String dsLevelText, dsLifeText, dsExpText, dsDmgText, dsAtkSpeedText, dsCharismaText;
            float dsHpPercent = 0.f;

            // Set once at Create(); static I18N labels, never re-synced (this window has no
            // language switcher of its own).
            Rml::String commandsLabel;
            Rml::String skillBasicActionLabel, skillRandomAttackLabel, skillAttackWithOwnerLabel, skillAttackTargetLabel;
        };
        RmlModelBinder<PetInfoRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

    private:
        CManager* m_pNewUIMng;
        POINT m_Pos;

        int m_aiDamage[2];
        float m_fAddDamagePercent;
    };
}

#endif // !defined(AFX_NEWUIPETINFORMATION_H__A1B6EB0C_4F9F_4285_B89A_7BB680BF84D6__INCLUDED_)
