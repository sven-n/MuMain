#include "stdafx.h"
#include "UI/Party/ChatRoomModel.h"

namespace UI::Party
{
void ChatRoomModel::Bind(Rml::DataModelConstructor& c)
{
    auto line = c.RegisterStruct<Line>();
    line.RegisterMember("speaker", &Line::speaker);
    line.RegisterMember("text", &Line::text);
    line.RegisterMember("type", &Line::type);
    auto pal = c.RegisterStruct<Pal>();
    pal.RegisterMember("name", &Pal::name);
    pal.RegisterMember("number", &Pal::number);
    c.RegisterArray<std::vector<Line>>();
    c.RegisterArray<std::vector<Pal>>();

    c.Bind("lines", &lines);
    c.Bind("pals", &pals);
    c.Bind("invite_pals", &invitePals);
    c.Bind("title", &title);
    c.Bind("draft", &draft);
    c.Bind("selected_invite", &selectedInvite);
    c.Bind("invite_button_label", &inviteButtonLabel);
    c.Bind("invite_label", &inviteLabel);
    c.Bind("send_label", &sendLabel);
    c.Bind("workspace_height", &workspaceHeight);
    c.Bind("root_x", &rootX);
    c.Bind("root_y", &rootY);
    c.Bind("show_invite", &showInvite);
    c.Bind("show_pals", &showPals);
    c.Bind("locked", &locked);
    c.Bind("has_title", &hasTitle);
    c.Bind("positioned", &positioned);
    c.Bind("maximized", &maximized);
}
} // namespace UI::Party
