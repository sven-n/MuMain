#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "Network/MoveCommandData.h"
#include "UI/HUD/MoveCommandRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "UI/Scaling/UITransform.h"

namespace Rml
{
    class ElementDocument;
    class Event;
}

namespace UI::MoveCommand
{
    // The panel's width when no theme slot sizes it; the theme lays out the rest.
    constexpr int kWindowWidth = 230;
}

namespace mu::ui::window
{
    class CMoveCommandWindow : public CObject
    {
    private:

        //$$AUTO_BUILD_LINE_ SHUFFLE_BEGIN
        CManager* m_pNewUIMng;
        std::list<SEASON3B::CMoveCommandData::MOVEINFODATA*>	m_listMoveInfoData;
        DWORD						m_dwMoveCommandKey;

        void BindRmlModel(Rml::DataModelConstructor& c, MoveCommandRmlModel& model);
        void OnRmlBuilt();
        UI::RmlBridge::ThemedView<MoveCommandRmlModel> m_RmlView{"move_command",
            [this](Rml::DataModelConstructor& c, MoveCommandRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/move_command.rml"}}, {.afterBuild = [this] { OnRmlBuilt(); }}};
        // One-shot: rewind the list to the top on the first frame after the window opens, once
        // the document is visible and RmlUi has laid it out. Re-asserting a scroll position every
        // frame would fight the player's own drag and wheel.
        bool						m_bRewindPending = false;
        float						m_FillWidth = 0.f;
        float						m_FillHeight = 0.f;

    public:
        CMoveCommandWindow();
        virtual ~CMoveCommandWindow();

        bool Create(CManager* pNewUIMng);
        void Release();

        bool SupportsFillPlacement() const override { return true; }
        void SetFillPlacementSize(float width, float height) override;
        bool GetFillMinimumSize(float& width, float& height) const override;

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        Rml::ElementDocument* GetPlacedDocument() const override { return m_RmlView.Document(); }


        virtual void OpenningProcess();
        void ClosingProcess();
        float GetLayerDepth();

        bool IsLuckySealBuff();
        bool IsMapMove(const std::wstring& src);

        void SetMoveCommandKey(DWORD dwKey);
        DWORD GetMoveCommandKey();

        BOOL IsTheMapInDifferentServer(const int iFromMapIndex, const int iToMapIndex) const;
        int GetMapIndexFromMovereq(const wchar_t* pszMapName);
        // Whether the character currently meets a warp list entry's own
        // requirements - level, zen, the wings for Icarus, the Uniria rule
        // for Atlans. The list's requirements are recomputed first, exactly
        // as the click path does before reading the flag.
        bool CanMoveToMap(const wchar_t* pszMapName);

    private:
        // The warp list entry a name belongs to, main or sub name, case
        // insensitively - the one lookup behind both public queries.
        SEASON3B::CMoveCommandData::MOVEINFODATA* FindMoveInfo(const wchar_t* pszMapName);
        void SetStrifeMap();
        void SettingCanMoveMap();
        void RefreshData();
        // Gives #panel a fill slot's size.
        void ApplyFillSize();

        void BuildRmlUi();
        void SyncRmlModel();
        void RebuildRowModel();
        // Warps to the list entry at `row` (a position in m_listMoveInfoData, as pushed into
        // MoveCommandRowEntry::index), if the character still meets its requirements.
        void RmlClickWarp(int row);
        // Scrolls the list one row per wheel notch, as the original did, instead of RmlUi's own
        // 80 dp step.
        void RmlWheelList(Rml::Event& event);
    };
};
