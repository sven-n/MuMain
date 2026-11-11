#include "stdafx.h"
#include "UI/Social/LetterWriteModel.h"

namespace UI::Social
{
void LetterWriteModel::Bind(Rml::DataModelConstructor& c)
{

    c.Bind("mailto", &mailto);
    c.Bind("subject", &subject);
    c.Bind("body", &body);
    c.Bind("title", &title);
    c.Bind("receiver_label", &receiverLabel);
    c.Bind("subject_label", &subjectLabel);
    c.Bind("send_label", &sendLabel);
    c.Bind("close_label", &closeLabel);
    c.Bind("prev_pose_label", &prevPoseLabel);
    c.Bind("next_pose_label", &nextPoseLabel);
    c.Bind("root_x", &rootX);
    c.Bind("root_y", &rootY);
    c.Bind("has_title", &hasTitle);
    c.Bind("positioned", &positioned);
    c.Bind("sending", &sending);
}
} // namespace UI::Social
