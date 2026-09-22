//*****************************************************************************
// File: NewUINPCQuest.h
//*****************************************************************************

#if !defined(AFX_NEWUINPCQUEST_H__7767F9B8_2F3F_4A7F_8C07_CD747D76A6D3__INCLUDED_)
#define AFX_NEWUINPCQUEST_H__7767F9B8_2F3F_4A7F_8C07_CD747D76A6D3__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Core/Window3DRenderMng.h"
#include "UI/Quests/NPCQuestRmlModel.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml { class ElementDocument; }

// RmlUi-based (npc_quest.rml/.rcss) except the live quest-condition item preview, which stays a
// native per-frame 3D draw (Render3D()/RenderItem3D()) -- genuine 3D content, the same class of
// permanent hybrid boundary CQuestProgress's own m_pSelectedRewardItem/::RenderItemInfo() already
// established, just for multiple always-visible rows instead of one on-hover popup.
//
// Split into two RmlUi documents, unlike every 2D-only sibling this session: the frame chrome
// (npc_quest_bg.rml, m_pRmlBgDoc) renders via RmlUiRuntime::GetBackgroundContext() so it paints
// BEFORE the native 3D preview each frame, while the actual content (npc_quest.rml, m_pRmlDoc) stays
// on the main context, which always renders last (on top of the 3D preview, as intended). Same
// mechanism CNPCShop/CMyInventory already use for their own live-3D icon grids.
namespace mu::ui::window
{
    class CNPCQuest : public CObject, public I3DRenderObj
    {
    private:
        enum
        {
            NPCQUEST_WIDTH = 190,
            NPCQUEST_HEIGHT = 429,
        };

        CManager* m_pNewUIMng;
        C3DRenderMng* m_pNewUI3DRenderMng;
        POINT					m_Pos;

        // Mirrors the native Lock()/UnLock() state RenderItemMobText()'s return value drove every
        // frame -- computed fresh in SyncRmlModel(), read by RmlClickComplete() the same way
        // ProcessBtns() used to read the button's own current Lock() state.
        bool m_bCompleteEnabled = false;

        RmlModelBinder<NPCQuestRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

        // The frame chrome must render behind the live-3D quest-item preview, but RmlUi's main
        // context always renders last -- so it goes through
        // RmlUiRuntime::GetBackgroundContext()/RenderBackgroundLayer() instead, same mechanism
        // CNPCShop's own NPCShopBgRmlModel already established. Root-transform passthrough only --
        // all the actual chrome is static markup/CSS in npc_quest_bg.rml.
        struct NPCQuestBgRmlModel
        {
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
        };
        RmlModelBinder<NPCQuestBgRmlModel> m_BgRmlBinder;
        Rml::ElementDocument* m_pRmlBgDoc = nullptr;

    public:
        CNPCQuest();
        virtual ~CNPCQuest();

        bool Create(CManager* pNewUIMng, C3DRenderMng* pNewUI3DRenderMng, int x, int y);
        void Release();

        void SetPos(int x, int y);
        void Show(bool bShow) override;

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        void Render3D();

        bool IsVisible() const;

        float GetLayerDepth();	//. 3.1f

        void ProcessOpening();
        bool ProcessClosing();

        void ReloadRmlTheme();

        // Invoked directly from RmlUi data-event-click bindings (see BuildRmlUi()), not polled.
        void RmlClickClose();
        void RmlClickAnswer(int nAnswerIndex);
        void RmlClickComplete();

    private:
        void BuildRmlUi();
        void SyncRmlModel();

        // Condition-evaluation half of the old RenderItemMobText() -- keeps
        // FindQuestItemsInInven()/GetKillMobCount() comparison logic (and its exact color/completion
        // mapping) byte-for-byte, just populates rows instead of drawing them. Returns the same
        // completion bool the native code used to gate the Complete button's Lock()/color state.
        bool BuildConditionRows(std::vector<NPCQuestConditionRow>& outRows);

        void RenderItem3D();
    };
}

#endif // !defined(AFX_NEWUINPCQUEST_H__7767F9B8_2F3F_4A7F_8C07_CD747D76A6D3__INCLUDED_)
