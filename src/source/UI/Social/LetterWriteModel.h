#pragma once

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Types.h>
#include <vector>

namespace UI::Social
{
struct LetterWriteModel
{

    // The three fields, two-way bound; C++ reads them back when Send is pressed.
    Rml::String mailto, subject, body;
    Rml::String title;
    Rml::String receiverLabel, subjectLabel, sendLabel, closeLabel, prevPoseLabel, nextPoseLabel;
    // window_shell's own contract: a titled panel placed at a final device-pixel top-left.
    float rootX = 0, rootY = 0;
    bool hasTitle = true, positioned = true;
    // Set while a send is in flight, so the button cannot fire twice (m_bIsSend).
    bool sending = false;

    void Bind(Rml::DataModelConstructor& constructor);
};
} // namespace UI::Social
