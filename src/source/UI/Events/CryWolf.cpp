
#include "stdafx.h"
#include "UI/Events/CryWolf.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "Character/CharacterManager.h"
#include "Audio/DSPlaySound.h"
#include "Guild/GuildTypes.h"
#include "UI/Widgets/UIControls.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"
#include "I18N/All.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"
#include "World/GameMaps/GMCrywolf1st.h"

#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/StringUtilities.h>

#include <algorithm>
#include <string>

extern bool	View_Bal;
extern char	Suc_Or_Fail;
extern char	View_Suc_Or_Fail;
extern float Deco_Insert;
extern char Message_Box;
extern wchar_t   Box_String[2][200];
extern int  Dark_elf_Num;
extern int Button_Down;
extern int BackUp_Key;
extern int Val_Hp;
extern int m_iHour, m_iMinute;
extern DWORD m_dwSyncTime;
extern int Delay;
extern int Add_Num;
extern bool Dark_Elf_Check;
extern int iNextNotice;

extern BYTE Rank;
extern int Exp;
extern BYTE Ranking[5];
extern CLASS_TYPE HeroClass[5];
extern int HeroScore[5];
extern wchar_t HeroName[5][MAX_USERNAME_SIZE + 1];

extern int BackUpMin;
extern bool TimeStart;
extern int Delay_Add_inter;
extern bool View_End_Result;
extern int nPastTick;
extern int BackUpTick;

extern BYTE m_OccupationState;
extern BYTE m_CrywolfState;
extern int m_StatueHP;

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
// The original drew the HUD at layer depth 10, over the inventory, the chat and the other panels:
// the document sits in the main context, pulled to the front when it is shown.
Rml::Context* CryWolfContext()
{
    return RmlUiRuntime::Instance().GetContext();
}
} // namespace

mu::ui::window::CCryWolf::CCryWolf()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;

    m_iHour = 0;
    m_iMinute = 0;
    m_iSecond = 0;
    m_dwSyncTime = 0;
    m_icntTime = 0;
    m_bTimeStart = false;
}

mu::ui::window::CCryWolf::~CCryWolf()
{
    Release();
}

bool mu::ui::window::CCryWolf::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_CRYWOLF, this);

    SetPos(x, y);

    LoadImages();
    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });
    return true;
}

void mu::ui::window::CCryWolf::ClosingProcess()
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();
}

float mu::ui::window::CCryWolf::GetLayerDepth()
{
    return 10.0f;
}

void mu::ui::window::CCryWolf::OpenningProcess()
{
    m_iHour = 0;
    m_iMinute = 0;
    m_iSecond = 0;
    m_dwSyncTime = 0;
    m_icntTime = 0;
    m_bTimeStart = false;
}

void mu::ui::window::CCryWolf::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);
    if (m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        Rml::Context* context = CryWolfContext();
        m_RmlBinder.Destroy(context);
        context->UnloadDocument(m_pRmlDoc);
        m_pRmlDoc = nullptr;
    }
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CCryWolf::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool mu::ui::window::CCryWolf::UpdateMouseEvent()
{
    return true;
}
bool mu::ui::window::CCryWolf::UpdateKeyEvent()
{
    return true;
}

bool mu::ui::window::CCryWolf::Render()
{
    // Nothing native left: the result, the battle HUD and the notice are RmlUi (SyncView()).
    // Kept because CObject requires the override.
    return true;
}

bool mu::ui::window::CCryWolf::Update()
{
    SyncView();
    return true;
}

namespace
{
// A file of Data/Interface, relative to crywolf.rml.
Rml::String InterfaceImage(const char* file)
{
    return Rml::String("../../../") + file;
}

Rml::String TexelRect(float x, float y, float width, float height)
{
    return Rml::CreateString("%g %g %g %g", x, y, width, height);
}

// RenderNumber2D(x, y, number, 14, 14): FontTest's 16 x 16 texel digit cells, 14 x 14 reference
// px, centred on x and 9.8 px apart.
void AddTimerDigits(std::vector<CryWolfSpriteEntry>& digits, const Rml::Vector2f& centre, int number)
{
    const std::string text = std::to_string(std::max(number, 0));
    float left = centre.x - 14.f * static_cast<float>(text.size()) / 2;
    for (const char digit : text)
    {
        digits.push_back({left, centre.y, InterfaceImage("FontTest.tga"),
                          TexelRect(static_cast<float>((digit - '0') * 16), 0.f, 16.f, 16.f)});
        left += 14.f * 0.7f;
    }
}

bool SameSprites(const std::vector<CryWolfSpriteEntry>& a, const std::vector<CryWolfSpriteEntry>& b)
{
    return a.size() == b.size() &&
           std::equal(a.begin(), a.end(), b.begin(), [](const CryWolfSpriteEntry& l, const CryWolfSpriteEntry& r)
                      { return l.left == r.left && l.top == r.top && l.src == r.src && l.rect == r.rect; });
}

} // namespace

void mu::ui::window::CCryWolf::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(CryWolfContext(), "crywolf",
                                                 [](Rml::DataModelConstructor& c, CryWolfRmlModel& model)
                                                 {
                                                     auto sprite = c.RegisterStruct<CryWolfSpriteEntry>();
                                                     sprite.RegisterMember("left", &CryWolfSpriteEntry::left);
                                                     sprite.RegisterMember("top", &CryWolfSpriteEntry::top);
                                                     sprite.RegisterMember("src", &CryWolfSpriteEntry::src);
                                                     sprite.RegisterMember("rect", &CryWolfSpriteEntry::rect);
                                                     c.RegisterArray<std::vector<CryWolfSpriteEntry>>();
                                                     auto image = c.RegisterStruct<CryWolfImageEntry>();
                                                     image.RegisterMember("shown", &CryWolfImageEntry::shown);
                                                     image.RegisterMember("src", &CryWolfImageEntry::src);
                                                     image.RegisterMember("rect", &CryWolfImageEntry::rect);
                                                     c.RegisterArray<std::vector<CryWolfImageEntry>>();
                                                     auto notice = c.RegisterStruct<CryWolfNoticeEntry>();
                                                     notice.RegisterMember("text", &CryWolfNoticeEntry::text);
                                                     notice.RegisterMember("heading", &CryWolfNoticeEntry::heading);
                                                     c.RegisterArray<std::vector<CryWolfNoticeEntry>>();

                                                     c.Bind("scale_x", &model.scaleX);
                                                     c.Bind("scale_y", &model.scaleY);
                                                     c.Bind("inverse_scale_x", &model.inverseScaleX);
                                                     c.Bind("inverse_scale_y", &model.inverseScaleY);
                                                     c.Bind("normal_text_px", &model.normalTextPx);
                                                     c.Bind("bold_text_px", &model.boldTextPx);
                                                     c.Bind("bold_line_px", &model.boldLinePx);
                                                     c.Bind("result_visible", &model.resultVisible);
                                                     c.Bind("banner_left", &model.bannerLeft);
                                                     c.Bind("banner_src", &model.bannerSrc);
                                                     c.Bind("banner_opacity", &model.bannerOpacity);
                                                     c.Bind("rank_label_left", &model.rankLabelLeft);
                                                     c.Bind("rank_details_visible", &model.rankDetailsVisible);
                                                     c.Bind("rank_letter_src", &model.rankLetterSrc);
                                                     c.Bind("exp_digits", &model.expDigits);
                                                     c.Bind("hud_visible", &model.hudVisible);
                                                     c.Bind("altars", &model.altars);
                                                     c.Bind("dark_elf_icon_src", &model.darkElfIconSrc);
                                                     c.Bind("dark_elf_text", &model.darkElfText);
                                                     c.Bind("balgass_visible", &model.balgassVisible);
                                                     c.Bind("balgass_text", &model.balgassText);
                                                     c.Bind("balgass_bar_width", &model.balgassBarWidth);
                                                     c.Bind("balgass_bar_rect", &model.balgassBarRect);
                                                     c.Bind("timer_digits", &model.timerDigits);
                                                     c.Bind("timer_urgent", &model.timerUrgent);
                                                     c.Bind("statue_bar_left", &model.statueBarLeft);
                                                     c.Bind("statue_bar_width", &model.statueBarWidth);
                                                     c.Bind("statue_bar_rect", &model.statueBarRect);
                                                     c.Bind("notices", &model.notices);
                                                 });
    if (modelCreated)
        m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(CryWolfContext(), "Data/Interface/RmlUi/crywolf.rml");
}

void mu::ui::window::CCryWolf::ReloadRmlTheme()
{
    if (!m_pRmlDoc)
        return;
    Rml::Context* context = CryWolfContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
}

// The original Render()'s first part: the result banner slides in from the left, holds for 400
// frames, slides out to the right and then opens the result dialog; the rank table stays, its
// label slides in, then the rank letter and the experience show. Frame-counted like the original.
void mu::ui::window::CCryWolf::SyncResult(CryWolfRmlModel& updated)
{
    constexpr int TotDelay = 400;

    if (Suc_Or_Fail == 1)
    {
        Delay++;
        if (Delay >= TotDelay)
        {
            Delay = 0;
            Suc_Or_Fail = 0;
        }
    }

    if (Suc_Or_Fail < 0)
        return;

    float A_Value = 0.f;
    const int aa = (Delay * 2) % 140;
    if (aa > 70)
        A_Value = 1.f - (static_cast<float>(aa - 70) * 0.01f);
    else
        A_Value = 0.3f + (static_cast<float>(aa) * 0.01f);

    if (Delay_Add_inter <= 0)
        Delay_Add_inter = 0;
    else
        Delay_Add_inter -= 15;

    updated.resultVisible = true;
    updated.bannerSrc = InterfaceImage(Add_Num == 11 ? "icon_success.tga" : "icon_failure.tga");
    updated.bannerOpacity = std::clamp(A_Value, 0.f, 1.f);
    if ((Delay * 15) > 479)
    {
        updated.bannerLeft = 150.f;
    }
    else if (Suc_Or_Fail == 0)
    {
        updated.bannerLeft = static_cast<float>(150 + (Delay * 15));

        Delay++;
        if ((Delay * 15) > 479)
        {
            Delay = 0;
            Suc_Or_Fail = -1;

            Delay_Add_inter = 390;
            View_End_Result = true;

            mu::ui::window::GenericDialogConfig cfg;
            wchar_t szResultText[300];
            mu_swprintf(szResultText, L"%ls    %ls    %ls    %ls", I18N::Game::Rank, I18N::Game::Character,
                        I18N::Game::Class, I18N::Game::Score);
            cfg.lines.push_back({szResultText, false});
            for (int i = 0; i < 5; i++)
            {
                if (HeroScore[i] == -1)
                    continue;
                mu_swprintf(szResultText, L"%d      %ls      %ls      %d", i + 1, HeroName[i],
                            gCharacterManager.GetCharacterClassText(HeroClass[i]), HeroScore[i]);
                cfg.lines.push_back({szResultText, false});
            }
            cfg.lines.push_back({L"    ", false});
            cfg.lines.push_back({L"    ", false});
            cfg.lines.push_back({L"    ", false});
            cfg.lines.push_back({L"    ", false});
            if (View_Suc_Or_Fail == 1)
            {
                cfg.lines.push_back({I18N::Game::MonsterStrengthDecreased10, false});
                cfg.lines.push_back({I18N::Game::_5IncreaseInCastleAndArenaInvitationCombineRate, false});
            }
            else
            {
                cfg.lines.push_back({I18N::Game::AllNPCsInCrywolfHaveBeenDeleted, false});
            }
            mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
        }
    }
    else
    {
        updated.bannerLeft = static_cast<float>(-329 + (Delay * 15));
    }

    updated.rankLabelLeft = static_cast<float>(250 + Delay_Add_inter);
    updated.rankDetailsVisible = Delay_Add_inter == 0;
    if (!updated.rankDetailsVisible)
        return;

    // icon_Rank_D, C, B, A, S for ranks 0 to 4. The original drew whatever image followed for any
    // other rank (a digit); the port draws no letter then.
    static const char* const RankLetters[] = {"icon_Rank_D.tga", "icon_Rank_C.tga", "icon_Rank_B.tga",
                                              "icon_Rank_A.tga", "icon_Rank_S.tga"};
    updated.rankLetterSrc = Rank < 5 ? InterfaceImage(RankLetters[Rank]) : Rml::String();

    // Nine digits, zero-padded: the experience's last nine digits (none for a negative value).
    int Exp_val[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    int Val = 0;
    for (int i = 0; i < 9; i++)
    {
        if (Exp < Val)
            break;
        if (Val > 0)
            Exp_val[8 - i] = (Exp / Val) % 10;
        else
        {
            Exp_val[8 - i] = Exp % 10;
            Val = 1;
        }
        Val *= 10;
    }
    for (int i = 0; i < 9; i++)
    {
        const Rml::String file = "icon_Rank_" + std::to_string(Exp_val[i]) + ".tga";
        updated.expDigits.push_back({true, InterfaceImage(file.c_str()), TexelRect(0.f, 0.f, 15.f, 19.f)});
    }
}

// The original Render()'s battle HUD, for the ready and start states.
void mu::ui::window::CCryWolf::SyncHud(CryWolfRmlModel& updated)
{
    if (M34CryWolf1st::Get_State_Only_Elf() == false)
        return;

    updated.hudVisible = true;

    // All five altars travel, shown or not, so each stays on its own place in the theme's row
    // whatever the others are doing.
    for (int ia = 0; ia < 5; ia++)
    {
        const BYTE Use = (m_AltarState[ia] & 0xf0) >> 4;
        const BYTE State = (m_AltarState[ia] & 0x0f);
        const char* file = nullptr;
        if (Use == CRYWOLF_ALTAR_STATE_CONTRACTED)
            file = State == 1   ? "in_main_number1_1.tga"
                   : State == 2 ? "in_main_number2_1.tga"
                                : "in_main_number0_2.tga";
        else if (State == 1)
            file = "in_main_number1.tga";
        else if (State == 2)
            file = "in_main_number2.tga";
        updated.altars.push_back({file != nullptr, file != nullptr ? InterfaceImage(file) : Rml::String(),
                                  TexelRect(0.f, 0.f, 12.f, 12.f)});
    }

    updated.darkElfIconSrc = InterfaceImage(Dark_elf_Num == 0 ? "in_main_icon_dl2.tga" : "in_main_icon_dl1.tga");
    wchar_t Text[300];
    mu_swprintf(Text, I18N::Game::DarkElfD12, Dark_elf_Num);
    updated.darkElfText = StringUtils::WideToNarrow(Text);

    // Balgass shows 21 frames after he appears, and leaves at once.
    if (View_Bal == true)
    {
        if (Deco_Insert < 21.f)
        {
            Deco_Insert += 1.f;
        }
        else
        {
            updated.balgassVisible = true;
            updated.balgassText = StringUtils::WideToNarrow(I18N::Game::Balgass);
            const float Hp = ((67.f / 100.f) * (float)Val_Hp);
            updated.balgassBarWidth = (68.f / 100.f) * (float)Val_Hp;
            updated.balgassBarRect = TexelRect(0.f, 0.f, Hp, 8.f);
        }
    }
    else if (Deco_Insert > 0.f)
    {
        Deco_Insert -= 1.f;
    }

    if (m_bTimeStart == true && m_CrywolfState == CRYWOLF_STATE_START)
    {
        updated.timerUrgent = View_Bal;
        m_iSecond = m_iSecond - static_cast<int>(GetTickCount() - m_dwSyncTime);
        m_dwSyncTime = GetTickCount();

        // The original drew a negative second count (a minus sign's garbage cell) when no new time
        // arrived in time; the seconds stop at 0 here.
        const int seconds = std::max(m_iSecond, 0) / 1000;
        if (m_iMinute < 10)
            AddTimerDigits(updated.timerDigits, {570.f, 402.f}, 0);
        AddTimerDigits(updated.timerDigits, {580.f, 402.f}, m_iMinute);
        if (seconds < 10)
            AddTimerDigits(updated.timerDigits, {597.f, 402.f}, 0);
        AddTimerDigits(updated.timerDigits, {607.f, 402.f}, seconds);

        if (m_iMinute <= 0 && m_iSecond <= 0)
        {
            m_bTimeStart = false;
            m_iSecond = 0;
            m_icntTime = 0;
        }
    }
    else
    {
        updated.timerUrgent = false;
        AddTimerDigits(updated.timerDigits, {570.f, 402.f}, 0);
        AddTimerDigits(updated.timerDigits, {580.f, 402.f}, 0);
        AddTimerDigits(updated.timerDigits, {597.f, 402.f}, 0);
        AddTimerDigits(updated.timerDigits, {607.f, 402.f}, 0);
    }

    // The statue's shield: the bar's right part, shortened from the left as the shield drops.
    const int HpS = 100 - m_StatueHP;
    const float Hp = ((88.f / 100.f) * (float)HpS);
    const float nx = ((89.f / 100.f) * (float)HpS);
    updated.statueBarLeft = 548.f + nx;
    updated.statueBarWidth = 89.f - nx;
    updated.statueBarRect = TexelRect(Hp, 0.f, (88.f / 100.f) * (float)(100 - HpS), 29.f);

    std::wstring texts[4];
    if (M34CryWolf1st::AdvanceNoticeTexts(texts))
    {
        for (int i = 0; i < 4; i++)
        {
            updated.notices.push_back({StringUtils::WideToNarrow(texts[i].c_str()), i == 0});
        }
    }
}

void mu::ui::window::CCryWolf::SyncView()
{
    BuildRmlUi();
    if (!m_pRmlDoc)
        return;

    CryWolfRmlModel updated;
    const bool onMap = IsVisible() && M34CryWolf1st::IsCyrWolf1st();
    if (onMap)
    {
        SyncResult(updated);
        SyncHud(updated);
    }

    const bool visible = updated.resultVisible || updated.hudVisible;
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_pRmlDoc, visible);
    if (!visible)
        return;

    // CManager scopes LayoutMode::Hud around the window: W/640 x H/480, no offset.
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    updated.scaleX = transform.scaleX;
    updated.scaleY = transform.scaleY;
    updated.inverseScaleX = 1.0f / transform.scaleX;
    updated.inverseScaleY = 1.0f / transform.scaleY;
    updated.normalTextPx = UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Normal, transform);
    updated.boldTextPx = UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Bold, transform);
    updated.boldLinePx =
        static_cast<float>(CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Bold)) * transform.scaleY;

    CryWolfRmlModel& model = m_RmlBinder.GetModel();
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::scaleX, "scale_x", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::scaleY, "scale_y", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::inverseScaleX, "inverse_scale_x", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::inverseScaleY, "inverse_scale_y", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::normalTextPx, "normal_text_px", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::boldTextPx, "bold_text_px", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::boldLinePx, "bold_line_px", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::resultVisible, "result_visible", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::bannerLeft, "banner_left", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::bannerSrc, "banner_src", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::bannerOpacity, "banner_opacity", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::rankLabelLeft, "rank_label_left", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::rankDetailsVisible, "rank_details_visible", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::rankLetterSrc, "rank_letter_src", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::hudVisible, "hud_visible", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::darkElfIconSrc, "dark_elf_icon_src", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::darkElfText, "dark_elf_text", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::balgassVisible, "balgass_visible", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::balgassText, "balgass_text", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::balgassBarWidth, "balgass_bar_width", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::balgassBarRect, "balgass_bar_rect", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::timerUrgent, "timer_urgent", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::statueBarLeft, "statue_bar_left", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::statueBarWidth, "statue_bar_width", updated);
    SyncFieldFrom(m_RmlBinder, &CryWolfRmlModel::statueBarRect, "statue_bar_rect", updated);
    auto syncSprites = [&](std::vector<CryWolfSpriteEntry> CryWolfRmlModel::* field, const char* name)
    {
        if (SameSprites(model.*field, updated.*field))
            return;
        model.*field = std::move(updated.*field);
        m_RmlBinder.MarkDirty(name);
    };
    syncSprites(&CryWolfRmlModel::timerDigits, "timer_digits");
    auto syncImages = [&](std::vector<CryWolfImageEntry> CryWolfRmlModel::* field, const char* name)
    {
        if (model.*field == updated.*field)
            return;
        model.*field = std::move(updated.*field);
        m_RmlBinder.MarkDirty(name);
    };
    syncImages(&CryWolfRmlModel::expDigits, "exp_digits");
    syncImages(&CryWolfRmlModel::altars, "altars");
    if (model.notices != updated.notices)
    {
        model.notices = std::move(updated.notices);
        m_RmlBinder.MarkDirty("notices");
    }
}

float mu::ui::window::CCryWolf::ConvertX(float x)
{
    return x * (float)WindowWidth / (float)REFERENCE_WIDTH;
}

float mu::ui::window::CCryWolf::ConvertY(float y)
{
    return y * (float)WindowHeight / (float)REFERENCE_HEIGHT;
}

bool mu::ui::window::CCryWolf::Render(int Posx, int Posy, int nPosx, int nPosy, float u, float v, float su, float sv, int Index, bool Scale, bool StartScale, float Alpha)
{
    const BYTE alpha = static_cast<BYTE>(std::clamp(Alpha, 0.f, 1.f) * 255.f);
    RenderImage(IMAGE_MVP_INTERFACE + Index, Posx, Posy, nPosx, nPosy, u, v, su, sv,
        RGBA(255, 255, 255, alpha));

    return true;
}

void mu::ui::window::CCryWolf::SetTime(int iHour, int iMinute)
{
    m_iHour = iHour;
    if (m_iMinute != iMinute)
    {
        m_iSecond = 60000;
        m_icntTime = 1;
        m_bTimeStart = true;
    }
    else if (m_icntTime == 1)
    {
        m_iSecond = 40000;
        m_icntTime++;
    }
    else if (m_icntTime == 2)
    {
        m_iSecond = 20000;
        m_icntTime = 0;
    }
    m_iMinute = iMinute;
}

void mu::ui::window::CCryWolf::InitTime()
{
    m_dwSyncTime = GetTickCount();
}

void mu::ui::window::CCryWolf::LoadImages()
{
    LoadBitmap(L"Interface\\in_bar.tga", IMAGE_MVP_INTERFACE, GL_LINEAR);
    LoadBitmap(L"Interface\\in_bar2.jpg", IMAGE_MVP_INTERFACE + 1, GL_LINEAR);
    LoadBitmap(L"Interface\\in_deco.tga", IMAGE_MVP_INTERFACE + 2, GL_LINEAR);
    LoadBitmap(L"Interface\\in_main-New.tga", IMAGE_MVP_INTERFACE + 3, GL_LINEAR);
    LoadBitmap(L"Interface\\in_main_icon_bal1.tga", IMAGE_MVP_INTERFACE + 4, GL_LINEAR);
    LoadBitmap(L"Interface\\in_main_icon_dl1.tga", IMAGE_MVP_INTERFACE + 5, GL_LINEAR);
    LoadBitmap(L"Interface\\in_main_icon_dl2.tga", IMAGE_MVP_INTERFACE + 6, GL_LINEAR);
    LoadBitmap(L"Interface\\in_main_number1.tga", IMAGE_MVP_INTERFACE + 7, GL_LINEAR);
    LoadBitmap(L"Interface\\in_main_number2.tga", IMAGE_MVP_INTERFACE + 8, GL_LINEAR);
    LoadBitmap(L"Interface\\in_main2-New.tga", IMAGE_MVP_INTERFACE + 9, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_failure.tga", IMAGE_MVP_INTERFACE + 10, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_success.tga", IMAGE_MVP_INTERFACE + 11, GL_LINEAR);
    LoadBitmap(L"Interface\\t_main-New.tga", IMAGE_MVP_INTERFACE + 12, GL_LINEAR);
    LoadBitmap(L"Interface\\m_b_no1.tga", IMAGE_MVP_INTERFACE + 13, GL_LINEAR);
    LoadBitmap(L"Interface\\m_b_no2.tga", IMAGE_MVP_INTERFACE + 14, GL_LINEAR);
    LoadBitmap(L"Interface\\m_b_no3.tga", IMAGE_MVP_INTERFACE + 15, GL_LINEAR);
    LoadBitmap(L"Interface\\m_b_ok1.tga", IMAGE_MVP_INTERFACE + 16, GL_LINEAR);
    LoadBitmap(L"Interface\\m_b_ok2.tga", IMAGE_MVP_INTERFACE + 17, GL_LINEAR);
    LoadBitmap(L"Interface\\m_b_ok3.tga", IMAGE_MVP_INTERFACE + 18, GL_LINEAR);
    LoadBitmap(L"Interface\\m_b_yes1.tga", IMAGE_MVP_INTERFACE + 19, GL_LINEAR);
    LoadBitmap(L"Interface\\m_b_yes2.tga", IMAGE_MVP_INTERFACE + 20, GL_LINEAR);
    LoadBitmap(L"Interface\\m_b_yes3.tga", IMAGE_MVP_INTERFACE + 21, GL_LINEAR);
    LoadBitmap(L"Interface\\m_main.tga", IMAGE_MVP_INTERFACE + 22, GL_LINEAR);
    LoadBitmap(L"Interface\\in_main_number1_1.tga", IMAGE_MVP_INTERFACE + 23, GL_LINEAR);
    LoadBitmap(L"Interface\\in_main_number2_1.tga", IMAGE_MVP_INTERFACE + 24, GL_LINEAR);
    LoadBitmap(L"Interface\\in_main_number0_2.tga", IMAGE_MVP_INTERFACE + 25, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_table.tga", IMAGE_MVP_INTERFACE + 26, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_rank.tga", IMAGE_MVP_INTERFACE + 27, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_D.tga", IMAGE_MVP_INTERFACE + 28, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_C.tga", IMAGE_MVP_INTERFACE + 29, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_B.tga", IMAGE_MVP_INTERFACE + 30, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_A.tga", IMAGE_MVP_INTERFACE + 31, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_S.tga", IMAGE_MVP_INTERFACE + 32, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_0.tga", IMAGE_MVP_INTERFACE + 33, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_1.tga", IMAGE_MVP_INTERFACE + 34, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_2.tga", IMAGE_MVP_INTERFACE + 35, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_3.tga", IMAGE_MVP_INTERFACE + 36, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_4.tga", IMAGE_MVP_INTERFACE + 37, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_5.tga", IMAGE_MVP_INTERFACE + 38, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_6.tga", IMAGE_MVP_INTERFACE + 39, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_7.tga", IMAGE_MVP_INTERFACE + 40, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_8.tga", IMAGE_MVP_INTERFACE + 41, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_9.tga", IMAGE_MVP_INTERFACE + 42, GL_LINEAR);
    LoadBitmap(L"Interface\\icon_Rank_exp.tga", IMAGE_MVP_INTERFACE + 43, GL_LINEAR);
    LoadBitmap(L"Interface\\m_main_rank.tga", IMAGE_MVP_INTERFACE + 44, GL_LINEAR);
}

void mu::ui::window::CCryWolf::UnloadImages()
{
    DeleteBitmap(IMAGE_MVP_INTERFACE);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 1);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 2);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 3);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 4);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 5);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 6);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 7);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 8);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 9);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 10);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 11);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 12);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 13);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 14);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 15);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 16);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 17);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 18);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 19);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 20);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 21);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 22);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 23);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 24);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 25);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 26);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 27);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 28);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 29);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 30);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 31);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 32);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 33);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 34);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 35);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 36);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 37);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 38);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 39);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 40);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 41);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 42);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 43);
    DeleteBitmap(IMAGE_MVP_INTERFACE + 44);
}
