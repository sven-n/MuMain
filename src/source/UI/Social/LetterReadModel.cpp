#include "stdafx.h"
#include "UI/Social/LetterReadModel.h"

namespace UI::Social
{
void LetterReadModel::Bind(Rml::DataModelConstructor& c)
{
    auto line = c.RegisterStruct<Line>();
    line.RegisterMember("text", &Line::text);
    c.RegisterArray<std::vector<Line>>();

    c.Bind("lines", &lines);
    c.Bind("title", &title);
    c.Bind("header", &header);
    c.Bind("reply_label", &replyLabel);
    c.Bind("delete_label", &deleteLabel);
    c.Bind("close_label", &closeLabel);
    c.Bind("prev_label", &prevLabel);
    c.Bind("next_label", &nextLabel);
    c.Bind("root_x", &rootX);
    c.Bind("root_y", &rootY);
    c.Bind("has_title", &hasTitle);
    c.Bind("positioned", &positioned);
}
} // namespace UI::Social
