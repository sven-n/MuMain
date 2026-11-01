//*****************************************************************************
// File: ServerSelWin.h
//*****************************************************************************
#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/RmlBridge/RmlThemedView.h"

#include <vector>

namespace Rml { class ElementDocument; }

class CServerGroup;
class CServerInfo;

namespace Data::ServerList
{
struct ButtonColor;
}

// Pure RmlUi 2D UI: RmlUi owns all of this window's rendering and click handling. Two
// always-visible columns laid out by server_select.rcss's flexbox rules -- server groups on the
// left, and the servers within whichever group is selected on the right (empty/hidden until a
// group is clicked). Self-centers via base.rcss's .center-both; UpdateMouseEvent() claims only its
// own rect, computed from this window's authored footprint centered the same way.
class CServerSelWin : public mu::ui::window::CObject
{
public:
    CServerSelWin();
    ~CServerSelWin() override;

    void Create();
    void Release();
    void UpdateDisplay();
    void Show(bool bShow) override;


    // Bound to the RmlUi document's data-event-click callbacks; acts immediately.
    void RmlClickSelectGroup(int nBtnPos);
    void RmlClickSelectServer(int nServerIndex);

    // Selects a server group and one of its servers without a click, doing
    // exactly what the two RmlUi click callbacks do. `groupName` empty or null
    // picks the first group in the list. Returns false when the list holds no
    // such group or server, or when the server is full.
    bool SelectServer(const wchar_t* groupName, int serverIndex);

#ifdef _EDITOR
    // The ID (CServerGroup::m_iServerIndex) of the open group, or -1; for the
    // editor, before it builds the groups again.
    int GetOpenGroupId() const;
    // After the editor built the groups again: shows them, with the group of
    // that ID open again wherever it is in the list now, or none when it is gone.
    void ShowRebuiltGroups(int openGroupId);
#endif

    // mu::ui::window::IObject
    bool Render() override;
    bool Update() override;
    bool UpdateMouseEvent() override;
    bool UpdateKeyEvent() override
    {
        return true;
    }
    // Below CCreditWin/CSysMenuWin: the two can be visible simultaneously, and CSysMenuWin wins click priority.
    float GetLayerDepth() override
    {
        return 20.0f;
    }

private:
    void SelectGroup(int nBtnPos);
    void SyncRmlModel();

    // Shared by RmlClickSelectServer() and SelectServer(). Returns false (and
    // leaves the window open) without connecting when pServerInfo is null or
    // the server is full/busy -- the caller decides what busy means to it.
    bool ConnectToServer(CServerInfo* pServerInfo);

    struct GroupEntry
    {
        Rml::String label;
        int btnPos = 0; // CServerGroup::m_iBtnPos -- passed back to RmlClickSelectGroup; 0 is the rare center/"test server" group
        bool checked = false;
    };
    // One of the load bar's segments (Data::ServerList::GetLoadBarSegments).
    struct SegmentEntry
    {
        // How much of it is lit, as a width ("35%").
        Rml::String lit;
        // "#00BD8B".
        Rml::String color;
    };
    // A color of the server list's: one bool per theme color class, since RmlUi's data-class-X
    // binds a class per boolean, not a name; none set is the default. `style` is a color of the
    // list's own ("#FF5A5A"), set as the color; empty for a theme color.
    struct ColorEntry
    {
        bool gray = false;
        bool orange = false;
        bool silver = false;
        Rml::String style;
    };
    struct ServerEntry
    {
        // The button's text without its load text (CServerListManager::GetServerButtonName), at the left, and
        // the load text of a full or overloaded server, at the right.
        Rml::String label;
        Rml::String loadText;
        int index = 0; // position within the selected group's server list (CServerGroup::GetServerInfo(i))
        // From CServerListManager::GetServerButtonColor.
        ColorEntry color;
        // The load text's look (Data::ServerList::LoadText). `loadSlant` slants it where the font
        // has no italic face.
        ColorEntry loadColor;
        bool loadBold = false;
        bool loadItalic = false;
        bool loadSlant = false;
        std::vector<SegmentEntry> segments;
        // The unlit part of the segments ("#202020"); empty for the theme's own.
        Rml::String emptyStyle;
        // No "checked"/selected state here -- clicking a server row connects immediately.
    };
    // A load text a button can show, in its look (CServerListManager::GetButtonLoadTexts).
    struct LoadSizerEntry
    {
        Rml::String text;
        bool bold = false;
        bool italic = false;
    };
    struct ServerSelRmlModel
    {
        // One merged, order-preserving list.
        std::vector<GroupEntry> groups;
        std::vector<ServerEntry> servers;
        // Every text a button can have, laid out unseen (server_select.rml's sizer row): the names of every
        // channel of every group shown and the load texts. The buttons are as wide as the longest of them, so
        // neither a load change nor another group resizes them.
        std::vector<Rml::String> sizerNames;
        std::vector<LoadSizerEntry> sizerLoads;

        // True once a group has been clicked and its server list requested/populated.
        bool serverListVisible = false;

        bool pvpNotice = false;
        Rml::String pvpNoticeLine0, pvpNoticeLine1, pvpNoticeLine2;

        Rml::String descriptionText;
    };
    static ColorEntry MakeColorEntry(const Data::ServerList::ButtonColor& color);
    // The texts the buttons are sized by (ServerSelRmlModel::sizerNames), of the groups shown.
    static void FillSizers(ServerSelRmlModel& model, const std::vector<const CServerGroup*>& groups);
    // The look of the {load} text, and the load bar, of a server with the load `percent`.
    static void SetLoadTextLook(ServerEntry& entry, int percent);
    static void SetLoadBar(ServerEntry& entry, int percent);
    // The button of one server of the selected group, at `index` in its list.
    ServerEntry MakeServerEntry(const CServerInfo& serverInfo, int index) const;
    void BindRmlModel(Rml::DataModelConstructor& c, ServerSelRmlModel& model);
    UI::RmlBridge::ThemedView<ServerSelRmlModel> m_RmlView{"server_select",
        [this](Rml::DataModelConstructor& c, ServerSelRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/server_select.rml"}}};

    int m_iSelectServerBtnIndex = -1;
    CServerGroup* m_pSelectServerGroup = nullptr;

    // Fixed, author-chosen footprint used only for UpdateMouseEvent()'s own-rect check
    // (self-centered the same way SceneUICoordinator.cpp centers every other window); covers
    // #panel's real max footprint (both columns plus the pvp notice/description text) with slack.
    static constexpr int kPanelWidth = 420;
    static constexpr int kPanelHeight = 460;
};

extern CServerSelWin g_ServerSelWin;
