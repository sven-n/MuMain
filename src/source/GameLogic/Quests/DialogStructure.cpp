// =============================================================
// Generated once from Data/Local/Eng/Dialog_eng.bmd by the removed DialogImporter tool (see git history).
// The texts are I18N::Dialog keys (src/Localization/Dialog.*.resx).
// =============================================================

#include "GameLogic/Quests/DialogStructure.h"

#include "I18N/All.h"

namespace GameLogic::Quests::Dialog
{
namespace
{

// clang-format off
constexpr Entry kEntries[MaxDialog] = {
    {0, {{-1, 0}}, &I18N::Dialog::Text0},  // idx 0
    {0, {{-1, 0}}, &I18N::Dialog::Text1},  // idx 1
    {0, {{-1, 0}}, &I18N::Dialog::Text2},  // idx 2
    {0, {{-1, 0}}, &I18N::Dialog::Text3},  // idx 3
    {0, {{-1, 0}}, &I18N::Dialog::Text4},  // idx 4
    {0, {{-1, 0}}, &I18N::Dialog::Text5},  // idx 5
    {0, {{-1, 0}}, &I18N::Dialog::Text6},  // idx 6
    {0, {{-1, 0}}, &I18N::Dialog::Text7},  // idx 7
    {0, {{-1, 0}}, &I18N::Dialog::Text8},  // idx 8
    {0, {{-1, 0}}, &I18N::Dialog::Text9},  // idx 9
    {0, {}, nullptr},  // idx 10
    {0, {}, nullptr},  // idx 11
    {0, {}, nullptr},  // idx 12
    {0, {}, nullptr},  // idx 13
    {0, {}, nullptr},  // idx 14
    {0, {}, nullptr},  // idx 15
    {0, {}, nullptr},  // idx 16
    {0, {}, nullptr},  // idx 17
    {0, {}, nullptr},  // idx 18
    {0, {}, nullptr},  // idx 19
    {0, {}, nullptr},  // idx 20
    {0, {}, nullptr},  // idx 21
    {0, {}, nullptr},  // idx 22
    {0, {}, nullptr},  // idx 23
    {0, {}, nullptr},  // idx 24
    {0, {}, nullptr},  // idx 25
    {0, {}, nullptr},  // idx 26
    {0, {}, nullptr},  // idx 27
    {0, {}, nullptr},  // idx 28
    {0, {}, nullptr},  // idx 29
    {0, {}, nullptr},  // idx 30
    {0, {}, nullptr},  // idx 31
    {0, {}, nullptr},  // idx 32
    {0, {}, nullptr},  // idx 33
    {0, {}, nullptr},  // idx 34
    {0, {}, nullptr},  // idx 35
    {0, {}, nullptr},  // idx 36
    {0, {}, nullptr},  // idx 37
    {0, {}, nullptr},  // idx 38
    {0, {}, nullptr},  // idx 39
    {0, {}, nullptr},  // idx 40
    {0, {}, nullptr},  // idx 41
    {0, {}, nullptr},  // idx 42
    {0, {}, nullptr},  // idx 43
    {0, {}, nullptr},  // idx 44
    {0, {}, nullptr},  // idx 45
    {0, {}, nullptr},  // idx 46
    {0, {}, nullptr},  // idx 47
    {0, {}, nullptr},  // idx 48
    {0, {}, nullptr},  // idx 49
    {1, {{74, -1, &I18N::Dialog::Answer50Slot0}, {-1, 0}}, &I18N::Dialog::Text50},  // idx 50
    {1, {{51, 2, &I18N::Dialog::Answer51Slot0}, {-1, 0}}, &I18N::Dialog::Text51},  // idx 51
    {1, {{75, -1, &I18N::Dialog::Answer52Slot0}, {-1, 0}}, &I18N::Dialog::Text52},  // idx 52
    {3, {{52, -1, &I18N::Dialog::Answer53Slot0}, {54, 1, &I18N::Dialog::Answer53Slot1}, {53, 2, &I18N::Dialog::Answer53Slot2}, {-1, 0}}, &I18N::Dialog::Text53},  // idx 53
    {1, {{54, 2, &I18N::Dialog::Answer54Slot0}, {-1, 0}}, &I18N::Dialog::Text54},  // idx 54
    {1, {{55, 3, &I18N::Dialog::Answer55Slot0}, {-1, 0}}, &I18N::Dialog::Text55},  // idx 55
    {1, {{57, -1, &I18N::Dialog::Answer56Slot0}, {-1, 0}}, &I18N::Dialog::Text56},  // idx 56
    {2, {{56, -1, &I18N::Dialog::Answer57Slot0}, {80, -1, &I18N::Dialog::Answer57Slot1}, {-1, 0}}, &I18N::Dialog::Text57},  // idx 57
    {2, {{57, -1, &I18N::Dialog::Answer58Slot0}, {81, -1, &I18N::Dialog::Answer58Slot1}, {-1, 0}}, &I18N::Dialog::Text58},  // idx 58
    {2, {{58, -1, &I18N::Dialog::Answer59Slot0}, {59, 2, &I18N::Dialog::Answer59Slot1}, {-1, 0}}, &I18N::Dialog::Text59},  // idx 59
    {2, {{61, 1, &I18N::Dialog::Answer60Slot0}, {60, 2, &I18N::Dialog::Answer60Slot1}, {-1, 0}}, &I18N::Dialog::Text60},  // idx 60
    {1, {{61, 2, &I18N::Dialog::Answer61Slot0}, {-1, 0}}, &I18N::Dialog::Text61},  // idx 61
    {1, {{76, -1, &I18N::Dialog::Answer62Slot0}, {-1, 0}}, &I18N::Dialog::Text62},  // idx 62
    {1, {{79, -1, &I18N::Dialog::Answer63Slot0}, {-1, 0}}, &I18N::Dialog::Text63},  // idx 63
    {1, {{77, -1, &I18N::Dialog::Answer64Slot0}, {-1, 0}}, &I18N::Dialog::Text64},  // idx 64
    {1, {{65, 2, &I18N::Dialog::Answer65Slot0}, {-1, 0}}, &I18N::Dialog::Text65},  // idx 65
    {1, {{78, -1, &I18N::Dialog::Answer66Slot0}, {-1, 0}}, &I18N::Dialog::Text66},  // idx 66
    {1, {{82, -1, &I18N::Dialog::Answer67Slot0}, {-1, 0}}, &I18N::Dialog::Text67},  // idx 67
    {1, {{68, 2, &I18N::Dialog::Answer68Slot0}, {-1, 0}}, &I18N::Dialog::Text68},  // idx 68
    {1, {{70, -1, &I18N::Dialog::Answer69Slot0}, {-1, 0}}, &I18N::Dialog::Text69},  // idx 69
    {1, {{70, 3, &I18N::Dialog::Answer70Slot0}, {-1, 0}}, &I18N::Dialog::Text70},  // idx 70
    {1, {{71, 2, &I18N::Dialog::Answer71Slot0}, {-1, 0}}, &I18N::Dialog::Text71},  // idx 71
    {1, {{72, 2, &I18N::Dialog::Answer72Slot0}, {-1, 0}}, &I18N::Dialog::Text72},  // idx 72
    {1, {{73, 2, &I18N::Dialog::Answer73Slot0}, {-1, 0}}, &I18N::Dialog::Text73},  // idx 73
    {1, {{74, 2, &I18N::Dialog::Answer74Slot0}, {-1, 0}}, &I18N::Dialog::Text74},  // idx 74
    {1, {{53, -1, &I18N::Dialog::Answer75Slot0}, {-1, 0}}, &I18N::Dialog::Text75},  // idx 75
    {1, {{63, -1, &I18N::Dialog::Answer76Slot0}, {-1, 0}}, &I18N::Dialog::Text76},  // idx 76
    {2, {{65, 1, &I18N::Dialog::Answer77Slot0}, {77, 2, &I18N::Dialog::Answer77Slot1}, {-1, 0}}, &I18N::Dialog::Text77},  // idx 77
    {1, {{78, 3, &I18N::Dialog::Answer78Slot0}, {-1, 0}}, &I18N::Dialog::Text78},  // idx 78
    {1, {{79, 3, &I18N::Dialog::Answer79Slot0}, {-1, 0}}, &I18N::Dialog::Text79},  // idx 79
    {2, {{57, -1, &I18N::Dialog::Answer80Slot0}, {58, -1, &I18N::Dialog::Answer80Slot1}, {-1, 0}}, &I18N::Dialog::Text80},  // idx 80
    {2, {{58, -1, &I18N::Dialog::Answer81Slot0}, {59, -1, &I18N::Dialog::Answer81Slot1}, {-1, 0}}, &I18N::Dialog::Text81},  // idx 81
    {2, {{68, 1, &I18N::Dialog::Answer82Slot0}, {82, 2, &I18N::Dialog::Answer82Slot1}, {-1, 0}}, &I18N::Dialog::Text82},  // idx 82
    {0, {}, nullptr},  // idx 83
    {0, {}, nullptr},  // idx 84
    {0, {}, nullptr},  // idx 85
    {0, {}, nullptr},  // idx 86
    {0, {}, nullptr},  // idx 87
    {0, {}, nullptr},  // idx 88
    {0, {}, nullptr},  // idx 89
    {1, {{91, -1, &I18N::Dialog::Answer90Slot0}, {-1, 0}}, &I18N::Dialog::Text90},  // idx 90
    {2, {{92, 1, &I18N::Dialog::Answer91Slot0}, {91, 2, &I18N::Dialog::Answer91Slot1}, {-1, 0}}, &I18N::Dialog::Text91},  // idx 91
    {1, {{92, 2, &I18N::Dialog::Answer92Slot0}, {-1, 0}}, &I18N::Dialog::Text92},  // idx 92
    {1, {{94, -1, &I18N::Dialog::Answer93Slot0}, {-1, 0}}, &I18N::Dialog::Text93},  // idx 93
    {1, {{94, 3, &I18N::Dialog::Answer94Slot0}, {-1, 0}}, &I18N::Dialog::Text94},  // idx 94
    {0, {}, nullptr},  // idx 95
    {0, {}, nullptr},  // idx 96
    {0, {}, nullptr},  // idx 97
    {0, {}, nullptr},  // idx 98
    {0, {}, nullptr},  // idx 99
    {1, {{101, -1, &I18N::Dialog::Answer100Slot0}, {-1, 0}}, &I18N::Dialog::Text100},  // idx 100
    {1, {{101, 2, &I18N::Dialog::Answer101Slot0}, {-1, 0}}, &I18N::Dialog::Text101},  // idx 101
    {1, {{103, -1, &I18N::Dialog::Answer102Slot0}, {-1, 0}}, &I18N::Dialog::Text102},  // idx 102
    {1, {{104, -1, &I18N::Dialog::Answer103Slot0}, {-1, 0}}, &I18N::Dialog::Text103},  // idx 103
    {3, {{102, -1, &I18N::Dialog::Answer104Slot0}, {105, 1, &I18N::Dialog::Answer104Slot1}, {104, 2, &I18N::Dialog::Answer104Slot2}, {-1, 0}}, &I18N::Dialog::Text104},  // idx 104
    {1, {{105, 2, &I18N::Dialog::Answer105Slot0}, {-1, 0}}, &I18N::Dialog::Text105},  // idx 105
    {0, {}, nullptr},  // idx 106
    {0, {}, nullptr},  // idx 107
    {1, {{109, -1, &I18N::Dialog::Answer108Slot0}, {-1, 0}}, &I18N::Dialog::Text108},  // idx 108
    {1, {{110, -1, &I18N::Dialog::Answer109Slot0}, {-1, 0}}, &I18N::Dialog::Text109},  // idx 109
    {1, {{111, -1, &I18N::Dialog::Answer110Slot0}, {-1, 0}}, &I18N::Dialog::Text110},  // idx 110
    {1, {{112, -1, &I18N::Dialog::Answer111Slot0}, {-1, 0}}, &I18N::Dialog::Text111},  // idx 111
    {1, {{112, 3, &I18N::Dialog::Answer112Slot0}, {-1, 0}}, &I18N::Dialog::Text112},  // idx 112
    {1, {{113, 2, &I18N::Dialog::Answer113Slot0}, {-1, 0}}, &I18N::Dialog::Text113},  // idx 113
    {1, {{114, 2, &I18N::Dialog::Answer114Slot0}, {-1, 0}}, &I18N::Dialog::Text114},  // idx 114
    {1, {{101, -1, &I18N::Dialog::Answer115Slot0}, {-1, 0}}, &I18N::Dialog::Text115},  // idx 115
    {1, {{116, 2, &I18N::Dialog::Answer116Slot0}, {-1, 0}}, &I18N::Dialog::Text116},  // idx 116
    {1, {{117, 2, &I18N::Dialog::Answer117Slot0}, {-1, 0}}, &I18N::Dialog::Text117},  // idx 117
    {1, {{118, 2, &I18N::Dialog::Answer118Slot0}, {-1, 0}}, &I18N::Dialog::Text118},  // idx 118
    {1, {{119, 2, &I18N::Dialog::Answer119Slot0}, {-1, 0}}, &I18N::Dialog::Text119},  // idx 119
    {0, {}, nullptr},  // idx 120
    {0, {}, nullptr},  // idx 121
    {0, {}, nullptr},  // idx 122
    {0, {}, nullptr},  // idx 123
    {0, {}, nullptr},  // idx 124
    {0, {}, nullptr},  // idx 125
    {0, {}, nullptr},  // idx 126
    {0, {}, nullptr},  // idx 127
    {0, {}, nullptr},  // idx 128
    {0, {}, nullptr},  // idx 129
    {1, {{131, -1, &I18N::Dialog::Answer130Slot0}, {-1, 0}}, &I18N::Dialog::Text130},  // idx 130
    {1, {{132, -1, &I18N::Dialog::Answer131Slot0}, {-1, 0}}, &I18N::Dialog::Text131},  // idx 131
    {1, {{133, -1, &I18N::Dialog::Answer132Slot0}, {-1, 0}}, &I18N::Dialog::Text132},  // idx 132
    {1, {{134, -1, &I18N::Dialog::Answer133Slot0}, {-1, 0}}, &I18N::Dialog::Text133},  // idx 133
    {1, {{134, 3, &I18N::Dialog::Answer134Slot0}, {-1, 0}}, &I18N::Dialog::Text134},  // idx 134
    {1, {{136, -1, &I18N::Dialog::Answer135Slot0}, {-1, 0}}, &I18N::Dialog::Text135},  // idx 135
    {1, {{137, -1, &I18N::Dialog::Answer136Slot0}, {-1, 0}}, &I18N::Dialog::Text136},  // idx 136
    {3, {{135, -1, &I18N::Dialog::Answer137Slot0}, {138, 1, &I18N::Dialog::Answer137Slot1}, {137, 2, &I18N::Dialog::Answer137Slot2}, {-1, 0}}, &I18N::Dialog::Text137},  // idx 137
    {1, {{138, 2, &I18N::Dialog::Answer138Slot0}, {-1, 0}}, &I18N::Dialog::Text138},  // idx 138
    {1, {{140, -1, &I18N::Dialog::Answer139Slot0}, {-1, 0}}, &I18N::Dialog::Text139},  // idx 139
    {1, {{141, -1, &I18N::Dialog::Answer140Slot0}, {-1, 0}}, &I18N::Dialog::Text140},  // idx 140
    {1, {{141, 3, &I18N::Dialog::Answer141Slot0}, {-1, 0}}, &I18N::Dialog::Text141},  // idx 141
    {1, {{143, -1, &I18N::Dialog::Answer142Slot0}, {-1, 0}}, &I18N::Dialog::Text142},  // idx 142
    {1, {{143, 2, &I18N::Dialog::Answer143Slot0}, {-1, 0}}, &I18N::Dialog::Text143},  // idx 143
    {1, {{144, 2, &I18N::Dialog::Answer144Slot0}, {-1, 0}}, &I18N::Dialog::Text144},  // idx 144
    {1, {{145, 2, &I18N::Dialog::Answer145Slot0}, {-1, 0}}, &I18N::Dialog::Text145},  // idx 145
    {1, {{146, 2, &I18N::Dialog::Answer146Slot0}, {-1, 0}}, &I18N::Dialog::Text146},  // idx 146
    {1, {{148, -1, &I18N::Dialog::Answer147Slot0}, {-1, 0}}, &I18N::Dialog::Text147},  // idx 147
    {1, {{149, -1, &I18N::Dialog::Answer148Slot0}, {-1, 0}}, &I18N::Dialog::Text148},  // idx 148
    {1, {{150, -1, &I18N::Dialog::Answer149Slot0}, {-1, 0}}, &I18N::Dialog::Text149},  // idx 149
    {1, {{151, -1, &I18N::Dialog::Answer150Slot0}, {-1, 0}}, &I18N::Dialog::Text150},  // idx 150
    {3, {{147, -1, &I18N::Dialog::Answer151Slot0}, {152, 1, &I18N::Dialog::Answer151Slot1}, {151, 2, &I18N::Dialog::Answer151Slot2}, {-1, 0}}, &I18N::Dialog::Text151},  // idx 151
    {1, {{153, -1, &I18N::Dialog::Answer152Slot0}, {-1, 0}}, &I18N::Dialog::Text152},  // idx 152
    {1, {{153, 2, &I18N::Dialog::Answer153Slot0}, {-1, 0}}, &I18N::Dialog::Text153},  // idx 153
    {1, {{155, 3, &I18N::Dialog::Answer154Slot0}, {-1, 0}}, &I18N::Dialog::Text154},  // idx 154
    {1, {{156, -1, &I18N::Dialog::Answer155Slot0}, {-1, 0}}, &I18N::Dialog::Text155},  // idx 155
    {1, {{156, 2, &I18N::Dialog::Answer156Slot0}, {-1, 0}}, &I18N::Dialog::Text156},  // idx 156
    {1, {{157, 2, &I18N::Dialog::Answer157Slot0}, {-1, 0}}, &I18N::Dialog::Text157},  // idx 157
    {1, {{159, -1, &I18N::Dialog::Answer158Slot0}, {-1, 0}}, &I18N::Dialog::Text158},  // idx 158
    {1, {{160, -1, &I18N::Dialog::Answer159Slot0}, {-1, 0}}, &I18N::Dialog::Text159},  // idx 159
    {1, {{161, -1, &I18N::Dialog::Answer160Slot0}, {-1, 0}}, &I18N::Dialog::Text160},  // idx 160
    {3, {{158, -1, &I18N::Dialog::Answer161Slot0}, {162, 1, &I18N::Dialog::Answer161Slot1}, {161, 2, &I18N::Dialog::Answer161Slot2}, {-1, 0}}, &I18N::Dialog::Text161},  // idx 161
    {1, {{163, -1, &I18N::Dialog::Answer162Slot0}, {-1, 0}}, &I18N::Dialog::Text162},  // idx 162
    {1, {{163, 2, &I18N::Dialog::Answer163Slot0}, {-1, 0}}, &I18N::Dialog::Text163},  // idx 163
    {1, {{165, 3, &I18N::Dialog::Answer164Slot0}, {-1, 0}}, &I18N::Dialog::Text164},  // idx 164
    {1, {{165, 2, &I18N::Dialog::Answer165Slot0}, {-1, 0}}, &I18N::Dialog::Text165},  // idx 165
    {1, {{166, 2, &I18N::Dialog::Answer166Slot0}, {-1, 0}}, &I18N::Dialog::Text166},  // idx 166
    {1, {{167, 2, &I18N::Dialog::Answer167Slot0}, {-1, 0}}, &I18N::Dialog::Text167},  // idx 167
    {1, {{169, -1, &I18N::Dialog::Answer168Slot0}, {-1, 0}}, &I18N::Dialog::Text168},  // idx 168
    {1, {{170, -1, &I18N::Dialog::Answer169Slot0}, {-1, 0}}, &I18N::Dialog::Text169},  // idx 169
    {1, {{171, -1, &I18N::Dialog::Answer170Slot0}, {-1, 0}}, &I18N::Dialog::Text170},  // idx 170
    {3, {{168, -1, &I18N::Dialog::Answer171Slot0}, {172, 1, &I18N::Dialog::Answer171Slot1}, {171, 2, &I18N::Dialog::Answer171Slot2}, {-1, 0}}, &I18N::Dialog::Text171},  // idx 171
    {1, {{172, 2, &I18N::Dialog::Answer172Slot0}, {-1, 0}}, &I18N::Dialog::Text172},  // idx 172
    {1, {{174, 3, &I18N::Dialog::Answer173Slot0}, {-1, 0}}, &I18N::Dialog::Text173},  // idx 173
    {1, {{174, 2, &I18N::Dialog::Answer174Slot0}, {-1, 0}}, &I18N::Dialog::Text174},  // idx 174
    {0, {}, nullptr},  // idx 175
    {0, {}, nullptr},  // idx 176
    {0, {}, nullptr},  // idx 177
    {0, {}, nullptr},  // idx 178
    {0, {}, nullptr},  // idx 179
    {0, {}, nullptr},  // idx 180
    {0, {}, nullptr},  // idx 181
    {0, {}, nullptr},  // idx 182
    {0, {}, nullptr},  // idx 183
    {0, {}, nullptr},  // idx 184
    {0, {}, nullptr},  // idx 185
    {0, {}, nullptr},  // idx 186
    {0, {}, nullptr},  // idx 187
    {0, {}, nullptr},  // idx 188
    {0, {}, nullptr},  // idx 189
    {0, {}, nullptr},  // idx 190
    {0, {}, nullptr},  // idx 191
    {0, {}, nullptr},  // idx 192
    {0, {}, nullptr},  // idx 193
    {0, {}, nullptr},  // idx 194
    {0, {}, nullptr},  // idx 195
    {0, {}, nullptr},  // idx 196
    {0, {}, nullptr},  // idx 197
    {0, {}, nullptr},  // idx 198
    {0, {}, nullptr},  // idx 199
};
// clang-format on

constexpr Entry kEmptyEntry = {0, {}, nullptr};

const wchar_t* TextOrEmpty(const wchar_t* const* text) noexcept
{
    return text != nullptr ? *text : L"";
}

} // namespace

const Entry& GetEntry(int dialogIndex) noexcept
{
    if (dialogIndex < 0 || dialogIndex >= MaxDialog)
        return kEmptyEntry;
    return kEntries[dialogIndex];
}

const wchar_t* GetText(int dialogIndex) noexcept
{
    return TextOrEmpty(GetEntry(dialogIndex).text);
}

const wchar_t* GetAnswerText(int dialogIndex, int answerSlot) noexcept
{
    if (answerSlot < 0 || answerSlot >= MaxAnswer)
        return L"";
    return TextOrEmpty(GetEntry(dialogIndex).answers[answerSlot].text);
}

} // namespace GameLogic::Quests::Dialog
