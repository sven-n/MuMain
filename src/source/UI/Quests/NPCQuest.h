//*****************************************************************************
// File: NewUINPCQuest.h
//*****************************************************************************

#if !defined(AFX_NEWUINPCQUEST_H__7767F9B8_2F3F_4A7F_8C07_CD747D76A6D3__INCLUDED_)
#define AFX_NEWUINPCQUEST_H__7767F9B8_2F3F_4A7F_8C07_CD747D76A6D3__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Inventory/ItemCameraTarget.h"
#include "UI/Quests/NPCQuestRmlModel.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlThemedView.h"

namespace Rml { class ElementDocument; }

// RmlUi-based (npc_quest.rml/.rcss); C++ draws the live quest-condition items (RenderItem3D())
// into the document's #nq_item, beside their rows.
namespace mu::ui::window
{
    class CNPCQuest : public CObject
    {
    private:
        enum
        {
            NPCQUEST_WIDTH = 190,
            NPCQUEST_HEIGHT = 429,
        };

        CManager* m_pNewUIMng;
        POINT					m_Pos;

        // Mirrors the native Lock()/UnLock() state RenderItemMobText()'s return value drove every
        // frame -- computed fresh in SyncRmlModel(), read by RmlClickComplete() the same way
        // ProcessBtns() used to read the button's own current Lock() state.
        bool m_bCompleteEnabled = false;

        void BindRmlModel(Rml::DataModelConstructor& c, NPCQuestRmlModel& model);
        UI::RmlBridge::ThemedView<NPCQuestRmlModel> m_RmlView{"npc_quest",
            [this](Rml::DataModelConstructor& c, NPCQuestRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/npc_quest.rml"}}};

    public:
        CNPCQuest();
        virtual ~CNPCQuest();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);
        void Show(bool bShow) override;

        bool UpdateMouseEvent();
        Rml::ElementDocument* GetPlacedDocument() const override { return m_RmlView.Document(); }
        const char* PlacedRootId() const override { return "content_root"; }
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        bool IsVisible() const;

        float GetLayerDepth();	//. 3.1f

        void ProcessOpening();
        bool ProcessClosing();


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
        // Last, so it is destroyed first.
        UI::Items::ItemCameraTarget m_ItemTarget{[this](const Rml::Vector2f&, const Rml::Vector2f&) { RenderItem3D(); },
                                                 this};
    };
}

#endif // !defined(AFX_NEWUINPCQUEST_H__7767F9B8_2F3F_4A7F_8C07_CD747D76A6D3__INCLUDED_)
