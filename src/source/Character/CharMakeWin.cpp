//*****************************************************************************
// File: CharMakeWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "CharMakeWin.h"
#include "Core/Input/Input.h"
#include "UI/Core/SceneUICoordinator.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Audio/DSPlaySound.h"
#include "Engine/AI/ZzzAI.h"
#include "Scenes/SceneCore.h"
#include "UI/Widgets/UIControls.h"
#include "I18N/All.h"
#include "Core/Globals/_enum.h"

#include "App/Platform/Windows/Local.h"
#include "CharacterManager.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cwchar>
#include <iterator>
#include <string>

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Core/Utilities/StringUtils.h"
#include "UI/Scaling/UITransform.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Event.h>

namespace
{
    constexpr std::array<int, MAX_CLASS> kClassButtonTextIds{
        20, 21, 22, 23, 24, 1687, 3150
    };

    constexpr std::size_t kMinCharacterNameLength = 4;

    constexpr int kSummonerDescriptionTextId = 1690;
    constexpr int kRageFighterDescriptionTextId = 3152;
    constexpr int kDefaultDescriptionBase = 1705;

    constexpr int kStatLabelBaseId = 1701;
    constexpr int kStatLineSpacing = 17;
    constexpr int kStatYOffset = 10;
    constexpr int kStatValueOffset = 54;
    constexpr int kStatTextOffsetX = 22;
    constexpr int kDarkLordStatHeight = 96;
    constexpr int kDefaultStatHeight = 80;
    constexpr const wchar_t* kDarkLordLeadershipStatValue = L"25";
    constexpr int kDarkLordLeadershipTextId = 1738;

    constexpr int kJobButtonHeight = 26;
    constexpr int kJobButtonStartY = 131;
    constexpr int kJobButtonSummonerRow = 3;
    constexpr int kJobButtonRageFighterY = 246;
    constexpr int kDescSpriteOffsetY = 355;
    constexpr int kStatSpriteOffsetY = 24;
    constexpr int kDescriptionTextOffsetX = 10;
    constexpr int kDescriptionTextOffsetY = 12;
    constexpr int kDescriptionLineSpacing = 19;

    struct ClassStats
    {
        std::array<const wchar_t*, 4> values;
    };

    constexpr std::array<ClassStats, MAX_CLASS> kClassStatTable{ {
        ClassStats{ { L"18", L"18", L"15", L"30" } }, // Knight
        ClassStats{ { L"28", L"20", L"25", L"10" } }, // Wizard
        ClassStats{ { L"22", L"25", L"20", L"15" } }, // Elf
        ClassStats{ { L"26", L"26", L"26", L"26" } }, // Magic Gladiator
        ClassStats{ { L"26", L"20", L"20", L"15" } }, // Dark Lord
        ClassStats{ { L"21", L"21", L"18", L"23" } }, // Summoner
        ClassStats{ { L"32", L"27", L"25", L"20" } }, // Rage Fighter
    } };

    struct ClassRenderParameters
    {
        bool overrideAngle;
        float angleX;
        float angleY;
        float angleZ;
        float scale;
        float positionOffsetX;
        float positionOffsetZ;
    };

    constexpr ClassRenderParameters GetRenderParameters(CLASS_TYPE classType)
    {
        switch (classType)
        {
        case CLASS_KNIGHT:
            return { true, 0.0f, 0.0f, -12.0f, 6.05f, 0.0f, 0.0f };
        case CLASS_WIZARD:
            return { true, 0.0f, 0.0f, -40.0f, 5.9f, 0.0f, 0.0f };
        case CLASS_ELF:
            return { true, 8.0f, 0.0f, 5.0f, 9.1f, 4.8f, 0.0f };
        case CLASS_DARK:
            return { true, 8.0f, 0.0f, -13.0f, 6.0f, 0.0f, 1.8f };
        case CLASS_DARK_LORD:
            return { true, 8.0f, 0.0f, -18.0f, 6.0f, 0.0f, 0.0f };
        case CLASS_SUMMONER:
            return { true, 2.0f, 0.0f, 2.0f, 9.1f, 4.8f, 4.0f };
        case CLASS_RAGEFIGHTER:
            return { false, 0.0f, 0.0f, 0.0f, 6.0f, 9.8f, -7.5f };
        default:
            return { false, 0.0f, 0.0f, 0.0f, 6.0f, 0.0f, 0.0f };
        }
    }

    constexpr int ResolveDescriptionTextId(CLASS_TYPE selectedClass)
    {
        if (selectedClass == CLASS_SUMMONER)
            return kSummonerDescriptionTextId;
        if (selectedClass == CLASS_RAGEFIGHTER)
            return kRageFighterDescriptionTextId;
        return kDefaultDescriptionBase + selectedClass;
    }
}

extern int g_iChatInputType;

void MoveCharacterCamera(vec3_t Origin, vec3_t Position, vec3_t Angle);

CCharMakeWin g_CharMakeWin;

CCharMakeWin::CCharMakeWin()
{
}

CCharMakeWin::~CCharMakeWin()
{
    Release();
}

void CCharMakeWin::Create()
{
    Release();

    CInput& rInput = CInput::Instance();

    // Real, visible full-screen dimming overlay -- see this class's header comment for why this
    // one (unlike CMsgWin/CSysMenuWin) genuinely needs to render. Mirrors CWin::Create()'s own
    // default-nTexID=-1 path exactly (Win.cpp).
    m_sprBg.Create(rInput.GetScreenWidth(), rInput.GetScreenHeight(), -1, 0, NULL, 0, 0, false);
    m_sprBg.SetAlpha(128);
    m_sprBg.SetColor(0, 0, 0);

    m_asprBack[CMW_SPR_STAT].Create(108, 80);

    m_asprBack[CMW_SPR_DESC].Create(454, 51);

    for (int spriteIndex = CMW_SPR_STAT; spriteIndex < CMW_SPR_MAX; ++spriteIndex)
    {
        m_asprBack[spriteIndex].SetAlpha(143);
        m_asprBack[spriteIndex].SetColor(0, 0, 0);
    }

    std::fill(&m_aszJobDesc[0][0],
        &m_aszJobDesc[0][0] + (CMW_DESC_LINE_MAX * CMW_DESC_ROW_MAX), L'\0');
    m_nDescLine = 0;

    m_nSelJob = CLASS_KNIGHT;
    m_aJobState[m_nSelJob].checked = true;

    // RmlUi migration -- guarded the same way every other migrated window's Create() is
    // (re-entrant on resolution change), so the document/model/array size are set up once, ever.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        BuildRmlUi();
        UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });
    }

    CSceneUICoordinator::Instance().GetNewStyleMng().AddUIObj(mu::ui::window::INTERFACE_CHAR_MAKE, this);

    UpdateDisplay();
    Show(false);
}

void CCharMakeWin::BuildRmlUi()
{
    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "char_make",
        [this](Rml::DataModelConstructor& c, CharMakeRmlModel& model)
        {
            model.jobs.resize(MAX_CLASS);

            // RegisterStruct()/RegisterArray() run against this Create() call's own
            // Rml::DataTypeRegister (RmlModelBinder.h), not one shared across calls -- so this
            // must re-run in full every time this registration function runs, including from
            // ReloadRmlTheme(); a guard skipping it on a later call would register nothing on
            // that call's (fresh, otherwise-empty) register and break the c.Bind() below.
            auto job = c.RegisterStruct<JobButtonEntry>();
            job.RegisterMember("rel_left", &JobButtonEntry::relLeft);
            job.RegisterMember("rel_top", &JobButtonEntry::relTop);
            job.RegisterMember("checked", &JobButtonEntry::checked);
            job.RegisterMember("disabled", &JobButtonEntry::disabled);
            job.RegisterMember("label", &JobButtonEntry::label);
            c.RegisterArray<std::vector<JobButtonEntry>>();

            c.Bind("jobs", &model.jobs);
            c.Bind("dark_lord_extra", &model.darkLordExtra);
            c.Bind("stat_label0", &model.statLabel0);
            c.Bind("stat_label1", &model.statLabel1);
            c.Bind("stat_label2", &model.statLabel2);
            c.Bind("stat_label3", &model.statLabel3);
            c.Bind("stat_label4", &model.statLabel4);
            c.Bind("stat_value0", &model.statValue0);
            c.Bind("stat_value1", &model.statValue1);
            c.Bind("stat_value2", &model.statValue2);
            c.Bind("stat_value3", &model.statValue3);
            c.Bind("desc_title", &model.descTitle);
            c.Bind("desc_line1", &model.descLine1);
            c.Bind("desc_line2", &model.descLine2);
            c.Bind("ok_label", &model.okLabel);
            c.Bind("cancel_label", &model.cancelLabel);

            c.BindEventCallback("charmake_select_job",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                {
                    if (arguments.size() == 1)
                        RmlClickJob(arguments[0].Get<int>(-1));
                });
            c.BindEventCallback("charmake_ok_click",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickOk(); });
            c.BindEventCallback("charmake_cancel_click",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickCancel(); });
        });

    if (modelCreated)
        m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/char_make.rml");
}

void CCharMakeWin::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return;

    // See CLoginWin::ReloadRmlTheme()'s comment on why this reads m_pRmlDoc directly.
    const bool wasVisible = m_pRmlDoc->IsVisible();
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
    SetPosition(m_nOriginX, m_nOriginY);
    UpdateDisplay();
    if (wasVisible) { SyncRmlModel(); if (m_pRmlDoc) m_pRmlDoc->Show(); }
}

void CCharMakeWin::Release()
{
    for (int i = 0; i < CMW_SPR_MAX; ++i)
        m_asprBack[i].Release();
    m_sprBg.Release();

    // See CLoginMainWin::PreRelease()'s identical comment.
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();

    // Base-class visibility reset, NOT the full CCharMakeWin::Show(false) override -- that touches
    // g_pSingleTextInputBox, which isn't guaranteed constructed yet the first time this runs
    // (CreateLoginScene() calls Release() on every window up front, including this one, even though
    // it only ever Create()s/Shows this window later, in CreateCharacterScene()). Every other
    // window's Create() ends with an unconditional Show(false) (see CCreditWin::Create()'s own
    // comment on this), which is what normally keeps CObject's IsVisible() correct before the first
    // real Show(true) -- but this window's Create() never even runs during the login scene, so
    // without this its CObject-level m_bRender default (true, from the constructor) stood
    // uncorrected, and IsVisible() reported true. CSceneUICoordinator::Update()'s ESC-opens-
    // system-menu check reads exactly this window's IsVisible() while on the login scene, so a
    // fresh process's first ESC press there fell through as if this dialog were covering the
    // screen, never opening the system menu.
    mu::ui::window::CObject::Show(false);
}

void CCharMakeWin::SetPosition(int nXCoord, int nYCoord)
{
    m_nOriginX = nXCoord;
    m_nOriginY = nYCoord;

    const int baseX = nXCoord + 346;
    m_asprBack[CMW_SPR_STAT].SetPosition(baseX, nYCoord + kStatSpriteOffsetY);

    constexpr int buttonHeight = kJobButtonHeight;
    int baseY = nYCoord + kJobButtonStartY;

    auto& jobs = m_RmlBinder.GetModel().jobs;
    auto pushJobPos = [&](int classIndex, int screenX, int screenY)
    {
        if (jobs.empty()) return; // model not created yet (RmlUi not up) -- nothing to push
        jobs[classIndex].relLeft = screenX - nXCoord;
        jobs[classIndex].relTop = screenY - nYCoord;
    };

    for (int classIndex = 0; classIndex < 3; ++classIndex)
    {
        const int y = baseY + classIndex * buttonHeight;
        pushJobPos(classIndex, baseX, y);
    }

    pushJobPos(CLASS_SUMMONER, baseX, baseY + kJobButtonSummonerRow * buttonHeight);

    baseY = nYCoord + kJobButtonRageFighterY;
    pushJobPos(CLASS_RAGEFIGHTER, baseX, baseY);

    for (int classIndex = CLASS_DARK; classIndex <= CLASS_DARK_LORD; ++classIndex)
    {
        const int row = (classIndex - CLASS_DARK) + 1;
        const int y = baseY + row * buttonHeight;
        pushJobPos(classIndex, baseX, y);
    }
    if (!jobs.empty())
        m_RmlBinder.MarkDirty("jobs");

    m_asprBack[CMW_SPR_DESC].SetPosition(nXCoord, nYCoord + kDescSpriteOffsetY);

    // RmlUi panel: only its own screen origin is still pushed here (genuine placement, same
    // carve-out as every other migrated window) -- width/height and every static child position
    // (stat_panel/desc_panel/input_frame/btn_ok/btn_cancel) moved to char_make.rcss (both themes),
    // since none of those five ever actually varies at runtime. #panel stays fixed-px rather than
    // joining the other migrated dialogs' dp auto-fit because the live 3D character-preview
    // viewport below reads this window's own real unscaled origin (m_nOriginX/Y) directly.
    if (m_pRmlDoc)
    {
        if (Rml::Element* panel = m_pRmlDoc->GetElementById("panel"))
        {
            panel->SetProperty("left", std::to_string(nXCoord) + "px");
            panel->SetProperty("top", std::to_string(nYCoord) + "px");
        }
    }
}

void CCharMakeWin::Show(bool bShow)
{
    mu::ui::window::CObject::Show(bShow);

    m_sprBg.Show(bShow);

    for (int i = 0; i < CMW_SPR_MAX; ++i)
        m_asprBack[i].Show(bShow);

    if (bShow)
    {
        InputTextWidth = 73;
        ClearInput();
        InputEnable = true;
        InputNumber = 1;
        InputTextMax[0] = MAX_USERNAME_SIZE;
        if (g_iChatInputType == 1)
        {
            // Text color left at InputBoxConfig's default (opaque black) rendered nearly invisible
            // against the input-frame's dark fill -- matches LoginWin.cpp's own m_pUsernameInputBox/
            // m_pPasswordInputBox light-cream convention (SetTextColor(255,255,230,210)) instead.
            g_pSingleTextInputBox->Configure({
                .textLimit = 10,
                .textAlpha = 255,
                .textR = 255,
                .textG = 230,
                .textB = 210,
            });
            g_pSingleTextInputBox->GiveFocus();
        }
    }
    else
    {
        if (g_iChatInputType == 1)
        {
            g_pSingleTextInputBox->SetText(nullptr);
            g_pSingleTextInputBox->SetState(UISTATE_HIDE);
        }
    }

    if (m_pRmlDoc)
    {
        if (bShow) { SyncRmlModel(); m_pRmlDoc->Show(); }
        else       m_pRmlDoc->Hide();
    }
}

void CCharMakeWin::UpdateDisplay()
{
    for (auto& state : m_aJobState)
        state.enabled = true;

#ifdef PBG_ADD_CHARACTERCARD
    for (int i = 0; i < CLASS_CHARACTERCARD_TOTALCNT; ++i)
    {
        if (!g_CharCardEnable.bCharacterEnable[i])
            m_aJobState[i + CLASS_DARK].enabled = false;
    }
#else //PBG_ADD_CHARACTERCARD
    m_aJobState[CLASS_SUMMONER].enabled = true;
#endif //PBG_ADD_CHARACTERCARD

    const bool isDarkLord = (m_nSelJob == CLASS_DARK_LORD);
    m_asprBack[CMW_SPR_STAT].SetSize(0, isDarkLord ? kDarkLordStatHeight : kDefaultStatHeight, Y);

    const int descriptionTextId = ResolveDescriptionTextId(m_nSelJob);
    m_nDescLine = ::SeparateTextIntoLines(
        I18N::Game::Lookup(descriptionTextId),
        m_aszJobDesc[0],
        CMW_DESC_LINE_MAX,
        CMW_DESC_ROW_MAX);

    SelectCreateCharacter();
}

void CCharMakeWin::RmlClickJob(int nClassIndex)
{
    if (nClassIndex < 0 || nClassIndex >= MAX_CLASS)
        return;
    if (!m_aJobState[nClassIndex].enabled)
        return;
    SelectJob(nClassIndex);
}

void CCharMakeWin::SelectJob(int classIndex)
{
    for (auto& state : m_aJobState)
        state.checked = false;
    m_aJobState[classIndex].checked = true;

    if (m_nSelJob != classIndex)
    {
        m_nSelJob = static_cast<CLASS_TYPE>(classIndex);
        UpdateDisplay();
    }
}

void CCharMakeWin::SubmitCreateCharacter()
{
    RequestCreateCharacter();
}

void CCharMakeWin::CloseDialog()
{
    Show(false);
}

bool CCharMakeWin::Update()
{
    if (!IsVisible())
        return true;

    // RmlUi's own resolved position for #input_text_anchor (char_make.rml/.rcss, both themes) is
    // the single source of truth for where the legacy-drawn name text starts -- same
    // GetElementById+GetAbsoluteOffset pattern as MainFrameWindow.cpp's item-hotkey/skill-list
    // anchors, replacing the old kInputSpriteOffsetY/kInputTextOffsetX/kInputTextOffsetY hardcoded
    // constants and the vestigial m_asprBack[CMW_SPR_INPUT] sprite that only ever existed to hold
    // that math (it was never rendered).
    //
    // Re-read every frame here, NOT once in SetPosition() -- SetPosition() pushes #panel's new
    // left/top via SetProperty(), but RmlUi doesn't resolve that into a real layout synchronously
    // (it resolves on this context's next Update() pass); a same-call GetAbsoluteOffset() right
    // after Create()+SetPosition() (the dialog's very first frame) reads pre-layout garbage, and
    // since SetPosition() is otherwise only called once more on an explicit resolution change,
    // that stale reading would otherwise never self-correct (this was a real bug, found via live
    // testing: the caret rendered in the wrong spot until a resolution change forced a second
    // SetPosition() call, long after RmlUi had since caught up). Reading here instead means at
    // worst a one-frame-late position on the very first frame this dialog opens, self-correcting
    // immediately after -- imperceptible, and this dialog's layout is otherwise static.
    if (m_pRmlDoc)
    {
        if (Rml::Element* pAnchor = m_pRmlDoc->GetElementById("input_text_anchor"))
        {
            m_fInputTextX = pAnchor->GetAbsoluteOffset().x;
            m_fInputTextY = pAnchor->GetAbsoluteOffset().y;
        }
    }

    if (g_iChatInputType == 1)
    {
        // Real pixels, not divided by g_fScreenRate_x/y -- CUITextInputBox::Render() rescales the
        // position it's given via ConvertPositionX/Y using *whatever transform is active when
        // Render() runs* (see RenderTextOnTop()'s own comment), a fundamentally different contract
        // than CSprite's "store real pixels, ignore the transform entirely". Dividing here relied
        // on a later multiply landing under the exact same ambient transform to cancel it back
        // out -- exactly the bug LoginWin.cpp's own SetPosition() already found and fixed for its
        // m_pUsernameInputBox/m_pPasswordInputBox (same class, same contract). GetAbsoluteOffset()
        // already returns real pixels (char_make.rcss is plain px, not dp -- RmlUi's
        // DensityIndependentPixelRatio only touches dp/ppi units, confirmed via
        // ElementStyle.cpp's ComputeLength), so no conversion at all is needed here.
        g_pSingleTextInputBox->SetPosition(int(m_fInputTextX), int(m_fInputTextY));
    }

    // A CMsgWin validation-error dialog (name too short/invalid/special) can be shown on top of
    // this one without hiding it first (see GetLayerDepth()'s own comment) -- skip this window's
    // own click/key consequences while that's up, same reasoning as CCharSelMainWin's modal gate.
    if (!g_MsgWin.IsVisible())
    {
        if (CInput::Instance().IsKeyDown(VK_RETURN))
        {
            ::PlayBuffer(SOUND_CLICK01);
            RequestCreateCharacter();
        }
        else if (CInput::Instance().IsKeyDown(VK_ESCAPE))
        {
            ::PlayBuffer(SOUND_CLICK01);
            Show(false);
        }
    }

    UpdateCreateCharacter();

    // Input polling, moved here from RenderControls() (matches CLoginWin's own DoAction()
    // placement) -- RenderTextOnTop() below now only draws, called from the post-RmlUi seam.
    if (g_iChatInputType == 1)
        g_pSingleTextInputBox->DoAction();

    return true;
}

void CCharMakeWin::RequestCreateCharacter()
{
    if (g_iChatInputType == 1)
        g_pSingleTextInputBox->GetText(InputText[0]);

    CSceneUICoordinator& rUIMng = CSceneUICoordinator::Instance();

    const std::wstring characterName = InputText[0];

    // todo: check with regex from server
    if (characterName.length() < kMinCharacterNameLength)
        rUIMng.PopUpMsgWin(MESSAGE_MIN_LENGTH);
    else if (::CheckName())
        rUIMng.PopUpMsgWin(MESSAGE_ID_SPACE_ERROR);
    else if (CheckSpecialText(InputText[0]))
        rUIMng.PopUpMsgWin(MESSAGE_SPECIAL_NAME);
    else
    {
        const auto classByte = static_cast<CharacterClassNumber>((CharacterView.Class << 2) + CharacterView.Skin);
        CurrentProtocolState = REQUEST_CREATE_CHARACTER;
        SocketClient->ToGameServer()->SendCreateCharacter(MU_C16(InputText[0]), classByte);
        //SendRequestCreateCharacter(InputText[0], CharacterView.Class, CharacterView.Skin);
        Show(false);
        rUIMng.PopUpMsgWin(MESSAGE_WAIT);
    }
}

bool CCharMakeWin::Render()
{
    // Real, visible dimming overlay -- see this class's header comment for why this one (unlike
    // CMsgWin/CSysMenuWin) genuinely needs rendering, not just RmlUi bookkeeping.
    m_sprBg.Render();

    // The live 3D preview stays exactly as before -- see this class's header comment for why it
    // composites correctly underneath RmlUi's later render pass without any changes here.
    RenderCreateCharacter();

    // All 2D chrome (job buttons, stat/description panels, input-frame background, OK/Cancel)
    // now renders via the RmlUi overlay -- see char_make.rml/.rcss. SyncRmlModel() is the only
    // thing this override still needs to do; RenderTextOnTop() (the actual input text) is called
    // from Winmain.cpp's SetPostRmlUiCallback instead of here, so it isn't drawn twice.
    SyncRmlModel();
    return true;
}

void CCharMakeWin::RenderTextOnTop()
{
    // Force identity so this agrees with Update()'s now-real-pixel m_fInputTextX/Y regardless of
    // which context runs this call (this window's own Render(), under whatever transform
    // CManager::Render() applies for its LayoutMode::Legacy, or Winmain.cpp's completely unscoped
    // post-RmlUi callback -- see this class's header comment on RenderTextOnTop() for why it's
    // called from there) -- same fix as CLoginWin::RenderTextOnTop()/CMsgWin's resident-password
    // gotcha. LayoutMode::Legacy resolves to a genuine identity transform (UITransform.h's own
    // comment), matching RenderCreateCharacter()'s BeginOpengl() call just above, which relies on
    // this same identity for the same reason.
    const auto transform = UI::Scaling::TransformForLayout(UI::Scaling::LayoutMode::Legacy, WindowWidth, WindowHeight);
    UI::Scaling::ScopedActiveTransform identity(transform);

    if (g_iChatInputType == 1)
        g_pSingleTextInputBox->Render();
    else if (g_iChatInputType == 0)
        ::RenderInputText(int(m_fInputTextX), int(m_fInputTextY), 0);
}

void CCharMakeWin::SyncRmlModel()
{
    if (!m_pRmlDoc) return;

    auto& model = m_RmlBinder.GetModel();
    auto& jobs = model.jobs;
    bool jobsDirty = false;
    for (int classIndex = 0; classIndex < MAX_CLASS; ++classIndex)
    {
        JobButtonEntry& entry = jobs[classIndex];
        const bool checked = m_aJobState[classIndex].checked;
        const bool disabled = !m_aJobState[classIndex].enabled;
        if (entry.checked != checked) { entry.checked = checked; jobsDirty = true; }
        if (entry.disabled != disabled) { entry.disabled = disabled; jobsDirty = true; }

        const std::string label = StringUtils::WideToNarrow(I18N::Game::Lookup(kClassButtonTextIds[classIndex]));
        if (entry.label != label) { entry.label = label; jobsDirty = true; }
    }
    if (jobsDirty)
        m_RmlBinder.MarkDirty("jobs");

    const bool isDarkLord = (m_nSelJob == CLASS_DARK_LORD);
    if (model.darkLordExtra != isDarkLord)
    {
        model.darkLordExtra = isDarkLord;
        m_RmlBinder.MarkDirty("dark_lord_extra");
    }

    auto syncLabel = [this](Rml::String CharMakeRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        const std::string utf8 = StringUtils::WideToNarrow(text);
        if (m_RmlBinder.GetModel().*field != utf8)
        {
            m_RmlBinder.GetModel().*field = utf8;
            m_RmlBinder.MarkDirty(boundName);
        }
    };
    syncLabel(&CharMakeRmlModel::statLabel0, "stat_label0", I18N::Game::Lookup(kStatLabelBaseId + 0));
    syncLabel(&CharMakeRmlModel::statLabel1, "stat_label1", I18N::Game::Lookup(kStatLabelBaseId + 1));
    syncLabel(&CharMakeRmlModel::statLabel2, "stat_label2", I18N::Game::Lookup(kStatLabelBaseId + 2));
    syncLabel(&CharMakeRmlModel::statLabel3, "stat_label3", I18N::Game::Lookup(kStatLabelBaseId + 3));
    syncLabel(&CharMakeRmlModel::statLabel4, "stat_label4", I18N::Game::Lookup(kDarkLordLeadershipTextId));

    const auto& stats = kClassStatTable[static_cast<std::size_t>(m_nSelJob)];
    syncLabel(&CharMakeRmlModel::statValue0, "stat_value0", stats.values[0]);
    syncLabel(&CharMakeRmlModel::statValue1, "stat_value1", stats.values[1]);
    syncLabel(&CharMakeRmlModel::statValue2, "stat_value2", stats.values[2]);
    syncLabel(&CharMakeRmlModel::statValue3, "stat_value3", stats.values[3]);
    // The Dark Lord leadership row's value is a fixed constant in the original (not part of
    // kClassStatTable) -- reproduced as-is, gated by dark_lord_extra above rather than a live
    // 5th table column.

    // Same I18N lookup table used to build jobs[i].label (see the loop above) -- the selected
    // class's own name, for the modern theme's description-panel title (legacy hides it).
    syncLabel(&CharMakeRmlModel::descTitle, "desc_title", I18N::Game::Lookup(kClassButtonTextIds[m_nSelJob]));
    syncLabel(&CharMakeRmlModel::descLine1, "desc_line1", m_nDescLine > 0 ? m_aszJobDesc[0] : L"");
    syncLabel(&CharMakeRmlModel::descLine2, "desc_line2", m_nDescLine > 1 ? m_aszJobDesc[1] : L"");

    syncLabel(&CharMakeRmlModel::okLabel, "ok_label", I18N::Game::OK);
    syncLabel(&CharMakeRmlModel::cancelLabel, "cancel_label", I18N::Game::Cancel);
}

void CCharMakeWin::SelectCreateCharacter()
{
    CharacterView.Class = m_nSelJob;
    CreateCharacterPointer(&CharacterView, static_cast<int>(MODEL_FACE) + CharacterView.Class, 0, 0);
    CharacterView.Object.Kind = 0;
    SetAction(&CharacterView.Object, 1);
}

void CCharMakeWin::UpdateCreateCharacter()
{
    if (!CharacterAnimation(&CharacterView, &CharacterView.Object))
        SetAction(&CharacterView.Object, 0);
}

void CCharMakeWin::RenderCreateCharacter()
{
    OBJECT* o = &CharacterView.Object;
    vec3_t Position, Angle;

    Vector(1.0f, 1.0f, 1.0f, o->Light);
    Vector(10, -500.f, 48.f, Position);
    Vector(-90.f, 0.f, 0.f, Angle);
    g_Camera.FOV = 10.f;
    MoveCharacterCamera(CharacterView.Object.Position, Position, Angle);

    // Real pixels, not divided by g_fScreenRate_x/y -- BeginOpengl() rescales its arguments by
    // *whatever transform is active when it runs* (ConvertPositionX/Y, ZzzOpenglUtil.cpp), same
    // hazard as CMsgWin's resident-password gotcha. Render() (below) runs under this window's own
    // LayoutMode::Legacy (identity) ScopedActiveTransform, so passing real pixels directly here is
    // what keeps the viewport aligned with this window's actual position.
    BeginOpengl(m_nOriginX, m_nOriginY, 410, 335);

    const ClassRenderParameters params = GetRenderParameters(CharacterView.Class);
    if (params.overrideAngle)
        Vector(params.angleX, params.angleY, params.angleZ, o->Angle);

    o->Scale = params.scale;

    if (params.positionOffsetX != 0.0f)
        CharacterView.Object.Position[0] += params.positionOffsetX;
    if (params.positionOffsetZ != 0.0f)
        CharacterView.Object.Position[2] += params.positionOffsetZ;

    RenderCharacter(&CharacterView, o);

    SetRenderViewport(0, 0, WindowWidth, WindowHeight);

    EndOpengl();
}
