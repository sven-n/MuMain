#include "stdafx.h"

#ifdef _EDITOR

#include "MuItemSyncUI.h"

#include "ItemEdits.h"
#include "ItemHeldByCharacter.h"
#include "ItemSelection.h"
#include "ItemSyncApply.h"
#include "ItemTextInput.h"
#include "ItemToolWindow.h"
#include "ItemUsageView.h"

#include "../MuEditor/Config/MuEditorConfig.h"
#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/Core/SecretStore.h"
#include "../MuEditor/UI/Common/KeyboardSelection.h"
#include "../MuEditor/UI/ItemEditor/ItemEditorActions.h"
#include "../MuEditor/UI/ItemEditor/ItemEditorPopups.h"
#include "../MuEditor/UI/Common/MuEditorWindowIds.h"
#include "../MuEditor/UI/Console/MuEditorConsoleUI.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "I18N/All.h"
#include "imgui.h"

#include <SDL3/SDL_dialog.h>

#include <algorithm>
#include <cstdint>
#include <set>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <utility>

using namespace MuEditor::Items;
using Data::Items::ItemDataIssue;

namespace
{
// Pixel widths before the editor UI scale.
constexpr float FieldsColumnWidth = 80.0f;
constexpr float FieldColumnWidth = 220.0f;
constexpr float KnownColumnWidth = 60.0f;
constexpr float ItemListHeight = 260.0f;
// What a short item list adds to its rows' height (the table's borders).
constexpr float ItemListBorders = 4.0f;
constexpr float FieldListHeight = 220.0f;
constexpr float PopupTextWidth = 480.0f;
constexpr float ApiKeyWidth = 320.0f;
constexpr const char* NoValue = "-";
constexpr const char* ExchangeFileName = "ItemExchange.json";
// The name of the checkbox column: a header takes the ID of its label under PushID(column), so
// it must differ from the rows' "##Checked" under PushID(row), or row 0 has the header's ID.
constexpr const char* CheckColumn = "##CheckColumn";
// The titles and change columns of Copy as Markdown: English whatever the editor's language, as
// the copied table is meant for issues and chats.
constexpr const char* MarkdownDifferFromServer = "Items that differ between the client and the server";
constexpr const char* MarkdownDifferFromFile = "Items that differ between the client and the file";
constexpr const char* MarkdownDifferBetweenFiles = "Items that differ from file A to file B";
constexpr const char* MarkdownOnlyInClientFromServer = "Items only in the client, not on the server";
constexpr const char* MarkdownOnlyInClientFromFile = "Items only in the client, not in the file";
constexpr const char* MarkdownInTheClient = "In the client";
constexpr const char* MarkdownFromAToB = "From A to B";
constexpr const char* ApplyPopupId = "###ItemSyncApply";
constexpr const char* ExportPopupId = "###ItemSyncExport";

constexpr ImVec4 ErrorColor{1.0f, 0.4f, 0.4f, 1.0f};
constexpr ImVec4 WarningColor{1.0f, 0.8f, 0.3f, 1.0f};
// The green the map and dev editors use.
constexpr ImVec4 SuccessColor{0.5f, 1.0f, 0.5f, 1.0f};

// The paths are UTF-8 text in the window.
std::filesystem::path ToPath(const std::string& text)
{
    return std::filesystem::path(std::u8string(text.begin(), text.end()));
}

std::string ToText(const std::filesystem::path& path)
{
    const std::u8string text = path.generic_u8string();
    return std::string(text.begin(), text.end());
}

float Scaled(float width)
{
    return width * g_MuEditorCore.GetUIScale();
}

// What the row does to the client's data.
const char* ChangeName(SyncChange change)
{
    switch (change)
    {
    case SyncChange::New:
        return I18N::Editor::Added;
    case SyncChange::Removed:
        return I18N::Editor::Removed;
    default:
        return I18N::Editor::Changed;
    }
}

// An item without a name on both sides is the client's entry without a name, unless two files
// are compared.
std::string ItemLabel(const SyncItem& row, bool comparingFiles)
{
    std::string name = row.name;
    const int itemType = Data::Items::MakeItemType(row.group, row.number);
    if (name.empty() && !comparingFiles && Data::Items::IsValidItemType(itemType) &&
        g_ItemDatabase.GetAllSlots()[itemType].HasEntry())
    {
        name = I18N::Editor::EntryWithoutAName;
    }
    return name + " (" + std::to_string(row.group) + "," + std::to_string(row.number) + ")";
}

// The fields a click on the row's box changes: New and Removed rows are one box.
bool IsWhole(const SyncItem& row)
{
    return row.change != SyncChange::Changed;
}

bool IsRowChecked(const SyncItem& row)
{
    if (IsWhole(row))
    {
        return row.checked;
    }
    return std::ranges::any_of(row.fields, [](const SyncField& field) { return field.checked; });
}

// Whether the row changes something when it is applied: the whole New or Removed item, or a
// field of a changed one.
bool AppliesSomething(const SyncItem& row)
{
    if (IsWhole(row))
    {
        return row.checked && row.locked.empty();
    }
    return std::ranges::any_of(row.fields,
                               [](const SyncField& field) { return field.checked && field.locked.empty(); });
}

// What a diff has unchecked, to leave it unchecked in the comparison after an Apply: the
// whole New and Removed items (field ""), the fields of the changed ones, the formula entries.
struct UncheckedRows
{
    std::set<std::tuple<int, int, std::string>> fields;
    std::set<std::string> formulas;
};

UncheckedRows FindUnchecked(const ItemSyncDiff& diff)
{
    UncheckedRows unchecked;
    for (const SyncItem& row : diff.items)
    {
        if (row.change != SyncChange::Changed)
        {
            if (!row.checked)
            {
                unchecked.fields.insert({row.group, row.number, ""});
            }
            continue;
        }
        for (const SyncField& field : row.fields)
        {
            if (!field.checked)
            {
                unchecked.fields.insert({row.group, row.number, field.field});
            }
        }
    }
    for (const SyncFormula& row : diff.formulas)
    {
        if (!row.checked)
        {
            unchecked.formulas.insert(row.entry);
        }
    }
    return unchecked;
}

void KeepUnchecked(ItemSyncDiff& diff, const UncheckedRows& unchecked)
{
    for (SyncItem& row : diff.items)
    {
        if (row.change != SyncChange::Changed)
        {
            row.checked = row.checked && !unchecked.fields.contains({row.group, row.number, ""});
            continue;
        }
        for (SyncField& field : row.fields)
        {
            field.checked = field.checked && !unchecked.fields.contains({row.group, row.number, field.field});
        }
    }
    for (SyncFormula& row : diff.formulas)
    {
        row.checked = row.checked && !unchecked.formulas.contains(row.entry);
    }
}

void RenderValue(const std::optional<std::string>& value)
{
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(value ? value->c_str() : NoValue);
    ImGui::PopTextWrapPos();
}

// The dialog's action is its userdata, so a dialog answers for its own button.
void* ToUserData(CMuItemSyncUI::FileAction action)
{
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(action));
}

void SDLCALL OnFileChosen(void* userdata, const char* const* files, int)
{
    // Nothing on an error, an empty list when the dialog was cancelled.
    if (files != nullptr && files[0] != nullptr)
    {
        const auto action = static_cast<CMuItemSyncUI::FileAction>(reinterpret_cast<std::uintptr_t>(userdata));
        g_MuItemSyncUI.ChooseFile(files[0], action);
    }
}

// The width of the column that says what a row does: its widest text.
float ChangeColumnWidth()
{
    float width = 0.0f;
    for (const char* text : {I18N::Editor::InTheClient, I18N::Editor::FromAToB, I18N::Editor::Added,
                             I18N::Editor::Changed, I18N::Editor::Removed})
    {
        width = std::max(width, ImGui::CalcTextSize(text).x);
    }
    return width;
}

// Reads the text of an exchange file (nothing: it could not be read) into `file`, its problems
// into `issues`; false when it cannot be used (an item it has twice is no obstacle: the diff locks
// its row).
bool ReadExchangeText(const std::optional<std::string>& text, const std::string& name,
                      Data::Items::ItemExchangeFile& file, std::vector<ItemDataIssue>& issues)
{
    std::vector<ItemDataIssue> fileIssues;
    if (text)
    {
        ReadItemExchangeJson(*text, name, file, fileIssues);
    }
    else
    {
        fileIssues.push_back({Data::Items::ItemDataIssueSeverity::Fatal, name, ItemDataIssue::NoItem,
                              ItemDataIssue::NoItem, "", "could not be read"});
    }
    const bool usable = Data::Items::CanImportItemExchange(file, fileIssues);
    issues.insert(issues.end(), fileIssues.begin(), fileIssues.end());
    return usable;
}

bool ReadExchangeFile(const std::filesystem::path& path, const std::string& name, Data::Items::ItemExchangeFile& file,
                      std::vector<ItemDataIssue>& issues)
{
    return ReadExchangeText(Data::Items::Json::ReadTextFile(path), name, file, issues);
}

// What an Update from server that has no file says.
std::string DescribeServerProblem(const ServerDownload& download)
{
    switch (download.problem)
    {
    case ServerUpdateProblem::NoUrl:
        return I18N::Editor::EnterTheAddressOfTheOpenMUAdminPanel;
    case ServerUpdateProblem::InsecureUrl:
        return I18N::Editor::TheAdminPanelAddressMustBeHttps;
    case ServerUpdateProblem::NoKey:
        return I18N::Format(I18N::Editor::EnterAnApiKeyOfTheAdminPanel0, {OpenMUApiKeyVariable});
    case ServerUpdateProblem::NoConnection:
        return I18N::Format(I18N::Editor::TheAdminPanelCouldNotBeReached0, {download.detail});
    case ServerUpdateProblem::KeyRefused:
        return I18N::Editor::TheAdminPanelDoesNotKnowTheApiKey;
    case ServerUpdateProblem::NotAllowed:
        return I18N::Editor::TheApiKeyMayNotDownloadTheItems;
    case ServerUpdateProblem::NoEndpoint:
        return I18N::Editor::TheAdminPanelHasNoItemExchange;
    case ServerUpdateProblem::TooManyRequests:
        return I18N::Editor::TooManyRequestsTryAgainInAMinute;
    case ServerUpdateProblem::OtherStatus:
        return I18N::Format(I18N::Editor::TheAdminPanelAnsweredWithHttpStatus0, {std::to_string(download.status)});
    case ServerUpdateProblem::Cancelled:
        return I18N::Editor::TheUpdateWasCancelled;
    case ServerUpdateProblem::None:
        break;
    }
    return {};
}

// How many rows add, change and remove an item.
void CountRows(const ItemSyncDiff& diff, int& added, int& changed, int& removed)
{
    added = changed = removed = 0;
    for (const SyncItem& row : diff.items)
    {
        added += row.change == SyncChange::New ? 1 : 0;
        changed += row.change == SyncChange::Changed ? 1 : 0;
        removed += row.change == SyncChange::Removed ? 1 : 0;
    }
}

// The file's path, and the server it is from when OpenMU wrote it.
void RenderFileLine(const char* format, const std::string& path, const Data::Items::ItemExchangeFile& file)
{
    ImGui::TextUnformatted(I18N::Format(format, {path}).c_str());
    if (file.source == Data::Items::ItemExchangeSourceOpenMU)
    {
        ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::FromOpenMUServer0,
                                               {file.server.empty() ? std::string(NoValue) : file.server})
                                      .c_str());
    }
}

// Whether both paths name one file (also when written differently); false while one is not there.
bool IsSameFile(const std::string& first, const std::string& second)
{
    std::error_code error;
    return !first.empty() && !second.empty() && std::filesystem::equivalent(ToPath(first), ToPath(second), error);
}
} // namespace

CMuItemSyncUI& CMuItemSyncUI::GetInstance()
{
    static CMuItemSyncUI instance;
    return instance;
}

void CMuItemSyncUI::Render(bool* open)
{
    // A mark in the title while there are changes that are not saved, as the Item Browser has.
    std::string title = I18N::Editor::ItemSync;
    if (g_ItemEdits.GetHistory().IsModifiedSinceSave())
    {
        title += " *";
    }
    if (ToolWindow::Begin(title.c_str(), MuEditor::WindowIds::ItemSync, open))
    {
        if (!m_knownLoaded)
        {
            std::vector<ItemDataIssue> issues;
            m_known = LoadKnownDifferences(GetKnownDifferencesFile(), issues);
            for (const ItemDataIssue& issue : issues)
            {
                g_MuEditorConsoleUI.LogEditor(issue.ToString());
            }
            m_knownLoaded = true;
        }
        TakeChosenFile();
        TakeServerDownload();
        RenderUnsaved();
        RenderButtons();
        RenderServer();
        RenderExportResult();
        // An Apply whose comparison afterwards could not read the file still says what it did.
        if (!m_diff && !m_result.empty())
        {
            ImGui::Separator();
            ImGui::PushTextWrapPos(0.0f);
            ImGui::TextColored(SuccessColor, "%s", m_result.c_str());
            ImGui::PopTextWrapPos();
        }
        if (m_diff)
        {
            ImGui::Separator();
            RenderSummary();
            // A comparison of two files is read-only.
            if (!m_comparingFiles)
            {
                RenderFieldToggle();
            }
            // Both say how many were compared, also when none of them differs.
            RenderItems();
            RenderFormulas();
            // Nothing to check or apply while both sides have the same data.
            if (!m_comparingFiles && !m_diff->IsEmpty())
            {
                RenderApply();
            }
        }
        RenderMessages();
        RenderApplyPopup();
        RenderExportPopup();
    }
    ToolWindow::End();
}

void CMuItemSyncUI::ChooseFile(std::string path, FileAction action)
{
    const std::scoped_lock lock(m_chosenMutex);
    m_chosenPath = std::move(path);
    m_chosenAction = action;
}

void CMuItemSyncUI::TakeChosenFile()
{
    // A confirmation is answered first: the file waits, so the confirmation keeps its diff.
    if (m_confirmApply || m_pendingExport)
    {
        return;
    }
    std::optional<std::string> path;
    FileAction action = FileAction::Import;
    {
        const std::scoped_lock lock(m_chosenMutex);
        path.swap(m_chosenPath);
        action = m_chosenAction;
    }
    if (!path)
    {
        return;
    }
    switch (action)
    {
    case FileAction::Import:
        Compare(ToPath(*path));
        break;
    case FileAction::Export:
        Export(ToPath(*path));
        break;
    case FileAction::CompareA:
        m_pathA = ToText(ToPath(*path));
        break;
    case FileAction::CompareB:
        m_pathB = ToText(ToPath(*path));
        break;
    }
}

void CMuItemSyncUI::OpenFileDialog(FileAction action)
{
    // SDL reads the filters until the dialog answers, so they stay; the label follows the editor's
    // language.
    static SDL_DialogFileFilter Filters[] = {{nullptr, "json"}};
    Filters[0].name = I18N::Editor::ItemExchangeFiles;
    if (action == FileAction::Export)
    {
        SDL_ShowSaveFileDialog(&OnFileChosen, ToUserData(action), nullptr, Filters, 1, ExchangeFileName);
    }
    else
    {
        SDL_ShowOpenFileDialog(&OnFileChosen, ToUserData(action), nullptr, Filters, 1, nullptr, false);
    }
}

void CMuItemSyncUI::LoadServerSettings()
{
    if (m_adminPanelUrlLoaded)
    {
        return;
    }
    m_adminPanelUrl = g_MuEditorConfig.GetOpenMUAdminPanelUrl();
    m_adminPanelUrlLoaded = true;
    LoadApiKey();
}

// The key the store keeps for the address, else the field's text, else MU_OPENMU_API_KEY.
void CMuItemSyncUI::LoadApiKey()
{
    m_apiKeyMessage.clear();
    const std::optional<std::string> remembered =
        m_adminPanelUrl.empty() ? std::nullopt : MuEditor::SecretStore::Read(GetApiKeySecretName(m_adminPanelUrl));
    m_rememberApiKey = remembered.has_value();
    if (remembered)
    {
        m_apiKey = *remembered;
    }
    else if (m_apiKey.empty())
    {
        m_apiKey = ReadOpenMUApiKey();
    }
}

// Remember on: the store keeps the key for the address; off (or no key): it forgets it.
void CMuItemSyncUI::StoreApiKey()
{
    m_apiKeyMessage.clear();
    if (m_adminPanelUrl.empty())
    {
        return;
    }
    const std::string name = GetApiKeySecretName(m_adminPanelUrl);
    const bool done = m_rememberApiKey && !m_apiKey.empty() ? MuEditor::SecretStore::Write(name, m_apiKey)
                                                            : MuEditor::SecretStore::Remove(name);
    if (!done)
    {
        m_apiKeyMessage =
            I18N::Format(I18N::Editor::TheKeyCouldNotBeStoredIn0, {MuEditor::SecretStore::GetStoreName()});
    }
}

void CMuItemSyncUI::RenderApiKey()
{
    ImGui::SetNextItemWidth(Scaled(ApiKeyWidth));
    InputString(I18N::Editor::ApiKey, m_apiKey, ImGuiInputTextFlags_Password);
    if (ImGui::IsItemDeactivatedAfterEdit() && m_rememberApiKey)
    {
        StoreApiKey();
    }
    ImGui::SameLine();
    const bool available = MuEditor::SecretStore::IsAvailable();
    ImGui::BeginDisabled(!available);
    if (ImGui::Checkbox(I18N::Editor::Remember, &m_rememberApiKey))
    {
        StoreApiKey();
    }
    ImGui::EndDisabled();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    {
        ImGui::SetTooltip("%s", available ? I18N::Format(I18N::Editor::KeepsTheKeyForThisAddressIn0,
                                                         {MuEditor::SecretStore::GetStoreName()})
                                                .c_str()
                                          : I18N::Editor::ThisBuildCannotKeepTheKey);
    }
    if (!m_apiKeyMessage.empty())
    {
        ImGui::TextColored(ErrorColor, "%s", m_apiKeyMessage.c_str());
    }
}

bool CMuItemSyncUI::UpdateFromServer()
{
    m_serverMessage.clear();
    if (m_download.IsRunning())
    {
        return false;
    }
    LoadServerSettings();
    ServerDownload refused;
    const std::optional<std::string> url = GetExchangeUrl(m_adminPanelUrl, refused.problem);
    const std::string apiKey = url ? m_apiKey : std::string();
    if (url && apiKey.empty())
    {
        refused.problem = ServerUpdateProblem::NoKey;
    }
    if (refused.problem != ServerUpdateProblem::None)
    {
        m_serverMessage = DescribeServerProblem(refused);
        return false;
    }
    m_downloadUrl = *url;
    return m_download.Start(*url, apiKey);
}

void CMuItemSyncUI::TakeServerDownload()
{
    std::optional<ServerDownload> download = m_download.TakeResult();
    if (!download)
    {
        return;
    }
    if (download->problem != ServerUpdateProblem::None)
    {
        m_serverMessage = DescribeServerProblem(*download);
        g_MuEditorConsoleUI.LogEditor(m_downloadUrl + ": " + m_serverMessage);
        return;
    }
    // A diff must not change under an open confirmation; the download is then dropped.
    if (m_confirmApply || m_pendingExport)
    {
        m_serverMessage = I18N::Editor::TheUpdateWasCancelled;
        return;
    }
    CompareServerFile(download->body, m_downloadUrl);
}

void CMuItemSyncUI::RenderServer()
{
    LoadServerSettings();
    const bool downloading = m_download.IsRunning();
    ImGui::BeginDisabled(downloading);
    if (ImGui::Button(I18N::Editor::UpdateFromServer))
    {
        UpdateFromServer();
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-Scaled(FieldColumnWidth));
    InputString(I18N::Editor::AdminPanelAddress, m_adminPanelUrl);
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        g_MuEditorConfig.SetOpenMUAdminPanelUrl(m_adminPanelUrl);
        g_MuEditorConfig.Save();
        LoadApiKey();
    }
    RenderApiKey();
    ImGui::EndDisabled();
    ImGui::PushTextWrapPos(0.0f);
    if (downloading)
    {
        ImGui::TextUnformatted(I18N::Format(I18N::Editor::Downloading0, {m_downloadUrl}).c_str());
        ImGui::SameLine();
        if (ImGui::SmallButton(I18N::Editor::Cancel))
        {
            m_download.Cancel();
        }
    }
    else if (!m_serverMessage.empty())
    {
        ImGui::TextColored(ErrorColor, "%s", m_serverMessage.c_str());
    }
    else
    {
        ImGui::TextDisabled("%s", I18N::Format(I18N::Editor::WhatUpdateFromServerDoes, {OpenMUApiKeyVariable}).c_str());
    }
    ImGui::PopTextWrapPos();
}

void CMuItemSyncUI::RenderButtons()
{
    if (ImGui::Button(I18N::Editor::Import))
    {
        OpenFileDialog(FileAction::Import);
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Export))
    {
        OpenFileDialog(FileAction::Export);
    }
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", I18N::Editor::WhatImportAndExportDo);
    ImGui::PopTextWrapPos();

    // The path of an import can be typed too, e.g. where no file dialog is at hand.
    ImGui::SetNextItemWidth(-Scaled(FieldColumnWidth));
    InputString(I18N::Editor::ExchangeFile, m_path);
    ImGui::SameLine();
    ImGui::BeginDisabled(m_path.empty());
    if (ImGui::Button(I18N::Editor::CompareAgain))
    {
        Compare(ToPath(m_path));
    }
    ImGui::EndDisabled();
    RenderCompareFiles();
}

void CMuItemSyncUI::RenderCompareFiles()
{
    const std::string header = std::string(I18N::Editor::CompareTwoFiles) + "###CompareFiles";
    if (!ImGui::CollapsingHeader(header.c_str()))
    {
        return;
    }
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("%s", I18N::Editor::WhatComparingFilesDoes);
    ImGui::PopTextWrapPos();
    for (const auto& [label, path, action] : {std::tuple{I18N::Editor::FileA, &m_pathA, FileAction::CompareA},
                                              std::tuple{I18N::Editor::FileB, &m_pathB, FileAction::CompareB}})
    {
        ImGui::PushID(static_cast<int>(action));
        ImGui::SetNextItemWidth(-Scaled(FieldColumnWidth));
        InputString(label, *path);
        ImGui::SameLine();
        if (ImGui::Button(I18N::Editor::Pick))
        {
            OpenFileDialog(action);
        }
        ImGui::PopID();
    }
    if (ImGui::Button(I18N::Editor::Swap))
    {
        std::swap(m_pathA, m_pathB);
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(m_pathA.empty() || m_pathB.empty());
    if (ImGui::Button(I18N::Editor::CompareTheFiles))
    {
        CompareFiles(ToPath(m_pathA), ToPath(m_pathB));
    }
    ImGui::EndDisabled();
}

void CMuItemSyncUI::RenderExportResult()
{
    if (m_exportResult.empty())
    {
        return;
    }
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextColored(m_exportFailed ? ErrorColor : SuccessColor, "%s", m_exportResult.c_str());
    ImGui::PopTextWrapPos();
}

void CMuItemSyncUI::Compare(const std::filesystem::path& path)
{
    m_fromServer = false;
    m_serverText.clear();
    m_path = ToText(path);
    CompareText(Data::Items::Json::ReadTextFile(path), m_path);
}

void CMuItemSyncUI::CompareServerFile(const std::string& text, const std::string& url)
{
    m_fromServer = true;
    m_serverText = text;
    CompareText(text, url);
}

void CMuItemSyncUI::CompareText(const std::optional<std::string>& text, const std::string& name)
{
    m_comparingFiles = false;
    m_toggleFields.clear();
    m_comparedPath = name;
    m_result.clear();
    m_messages.clear();
    m_messagesAreErrors = false;
    m_diff.reset();
    m_shownItem = -1;
    m_shownUsage = {};
    m_copiedList = ItemList::None;
    m_file = {};

    std::vector<ItemDataIssue> issues;
    const bool usable = ReadExchangeText(text, name, m_file, issues);
    for (const ItemDataIssue& issue : issues)
    {
        m_messages.push_back(issue.ToString());
    }
    if (!usable)
    {
        m_messagesAreErrors = true;
        return;
    }

    std::vector<std::string> locales;
    for (const char* locale : I18N::GetAvailableLocales())
    {
        locales.emplace_back(locale);
    }
    const SyncOptions options{&m_known, locales, [](int itemType) { return IsHeldByCharacter(itemType); }};
    m_diff = BuildItemSyncDiff(BuildClientComparison(m_file), m_file, options);
    m_diffVersion = g_ItemDatabase.GetVersion();
    std::set<std::string> names;
    for (const SyncItem& row : m_diff->items)
    {
        if (row.change == SyncChange::Changed)
        {
            for (const SyncField& field : row.fields)
            {
                names.insert(field.field);
            }
        }
    }
    m_toggleFields.assign(names.begin(), names.end());
}

void CMuItemSyncUI::CompareFiles(const std::filesystem::path& a, const std::filesystem::path& b)
{
    m_comparingFiles = true;
    m_toggleFields.clear();
    m_pathA = ToText(a);
    m_pathB = ToText(b);
    m_comparedA = m_pathA;
    m_comparedB = m_pathB;
    // No import is shown.
    m_comparedPath.clear();
    m_confirmApply = false;
    m_result.clear();
    m_messages.clear();
    m_messagesAreErrors = false;
    m_diff.reset();
    m_shownItem = -1;
    m_shownUsage = {};
    m_copiedList = ItemList::None;
    m_fileA = {};
    m_file = {};

    std::vector<ItemDataIssue> issues;
    const bool usableA = ReadExchangeFile(a, m_comparedA, m_fileA, issues);
    const bool usableB = ReadExchangeFile(b, m_comparedB, m_file, issues);
    for (const ItemDataIssue& issue : issues)
    {
        m_messages.push_back(issue.ToString());
    }
    if (!usableA || !usableB)
    {
        m_messagesAreErrors = true;
        return;
    }
    SyncOptions options;
    options.known = &m_known;
    options.comparingFiles = true;
    m_diff = BuildItemSyncDiff(m_fileA, m_file, options);
}

CMuItemSyncUI::ApplyBlocker CMuItemSyncUI::GetApplyBlocker() const
{
    // A file from the server is not the path field's.
    if (!m_fromServer && m_path != m_comparedPath)
    {
        return ApplyBlocker::OtherPath;
    }
    // The diff shows the client's data as it was when it was made.
    if (g_ItemDatabase.GetVersion() != m_diffVersion)
    {
        return ApplyBlocker::DataChanged;
    }
    const bool anyChecked = m_diff && (std::ranges::any_of(m_diff->items, AppliesSomething) ||
                                       std::ranges::any_of(m_diff->formulas, &SyncFormula::checked));
    return anyChecked ? ApplyBlocker::None : ApplyBlocker::NothingChecked;
}

CMuItemSyncUI::CheckedChanges CMuItemSyncUI::CountCheckedChanges() const
{
    CheckedChanges changes;
    if (!m_diff)
    {
        return changes;
    }
    for (const SyncItem& row : m_diff->items)
    {
        if (!AppliesSomething(row))
        {
            continue;
        }
        switch (row.change)
        {
        case SyncChange::New:
            ++changes.added;
            break;
        case SyncChange::Changed:
            ++changes.changed;
            break;
        case SyncChange::Removed:
            ++changes.removed;
            break;
        }
    }
    changes.formulaEntries = static_cast<int>(std::ranges::count_if(m_diff->formulas, &SyncFormula::checked));
    return changes;
}

bool CMuItemSyncUI::Apply()
{
    if (m_comparingFiles || !m_diff || GetApplyBlocker() != ApplyBlocker::None)
    {
        return false;
    }
    // What is applied, for the line that reports it.
    const CheckedChanges changes = CountCheckedChanges();

    std::vector<ItemDataIssue> issues;
    if (!ApplyItemSyncImport(*m_diff, m_file, issues))
    {
        m_result.clear();
        m_messages.clear();
        m_messages.push_back(I18N::Editor::TheChecksRefuseTheChange);
        bool formulasRefused = false;
        for (const ItemDataIssue& issue : issues)
        {
            m_messages.push_back(issue.ToString());
            // The rows of the items with fatal problems are unchecked; a fatal problem of no item is one of the
            // formulas, which an issue cannot trace back to its row: all of them are unchecked.
            formulasRefused = formulasRefused || issue.group == ItemDataIssue::NoItem;
            for (SyncItem& row : m_diff->items)
            {
                if (row.group == issue.group && row.number == issue.number)
                {
                    SetRowChecked(row, false);
                }
            }
        }
        if (formulasRefused)
        {
            for (SyncFormula& row : m_diff->formulas)
            {
                row.checked = false;
            }
        }
        m_messagesAreErrors = true;
        return false;
    }
    const std::string done = I18N::Format(
        I18N::Editor::Applied0ItemsAnd1FormulaEntries,
        {std::to_string(changes.added + changes.changed + changes.removed), std::to_string(changes.formulaEntries)});
    const std::string path = m_comparedPath;
    g_MuEditorConsoleUI.LogEditor(path + ": " + done);
    // The differences that are left; what was not checked stays unchecked, so a second click
    // does not take it after all.
    const UncheckedRows unchecked = FindUnchecked(*m_diff);
    if (m_fromServer)
    {
        CompareServerFile(std::string(m_serverText), path);
    }
    else
    {
        Compare(ToPath(path));
    }
    if (m_diff)
    {
        KeepUnchecked(*m_diff, unchecked);
    }
    m_result = done;
    return true;
}

void CMuItemSyncUI::Export(const std::filesystem::path& path)
{
    m_exportPath = ToText(path);
    m_exportResult.clear();
    m_exportFailed = false;
    m_pendingExport.reset();
    const ItemSyncExportPreview preview = PreviewItemSyncExport(path);
    if (!preview.fileExists)
    {
        WriteExport();
        return;
    }
    if (preview.ChangesNothing())
    {
        m_exportResult = I18N::Format(I18N::Editor::TheFileAlreadyHasTheClientData, {m_exportPath});
        return;
    }
    // A file that is there is replaced only after the confirmation (RenderExportPopup).
    m_pendingExport = preview;
}

bool CMuItemSyncUI::ConfirmExport()
{
    if (!m_pendingExport)
    {
        return false;
    }
    m_pendingExport.reset();
    return WriteExport();
}

void CMuItemSyncUI::CancelExport()
{
    m_pendingExport.reset();
}

bool CMuItemSyncUI::WriteExport()
{
    std::vector<ItemDataIssue> issues;
    size_t writtenItems = 0;
    if (!WriteItemSyncExport(ToPath(m_exportPath), writtenItems, issues))
    {
        m_exportFailed = true;
        m_exportResult = I18N::Editor::TheExportCouldNotBeWritten;
        for (const ItemDataIssue& issue : issues)
        {
            m_exportResult += "\n" + issue.ToString();
            g_MuEditorConsoleUI.LogEditor(issue.ToString());
        }
        return false;
    }
    m_exportFailed = false;
    m_exportResult = I18N::Format(I18N::Editor::Exported0ItemsTo1, {std::to_string(writtenItems), m_exportPath});
    g_MuEditorConsoleUI.LogEditor(m_exportResult);
    // A comparison of that file (also one that could not read it) shows what it held before.
    if (m_comparingFiles && (IsSameFile(m_comparedA, m_exportPath) || IsSameFile(m_comparedB, m_exportPath)))
    {
        CompareFiles(ToPath(m_comparedA), ToPath(m_comparedB));
    }
    else if (!m_comparingFiles && IsSameFile(m_comparedPath, m_exportPath))
    {
        Compare(ToPath(m_comparedPath));
    }
    return true;
}

// Apply changes the editor's items only: while they differ from the saved ones, Save Items is
// at hand here too.
void CMuItemSyncUI::RenderUnsaved()
{
    if (!g_ItemEdits.GetHistory().IsModifiedSinceSave())
    {
        return;
    }
    CItemEditorActions::RenderSaveButton();
    CItemEditorPopups::RenderAll();
    ImGui::SameLine();
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextColored(WarningColor, "%s", I18N::Editor::NotSavedYetTheChangesAreOnlyInTheEditor);
    ImGui::PopTextWrapPos();
    ImGui::Separator();
}

// The heading of the incoming values: the server's after Update from server, else the file's.
const char* CMuItemSyncUI::FromTheSource() const
{
    return m_fromServer ? I18N::Editor::FromTheServer : I18N::Editor::FromTheFile;
}

void CMuItemSyncUI::SetAllChecked(bool checked)
{
    SetAllRowsChecked(*m_diff, checked);
}

void CMuItemSyncUI::RenderSummary()
{
    int added = 0;
    int changed = 0;
    int removed = 0;
    CountRows(*m_diff, added, changed, removed);
    if (m_comparingFiles)
    {
        RenderFileLine(I18N::Editor::FileA0, m_comparedA, m_fileA);
        RenderFileLine(I18N::Editor::FileB0, m_comparedB, m_file);
        ImGui::PushTextWrapPos(0.0f);
        if (m_diff->IsEmpty())
        {
            ImGui::TextColored(SuccessColor, "%s",
                               m_fileA.formulas && m_file.formulas ? I18N::Editor::TheFilesMatch
                                                                   : I18N::Editor::TheFilesMatchInAllItems);
        }
        else
        {
            ImGui::TextUnformatted(I18N::Format(I18N::Editor::TheFileDiffCounts,
                                                {std::to_string(added), std::to_string(changed),
                                                 std::to_string(removed), std::to_string(m_diff->formulas.size())})
                                       .c_str());
        }
        ImGui::PopTextWrapPos();
        return;
    }
    RenderFileLine(m_fromServer ? I18N::Editor::UpdateFrom0 : I18N::Editor::ImportFrom0, m_comparedPath, m_file);
    ImGui::PushTextWrapPos(0.0f);
    // What the last Apply did comes before what it left.
    if (!m_result.empty())
    {
        ImGui::TextColored(SuccessColor, "%s", m_result.c_str());
    }
    // The items only the client has are no difference the file makes: they stay as they are.
    if (added + changed == 0 && m_diff->formulas.empty())
    {
        // The client always has formulas, so they were compared when the file has them too.
        const char* matches = I18N::Editor::EveryItemOfTheFileMatchesTheClient;
        if (removed == 0)
        {
            matches = m_file.formulas ? I18N::Editor::TheFileMatchesTheClient
                                      : I18N::Editor::TheFileMatchesTheClientInAllItems;
        }
        ImGui::TextColored(SuccessColor, "%s", matches);
        ImGui::TextDisabled("%s", I18N::Editor::HowToSeeDifferences);
    }
    else
    {
        if (!m_result.empty())
        {
            ImGui::TextDisabled("%s", I18N::Editor::StillListedBelowTheRowsThatWereNotChecked);
        }
        ImGui::TextUnformatted(
            I18N::Format(I18N::Editor::TheDiffCounts,
                         {std::to_string(added), std::to_string(changed), std::to_string(m_diff->formulas.size())})
                .c_str());
    }
    if (removed > 0)
    {
        ImGui::TextUnformatted(
            I18N::Format(I18N::Editor::ItemsOnlyInTheClient0StayUnlessChecked, {std::to_string(removed)}).c_str());
    }
    if (g_ItemDatabase.GetVersion() != m_diffVersion)
    {
        ImGui::TextColored(WarningColor, "%s", I18N::Editor::TheClientDataChangedSinceTheComparison);
    }
    ImGui::PopTextWrapPos();
    if (m_diff->IsEmpty())
    {
        return;
    }
    if (ImGui::SmallButton(I18N::Editor::CheckAll))
    {
        SetAllChecked(true);
    }
    ImGui::SameLine();
    if (ImGui::SmallButton(I18N::Editor::CheckNone))
    {
        SetAllChecked(false);
    }
}

void CMuItemSyncUI::RenderFieldToggle()
{
    if (m_toggleFields.empty())
    {
        return;
    }
    if (std::ranges::find(m_toggleFields, m_toggleField) == m_toggleFields.end())
    {
        m_toggleField = m_toggleFields.front();
    }
    ImGui::SetNextItemWidth(Scaled(FieldColumnWidth));
    if (ImGui::BeginCombo(I18N::Editor::AFieldInAllItems, m_toggleField.c_str()))
    {
        for (const std::string& name : m_toggleFields)
        {
            if (ImGui::Selectable(name.c_str(), name == m_toggleField))
            {
                m_toggleField = name;
            }
        }
        ImGui::EndCombo();
    }
    for (const bool checked : {true, false})
    {
        ImGui::SameLine();
        if (ImGui::SmallButton(checked ? I18N::Editor::CheckThem : I18N::Editor::UncheckThem))
        {
            for (SyncItem& row : m_diff->items)
            {
                for (SyncField& field : row.fields)
                {
                    if (row.change == SyncChange::Changed && field.field == m_toggleField)
                    {
                        field.checked = checked && field.locked.empty();
                    }
                }
            }
        }
    }
}

void CMuItemSyncUI::RenderItems()
{
    // An import lists the items only the client has apart from the others, folded: the file does
    // not change them. A comparison of two files lists them with the others.
    std::vector<int> differ;
    std::vector<int> onlyInClient;
    for (int index = 0; index < static_cast<int>(m_diff->items.size()); ++index)
    {
        const bool apart = !m_comparingFiles && IsOnlyInClient(m_diff->items[static_cast<size_t>(index)]);
        (apart ? onlyInClient : differ).push_back(index);
    }
    if (differ.empty())
    {
        if (m_diff->comparedItems > 0)
        {
            const char* format =
                m_comparingFiles ? I18N::Editor::ItemsAll0AreTheSameInBothFiles : I18N::Editor::ItemsAll0AreTheSame;
            ImGui::TextDisabled("%s", I18N::Format(format, {std::to_string(m_diff->sameItems)}).c_str());
        }
    }
    else
    {
        const int compared = m_diff->comparedItems - static_cast<int>(onlyInClient.size());
        const std::string header =
            I18N::Format(I18N::Editor::ItemsThatDiffer0Of1, {std::to_string(differ.size()), std::to_string(compared)}) +
            "###SyncItems";
        if (RenderItemListHeader(ItemList::Differ, header, differ))
        {
            RenderItemTable("##SyncItems", differ);
        }
    }
    if (!onlyInClient.empty())
    {
        const char* format =
            m_fromServer ? I18N::Editor::OnlyInTheClient0NotOnTheServer : I18N::Editor::OnlyInTheClient0NotInTheFile;
        const std::string header = I18N::Format(format, {std::to_string(onlyInClient.size())}) + "###SyncOnlyInClient";
        if (RenderItemListHeader(ItemList::OnlyInClient, header, onlyInClient))
        {
            RenderItemTable("##SyncOnlyInClient", onlyInClient);
        }
    }
}

bool CMuItemSyncUI::RenderItemListHeader(ItemList list, const std::string& header, const std::vector<int>& rows)
{
    // The header spans the window; the button is drawn over its right end.
    const float right = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x;
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_AllowOverlap;
    if (list == ItemList::Differ)
    {
        flags |= ImGuiTreeNodeFlags_DefaultOpen;
    }
    const bool open = ImGui::CollapsingHeader(header.c_str(), flags);
    const std::string label =
        (m_copiedList == list ? I18N::Format(I18N::Editor::Copied0Rows, {std::to_string(m_copiedRows)})
                              : std::string(I18N::Editor::CopyAsMarkdown)) +
        "###CopyItemList";
    const float width = ImGui::CalcTextSize(label.c_str(), nullptr, true).x + ImGui::GetStyle().FramePadding.x * 2.0f;
    ImGui::SameLine(right - width);
    ImGui::PushID(static_cast<int>(list));
    if (ImGui::SmallButton(label.c_str()))
    {
        CopyItemList(list, rows);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("%s", I18N::Editor::CopiesTheListAsAMarkdownTable);
    }
    ImGui::PopID();
    return open;
}

void CMuItemSyncUI::CopyItemList(ItemList list, const std::vector<int>& rows)
{
    std::vector<const SyncItem*> items;
    items.reserve(rows.size());
    for (const int index : rows)
    {
        items.push_back(&m_diff->items[static_cast<size_t>(index)]);
    }
    const char* title = m_fromServer ? MarkdownOnlyInClientFromServer : MarkdownOnlyInClientFromFile;
    std::string_view changeColumn;
    if (list == ItemList::Differ)
    {
        title = m_comparingFiles ? MarkdownDifferBetweenFiles
                                 : (m_fromServer ? MarkdownDifferFromServer : MarkdownDifferFromFile);
        changeColumn = m_comparingFiles ? MarkdownFromAToB : MarkdownInTheClient;
    }
    const std::string text =
        WriteSyncRowsMarkdown(items, std::string(title) + " (" + std::to_string(rows.size()) + ")", changeColumn);
    ImGui::SetClipboardText(text.c_str());
    m_copiedList = list;
    m_copiedRows = static_cast<int>(rows.size());
}

void CMuItemSyncUI::RenderItemTable(const char* id, const std::vector<int>& rows)
{
    // A comparison of two files has no boxes.
    const bool checkable = !m_comparingFiles;
    constexpr ImGuiTableFlags Flags =
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;
    // A short list is as high as its rows (and the head), a long one scrolls.
    const float rowHeight = ImGui::GetFrameHeight() + ImGui::GetStyle().CellPadding.y * 2.0f;
    const float height =
        std::min(Scaled(ItemListHeight), rowHeight * static_cast<float>(rows.size() + 1) + Scaled(ItemListBorders));
    if (ImGui::BeginTable(id, checkable ? 4 : 3, Flags, ImVec2(0.0f, height)))
    {
        ImGui::TableSetupScrollFreeze(0, 1);
        if (checkable)
        {
            ImGui::TableSetupColumn(CheckColumn, ImGuiTableColumnFlags_WidthFixed, ImGui::GetFrameHeight());
        }
        ImGui::TableSetupColumn(checkable ? I18N::Editor::InTheClient : I18N::Editor::FromAToB,
                                ImGuiTableColumnFlags_WidthFixed, ChangeColumnWidth());
        ImGui::TableSetupColumn(I18N::Editor::Item);
        ImGui::TableSetupColumn(I18N::Editor::Fields, ImGuiTableColumnFlags_WidthFixed, Scaled(FieldsColumnWidth));
        ImGui::TableHeadersRow();
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(rows.size()));
        while (clipper.Step())
        {
            for (int line = clipper.DisplayStart; line < clipper.DisplayEnd; ++line)
            {
                const int index = rows[static_cast<size_t>(line)];
                SyncItem& row = m_diff->items[static_cast<size_t>(index)];
                ImGui::PushID(index);
                ImGui::TableNextRow();
                if (checkable)
                {
                    ImGui::TableNextColumn();
                    bool checked = IsRowChecked(row);
                    ImGui::BeginDisabled(!row.locked.empty());
                    if (ImGui::Checkbox("##Checked", &checked))
                    {
                        SetRowChecked(row, checked);
                    }
                    ImGui::EndDisabled();
                }
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(ChangeName(row.change));
                ImGui::TableNextColumn();
                if (row.known)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
                }
                // The arrow keys move through the rows and show their fields as a click does.
                if (ImGui::Selectable(ItemLabel(row, m_comparingFiles).c_str(), m_shownItem == index,
                                      ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap |
                                          MuEditor::KeyboardSelection::RowFlags))
                {
                    m_shownItem = index;
                    const int itemType = Data::Items::MakeItemType(row.group, row.number);
                    if (g_ItemDatabase.Find(itemType) != nullptr)
                    {
                        g_ItemSelection.Select(itemType);
                    }
                    const bool removedFromClient = checkable && row.change == SyncChange::Removed;
                    m_shownUsage = removedFromClient ? FindItemUsage(itemType) : ItemUsage{};
                }
                if (row.known)
                {
                    ImGui::PopStyleColor();
                }
                ImGui::TableNextColumn();
                ImGui::Text("%d", static_cast<int>(row.fields.size()));
                ImGui::PopID();
            }
        }
        ImGui::EndTable();
    }
    // The shown item's fields come under the list it is in.
    if (std::ranges::find(rows, m_shownItem) != rows.end())
    {
        RenderItemFields(m_diff->items[static_cast<size_t>(m_shownItem)]);
    }
}

void CMuItemSyncUI::RenderItemFields(SyncItem& row)
{
    ImGui::TextUnformatted(I18N::Format(I18N::Editor::FieldsOf0, {ItemLabel(row, m_comparingFiles)}).c_str());
    if (!row.locked.empty())
    {
        ImGui::TextColored(WarningColor, "%s", row.locked.c_str());
    }
    // What Remove item lists before it removes an item (not for a comparison of two files).
    if (!m_comparingFiles && row.change == SyncChange::Removed)
    {
        ImGui::TextUnformatted(I18N::Editor::TheItemAndItsModelEntryAreRemoved);
        if (m_shownUsage.GoesWithItem())
        {
            ImGui::TextUnformatted(I18N::Editor::RemovedWithIt);
            UsageView::RenderGoingWithItem(m_shownUsage);
        }
        ImGui::TextUnformatted(I18N::Editor::WhatUsesItStaysAtItsNumber);
        UsageView::RenderUsesAtNumber(m_shownUsage);
    }
    if (!m_comparingFiles && row.change == SyncChange::New)
    {
        const auto incoming = std::ranges::find_if(m_file.items, [&](const Data::Items::ItemDefinition& item)
                                                   { return item.group == row.group && item.number == row.number; });
        if (incoming != m_file.items.end())
        {
            for (const std::string& note : DescribeNewItem(*incoming))
            {
                ImGui::TextColored(WarningColor, "%s", note.c_str());
            }
        }
    }
    if (IsWhole(row))
    {
        bool known = row.known;
        if (ImGui::Checkbox(I18N::Editor::Known, &known))
        {
            SetKnown(row.group, row.number, "", known);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", I18N::Editor::WhatAKnownDifferenceIs);
        }
    }
    if (row.fields.empty())
    {
        return;
    }
    constexpr ImGuiTableFlags Flags =
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;
    const bool changed = row.change == SyncChange::Changed;
    // A comparison of two files has no boxes, and its sides are file A and file B; an item only
    // file A has lists file A's values.
    const bool checkable = !m_comparingFiles;
    const bool removed = row.change == SyncChange::Removed;
    const int columns = (changed ? 5 : 2) + (checkable ? 1 : 0);
    if (!ImGui::BeginTable("##SyncFields", columns, Flags, ImVec2(0.0f, Scaled(FieldListHeight))))
    {
        return;
    }
    ImGui::TableSetupScrollFreeze(0, 1);
    if (checkable)
    {
        ImGui::TableSetupColumn(CheckColumn, ImGuiTableColumnFlags_WidthFixed, ImGui::GetFrameHeight());
    }
    ImGui::TableSetupColumn(I18N::Editor::Field, ImGuiTableColumnFlags_WidthFixed, Scaled(FieldColumnWidth));
    if (changed)
    {
        ImGui::TableSetupColumn(checkable ? I18N::Editor::Now : I18N::Editor::FileA);
    }
    ImGui::TableSetupColumn(removed ? I18N::Editor::FileA : checkable ? FromTheSource() : I18N::Editor::FileB);
    if (changed)
    {
        ImGui::TableSetupColumn(I18N::Editor::Note);
        ImGui::TableSetupColumn(I18N::Editor::Known, ImGuiTableColumnFlags_WidthFixed, Scaled(KnownColumnWidth));
    }
    ImGui::TableHeadersRow();
    for (size_t index = 0; index < row.fields.size(); ++index)
    {
        SyncField& field = row.fields[index];
        ImGui::PushID(static_cast<int>(index));
        ImGui::TableNextRow();
        if (checkable)
        {
            ImGui::TableNextColumn();
            if (changed)
            {
                ImGui::BeginDisabled(!field.locked.empty());
                ImGui::Checkbox("##Checked", &field.checked);
                ImGui::EndDisabled();
            }
        }
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(field.field.c_str());
        if (changed)
        {
            ImGui::TableNextColumn();
            RenderValue(field.current);
        }
        ImGui::TableNextColumn();
        RenderValue(removed ? field.current : field.incoming);
        if (changed)
        {
            ImGui::TableNextColumn();
            const std::string& note = !field.locked.empty() ? field.locked : field.inexact;
            if (!note.empty())
            {
                ImGui::PushTextWrapPos(0.0f);
                ImGui::TextColored(WarningColor, "%s", note.c_str());
                ImGui::PopTextWrapPos();
            }
            ImGui::TableNextColumn();
            bool known = field.known;
            if (ImGui::Checkbox("##Known", &known))
            {
                SetKnown(row.group, row.number, field.field, known);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s", I18N::Editor::WhatAKnownDifferenceIs);
            }
        }
        ImGui::PopID();
    }
    ImGui::EndTable();
}

void CMuItemSyncUI::RenderFormulas()
{
    const bool checkable = !m_comparingFiles;
    // A file without formulas leaves the client's as they are; two files are compared by formula
    // while both have them.
    if (!m_file.formulas || (m_comparingFiles && !m_fileA.formulas))
    {
        ImGui::PushTextWrapPos(0.0f);
        ImGui::TextDisabled("%s", m_comparingFiles ? I18N::Editor::FormulasNotComparedAFileHasNone
                                                   : I18N::Editor::FormulasNotComparedTheFileHasNone);
        ImGui::PopTextWrapPos();
        return;
    }
    if (m_diff->formulas.empty())
    {
        if (m_diff->comparedFormulaEntries > 0)
        {
            const char* format = m_comparingFiles ? I18N::Editor::FormulaEntriesAll0AreTheSameInBothFiles
                                                  : I18N::Editor::FormulaEntriesAll0AreTheSame;
            ImGui::TextDisabled("%s", I18N::Format(format, {std::to_string(m_diff->sameFormulaEntries)}).c_str());
        }
        return;
    }
    const std::string header =
        I18N::Format(I18N::Editor::FormulaEntriesThatDiffer0Of1,
                     {std::to_string(m_diff->formulas.size()), std::to_string(m_diff->comparedFormulaEntries)}) +
        "###SyncFormulas";
    if (!ImGui::CollapsingHeader(header.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
    {
        return;
    }
    constexpr ImGuiTableFlags Flags =
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;
    if (!ImGui::BeginTable("##SyncFormulas", checkable ? 5 : 4, Flags, ImVec2(0.0f, Scaled(FieldListHeight))))
    {
        return;
    }
    ImGui::TableSetupScrollFreeze(0, 1);
    if (checkable)
    {
        ImGui::TableSetupColumn(CheckColumn, ImGuiTableColumnFlags_WidthFixed, ImGui::GetFrameHeight());
    }
    ImGui::TableSetupColumn(I18N::Editor::Entry, ImGuiTableColumnFlags_WidthFixed, Scaled(FieldColumnWidth));
    ImGui::TableSetupColumn(checkable ? I18N::Editor::Now : I18N::Editor::FileA);
    ImGui::TableSetupColumn(checkable ? FromTheSource() : I18N::Editor::FileB);
    ImGui::TableSetupColumn(I18N::Editor::Known, ImGuiTableColumnFlags_WidthFixed, Scaled(KnownColumnWidth));
    ImGui::TableHeadersRow();
    for (size_t index = 0; index < m_diff->formulas.size(); ++index)
    {
        SyncFormula& row = m_diff->formulas[index];
        ImGui::PushID(static_cast<int>(index));
        ImGui::TableNextRow();
        if (checkable)
        {
            ImGui::TableNextColumn();
            ImGui::Checkbox("##Checked", &row.checked);
        }
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(row.entry.c_str());
        ImGui::TableNextColumn();
        RenderValue(row.current);
        ImGui::TableNextColumn();
        RenderValue(row.incoming);
        ImGui::TableNextColumn();
        bool known = row.known;
        if (ImGui::Checkbox("##Known", &known))
        {
            SetKnownFormula(row.entry, known);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s", I18N::Editor::WhatAKnownDifferenceIs);
        }
        ImGui::PopID();
    }
    ImGui::EndTable();
}

void CMuItemSyncUI::RenderApply()
{
    const ApplyBlocker blocker = GetApplyBlocker();
    ImGui::BeginDisabled(blocker != ApplyBlocker::None);
    // It asks first (RenderApplyPopup).
    if (ImGui::Button(I18N::Editor::ApplyTheCheckedChanges))
    {
        m_confirmApply = true;
    }
    ImGui::EndDisabled();
    ImGui::PushTextWrapPos(0.0f);
    switch (blocker)
    {
    case ApplyBlocker::None:
        ImGui::TextDisabled("%s", I18N::Editor::AfterApplyingTheWindowComparesAgain);
        break;
    case ApplyBlocker::OtherPath:
        ImGui::TextColored(WarningColor, "%s", I18N::Editor::ThePathIsNotTheComparedFile);
        break;
    case ApplyBlocker::NothingChecked:
        ImGui::TextColored(WarningColor, "%s", I18N::Editor::NothingIsChecked);
        break;
    case ApplyBlocker::DataChanged:
        // The summary says so.
        break;
    }
    ImGui::PopTextWrapPos();
}

void CMuItemSyncUI::RenderApplyPopup()
{
    // Open while it waits for its answer, also when ImGui closed it (e.g. the editor was hidden).
    if (m_confirmApply && !m_diff)
    {
        m_confirmApply = false;
    }
    if (m_confirmApply && !ImGui::IsPopupOpen(ApplyPopupId))
    {
        ImGui::OpenPopup(ApplyPopupId);
    }
    const std::string title = std::string(I18N::Editor::ApplyTheImport) + ApplyPopupId;
    if (!ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    const CheckedChanges changes = CountCheckedChanges();
    ImGui::PushTextWrapPos(Scaled(PopupTextWidth));
    ImGui::TextUnformatted(I18N::Format(I18N::Editor::TheImportChanges,
                                        {m_comparedPath, std::to_string(changes.added), std::to_string(changes.changed),
                                         std::to_string(changes.removed), std::to_string(changes.formulaEntries)})
                               .c_str());
    if (changes.removed > 0)
    {
        ImGui::TextColored(
            WarningColor, "%s",
            I18N::Format(I18N::Editor::ItRemoves0ItemsFromTheClient, {std::to_string(changes.removed)}).c_str());
    }
    ImGui::TextDisabled("%s", I18N::Editor::WhatTheImportLeaves);
    ImGui::PopTextWrapPos();
    ImGui::BeginDisabled(GetApplyBlocker() != ApplyBlocker::None);
    if (ImGui::Button(I18N::Editor::Apply))
    {
        m_confirmApply = false;
        Apply();
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        m_confirmApply = false;
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void CMuItemSyncUI::RenderExportPopup()
{
    // Open while the export waits for its answer, also when ImGui closed it.
    if (m_pendingExport && !ImGui::IsPopupOpen(ExportPopupId))
    {
        ImGui::OpenPopup(ExportPopupId);
    }
    const std::string title = std::string(I18N::Editor::ReplaceTheExchangeFile) + ExportPopupId;
    if (!ImGui::BeginPopupModal(title.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        return;
    }
    if (!m_pendingExport)
    {
        ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
        return;
    }
    const ItemSyncExportPreview& preview = *m_pendingExport;
    std::string text;
    if (!preview.readable)
    {
        text = I18N::Format(I18N::Editor::TheFileCannotBeReadReplaceIt, {m_exportPath});
    }
    else if (preview.fileHasFormulas)
    {
        text = I18N::Format(I18N::Editor::TheExportChanges,
                            {m_exportPath, std::to_string(preview.added), std::to_string(preview.changed),
                             std::to_string(preview.removed), std::to_string(preview.formulaEntries)});
    }
    else
    {
        text = I18N::Format(I18N::Editor::TheExportChangesTheFormulasAreAdded,
                            {m_exportPath, std::to_string(preview.added), std::to_string(preview.changed),
                             std::to_string(preview.removed)});
    }
    ImGui::PushTextWrapPos(Scaled(PopupTextWidth));
    ImGui::TextUnformatted(text.c_str());
    ImGui::PopTextWrapPos();
    if (ImGui::Button(I18N::Editor::Replace))
    {
        ConfirmExport();
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button(I18N::Editor::Cancel))
    {
        CancelExport();
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void CMuItemSyncUI::RenderMessages()
{
    for (const std::string& message : m_messages)
    {
        ImGui::PushTextWrapPos(0.0f);
        if (m_messagesAreErrors)
        {
            ImGui::TextColored(ErrorColor, "%s", message.c_str());
        }
        else
        {
            ImGui::TextUnformatted(message.c_str());
        }
        ImGui::PopTextWrapPos();
    }
}

void CMuItemSyncUI::SetKnown(int group, int number, const std::string& field, bool known)
{
    if (known)
    {
        m_known.fields.insert({group, number, field});
    }
    else
    {
        m_known.fields.erase({group, number, field});
    }
    for (SyncItem& row : m_diff->items)
    {
        if (row.group != group || row.number != number)
        {
            continue;
        }
        if (field.empty())
        {
            row.known = known;
            row.checked = !known && row.locked.empty() && row.change == SyncChange::New;
        }
        for (SyncField& entry : row.fields)
        {
            if (entry.field == field)
            {
                entry.known = known;
                entry.checked = !known && entry.locked.empty() && entry.inexact.empty();
            }
        }
    }
    SaveKnownDifferences();
}

void CMuItemSyncUI::SetKnownFormula(const std::string& entry, bool known)
{
    if (known)
    {
        m_known.formulas.insert(entry);
    }
    else
    {
        m_known.formulas.erase(entry);
    }
    for (SyncFormula& row : m_diff->formulas)
    {
        if (row.entry == entry)
        {
            row.known = known;
            row.checked = !known;
        }
    }
    SaveKnownDifferences();
}

void CMuItemSyncUI::SaveKnownDifferences()
{
    std::vector<ItemDataIssue> issues;
    Data::Items::WriteItemTextFile(GetKnownDifferencesFile(), WriteKnownDifferencesJson(m_known), issues);
    for (const ItemDataIssue& issue : issues)
    {
        g_MuEditorConsoleUI.LogEditor(issue.ToString());
    }
}

#endif // _EDITOR
