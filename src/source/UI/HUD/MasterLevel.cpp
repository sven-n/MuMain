#include "stdafx.h"
#include "App/Platform/Windows/Winmain.h"
#include "Render/Textures/ZzzTexture.h"
#include "GameLogic/Items/CSItemOption.h"
#include "UI/Social/SocialWindowBase.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/HUD/MasterLevel.h"
#include "UI/HUD/Skills/MasterSkillTreeLayout.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "GameLogic/Skills/SkillManager.h"
#include "Engine/Object/ZzzInventory.h"
#include "UI/Scaling/UITransform.h"

#include "Core/Utilities/StringUtils.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlTooltip.h"
#include "UI/Tooltip/LegacyTextListTooltip.h"
#include <RmlUi/Core/ElementDocument.h>

namespace
{
    _MASTER_SKILLTREE_DATA m_stMasterSkillTreeData[MAX_MASTER_SKILL_DATA];
    _MASTER_SKILL_TOOLTIP m_stMasterSkillTooltip[MAX_MASTER_SKILL_DATA];

    // Distinct Tooltip::Owner tokens: each hint hides only itself, so whichever of the three is
    // re-shown this frame is never clobbered by another's Hide().
    const int kNodeHintOwner = 0;
    const int kExperienceHintOwner = 0;

    // Where the original anchored its hints, reference px: the node hint under the icon's left
    // edge (above it for the lower ranks), the EXP hint under the label.
    constexpr int kIconOffsetX = 8;
    constexpr int kNodeHintBelowIcon = 33;
    constexpr int kNodeHintFlipTop = 300;
    constexpr float kExperienceHintX = 466.0f;
    constexpr float kExperienceHintY = 26.0f;

    // The tree's own rectangle: 640 wide, down to the bottom of its 428-high background art.
    constexpr int kTreeHeight = 428;

    template <typename Model>
    void SyncString(RmlModelBinder<Model>& binder, Rml::String Model::* field, const char* name, Rml::String value)
    {
        Model& model = binder.GetModel();
        if (model.*field == value)
            return;
        model.*field = std::move(value);
        binder.MarkDirty(name);
    }

    template <typename Model>
    void SyncFloat(RmlModelBinder<Model>& binder, float Model::* field, const char* name, float value)
    {
        Model& model = binder.GetModel();
        if (model.*field == value)
            return;
        model.*field = value;
        binder.MarkDirty(name);
    }
}


mu::ui::window::CMasterLevel::CMasterLevel()
{
    m_pNewUIMng = nullptr;
    this->ConsumePoint = 0;
    this->CurSkillID = 0;
    this->classCode = MASTER_SKILL_TREE_CLASS_NONE;
    this->CategoryTextIndex = 0;
    this->ClassNameTextIndex = 0;
    this->InitMasterSkillPoint();
    this->ClearSkillTreeData();
    this->ClearSkillTooltipData();
}

mu::ui::window::CMasterLevel::~CMasterLevel()
{
    this->Release();
}

BYTE mu::ui::window::CMasterLevel::GetConsumePoint() const
{
    return this->ConsumePoint;
}

int mu::ui::window::CMasterLevel::GetCurSkillID() const
{
    return this->CurSkillID;
}

bool mu::ui::window::CMasterLevel::Create(CManager* pNewUIMng)
{
    if (nullptr == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_MASTER_LEVEL, this);

    this->SetPos();

    this->LoadImages();

    BuildRmlUi();

    return true;
}

void mu::ui::window::CMasterLevel::Release()
{
    this->ClearSkillTreeData();
    this->ClearSkillTooltipData();
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = nullptr;
    }

    m_RmlView.Release();
    m_RmlBgView.Release();
}

void mu::ui::window::CMasterLevel::SetPos()
{
    this->PosX = 0;
    this->PosY = 0;
    this->width = REFERENCE_WIDTH;
    this->height = kTreeHeight;
}

void mu::ui::window::CMasterLevel::OpenMasterSkillTreeData(const wchar_t* path)
{
    memset(m_stMasterSkillTreeData, 0, sizeof(m_stMasterSkillTreeData));

    FILE* fp = _wfopen(path, L"rb");

    wchar_t Text[256];

    if (fp == nullptr)
    {
        mu_swprintf(Text, L"%ls - File not exist.", path);
        g_ErrorReport.Write(Text);
        MessageBox(g_hWnd, Text, nullptr, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        return;
    }

    constexpr int Size = sizeof(_MASTER_SKILLTREE_DATA);

    auto Buffer = new BYTE[Size * MAX_MASTER_SKILL_DATA];

    fread(Buffer, Size * MAX_MASTER_SKILL_DATA, 1, fp);

    DWORD dwCheckSum;

    fread(&dwCheckSum, sizeof(DWORD), 1u, fp);

    fclose(fp);

    if (dwCheckSum != GenerateCheckSum2(Buffer, 12288, 0x2BC1))
    {
        mu_swprintf(Text, L"%ls - File corrupted.", path);
        g_ErrorReport.Write(Text);
        MessageBox(g_hWnd, Text, nullptr, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        return;
    }

    BYTE* pSeek = Buffer;

    for (int i = 0; i < MAX_MASTER_SKILL_DATA; i++)
    {
        BuxConvert(pSeek, Size);

        memcpy(&m_stMasterSkillTreeData[i], pSeek, Size);

        pSeek += Size;

        if (pSeek == nullptr)
        {
            break;
        }
    }

    delete[] Buffer;
}

void mu::ui::window::CMasterLevel::OpenMasterSkillTooltip(const wchar_t* path)
{
    memset(m_stMasterSkillTooltip, 0, sizeof(m_stMasterSkillTooltip));

    FILE* fp = _wfopen(path, L"rb");

    if (fp == nullptr)
    {
        wchar_t Text[256];
        mu_swprintf(Text, L"%ls - File not exist.", path);
        g_ErrorReport.Write(Text);
        MessageBox(g_hWnd, Text, nullptr, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        return;
    }

    constexpr int record_size = sizeof(_MASTER_SKILL_TOOLTIP_FILE);
    auto file_buffer = new BYTE[record_size * MAX_MASTER_SKILL_DATA];
    fread(file_buffer, record_size * MAX_MASTER_SKILL_DATA, 1, fp);
    DWORD dwCheckSum;
    fread(&dwCheckSum, sizeof(DWORD), 1u, fp);
    fclose(fp);

    BYTE* pSeek = file_buffer;

    for (int i = 0; i < MAX_MASTER_SKILL_DATA; i++)
    {
        BuxConvert(pSeek, record_size);

        _MASTER_SKILL_TOOLTIP_FILE current{ };
        memcpy(&current, pSeek, record_size);

        const auto target = &m_stMasterSkillTooltip[i];
        target->SkillNumber = static_cast<ActionSkillType>(current.SkillNumber);
        target->ClassCode = static_cast<MASTER_SKILL_TREE_CLASS>(current.ClassCode);
        CMultiLanguage::ConvertFromUtf8(target->Info1, current.Info1);
        CMultiLanguage::ConvertFromUtf8(target->Info2, current.Info2);
        CMultiLanguage::ConvertFromUtf8(target->Info3, current.Info3);
        CMultiLanguage::ConvertFromUtf8(target->Info4, current.Info4);
        CMultiLanguage::ConvertFromUtf8(target->Info5, current.Info5);
        CMultiLanguage::ConvertFromUtf8(target->Info6, current.Info6);
        CMultiLanguage::ConvertFromUtf8(target->Info7, current.Info7);

        pSeek += record_size;

        if (pSeek == nullptr)
        {
            break;
        }
    }

    delete[] file_buffer;
}

void mu::ui::window::CMasterLevel::InitMasterSkillPoint()
{
    for (int i = 0; i < 3; i++)
    {
        this->CategoryPoint[i] = 0;
        for (int k = 0; k < 10; k++)
        {
            this->skillPoint[i][k] = 0;
        }
    }
}

void mu::ui::window::CMasterLevel::SetMasterType(CLASS_TYPE Class)
{
    switch (Class)
    {
    case CLASS_GRANDMASTER:
        this->classCode = MASTER_SKILL_TREE_CLASS_GRANDMASTER;
        break;
    case CLASS_BLADEMASTER:
        this->classCode = MASTER_SKILL_TREE_CLASS_BLADEMASTER;
        break;
    case CLASS_HIGHELF:
        this->classCode = MASTER_SKILL_TREE_CLASS_HIGHELF;
        break;
    case CLASS_DUELMASTER:
        this->classCode = MASTER_SKILL_TREE_CLASS_DUELMASTER;
        break;
    case CLASS_LORDEMPEROR:
        this->classCode = MASTER_SKILL_TREE_CLASS_LORDEMPEROR;
        break;
    case CLASS_DIMENSIONMASTER:
        this->classCode = MASTER_SKILL_TREE_CLASS_DIMENSIONMASTER;
        break;
    case CLASS_TEMPLENIGHT:
        this->classCode = MASTER_SKILL_TREE_CLASS_TEMPLEKNIGHT;
        break;
    default:
        break;
    }

    this->SetMasterSkillTreeData();

    this->SetMasterSkillToolTipData();

    switch (Class)
    {
    case CLASS_WIZARD:
    case CLASS_SOULMASTER:
    case CLASS_GRANDMASTER:
        this->CategoryTextIndex = 1751;
        this->ClassNameTextIndex = 1669;
        break;
    case CLASS_KNIGHT:
    case CLASS_BLADEKNIGHT:
    case CLASS_BLADEMASTER:
        this->CategoryTextIndex = 1755;
        this->ClassNameTextIndex = 1668;
        break;
    case CLASS_ELF:
    case CLASS_MUSEELF:
    case CLASS_HIGHELF:
        this->CategoryTextIndex = 1759;
        this->ClassNameTextIndex = 1670;
        break;
    case CLASS_DARK:
    case CLASS_DUELMASTER:
        this->CategoryTextIndex = 1763;
        this->ClassNameTextIndex = 1671;
        break;
    case CLASS_DARK_LORD:
    case CLASS_LORDEMPEROR:
        this->CategoryTextIndex = 1767;
        this->ClassNameTextIndex = 1672;
        break;
    case CLASS_SUMMONER:
    case CLASS_BLOODYSUMMONER:
    case CLASS_DIMENSIONMASTER:
        this->CategoryTextIndex = 3136;
        this->ClassNameTextIndex = 1689;
        break;
    case CLASS_RAGEFIGHTER:
    case CLASS_TEMPLENIGHT:
        this->CategoryTextIndex = 3330;
        this->ClassNameTextIndex = 3151;
        break;
    default:
        return;
    }
}

void mu::ui::window::CMasterLevel::SetMasterSkillTreeData()
{
    this->ClearSkillTreeData();

    for (int i = 0; i < MAX_MASTER_SKILL_DATA; i++)
    {
        if (m_stMasterSkillTreeData[i].Index == 0)
        {
            break;
        }

        if ((this->classCode & m_stMasterSkillTreeData[i].ClassCode) == 0)
        {
            continue;
        }

        if (!this->map_masterData.insert(std::pair<BYTE, _MASTER_SKILLTREE_DATA>(m_stMasterSkillTreeData[i].Index, m_stMasterSkillTreeData[i])).second)
        {
            break;
        }
    }
}

void mu::ui::window::CMasterLevel::SetMasterSkillToolTipData()
{
    this->ClearSkillTooltipData();

    for (int i = 0; i < MAX_MASTER_SKILL_DATA; i++)
    {
        if (m_stMasterSkillTooltip[i].SkillNumber == 0)
        {
            break;
        }

        if ((this->classCode & m_stMasterSkillTooltip[i].ClassCode) == 0)
        {
            continue;
        }

        if (!this->map_masterSkillToolTip.insert(std::pair(m_stMasterSkillTooltip[i].SkillNumber, m_stMasterSkillTooltip[i])).second)
        {
            break;
        }
    }
}

bool mu::ui::window::CMasterLevel::SetMasterSkillTreeInfo(int index, BYTE skillLevel, float value, float nextvalue)
{
    const auto it = this->map_masterData.find(index);

    if (it == this->map_masterData.end())
    {
        return false;
    }

    const CSkillTreeInfo skillInfo = { skillLevel, value, nextvalue };
    CharacterAttribute->MasterSkillInfo[it->second.Skill] = skillInfo;

    this->CategoryPoint[it->second.Group] += skillLevel;

    return true;
}

int mu::ui::window::CMasterLevel::SetDivideString(wchar_t* text, int isItemTollTip, int TextNum, int iTextColor, int iTextBold, bool isPercent)
{
    if (text == nullptr)
    {
        return TextNum;
    }

    constexpr wchar_t alpszDst[10][256] = {};

    int  nLine = 0;

    if (isItemTollTip == 0)
    {
        nLine = DivideStringByPixel((LPTSTR)alpszDst, 10, 256, text, 150, true, 35);
    }
    else if (isItemTollTip == 1)
    {
        nLine = DivideStringByPixel((LPTSTR)alpszDst, 10, 256, text, 200, true, 35);
    }

    for (int i = 0; i < nLine; i++)
    {
        TextListColor[TextNum] = iTextColor;

        TextBold[TextNum] = iTextBold;

        std::wstring cText = alpszDst[i];

        if (isPercent)
        {
            for (int j = cText.find(L"%", 0); j != -1; j = cText.find(L"%", j + 2))
            {
                cText.insert(j, L"%");
            }
        }

        mu_swprintf(TextList[TextNum], cText.c_str());

        TextNum++;
    }

    return TextNum;
}

bool mu::ui::window::CMasterLevel::Render()
{
    // Nothing native left: the background, the nodes, the header and the close button are all
    // RmlUi (master_level.rml). Kept because CObject requires the override.
    return true;
}

bool mu::ui::window::CMasterLevel::Update()
{
    SyncRmlModel();
    return true;
}

bool mu::ui::window::CMasterLevel::UpdateMouseEvent()
{
    // Node presses, hints and the close button are RmlUi events now. What is left is claiming the
    // tree's own rectangle, so a click on it never reaches a window or the world underneath.
    if (mu::ui::window::WindowGeometry(this->PosX, this->PosY, this->width, this->height).Contains(MouseX, MouseY) == true)
    {
        return false;
    }

    return true;
}

bool mu::ui::window::CMasterLevel::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_MASTER_LEVEL) == false || mu::ui::window::IsPress(VK_ESCAPE) == false && mu::ui::window::IsPress('A') == false)
    {
        return true;
    }

    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MASTER_LEVEL);

    PlayBuffer(SOUND_CLICK01);

    return false;
}

float mu::ui::window::CMasterLevel::GetLayerDepth()
{
    return 10.1000004;
}

void mu::ui::window::CMasterLevel::LoadImages()
{
    // The tree itself draws from RmlUi spritesheets over the same files (master_level.rcss); the
    // two icon atlases stay loaded for the HUD skill slots and the MU Helper, which still draw
    // master skill icons natively from these slots.
    LoadBitmap(L"Interface\\new_Master_Icon.jpg", IMAGE_MASTER_INTERFACE + 2, GL_LINEAR, GL_CLAMP, true, false);
    LoadBitmap(L"Interface\\new_Master_Non_Icon.jpg", IMAGE_MASTER_INTERFACE + 3, GL_LINEAR);
}

void mu::ui::window::CMasterLevel::UnloadImages()
{
    DeleteBitmap(IMAGE_MASTER_INTERFACE + 2, false);
    DeleteBitmap(IMAGE_MASTER_INTERFACE + 3, false);
}

void mu::ui::window::CMasterLevel::BindRmlModel(Rml::DataModelConstructor& c, MasterLevelRmlModel& model)
{
    c.Bind("scale_x", &model.scaleX);
    c.Bind("scale_y", &model.scaleY);
    c.Bind("inverse_scale_x", &model.inverseScaleX);
    c.Bind("inverse_scale_y", &model.inverseScaleY);
    c.Bind("text_px", &model.textPx);

    c.Bind("class_name_text", &model.classNameText);
    c.Bind("close_hint", &model.closeHint);
    c.Bind("master_level_text", &model.masterLevelText);
    c.Bind("level_point_text", &model.levelPointText);
    c.Bind("experience_text", &model.experienceText);
    c.Bind("column_text_0", &model.columnText0);
    c.Bind("column_text_1", &model.columnText1);
    c.Bind("column_text_2", &model.columnText2);

    auto node = c.RegisterStruct<MasterLevelNodeEntry>();
    node.RegisterMember("id", &MasterLevelNodeEntry::id);
    node.RegisterMember("column", &MasterLevelNodeEntry::column);
    node.RegisterMember("slot", &MasterLevelNodeEntry::slot);
    node.RegisterMember("rank", &MasterLevelNodeEntry::rank);
    node.RegisterMember("icon", &MasterLevelNodeEntry::icon);
    node.RegisterMember("usable", &MasterLevelNodeEntry::usable);
    node.RegisterMember("arrow", &MasterLevelNodeEntry::arrow);
    node.RegisterMember("level_text", &MasterLevelNodeEntry::levelText);
    c.RegisterArray<std::vector<MasterLevelNodeEntry>>();
    c.Bind("nodes", &model.nodes);

    // Only recorded here and acted on in Update(): the press opens a modal dialog, which
    // must not happen inside this document's own event dispatch.
    c.BindEventCallback("master_node_press",
                        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                        {
                            if (arguments.size() == 1)
                                m_PressedNodeId = arguments[0].Get<int>(-1);
                        });
    c.BindEventCallback("master_node_hover",
                        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                        {
                            if (arguments.size() == 1)
                                m_HoveredNodeId = arguments[0].Get<int>(-1);
                        });
    c.BindEventCallback("master_experience_hover",
                        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& arguments)
                        {
                            if (arguments.size() == 1)
                                m_bExperienceHovered = arguments[0].Get<int>(0) != 0;
                        });
    c.BindEventCallback("master_close", [](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        { g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MASTER_LEVEL); });
}

void mu::ui::window::CMasterLevel::BuildRmlUi()
{
    m_RmlView.Ensure();
    m_RmlBgView.Ensure();
}

void mu::ui::window::CMasterLevel::SyncRmlModel()
{
    BuildRmlUi();
    if (!m_RmlView.Document())
        return;

    const bool visible = IsVisible();
    const bool wasVisible = m_RmlView.Document()->IsVisible();
    // Show() pulls the tree to the front of the main context, above the HUD documents it covers,
    // as the original drew it over them.
    UI::RmlBridge::SyncDocumentVisibility(m_RmlView.Document(), visible);
    SyncBackgroundVisibility(visible);

    if (!visible)
    {
        if (wasVisible)
            ResetHints();
        return;
    }

    SyncTransform();
    SyncHeaderTexts();
    RebuildNodeModel();

    if (m_PressedNodeId >= 0)
    {
        const int nodeId = m_PressedNodeId;
        m_PressedNodeId = -1;
        OnNodePressed(nodeId);
    }

    SyncHints();
}

void mu::ui::window::CMasterLevel::SyncBackgroundVisibility(bool visible)
{
    if (m_RmlBgView.Document() == nullptr || m_RmlBgView.Document()->IsVisible() == visible)
        return;

    if (!visible)
    {
        m_RmlBgView.Document()->Hide();
        return;
    }

    // Behind every other background document: the bottom HUD's own art draws over this black.
    m_RmlBgView.Document()->Show(Rml::ModalFlag::None, Rml::FocusFlag::None);
    m_RmlBgView.Document()->PushToBack();
}

void mu::ui::window::CMasterLevel::SyncTransform()
{
    // CManager scopes LayoutMode::Hud around this window: W/640 x H/480, no offset. The inverse is
    // pushed rather than computed in the markup so each text leaf's transform stays a plain
    // binding.
    const UI::Scaling::Transform transform = UI::Scaling::GetActiveTransform();
    SyncFloat(m_RmlView.Binder(), &MasterLevelRmlModel::scaleX, "scale_x", transform.scaleX);
    SyncFloat(m_RmlView.Binder(), &MasterLevelRmlModel::scaleY, "scale_y", transform.scaleY);
    SyncFloat(m_RmlView.Binder(), &MasterLevelRmlModel::inverseScaleX, "inverse_scale_x", 1.0f / transform.scaleX);
    SyncFloat(m_RmlView.Binder(), &MasterLevelRmlModel::inverseScaleY, "inverse_scale_y", 1.0f / transform.scaleY);
    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());
}

void mu::ui::window::CMasterLevel::SyncHeaderTexts()
{
    wchar_t buffer[256] = {};

    SyncString(m_RmlView.Binder(), &MasterLevelRmlModel::classNameText, "class_name_text",
               StringUtils::WideToNarrow(I18N::Game::Lookup(this->ClassNameTextIndex)));
    SyncString(m_RmlView.Binder(), &MasterLevelRmlModel::closeHint, "close_hint",
               StringUtils::WideToNarrow(I18N::Game::Close388));

    mu_swprintf(buffer, I18N::Game::MasterLevelD, Master_Level_Data.nMLevel);
    SyncString(m_RmlView.Binder(), &MasterLevelRmlModel::masterLevelText, "master_level_text",
               StringUtils::WideToNarrow(buffer));

    mu_swprintf(buffer, I18N::Game::LevelPointD, Master_Level_Data.nMLevelUpMPoint);
    SyncString(m_RmlView.Binder(), &MasterLevelRmlModel::levelPointText, "level_point_text",
               StringUtils::WideToNarrow(buffer));

    Rml::String experienceText;
    if (Master_Level_Data.lNext_MasterLevel_Experince != 0)
    {
        const double percent = UI::Skills::MasterTree::ExperiencePercent(
            {Master_Level_Data.nMLevel, Master_Level_Data.lMasterLevel_Experince,
             Master_Level_Data.lNext_MasterLevel_Experince});
        mu_swprintf(buffer, I18N::Game::EXP62f, percent);
        experienceText = StringUtils::WideToNarrow(buffer);
    }
    SyncString(m_RmlView.Binder(), &MasterLevelRmlModel::experienceText, "experience_text", std::move(experienceText));

    Rml::String MasterLevelRmlModel::* const columnFields[MAX_MASTER_SKILL_CATEGORY] = {
        &MasterLevelRmlModel::columnText0, &MasterLevelRmlModel::columnText1, &MasterLevelRmlModel::columnText2};
    const char* const columnNames[MAX_MASTER_SKILL_CATEGORY] = {"column_text_0", "column_text_1", "column_text_2"};
    for (int column = 0; column < MAX_MASTER_SKILL_CATEGORY; ++column)
    {
        mu_swprintf(buffer, I18N::Game::Lookup(this->CategoryTextIndex + column), this->CategoryPoint[column]);
        SyncString(m_RmlView.Binder(), columnFields[column], columnNames[column], StringUtils::WideToNarrow(buffer));
    }
}

void mu::ui::window::CMasterLevel::RebuildNodeModel()
{
    MasterLevelRmlModel& model = m_RmlView.GetModel();

    std::vector<MasterLevelNodeEntry> nodes;
    nodes.reserve(this->map_masterData.size());

    // In tree order, as native drew them: IsNodeUsable()'s rank check reads the levels the
    // previous ranks recorded on the way (CheckRankPoint()), and later nodes paint over the arrows
    // of earlier ones.
    for (const auto& [treeIndex, skillData] : this->map_masterData)
    {
        if (skillData.Group >= MAX_MASTER_SKILL_CATEGORY)
            continue;

        const SKILL_ATTRIBUTE& skillAttribute = SkillAttribute[skillData.Skill];
        const bool usable = IsNodeUsable(skillData);

        MasterLevelNodeEntry entry;
        entry.id = treeIndex;
        entry.column = skillData.Group;
        entry.slot = UI::Skills::MasterTree::SlotInRank(skillData.Index);
        entry.rank = skillAttribute.SkillRank;
        entry.icon = "image(" + UI::Skills::MasterTree::IconSpriteName(skillAttribute.Magic_Icon, usable) + ")";
        entry.usable = usable;
        entry.arrow = skillData.ArrowDirection;
        entry.levelText = std::to_string(CharacterAttribute->MasterSkillInfo[skillData.Skill].GetSkillLevel());
        nodes.push_back(std::move(entry));
    }

    // Only publish a genuine change: the tree is static until a point is spent or equipment
    // changes, and marking the array dirty re-runs every node's bindings.
    const bool changed = nodes.size() != model.nodes.size() ||
                         !std::equal(nodes.begin(), nodes.end(), model.nodes.begin(),
                                     [](const MasterLevelNodeEntry& a, const MasterLevelNodeEntry& b)
                                     {
                                         return a.id == b.id && a.column == b.column && a.slot == b.slot &&
                                                a.rank == b.rank &&
                                                a.icon == b.icon && a.usable == b.usable && a.arrow == b.arrow &&
                                                a.levelText == b.levelText;
                                     });
    if (!changed)
        return;

    model.nodes = std::move(nodes);
    m_RmlView.MarkDirty("nodes");
}

bool mu::ui::window::CMasterLevel::IsNodeUsable(const _MASTER_SKILLTREE_DATA& skillData)
{
    const auto skill = skillData.Skill;
    const BYTE rank = SkillAttribute[skill].SkillRank;
    const BYTE skillLevel = CharacterAttribute->MasterSkillInfo[skill].GetSkillLevel();

    return this->CheckParentSkill(skillData) && this->CheckRankPoint(skillData.Group, rank, skillLevel) &&
           this->CheckBeforeSkill(skill, skillLevel) && g_csItemOption.IsNonWeaponSkillOrIsSkillEquipped(skill);
}

void mu::ui::window::CMasterLevel::SyncHints()
{
    // Native showed no hint on the frame of a press.
    const bool pressing = mu::ui::window::IsPress(VK_LBUTTON);

    if (pressing || m_HoveredNodeId < 0 || !ShowNodeHint(m_HoveredNodeId))
        UI::RmlBridge::Tooltip::Hide(&kNodeHintOwner);

    if (!pressing && m_bExperienceHovered)
        ShowExperienceHint();
    else
        UI::RmlBridge::Tooltip::Hide(&kExperienceHintOwner);
}

void mu::ui::window::CMasterLevel::ResetHints()
{
    // A document hidden under the pointer sends no mouseout.
    m_HoveredNodeId = -1;
    m_PressedNodeId = -1;
    m_bExperienceHovered = false;
    UI::RmlBridge::Tooltip::Hide(&kNodeHintOwner);
    UI::RmlBridge::Tooltip::Hide(&kExperienceHintOwner);
}

void mu::ui::window::CMasterLevel::ShowExperienceHint()
{
    TextList[0][0] = 0;
    TextBold[0] = 0;
    TextListColor[0] = 0;
    mu_swprintf(TextList[0], L"%I64d / %I64d", Master_Level_Data.lMasterLevel_Experince,
                Master_Level_Data.lNext_MasterLevel_Experince);

    const UI::Scaling::Transform activeTransform = UI::Scaling::GetActiveTransform();
    UI::Tooltip::ShowLegacyTextList(
        1,
        UI::Scaling::PositionX(activeTransform, kExperienceHintX),
        UI::Scaling::PositionY(activeTransform, kExperienceHintY),
        UI::Tooltip::Placement::Below, &kExperienceHintOwner);
}

bool mu::ui::window::CMasterLevel::ShowNodeHint(int nodeId)
{
    const auto it = this->map_masterData.find(static_cast<BYTE>(nodeId));
    if (it == this->map_masterData.end() || it->second.Group >= MAX_MASTER_SKILL_CATEGORY)
        return false;

    const auto tooltip = this->map_masterSkillToolTip.find(it->second.Skill);
    if (tooltip == this->map_masterSkillToolTip.end())
        return false;

    const int lineCount = this->BuildNodeHintLines(it->second, tooltip->second);

    // The theme places the node, so the hint reads the hovered one's own box back; the grid the
    // original drew it on is the first-frame fallback, the same convention as the panel sizes.
    const auto position =
        UI::Skills::MasterTree::NodeBoxPosition(it->second.Group, UI::Skills::MasterTree::SlotInRank(it->second.Index),
                                                SkillAttribute[it->second.Skill].SkillRank);
    float nodeLeft = static_cast<float>(position.left);
    float nodeTop = static_cast<float>(position.top);
    const std::string nodeElementId = "node_" + std::to_string(nodeId);
    UI::RmlBridge::RefreshLogicalAnchorPosition(m_RmlView.Document(), "panel", nodeElementId.c_str(), POINT{0, 0}, nodeLeft,
                                                nodeTop);
    const UI::Scaling::Transform activeTransform = UI::Scaling::GetActiveTransform();
    UI::Tooltip::ShowLegacyTextList(
        lineCount,
        UI::Scaling::PositionX(activeTransform, nodeLeft + kIconOffsetX),
        UI::Scaling::PositionY(activeTransform, nodeTop + kNodeHintBelowIcon),
        nodeTop > kNodeHintFlipTop ? UI::Tooltip::Placement::Above : UI::Tooltip::Placement::Below,
        &kNodeHintOwner);
    return true;
}

int mu::ui::window::CMasterLevel::BuildNodeHintLines(const _MASTER_SKILLTREE_DATA& skillData,
                                                     const _MASTER_SKILL_TOOLTIP& tooltip)
{
    const auto Skill = skillData.Skill;
    const SKILL_ATTRIBUTE* p = &SkillAttribute[Skill];

    auto skillInfo = CharacterAttribute->MasterSkillInfo[Skill];
    const auto skillLevel = skillInfo.GetSkillLevel();
    auto skillValue = skillInfo.GetSkillValue();
    const auto skillNextValue = skillInfo.GetSkillNextValue();

    for (int i = 0; i < 30; i++)
    {
        TextList[i][0] = 0;
    }

    memset(TextBold, 0, sizeof(TextBold));

    for (int i = 0; i < 30; i++)
    {
        TextListColor[i] = i == 0 ? TEXT_COLOR_YELLOW : TEXT_COLOR_WHITE;
    }

    int lineCount = 0;

    mu_swprintf(TextList[lineCount], L"%ls", p->Name);

    TextBold[lineCount] = true;

    lineCount++;

    mu_swprintf(TextList[lineCount], tooltip.Info1, p->SkillRank, skillLevel, skillData.MaxLevel);

    lineCount++;

    wchar_t buffer[512] = {};

    if (skillData.DefValue == -1.0f)
    {
        mu_swprintf(buffer, tooltip.Info2);
    }
    else
    {
        mu_swprintf(buffer, tooltip.Info2, skillLevel != 0 ? skillValue : skillData.DefValue);
    }

    lineCount = this->SetDivideString(buffer, 0, lineCount, 0, 0, true);

    if (skillLevel != 0 && skillLevel < skillData.MaxLevel)
    {
        mu_swprintf(buffer, I18N::Game::NextLevel);

        lineCount = this->SetDivideString(buffer, 0, lineCount, 4, 0, true);

        TextBold[lineCount] = 1;

        mu_swprintf(buffer, tooltip.Info2, skillNextValue);

        lineCount = this->SetDivideString(buffer, 0, lineCount, 0, 0, true);
    }

    if (skillLevel < skillData.MaxLevel)
    {
        mu_swprintf(buffer, I18N::Game::Requirements3329);

        lineCount = this->SetDivideString(buffer, 0, lineCount, 1, 0, true);

        TextBold[lineCount] = 1;

        mu_swprintf(buffer, tooltip.Info3, skillData.RequiredPoints);

        if (skillData.RequiredPoints <= Master_Level_Data.nMLevelUpMPoint)
        {
            lineCount = this->SetDivideString(buffer, 0, lineCount, 0, 0, true);
        }
        else
        {
            lineCount = this->SetDivideString(buffer, 0, lineCount, 2, 0, true);
        }
    }

    int iTextColor = this->CheckBeforeSkill(Skill, skillLevel) == true ? 0 : 2;

    mu_swprintf(buffer, tooltip.Info4);

    lineCount = this->SetDivideString(buffer, 0, lineCount, iTextColor, 0, true);

    if (skillLevel < skillData.MaxLevel && p->SkillRank != 1)
    {
        iTextColor = this->CheckRankPoint(skillData.Group, p->SkillRank, skillLevel) == true ? 0 : 2;

        mu_swprintf(buffer, tooltip.Info5);

        lineCount = this->SetDivideString(buffer, 0, lineCount, iTextColor, 0, true);

        for (int i = 0; i < MAX_MASTER_SKILL_REQUIRES; i++)
        {
            const auto RequireSkill = skillData.RequireSkill[i];

            if (RequireSkill >= AT_SKILL_MASTER_BEGIN && RequireSkill <= AT_SKILL_MASTER_END)
            {
                auto requiredSkill = CharacterAttribute->MasterSkillInfo[RequireSkill];
                iTextColor = requiredSkill.GetSkillValue() < 10 ? 2 : 0;
                mu_swprintf(buffer, i == 0 ? tooltip.Info6 : tooltip.Info7);
                lineCount = this->SetDivideString(buffer, 0, lineCount, iTextColor, 0, true);
            }
        }
    }

    return lineCount;
}

void mu::ui::window::CMasterLevel::OnNodePressed(int nodeId)
{
    const auto it = this->map_masterData.find(static_cast<BYTE>(nodeId));
    if (it == this->map_masterData.end())
        return;

    this->CheckAttributeArea(it->second);
}

bool mu::ui::window::CMasterLevel::CheckAttributeArea(const _MASTER_SKILLTREE_DATA& skillData)
{
    if (skillData.Group < 0 || skillData.Group >= 3)
    {
        return false;
    }

    const auto lpskill = &SkillAttribute[skillData.Skill];

    if (lpskill == nullptr)
    {
        return false;
    }

    const auto skillPoint = CharacterAttribute->MasterSkillInfo[skillData.Skill].GetSkillLevel();

    PlayBuffer(SOUND_CLICK01);

    if (!this->CheckSkillPoint(Master_Level_Data.nMLevelUpMPoint, skillData, skillPoint))
    {
        return true;
    }

    if (!g_csItemOption.IsNonWeaponSkillOrIsSkillEquipped(skillData.Skill))
    {
        mu::ui::window::CreateOkMessageBox(I18N::Game::YouNeedToWearTheRequiredEquipmentToLevelUpThisSkill);
        return true;
    }

    if (!this->CheckParentSkill(skillData)
        || !this->CheckRankPoint(skillData.Group, lpskill->SkillRank, skillPoint)
        || !this->CheckBeforeSkill(skillData.Skill, skillPoint))
    {
        mu::ui::window::CreateOkMessageBox(I18N::Game::YouMustMeetAllSkillRequirements);

        return true;
    }

    this->ConsumePoint = skillData.RequiredPoints;
    
    this->CurSkillID = skillData.Skill;

    wchar_t szMasterLevelText[256];
    mu_swprintf(szMasterLevelText, I18N::Game::MasterLevelPointRequirementD, g_pMasterLevelInterface->GetConsumePoint());
    mu::ui::window::GenericDialogConfig cfg;
    cfg.showCancel = true;
    cfg.lines = {
        { I18N::Game::WouldYouLikeToStrengthenTheSkill, false },
        { szMasterLevelText, false },
    };
    cfg.onPrimary = []
    {
        SocketClient->ToGameServer()->SendAddMasterSkillPoint(g_pMasterLevelInterface->GetCurSkillID());
        MouseLButton = false;
        MouseLButtonPop = false;
        MouseLButtonPush = false;
    };
    cfg.onCancel = []
    {
        MouseLButton = false;
        MouseLButtonPop = false;
        MouseLButtonPush = false;
    };
    mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));

    MouseLButton = false;

    MouseLButtonPop = false;

    MouseLButtonPush = false;

    return true;
}

bool mu::ui::window::CMasterLevel::CheckSkillPoint(WORD mLevelUpPoint, const _MASTER_SKILLTREE_DATA& skillData, BYTE skillLevel)
{

    if (skillLevel >= skillData.MaxLevel)
    {
        mu::ui::window::CreateOkMessageBox(I18N::Game::YouCanTRaiseAnyMoreLevels);
        return false;
    }

    if (mLevelUpPoint >= skillData.RequiredPoints)
    {
        return true;
    }

    wchar_t Buffer[358] = {};

    mu_swprintf(Buffer, I18N::Game::YouCanTRaiseAnyMoreLevels, skillData.RequiredPoints - mLevelUpPoint);

    mu::ui::window::CreateOkMessageBox(Buffer);

    return false;
}

bool mu::ui::window::CMasterLevel::CheckParentSkill(const _MASTER_SKILLTREE_DATA& masterSkill)
{
    for (int i = 0; i < MAX_MASTER_SKILL_REQUIRES; i++)
    {
        const auto requiredSkill = masterSkill.RequireSkill[i];
        if (requiredSkill == AT_SKILL_UNDEFINED)
        {
            return true;
        }

        if (requiredSkill < AT_SKILL_MASTER_BEGIN || requiredSkill > AT_SKILL_MASTER_END)
        {
            return true;
        }

        const auto reqSkillLevel = CharacterAttribute->MasterSkillInfo[requiredSkill].GetSkillLevel();
        if (reqSkillLevel < MASTER_SKILL_LEVEL_REQ_FOR_NEXT_RANK)
        {
            return false;
        }
    }

    return true;
}

bool mu::ui::window::CMasterLevel::CheckRankPoint(BYTE group, BYTE rank, BYTE skillLevel)
{
    if (this->skillPoint[group][rank] < skillLevel)
    {
        this->skillPoint[group][rank] = skillLevel;
    }

    if (rank == 1)
    {
        return true;
    }

    return this->skillPoint[group][rank - 1] >= 10;
}

bool mu::ui::window::CMasterLevel::CheckBeforeSkill(ActionSkillType skill, BYTE skillLevel)
{
    if (skillLevel != 0)
    {
        return true;
    }

    const auto Index = SkillAttribute[skill].SkillBrand;

    if (Index == 0)
    {
        return true;
    }

    const SKILL_ATTRIBUTE* lpSkill = &SkillAttribute[Index];

    if (lpSkill == nullptr)
    {
        return false;
    }

    if (lpSkill->SkillUseType == 4)
    {
        return true;
    }

    for (int i = 0; i < MAX_MAGIC; i++)
    {
        if (CharacterAttribute->Skill[i] == Index)
        {
            return true;
        }
    }

    return false;
}

void mu::ui::window::CMasterLevel::SkillUpgrade(int index, BYTE skillLevel, float value, float nextValue)
{
    const auto it = this->map_masterData.find(index);
    if (it == this->map_masterData.end())
    {
        return;
    }

    const auto realSkill = it->second.Skill;
    const int oldLevel = CharacterAttribute->MasterSkillInfo[realSkill].GetSkillLevel();

    const CSkillTreeInfo skillTreeInfo = { skillLevel, value, nextValue };
    CharacterAttribute->MasterSkillInfo[realSkill] = skillTreeInfo;

    // And update the category points
    const int addedPoints = skillLevel - oldLevel;
    this->CategoryPoint[it->second.Group] += addedPoints;
}

void mu::ui::window::CMasterLevel::ClearSkillTreeData()
{
    if (!map_masterSkillToolTip.empty())
        this->map_masterData.clear();
}

void mu::ui::window::CMasterLevel::ClearSkillTooltipData()
{
    if (!map_masterSkillToolTip.empty())
        this->map_masterSkillToolTip.clear();
}
