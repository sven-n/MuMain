#include "App/stdafx.h"

#include <doctest.h>

#include "I18N/All.h"
#include "GameLogic/Quests/DialogStructure.h"
#include "LegacyTextComparison.h"

namespace Dialog = GameLogic::Quests::Dialog;

namespace
{
// The legacy ids I18N::Dialog used: the NPC line at the dialog index, a reply at 1000 + index * 10 + slot.
int LegacyAnswerId(int dialogIndex, int answerSlot)
{
    return 1000 + dialogIndex * 10 + answerSlot;
}
} // namespace

TEST_CASE("dialog NPC lines match the legacy ids in every locale [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        // Dialog indexes come from Quest.bmd and from a BYTE packet field: below 256.
        for (int dialogIndex = -5; dialogIndex < 256; ++dialogIndex)
        {
            CHECK_MESSAGE(LegacyTextComparison::SameText(Dialog::GetText(dialogIndex),
                                                         I18N::Dialog::Lookup(dialogIndex)),
                          locale << " " << dialogIndex);
        }
    });
}

TEST_CASE("dialog reply labels match the legacy ids in every locale [i18n][legacy]")
{
    LegacyTextComparison::ForEachLocale([&](const char* locale) {
        for (int dialogIndex = 0; dialogIndex < Dialog::MaxDialog; ++dialogIndex)
        {
            const auto& entry = Dialog::GetEntry(dialogIndex);
            for (int answerSlot = 0; answerSlot < entry.numAnswer; ++answerSlot)
            {
                CHECK_MESSAGE(LegacyTextComparison::SameText(Dialog::GetAnswerText(dialogIndex, answerSlot),
                                                             I18N::Dialog::Lookup(LegacyAnswerId(dialogIndex, answerSlot))),
                              locale << " " << dialogIndex << "/" << answerSlot);
            }
        }
    });
}

TEST_CASE("the dialog table holds the same slots as the legacy ids [i18n][legacy]")
{
    // Each legacy id in the Dialog group is an NPC line (0..199) or the label of a used reply slot.
    int textCount = 0;
    int answerCount = 0;
    for (int dialogIndex = 0; dialogIndex < Dialog::MaxDialog; ++dialogIndex)
    {
        const auto& entry = Dialog::GetEntry(dialogIndex);
        CHECK_MESSAGE(entry.text == I18N::Dialog::LookupSlot(dialogIndex), dialogIndex);
        if (entry.text != nullptr)
            ++textCount;
        for (int answerSlot = 0; answerSlot < Dialog::MaxAnswer; ++answerSlot)
        {
            CHECK_MESSAGE(entry.answers[answerSlot].text ==
                              I18N::Dialog::LookupSlot(LegacyAnswerId(dialogIndex, answerSlot)),
                          dialogIndex << "/" << answerSlot);
            CHECK_MESSAGE((entry.answers[answerSlot].text != nullptr) == (answerSlot < entry.numAnswer),
                          dialogIndex << "/" << answerSlot);
            if (entry.answers[answerSlot].text != nullptr)
                ++answerCount;
        }
    }
    CHECK(textCount == 111);
    CHECK(answerCount == 122);
}
