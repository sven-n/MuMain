
#if !defined(AFX_NEWUIGUILDMAKEWINDOW_H__68B0DE4B_7E07_4928_B8CF_2F7A6A15EEBD__INCLUDED_)
#define AFX_NEWUIGUILDMAKEWINDOW_H__68B0DE4B_7E07_4928_B8CF_2F7A6A15EEBD__INCLUDED_

#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Social/SocialWindowCore.h"   // UISTATES
#include "Guild/GuildMakeRmlModel.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/RmlBridge/RmlModelBinder.h"

namespace Rml
{
class Element;
class ElementDocument;
} // namespace Rml

namespace mu::ui::window
{
    class CManager;
    // The guild master NPC's guild creation window (intro, name and mark editor, summary), docked
    // right. guild_make.rml draws it, the name field included; C++ keeps the pages, the mark
    // painting (native hit tests on the grid and palette), the checks and the requests.
    class CGuildMakeWindow : public CObject
    {
    public:
        enum
        {
            GUILDMAKE_WIDTH = 190,
            GUILDMAKE_HEIGHT = 429,
        };

        enum GUILDMAKE_STATE
        {
            GUILDMAKE_INFO = 0,
            GUILDMAKE_MARK,
            GUILDMAKE_RESULTINFO,
        };

        enum
        {
            MAXGUILDNAME = 8,
        };

        // The buttons RmlUi reports (guild_make.rml's guild_make_button(n)).
        enum GUILDMAKE_BUTTON
        {
            GUILDMAKEBUTTON_NONE = -1,
            GUILDMAKEBUTTON_MAKE = 0,
            GUILDMAKEBUTTON_BACK,
            GUILDMAKEBUTTON_NEXT,
            GUILDMAKEBUTTON_EXIT,
        };

    public:
        CGuildMakeWindow();
        virtual ~CGuildMakeWindow();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void ClosingProcess();

    public:
        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();

    private:
        void UpdateGMInfo(GUILDMAKE_BUTTON button);
        void UpdateGMMark(GUILDMAKE_BUTTON button);
        void UpdateGMResultInfo(GUILDMAKE_BUTTON button);

    public:
        bool Render();

        void ReloadRmlTheme();

    private:
        void BuildRmlUi();
        void SyncRmlModel();
        void SyncContent();
        Rml::Element* GetNameField() const;
        void ReadNameField(wchar_t* text, int length) const;

    public:
        void SetPos(int x, int y);
        const POINT& GetPos();
        float GetLayerDepth();	//. 4.4f

    private:
        void ChangeWindowState(const GUILDMAKE_STATE state);
        void ChangeEditBox(const UISTATES type);

    private:
        CManager* m_pNewUIMng;

        POINT					m_Pos;
        GUILDMAKE_STATE			m_GuildMakeState;

        // The name field (the original's CUITextInputBox): shown on the mark page only.
        bool m_NameFieldShown = false;
        bool m_NameFieldFocusPending = false;
        RmlModelBinder<GuildMakeRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;
        GUILDMAKE_BUTTON m_PendingButton = GUILDMAKEBUTTON_NONE;
    };

    inline
        void CGuildMakeWindow::SetPos(int x, int y)
    {
        m_Pos.x = x; m_Pos.y = y;
    }

    inline
        const POINT& CGuildMakeWindow::GetPos()
    {
        return m_Pos;
    }
}

#endif // !defined(AFX_NEWUIGUILDMAKEWINDOW_H__68B0DE4B_7E07_4928_B8CF_2F7A6A15EEBD__INCLUDED_)
