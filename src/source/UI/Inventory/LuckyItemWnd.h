
#if !defined(AFX_NEWUILUCKYITEMWND_H__F57DF84F_0A44_444A_838D_78CBC35544EB__INCLUDED_)
#define AFX_NEWUILUCKYITEMWND_H__F57DF84F_0A44_444A_838D_78CBC35544EB__INCLUDED_

#pragma once

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Core/WindowObject.h"
#include "UI/Inventory/InventoryCtrl.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Inventory/MyInventory.h"
#include "UI/RmlBridge/RmlThemedView.h"

#include <vector>

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
#define	LUCKYITEMMAXLINE	20
    enum eLUCKYITEMTYPE { eLuckyItemType_None = 0, eLuckyItemType_Trade, eLuckyItemType_Refinery, eLuckyItemAct_End };
    enum eLUCKYITEM { eLuckyItem_None = 0, eLuckyItem_Move, eLuckyItem_Act, eLuckyITem_Result, eLuckyItem_End };

    struct sText
    {
        int		s_nTextIndex;	// 글로벌 텍스트 인덱스
        DWORD	s_dwColor;		// 텍스트 색깔
        int		s_nLine;		// 텍스트 정렬
    };

    class CLuckyItemWnd : public CObject
    {
    private:
        CManager* m_pNewUIMng;
        CInventoryCtrl* m_pNewInventoryCtrl;
        float					m_fInvenClr[3];
        float					m_fInvenClrWarning[3];
        wchar_t			m_szSubject[255];
        sText					m_sText[LUCKYITEMMAXLINE];
        int						m_nTextMaxLine;
        POINT					m_ptPos;
        int						m_nResult;
        int						m_nMixEffectTimer;
        eLUCKYITEMTYPE			m_eType;
        eLUCKYITEM				m_eWndAction;
        eLUCKYITEM				m_eEnd;

        // Window frame/title/mix-button/result-description text block are RmlUi; only the inventory
        // grid and the mix-completion sparkle effect stay native -- grid icons are live 3D renders
        // (same reasoning as CStorageInventoryExt), and the sparkle effect is a native 2D particle
        // overlay with no RmlUi equivalent. Both panel sizes belong to the active theme's RCSS.
        struct LuckyLine
        {
            Rml::String text;
            Rml::String color; // "rgba(r,g,b,a)"
            Rml::String align; // "left" or "center" -- former RT3_SORT_LEFT/RT3_SORT_CENTER
            bool operator==(const LuckyLine&) const = default;
        };
        struct LuckyItemRmlModel
        {
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
            float textPx = 0.f; // native text size in physical px (RmlRootTransform.h)
            float panelWidth = 190.f;
            Rml::String title;
            Rml::String mixTooltip;
            bool mixVisible = true;

            // Former Render_Frame()'s m_sText[]/AddText() loop -- one entry per slot in
            // [0, m_nTextMaxLine), including blank spacer slots (empty text), so line spacing
            // matches the original's fixed per-slot vertical rhythm without duplicating its pixel
            // math (see SyncMixLines() equivalent in SyncRmlModel(), LuckyItemWnd.cpp).
            std::vector<LuckyLine> textLines;
        };
        void BindRmlModel(Rml::DataModelConstructor& c, LuckyItemRmlModel& model);
        UI::RmlBridge::ThemedView<LuckyItemRmlModel> m_RmlView{"lucky_item",
            [this](Rml::DataModelConstructor& c, LuckyItemRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/lucky_item.rml"}}};

        // The frame background panel must render behind the grid's live 3D icons, but RmlUi's
        // main context always renders last -- so it goes through
        // RmlUiRuntime::GetBackgroundContext()/RenderBackgroundLayer() instead (see
        // CStorageInventoryExt's identical StorageExtBgRmlModel for the full mechanism).
        struct LuckyItemBgRmlModel
        {
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
        };
        static void BindRmlBgModel(Rml::DataModelConstructor& c, LuckyItemBgRmlModel& model);
        UI::RmlBridge::ThemedView<LuckyItemBgRmlModel> m_RmlBgView{"lucky_item_bg", BindRmlBgModel,
            {{"Data/Interface/RmlUi/lucky_item_bg.rml", [] { return RmlUiRuntime::Instance().GetBackgroundContext(); }}}};

        void BuildRmlUi();
        void SyncRmlModel();

    private:
        void	SetFrame_Text(eLUCKYITEM _eType);
        bool	Process_InventoryCtrl(void);

        int		GetLuckyItemRate(int _nType);
        void	Render_Frame(void);
        void	RenderMixEffect(void);
        void	Reset(void);
        void	AddText(int _nGlobalTextIndex, DWORD _dwColor = 0xFFFFFFFF, int _bLine = RT3_SORT_CENTER);

        bool	Check_LuckyItem_Trade(ITEM* _pItem);
        bool	Check_LuckyItem_Refinery(ITEM* _pItem);

        static void UI2DEffectCallback(LPVOID pClass, DWORD dwParamA, DWORD dwParamB);
    public:
        CInventoryCtrl* GetInventoryCtrl() const;

        int		SetActAction();
        STORAGE_TYPE SetMoveAction();
        void	GetResult(BYTE _byResult, int _nIndex, std::span<const BYTE> pbyItemPacket);
        bool	Process_BTN_Action(void);
        bool	Process_InventoryCtrl_InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket);
        void	Process_InventoryCtrl_DeleteItem(int iIndex);
        bool	Check_LuckyItem(ITEM* _pItem);
        bool	Check_LuckyItem_InWnd(void);

        // Virtual overrides
        bool	Create(CManager* pNewUIMng, int x, int y);
        void	Release(void);
        void	OpeningProcess(void);
        bool	ClosingProcess(void);
        bool	UpdateMouseEvent();
        bool	UpdateKeyEvent();
        bool	Update();
        bool	Render();
        float	GetLayerDepth();	//. 3.4f

        __inline void	SetAct(eLUCKYITEMTYPE _eAct) { m_eType = _eAct; }
        __inline void	SetPos(int _nX, int _nY) { m_ptPos.x = _nX, m_ptPos.y = _nY; }

        __inline eLUCKYITEMTYPE	GetAct(void) { return m_eType; }
        CLuckyItemWnd();
        virtual ~CLuckyItemWnd();
    };
}
#endif // !defined(AFX_NEWUILUCKYITEMWND_H__F57DF84F_0A44_444A_838D_78CBC35544EB__INCLUDED_)
