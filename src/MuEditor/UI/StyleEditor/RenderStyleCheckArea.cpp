#include "stdafx.h"

#ifdef _EDITOR

#include "RenderStyleCheckArea.h"

#include "../MuEditor/Core/MuEditorCore.h"
#include "../MuEditor/UI/ItemTools/ItemCheckTargets.h"
#include "../MuEditor/UI/ItemTools/ItemSelection.h"
#include "Data/DataHandler/EffectData/EffectTypeStorage.h"
#include "Data/DataHandler/LookData/LookDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/LookData/LookChecks.h"
#include "Data/GameData/LookData/LookJson.h"
#include "I18N/All.h"
#include "Render/Models/ZzzBMD.h"
#include "imgui.h"

#ifndef _WIN32
#include "Core/Platform/PathResolve.h"
#endif

#include <charconv>
#include <filesystem>
#include <iterator>
#include <string>
#include <system_error>

namespace MuEditor::Looks
{
namespace
{
using Data::Items::ItemDataIssue;

bool TextureFileExists(const std::string& texture)
{
    std::string file = Data::Looks::GetTextureFilePath(texture);
#ifndef _WIN32
    file = MuResolvePath(file.c_str());
#endif
    std::error_code error;
    return std::filesystem::exists(file, error);
}

int MeshCount(int itemType)
{
    return Models[Data::Items::ToModelSlot(itemType)].NumMeshs;
}

bool IsAboutAnItem(const ItemDataIssue& issue)
{
    return Data::Items::IsValidItemId(issue.group, issue.number);
}

class RenderStyleCheckArea final : public DataChecks::DataCheckArea
{
public:
    const char* GetName() const override
    {
        return I18N::Editor::RenderStyles;
    }

    void Check(std::vector<ItemDataIssue>& issues) override
    {
        const std::filesystem::path directory = Data::Effects::GetEffectDataDirectory();
        Data::Looks::RenderStylesLoadResult result = Data::Looks::LoadRenderStyleFile(directory);
        const std::filesystem::path file = directory / Data::Looks::GetRenderStylesFileName();
        const std::optional<std::string> text = Data::Items::Json::ReadTextFile(file);
        m_lookNames = text ? Data::Looks::ReadLookNames(*text) : std::vector<std::string>{};
        if (!Data::Items::HasFatal(result.issues))
        {
            const Data::Looks::RenderStyleCheckInput input{.models = g_ItemModelDatabase.GetAllSlots(),
                                                           .textureExists = TextureFileExists,
                                                           .meshCount = MeshCount};
            Data::Looks::CheckRenderStyles(result.looks, file.generic_string(), input, result.issues);
        }
        issues.insert(issues.end(), std::make_move_iterator(result.issues.begin()),
                      std::make_move_iterator(result.issues.end()));
    }

    // "chromeMesh0", "chromeMesh0: Kris (0,0)" for a problem about an item, or the file.
    void RenderSubject(const ItemDataIssue& issue) const override
    {
        const std::string look = LookOf(issue);
        if (IsAboutAnItem(issue))
        {
            const int itemType = Data::Items::MakeItemType(issue.group, issue.number);
            const std::string item = g_ItemDatabase.Find(itemType) != nullptr
                                         ? g_ItemDatabase.GetLogName(itemType)
                                         : "(" + std::to_string(issue.group) + "," + std::to_string(issue.number) + ")";
            ImGui::TextUnformatted((look.empty() ? item : look + ": " + item).c_str());
            return;
        }
        if (!look.empty())
        {
            ImGui::TextUnformatted(look.c_str());
            return;
        }
        ImGui::TextDisabled("%s", issue.source.c_str());
    }

    void Open(const ItemDataIssue& issue, bool clicked) override
    {
        if (!IsAboutAnItem(issue))
        {
            return;
        }
        g_ItemSelection.Select(Data::Items::MakeItemType(issue.group, issue.number));
        if (!clicked)
        {
            return;
        }
        const Items::ItemToolWindowRef tool = Items::GetToolWindow(Items::ItemTool::Model);
        g_MuEditorCore.GetToolWindows().*tool.open = true;
        ImGui::SetWindowFocus(tool.windowId);
    }

private:
    // The look of a problem in "looks[3]...", as the file named it at the last check; empty for one of the file.
    std::string LookOf(const ItemDataIssue& issue) const
    {
        constexpr std::string_view Prefix = "looks[";
        const std::string_view field = issue.field;
        if (!field.starts_with(Prefix))
        {
            return "";
        }
        std::size_t index = 0;
        const std::from_chars_result read =
            std::from_chars(field.data() + Prefix.size(), field.data() + field.size(), index);
        if (read.ec != std::errc{} || index >= m_lookNames.size())
        {
            return "";
        }
        return m_lookNames[index];
    }

    // The names of the file's looks in file order, at the last check.
    std::vector<std::string> m_lookNames;
};
} // namespace

std::unique_ptr<DataChecks::DataCheckArea> CreateRenderStyleCheckArea()
{
    return std::make_unique<RenderStyleCheckArea>();
}
} // namespace MuEditor::Looks

#endif // _EDITOR
