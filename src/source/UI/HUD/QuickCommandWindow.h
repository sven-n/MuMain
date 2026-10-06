
#if !defined(AFX_NEWUIQUICKCOMMANDWINDOW_H__3A1D6614_8C41_4066_A831_2954B3C461D5__INCLUDED_)
#define AFX_NEWUIQUICKCOMMANDWINDOW_H__3A1D6614_8C41_4066_A831_2954B3C461D5__INCLUDED_

#pragma once

#include "UI/Core/WindowManager.h"
#include "UI/HUD/QuickCommandRmlModel.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzCharacter.h"

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
    class CQuickCommandWindow : public CObject
    {
    public:
        CQuickCommandWindow();
        virtual ~CQuickCommandWindow();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        float GetLayerDepth();	//. 2.0f
        float GetKeyEventOrder();	// 10.f;

        void OpenningProcess();
        void ClosingProcess();
        void OpenQuickCommand(const wchar_t* strID, int iIndex, int x, int y);
        void CloseQuickCommand();
        void SetID(const wchar_t* strID);
        void SetSelectedCharacterIndex(int iIndex);

    private:
        void BuildRmlUi();
        void SyncRmlModel();
        void SyncRows();

    private:
        CManager* m_pNewUIMng;
        POINT			m_Pos;

        int m_iSelectedIndex;
        wchar_t m_strID[32];
        int m_iSelectedCharacterIndex;

        void BindRmlModel(Rml::DataModelConstructor& c, QuickCommandRmlModel& model);
        UI::RmlBridge::ThemedView<QuickCommandRmlModel> m_RmlView{"quick_command",
            [this](Rml::DataModelConstructor& c, QuickCommandRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/quick_command.rml"}}};
    };
}

#endif // !defined(AFX_NEWUIQUICKCOMMANDWINDOW_H__3A1D6614_8C41_4066_A831_2954B3C461D5__INCLUDED_)
