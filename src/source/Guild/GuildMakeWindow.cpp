
#include "stdafx.h"
#include "GuildMakeWindow.h"
#include "UI/Core/WindowManager.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "Audio/DSPlaySound.h"
#include "I18N/All.h"

#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInventory.h"
#include "App/Platform/Windows/Local.h"
#include "UI/Core/WindowSystem.h"

#include "Core/Utilities/StringUtils.h"
#include "Guild/GuildMarkPalette.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "Render/Text/CUIRenderTextSDLTtf.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"

#include <RmlUi/Core/ElementDocument.h>

#include <cstdio>
#include <iterator>

extern MARK_t		GuildMark[MAX_MARKS];
extern int			SelectMarkColor;
extern unsigned int MarkColor[16];

namespace
{
    BOOL IsGuildName(const wchar_t* szName)
    {
        if (wcslen(szName) >= 4)
            return TRUE;
        else
            return FALSE;
    }

    BOOL IsGuildMark()
    {
        BOOL bDraw = FALSE;

        for (int i = 0; i < 64; i++)
        {
            if (GuildMark[MARK_EDIT].Mark[i] != 0)
                return TRUE;
        }

        return FALSE;
    }

    void UpdateEditGuildMark(int iPos_x, int iPos_y)
    {
        int i, j;
        float x, y;
        Hero->Object.Angle[2] = 90.f + 22.5f;
        for (i = 0; i < 8; ++i)
        {
            for (j = 0; j < 8; ++j)
            {
                x = iPos_x + j * 15 + 50;
                y = iPos_y + i * 15 + 100;
                if (MouseX >= x && MouseX < x + 15 && MouseY >= y && MouseY < y + 15)
                {
                    if (MouseLButton)
                        GuildMark[MARK_EDIT].Mark[i * 8 + j] = SelectMarkColor;
                    if (MouseRButton)
                        GuildMark[MARK_EDIT].Mark[i * 8 + j] = 0;
                }
            }
        }
        for (i = 0; i < 2; ++i)
        {
            for (j = 0; j < 8; ++j)
            {
                x = iPos_x + j * 20 + 15;
                y = iPos_y + i * 20 + 260;
                if (MouseX >= x && MouseX < x + 20 && MouseY >= y && MouseY < y + 20)
                {
                    if (MouseLButtonPush)
                    {
                        MouseLButtonPush = FALSE;
                        PlayBuffer(SOUND_CLICK01);
                        SelectMarkColor = i * 8 + j;
                    }
                }
            }
        }
    }

    // RenderGuildColor()'s cell: RenderColorQuadARGB() reads MarkColor[] (built for the mark
    // texture, red in the low byte) as ARGB, so the editor shows red and blue swapped -- the
    // original's own look, kept. Index 0 is the black cell with a grey cross.
    mu::ui::window::GuildMakeCellEntry EditorCell(int index)
    {
        if (index <= 0 || static_cast<std::size_t>(index) >= std::size(MarkColor))
            return {"#000000ff", true};
        const unsigned int argb = MarkColor[static_cast<std::size_t>(index)];
        char color[16] = {};
        std::snprintf(color, sizeof(color), "#%02x%02x%02x%02x", (argb >> 16) & 0xFFu, (argb >> 8) & 0xFFu,
                      argb & 0xFFu, (argb >> 24) & 0xFFu);
        return {color, false};
    }

    // RenderText() shrinks a text wider than its box to fit it: the size it drew `text` at.
    float TextPxInBox(const UI::Scaling::Transform& transform, const wchar_t* text, float boxWidth)
    {
        g_pRenderText->SetFont(g_hFont);
        const int width = g_pRenderText->MeasureText(text, static_cast<int>(wcslen(text))).cx;
        return UI::Scaling::NativeTextPixelSizeInBox(UI::Scaling::FontRole::Normal, transform,
                                                     static_cast<float>(width), boxWidth);
    }
};

using namespace SEASON3B;
using namespace mu::ui::window;

CGuildMakeWindow::CGuildMakeWindow() : m_pNewUIMng(NULL), m_GuildMakeState(GUILDMAKE_INFO) {}

CGuildMakeWindow::~CGuildMakeWindow()
{
    Release();
}

bool CGuildMakeWindow::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_NPCGUILDMASTER, this);
    SetPos(x, y);

    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}

void CGuildMakeWindow::Release()
{
    UI::RmlBridge::UnregisterForThemeReload(this);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

float CGuildMakeWindow::GetLayerDepth()
{
    return 4.3f;
}

void CGuildMakeWindow::ClosingProcess()
{
    // Save any text in the editbox before closing
    if (m_GuildMakeState == GUILDMAKE_MARK && m_NameFieldShown)
    {
        wchar_t tempText[GuildConstants::MakeWindow::TEMP_TEXT_BUFFER_SIZE];
        memset(&tempText, 0, sizeof(tempText));
        ReadNameField(tempText, GuildConstants::MakeWindow::TEMP_TEXT_BUFFER_SIZE);
        if (tempText[0] != L'\0')
        {
            // Bounded copy: GuildName holds GUILD_NAME_BUFFER_SIZE wchar_t, but
            // tempText is far larger. An over-length name would otherwise run off
            // GuildName into the adjacent Mark[] (and, at MARK_EDIT = the last
            // slot, off the end of GuildMark[]) - worse on Linux where wchar_t is
            // 4 bytes. Truncate and always null-terminate.
            wcsncpy(GuildMark[MARK_EDIT].GuildName, tempText, GuildConstants::GUILD_NAME_BUFFER_SIZE - 1);
            GuildMark[MARK_EDIT].GuildName[GuildConstants::GUILD_NAME_BUFFER_SIZE - 1] = L'\0';
        }
    }

    ChangeWindowState(GUILDMAKE_INFO);
    ChangeEditBox(UISTATE_HIDE);

    SocketClient->ToGameServer()->SendGuildMasterAnswer(false);
}

void CGuildMakeWindow::ChangeWindowState(const GUILDMAKE_STATE state)
{
    m_GuildMakeState = state;
}

void CGuildMakeWindow::ChangeEditBox(const UISTATES type)
{
    Rml::Element* field = GetNameField();
    if (type == UISTATE_NORMAL)
    {
        // Restore guild name if it exists BEFORE setting state
        SyncField(m_RmlBinder, &GuildMakeRmlModel::guildName, "guild_name",
                  Rml::String(StringUtils::WideToNarrow(GuildMark[MARK_EDIT].GuildName)));
        m_NameFieldShown = true;
        // Focused once the document shows it (SyncRmlModel()), as GiveFocus() did.
        m_NameFieldFocusPending = true;
    }
    else
    {
        SyncField(m_RmlBinder, &GuildMakeRmlModel::guildName, "guild_name", Rml::String());
        if (field != nullptr)
            field->Blur();
        m_NameFieldShown = false;
        m_NameFieldFocusPending = false;
    }
}

Rml::Element* CGuildMakeWindow::GetNameField() const
{
    return m_pRmlDoc != nullptr ? m_pRmlDoc->GetElementById("name_field") : nullptr;
}

void CGuildMakeWindow::ReadNameField(wchar_t* text, int length) const
{
    if (length <= 0)
        return;
    // The model, not the element: data-value writes the typed text back into it.
    const std::wstring value = StringUtils::NarrowToWide(m_RmlBinder.GetModel().guildName);
    wcsncpy(text, value.c_str(), static_cast<size_t>(length - 1));
    text[length - 1] = L'\0';
}

void CGuildMakeWindow::UpdateGMInfo(GUILDMAKE_BUTTON button)
{
    if (button == GUILDMAKEBUTTON_MAKE)
    {
        SocketClient->ToGameServer()->SendGuildMasterAnswer(true);
        ChangeWindowState(GUILDMAKE_MARK);
        ChangeEditBox(UISTATE_NORMAL);
    }
    else if (button == GUILDMAKEBUTTON_EXIT)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_NPCGUILDMASTER);
    }
}

void CGuildMakeWindow::UpdateGMMark(GUILDMAKE_BUTTON button)
{
    if (button == GUILDMAKEBUTTON_BACK)
    {
        // Save the current text before going back
        wchar_t tempText[GuildConstants::MakeWindow::TEMP_TEXT_BUFFER_SIZE];
        memset(&tempText, 0, sizeof(tempText));
        ReadNameField(tempText, GuildConstants::MakeWindow::TEMP_TEXT_BUFFER_SIZE);
        if (tempText[0] != L'\0')
        {
            // Bounded copy: GuildName holds GUILD_NAME_BUFFER_SIZE wchar_t, but
            // tempText is far larger. An over-length name would otherwise run off
            // GuildName into the adjacent Mark[] (and, at MARK_EDIT = the last
            // slot, off the end of GuildMark[]) - worse on Linux where wchar_t is
            // 4 bytes. Truncate and always null-terminate.
            wcsncpy(GuildMark[MARK_EDIT].GuildName, tempText, GuildConstants::GUILD_NAME_BUFFER_SIZE - 1);
            GuildMark[MARK_EDIT].GuildName[GuildConstants::GUILD_NAME_BUFFER_SIZE - 1] = L'\0';
        }

        ChangeWindowState(GUILDMAKE_INFO);
        ChangeEditBox(UISTATE_HIDE);
    }
    else if (button == GUILDMAKEBUTTON_NEXT)
    {
        wchar_t tempText[GuildConstants::MakeWindow::TEMP_TEXT_BUFFER_SIZE];
        memset(&tempText, 0, sizeof(tempText));

        ReadNameField(tempText, GuildConstants::MakeWindow::TEMP_TEXT_BUFFER_SIZE);

        if (CheckSpecialText(tempText) == true)
        {
            mu::ui::window::CreateOkMessageBox(I18N::Game::CannotUseSymbols);
        }
        else if (IsGuildName(tempText) == FALSE)
        {
            CreateOkMessageBox(I18N::Game::TypeMoreThan4Letters);
        }
        else if (IsGuildMark() == FALSE)
        {
            CreateOkMessageBox(I18N::Game::PleaseDrawYourGuildEmblem);
        }
        else
        {
            // Bounded copy: see above.
            wcsncpy(GuildMark[MARK_EDIT].GuildName, tempText, GuildConstants::GUILD_NAME_BUFFER_SIZE - 1);
            GuildMark[MARK_EDIT].GuildName[GuildConstants::GUILD_NAME_BUFFER_SIZE - 1] = L'\0';
            ChangeWindowState(GUILDMAKE_RESULTINFO);
            ChangeEditBox(UISTATE_HIDE);
        }
    }
}

void CGuildMakeWindow::UpdateGMResultInfo(GUILDMAKE_BUTTON button)
{
    if (button == GUILDMAKEBUTTON_BACK)
    {
        ChangeWindowState(GUILDMAKE_MARK);
        ChangeEditBox(UISTATE_NORMAL);
    }
    else if (button == GUILDMAKEBUTTON_NEXT)
    {
        BYTE Mark[32];
        for (int i = 0; i < 64; i++)
        {
            if (i % 2 == 0)
                Mark[i / 2] = GuildMark[MARK_EDIT].Mark[i] << 4;
            else
                Mark[i / 2] += GuildMark[MARK_EDIT].Mark[i];
        }

        SocketClient->ToGameServer()->SendGuildCreateRequest(MU_C16(GuildMark[MARK_EDIT].GuildName), Mark, sizeof Mark);
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_NPCGUILDMASTER);
    }
}

bool CGuildMakeWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_NPCGUILDMASTER) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_NPCGUILDMASTER);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    return true;
}

bool CGuildMakeWindow::Update()
{
    // A button RmlUi reported (the original's CButton handling in UpdateMouseEvent()).
    const GUILDMAKE_BUTTON button = m_PendingButton;
    m_PendingButton = GUILDMAKEBUTTON_NONE;
    if (IsVisible() && button != GUILDMAKEBUTTON_NONE)
    {
        switch (m_GuildMakeState)
        {
        case GUILDMAKE_INFO:
            UpdateGMInfo(button);
            break;
        case GUILDMAKE_MARK:
            UpdateGMMark(button);
            break;
        case GUILDMAKE_RESULTINFO:
            UpdateGMResultInfo(button);
            break;
        }
    }

    SyncRmlModel();
    return true;
}

bool CGuildMakeWindow::UpdateMouseEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_NPCGUILDMASTER) == false)
    {
        return true;
    }

    // Top-right corner close "X" (shared frame): hides + swallows the click.
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_NPCGUILDMASTER))
    {
        return false;
    }

    // The buttons are RmlUi's (see Update()); the mark is still painted by the native hit tests
    // on its grid and palette, which guild_make.rml leaves to the pointer.
    if (m_GuildMakeState == GUILDMAKE_MARK)
    {
        UpdateEditGuildMark(m_Pos.x, m_Pos.y);
    }

    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, GUILDMAKE_WIDTH, GUILDMAKE_HEIGHT).Contains(MouseX, MouseY))
    {
        if (mu::ui::window::IsPress(VK_RBUTTON))
        {
            MouseRButton = false;
            MouseRButtonPop = false;
            MouseRButtonPush = false;
        }

        return false;
    }

    return true;
}

bool CGuildMakeWindow::Render()
{
    // Nothing native left: the frame, the pages, the mark and the name field are RmlUi. Kept
    // because CObject requires the override.
    return true;
}

void CGuildMakeWindow::BuildRmlUi()
{
    if (m_pRmlDoc || !RmlUiRuntime::Instance().IsCreated())
        return;

    const bool modelCreated = m_RmlBinder.Create(
        RmlUiRuntime::Instance().GetContext(), "guild_make",
        [this](Rml::DataModelConstructor& c, GuildMakeRmlModel& model)
        {
            c.Bind("root_x", &model.rootX);
            c.Bind("root_y", &model.rootY);
            c.Bind("root_scale", &model.rootScale);
            c.Bind("text_px", &model.textPx);
            c.Bind("page", &model.page);
            c.Bind("title_text", &model.titleText);
            c.Bind("title_px", &model.titlePx);
            c.Bind("info_text", &model.infoText);
            c.Bind("info_px", &model.infoPx);
            c.Bind("make_text", &model.makeText);
            c.Bind("back_text", &model.backText);
            c.Bind("next_text", &model.nextText);
            c.Bind("name_text", &model.nameText);
            c.Bind("result_text", &model.resultText);
            c.Bind("result_px", &model.resultPx);
            c.Bind("palette_hint_1", &model.paletteHint1);
            c.Bind("palette_hint_2", &model.paletteHint2);
            c.Bind("exit_tooltip", &model.exitTooltip);
            c.Bind("guild_name", &model.guildName);
            c.Bind("label_top", &model.labelTop);
            c.Bind("label_line_px", &model.labelLinePx);
            auto cell = c.RegisterStruct<GuildMakeCellEntry>();
            cell.RegisterMember("color", &GuildMakeCellEntry::color);
            cell.RegisterMember("empty", &GuildMakeCellEntry::empty);
            c.RegisterArray<std::vector<GuildMakeCellEntry>>();
            c.Bind("cells", &model.cells);
            c.Bind("palette", &model.palette);
            c.Bind("selected", &model.selected);
            c.RegisterArray<std::vector<Rml::String>>();
            c.Bind("mark_cells", &model.markCells);
            c.BindEventCallback("guild_make_button",
                                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                                {
                                    if (arguments.size() == 1)
                                        m_PendingButton = static_cast<GUILDMAKE_BUTTON>(arguments[0].Get<int>(-1));
                                });
        });
    if (!modelCreated)
        return;

    m_pRmlDoc =
        UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/guild_make.rml");
}

void CGuildMakeWindow::ReloadRmlTheme()
{
    if (!m_pRmlDoc)
        return;
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    const Rml::String typed = m_RmlBinder.GetModel().guildName;
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
    if (m_NameFieldShown)
    {
        // ChangeEditBox() reseeds from the game data, so carry what was typed across the rebuild --
        // Destroy() above cleared the model, which is now where the name lives.
        ChangeEditBox(UISTATE_NORMAL);
        SyncField(m_RmlBinder, &GuildMakeRmlModel::guildName, "guild_name", Rml::String(typed));
    }
}

void CGuildMakeWindow::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_pRmlDoc)
        return;

    // Layer depth 4.3: over the HUD like every panel the original opened.
    UI::RmlBridge::SyncDocumentVisibilityInFront(m_pRmlDoc, IsVisible());
    if (!IsVisible())
        return;

    UI::RmlBridge::SyncRootTransform(m_RmlBinder, m_Pos);
    UI::RmlBridge::SyncNativeTextSize(m_RmlBinder);
    SyncContent();

    if (m_NameFieldFocusPending && m_pRmlDoc->IsVisible() && m_RmlBinder.GetModel().page == GUILDMAKE_MARK)
    {
        if (Rml::Element* field = GetNameField())
        {
            field->Focus();
            if (field->IsPseudoClassSet("focus"))
                m_NameFieldFocusPending = false;
        }
    }
}

void CGuildMakeWindow::SyncContent()
{
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    SyncField(m_RmlBinder, &GuildMakeRmlModel::page, "page", static_cast<int>(m_GuildMakeState));
    SyncField(m_RmlBinder, &GuildMakeRmlModel::titleText, "title_text", StringUtils::WideToNarrow(I18N::Game::Guild));
    SyncField(m_RmlBinder, &GuildMakeRmlModel::titlePx, "title_px", TextPxInBox(transform, I18N::Game::Guild, 190.f));
    SyncField(m_RmlBinder, &GuildMakeRmlModel::infoText, "info_text",
              StringUtils::WideToNarrow(I18N::Game::DoYouWishToBeTheGuildMaster));
    SyncField(m_RmlBinder, &GuildMakeRmlModel::infoPx, "info_px",
              TextPxInBox(transform, I18N::Game::DoYouWishToBeTheGuildMaster, 190.f));
    SyncField(m_RmlBinder, &GuildMakeRmlModel::makeText, "make_text",
              StringUtils::WideToNarrow(I18N::Game::CreateGuild));
    SyncField(m_RmlBinder, &GuildMakeRmlModel::backText, "back_text", StringUtils::WideToNarrow(I18N::Game::Back));
    SyncField(m_RmlBinder, &GuildMakeRmlModel::nextText, "next_text", StringUtils::WideToNarrow(I18N::Game::Next));
    SyncField(m_RmlBinder, &GuildMakeRmlModel::nameText, "name_text", StringUtils::WideToNarrow(I18N::Game::NAME));
    SyncField(m_RmlBinder, &GuildMakeRmlModel::paletteHint1, "palette_hint_1",
              StringUtils::WideToNarrow(I18N::Game::AfterSelectingAColorWith));
    SyncField(m_RmlBinder, &GuildMakeRmlModel::paletteHint2, "palette_hint_2",
              StringUtils::WideToNarrow(I18N::Game::TheMousePleaseDraw));
    SyncField(m_RmlBinder, &GuildMakeRmlModel::exitTooltip, "exit_tooltip",
              StringUtils::WideToNarrow(I18N::Game::Close388));
    wchar_t result[100] = {};
    mu_swprintf(result, L"%ls : %ls", I18N::Game::NAME, GuildMark[MARK_EDIT].GuildName);
    SyncField(m_RmlBinder, &GuildMakeRmlModel::resultText, "result_text", StringUtils::WideToNarrow(result));
    SyncField(m_RmlBinder, &GuildMakeRmlModel::resultPx, "result_px", TextPxInBox(transform, result, 190.f));

    const int lineHeight = CUIRenderTextSDLTtf::LineHeight(UI::Scaling::FontRole::Normal);
    const int labelTop = 29 / 2 - lineHeight / 2;
    SyncField(m_RmlBinder, &GuildMakeRmlModel::labelTop, "label_top", static_cast<float>(labelTop));
    SyncField(m_RmlBinder, &GuildMakeRmlModel::labelLinePx, "label_line_px",
              static_cast<float>(lineHeight) * transform.scaleY);

    if (m_GuildMakeState == GUILDMAKE_INFO)
        return;

    // The mark: CreateGuildMark(MARK_EDIT) fills MarkColor[], which the editor's cells read.
    CreateGuildMark(MARK_EDIT);
    GuildMakeRmlModel& model = m_RmlBinder.GetModel();
    std::vector<GuildMakeCellEntry> cells;
    std::vector<Rml::String> markCells;
    // RenderEditGuildMark(): the grid's cells row by row, then the palette's; the theme lays both
    // out, and the selected colour has its own swatch.
    for (int i = 0; i < Guild::MarkPalette::CellCount; ++i)
    {
        cells.push_back(EditorCell(GuildMark[MARK_EDIT].Mark[i]));
        markCells.push_back(Guild::MarkPalette::CellColor(GuildMark[MARK_EDIT].Mark[i]));
    }
    std::vector<GuildMakeCellEntry> palette;
    for (int i = 0; i < Guild::MarkPalette::ColorCount; ++i)
        palette.push_back(EditorCell(i));

    if (model.cells != cells)
    {
        model.cells = std::move(cells);
        m_RmlBinder.MarkDirty("cells");
    }
    if (model.palette != palette)
    {
        model.palette = std::move(palette);
        m_RmlBinder.MarkDirty("palette");
    }
    SyncField(m_RmlBinder, &GuildMakeRmlModel::selected, "selected", EditorCell(SelectMarkColor));
    SyncField(m_RmlBinder, &GuildMakeRmlModel::markCells, "mark_cells", std::move(markCells));
}
