#include "stdafx.h"
#include "WinMain.h"
#include "ZzzTexture.h"
#include "CSItemOption.h"
#include "UIControls.h"
#include "NewUISystem.h"
#include "NewUIMasterLevel.h"

#include "DSPlaySound.h"
#include "NewUICommonMessageBox.h"
#include "SkillManager.h"

// TODO: are these 2 still needed?
char Need_Point = 0;
int	 In_Skill = 0;

_MASTER_SKILLTREE_DATA m_stMasterSkillTreeData[MAX_MASTER_SKILL_DATA];
_MASTER_SKILL_TOOLTIP m_stMasterSkillTooltip[MAX_MASTER_SKILL_DATA];

SEASON3B::CNewUIMasterLevel::CNewUIMasterLevel()
{
    m_pNewUIMng = nullptr;
    this->ConsumePoint = 0;
    this->CurSkillID = 0;
    this->classCode = 0;
    this->CategoryTextIndex = 0;
    this->categoryPos[0] = { 11,55 };
    this->categoryPos[1] = { 221,55 };
    this->categoryPos[2] = { 431,55 };
    this->InitMasterSkillPoint();
    this->ClearSkillTreeInfo();
    this->ClearSkillTreeData();
    this->ClearSkillTooltipData();
}

SEASON3B::CNewUIMasterLevel::~CNewUIMasterLevel()
{
    this->Release();
}

void SEASON3B::CNewUIMasterLevel::ClearSkillTreeInfo()
{
    if (!this->map_skilltreeinfo.empty())
        this->map_skilltreeinfo.clear();
}

BYTE SEASON3B::CNewUIMasterLevel::GetConsumePoint()
{
    return this->ConsumePoint;
}

int SEASON3B::CNewUIMasterLevel::GetCurSkillID()
{
    return this->CurSkillID;
}

bool SEASON3B::CNewUIMasterLevel::Create(CNewUIManager* pNewUIMng)
{
    if (nullptr == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_MASTER_LEVEL, this);

    this->SetPos();

    this->LoadImages();

    this->m_CloseBT.ChangeButtonImgState(true, IMAGE_MASTER_INTERFACE + 5, false, false, false);

    this->m_CloseBT.ChangeButtonInfo(611, 9, 13, 14);

    this->m_CloseBT.ChangeToolTipText(GlobalText[1002]);

    for (int i = 0; i < MAX_MASTER_SKILL_CATEGORY; i++)
    {
        this->ButtonX[i] = 0;

        this->ButtonY[i] = 0;
    }

    return true;
}

void SEASON3B::CNewUIMasterLevel::Release()
{
    this->ClearSkillTreeInfo();
    this->ClearSkillTreeData();
    this->ClearSkillTooltipData();
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = nullptr;
    }
}

void SEASON3B::CNewUIMasterLevel::SetPos()
{
    this->PosX = 0;
    this->PosY = 0;
    this->width = 640;
    this->height = 428;
}

void SEASON3B::CNewUIMasterLevel::OpenMasterSkillTreeData(wchar_t* path)
{
    memset(m_stMasterSkillTreeData, 0, sizeof(m_stMasterSkillTreeData));

    FILE* fp = _wfopen(path, L"rb");

    wchar_t Text[256];

    if (fp == nullptr)
    {
        swprintf(Text, L"%s - File not exist.", path);
        g_ErrorReport.Write(Text);
        MessageBox(g_hWnd, Text, nullptr, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        return;
    }

    int Size = sizeof(_MASTER_SKILLTREE_DATA);

    BYTE* Buffer = new BYTE[Size * MAX_MASTER_SKILL_DATA];

    fread(Buffer, Size * MAX_MASTER_SKILL_DATA, 1, fp);

    DWORD dwCheckSum;

    fread(&dwCheckSum, sizeof(DWORD), 1u, fp);

    fclose(fp);

    if (dwCheckSum != GenerateCheckSum2(Buffer, 12288, 0x2BC1))
    {
        swprintf(Text, L"%s - File corrupted.", path);
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

void SEASON3B::CNewUIMasterLevel::OpenMasterSkillTooltip(wchar_t* path)
{
    memset(m_stMasterSkillTooltip, 0, sizeof(m_stMasterSkillTooltip));

    FILE* fp = _wfopen(path, L"rb");

    if (fp == nullptr)
    {
        wchar_t Text[256];
        swprintf(Text, L"%s - File not exist.", path);
        g_ErrorReport.Write(Text);
        MessageBox(g_hWnd, Text, nullptr, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        return;
    }

    int record_size = sizeof(_MASTER_SKILL_TOOLTIP_FILE);
    BYTE* file_buffer = new BYTE[record_size * MAX_MASTER_SKILL_DATA];
    fread(file_buffer, record_size * MAX_MASTER_SKILL_DATA, 1, fp);
    DWORD dwCheckSum;
    fread(&dwCheckSum, sizeof(DWORD), 1u, fp);
    fclose(fp);


    /*
    if (dwCheckSum != GenerateCheckSum2(Buffer, 12288, 0x2BC1))
    {
        swprintf(Text, "%s - File corrupted.", path);
        g_ErrorReport.Write(Text);
        MessageBox(g_hWnd, Text, NULL, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        return;
    }
    */
    BYTE* pSeek = file_buffer;

    for (int i = 0; i < MAX_MASTER_SKILL_DATA; i++)
    {
        BuxConvert(pSeek, record_size);

        _MASTER_SKILL_TOOLTIP_FILE current{ };
        memcpy(&current, pSeek, record_size);

        const auto target = &m_stMasterSkillTooltip[i];
        target->SkillNumber = static_cast<ActionSkillType>(current.SkillNumber);
        target->ClassCode = current.ClassCode;
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

void SEASON3B::CNewUIMasterLevel::InitMasterSkillPoint()
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

void SEASON3B::CNewUIMasterLevel::SetMasterType(CLASS_TYPE Class)
{
    switch (Class)
    {
    case CLASS_GRANDMASTER:
        this->classCode = 2;
        break;
    case CLASS_BLADEMASTER:
        this->classCode = 1;
        break;
    case CLASS_HIGHELF:
        this->classCode = 4;
        break;
    case CLASS_DUELMASTER:
        this->classCode = 16;
        break;
    case CLASS_LORDEMPEROR:
        this->classCode = 32;
        break;
    case CLASS_DIMENSIONMASTER:
        this->classCode = 8;
        break;
    case CLASS_TEMPLENIGHT:
        this->classCode = 64;
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

void SEASON3B::CNewUIMasterLevel::SetMasterSkillTreeData()
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

void SEASON3B::CNewUIMasterLevel::SetMasterSkillToolTipData()
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

bool SEASON3B::CNewUIMasterLevel::SetMasterSkillTreeInfo(int index, int SkillPoint, float value, float nextvalue)
{
    std::map<BYTE, _MASTER_SKILLTREE_DATA>::iterator it = this->map_masterData.find(index);

    if (it == this->map_masterData.end())
    {
        return true;
    }

    CSkillTreeInfo skillInfo;

    skillInfo.SetSkillTreeInfo(SkillPoint, value, nextvalue);

    this->CategoryPoint[it->second.Group] += SkillPoint;

    if (this->map_skilltreeinfo.insert(std::pair(it->second.Skill, skillInfo)).second == false)
    {
        return false;
    }


    return true;
}

int SEASON3B::CNewUIMasterLevel::SetDivideString(wchar_t* text, int isItemTollTip, int TextNum, int iTextColor, int iTextBold, bool isPercent)
{
    if (text == nullptr)
    {
        return TextNum;
    }

    wchar_t alpszDst[10][256] = {};

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

        swprintf(TextList[TextNum], cText.c_str());

        TextNum++;
    }

    return TextNum;
}

bool SEASON3B::CNewUIMasterLevel::Render()
{
    EnableAlphaTest();
    glColor4f(1.0, 1.0, 1.0, 1.0);
    RenderImage(IMAGE_MASTER_INTERFACE, this->PosX, this->PosY, Bitmaps[IMAGE_MASTER_INTERFACE].Width, Bitmaps[IMAGE_MASTER_INTERFACE].Height);
    RenderImage(IMAGE_MASTER_INTERFACE + 1, this->PosX + Bitmaps[IMAGE_MASTER_INTERFACE].Width, this->PosY, Bitmaps[IMAGE_MASTER_INTERFACE + 1].Width, Bitmaps[IMAGE_MASTER_INTERFACE + 1].Height);
    this->RenderIcon();
    this->m_CloseBT.Render();
    DisableAlphaBlend();
    this->RenderText();

    return true;
}

bool SEASON3B::CNewUIMasterLevel::Update()
{
    return true;
}

bool SEASON3B::CNewUIMasterLevel::UpdateMouseEvent()
{
    if (this->m_CloseBT.UpdateMouseEvent() == true)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_MASTER_LEVEL);

        return true;
    }

    bool result = true;

    if (SEASON3B::IsPress(VK_LBUTTON) == true)
    {
        result = this->CheckMouse(MouseX, MouseY);

        if (result == false)
        {
            PlayBuffer(SOUND_CLICK01);
        }
    }

    for (int i = 0; i < MAX_MASTER_SKILL_CATEGORY; i++)
    {
        if (this->ButtonX[i] == 1 && SEASON3B::IsPress(VK_LBUTTON) == true)
        {
            this->ButtonX[i] = 0;

            return true;
        }
    }

    this->CheckBtn();

    if (SEASON3B::CheckMouseIn(this->PosX, this->PosY, this->width, this->height) == true)
    {
        return false;
    }

    return result;
}

bool SEASON3B::CNewUIMasterLevel::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MASTER_LEVEL) == false || SEASON3B::IsPress(VK_ESCAPE) == false && SEASON3B::IsPress('A') == false)
    {
        return true;
    }

    g_pNewUISystem->Hide(SEASON3B::INTERFACE_MASTER_LEVEL);

    PlayBuffer(SOUND_CLICK01);

    return false;
}

float SEASON3B::CNewUIMasterLevel::GetLayerDepth()
{
    return 10.1000004;
}

void SEASON3B::CNewUIMasterLevel::LoadImages()
{
    LoadBitmap(L"Interface\\new_Master_back01.jpg", IMAGE_MASTER_INTERFACE, GL_LINEAR, GL_REPEAT, true, false);
    LoadBitmap(L"Interface\\new_Master_back02.jpg", IMAGE_MASTER_INTERFACE + 1, GL_LINEAR, GL_REPEAT, true, false);
    LoadBitmap(L"Interface\\new_Master_Icon.jpg", IMAGE_MASTER_INTERFACE + 2, GL_LINEAR, GL_CLAMP, true, false);
    LoadBitmap(L"Interface\\new_Master_Non_Icon.jpg", IMAGE_MASTER_INTERFACE + 3, GL_LINEAR);
    LoadBitmap(L"Interface\\new_Master_box.tga", IMAGE_MASTER_INTERFACE + 4, GL_LINEAR);
    LoadBitmap(L"Interface\\new_Master_exit.jpg", IMAGE_MASTER_INTERFACE + 5, GL_LINEAR);
    LoadBitmap(L"Interface\\new_Master_arrow01.tga", IMAGE_MASTER_INTERFACE + 6, GL_LINEAR);
    LoadBitmap(L"Interface\\new_Master_arrow02.tga", IMAGE_MASTER_INTERFACE + 7, GL_LINEAR);
    LoadBitmap(L"Interface\\new_Master_arrow03.tga", IMAGE_MASTER_INTERFACE + 8, GL_LINEAR);
    LoadBitmap(L"Interface\\new_Master_arrow04.tga", IMAGE_MASTER_INTERFACE + 9, GL_LINEAR);
    LoadBitmap(L"Interface\\new_Master_arrow05.tga", IMAGE_MASTER_INTERFACE + 10, GL_LINEAR);
    LoadBitmap(L"Interface\\new_Master_arrow06.tga", IMAGE_MASTER_INTERFACE + 11, GL_LINEAR);
    LoadBitmap(L"Interface\\new_Master_arrow07.tga", IMAGE_MASTER_INTERFACE + 12, GL_LINEAR);
    LoadBitmap(L"Interface\\new_Master_arrow08.tga", IMAGE_MASTER_INTERFACE + 13, GL_LINEAR);
}

void SEASON3B::CNewUIMasterLevel::UnloadImages()
{
    for (int i = 0; i < 14; i++)
    {
        DeleteBitmap(i + IMAGE_MASTER_INTERFACE, false);
    }
}

void SEASON3B::CNewUIMasterLevel::RenderText()
{
    g_pRenderText->SetFont(g_hFont);

    if (SEASON3B::IsPress(VK_LBUTTON) == false && SEASON3B::CheckMouseIn(458, 11, 81, 10) == true)
    {
        TextList[0][0] = 0;
        TextBold[0] = 0;
        TextListColor[0] = 0;
        swprintf(TextList[0], L"%I64d / %I64d", Master_Level_Data.lMasterLevel_Experince, Master_Level_Data.lNext_MasterLevel_Experince);
        RenderTipTextList(466, 26, 1, 0, 3, 0, 1);
    }

    g_pRenderText->SetTextColor(255, 255, 255, 0xFFu);

    g_pRenderText->SetBgColor(0, 0, 0, 1u);

    wchar_t Buffer[256] = {};

    swprintf(Buffer, GlobalText[1746], Master_Level_Data.nMLevel);

    g_pRenderText->RenderText(275, 11, Buffer, 0, 0, 1, 0);

    swprintf(Buffer, GlobalText[1747], Master_Level_Data.nMLevelUpMPoint);

    g_pRenderText->RenderText(372, 11, Buffer, 0, 0, 1, 0);

    if (Master_Level_Data.lNext_MasterLevel_Experince != 0)
    {
        __int64 iTotalLevel = Master_Level_Data.nMLevel + 400;				// 종합레벨 - 400렙이 만렙이기 때문에 더해준다.
        __int64 iTOverLevel = iTotalLevel - 255;		// 255레벨 이상 기준 레벨
        __int64 iBaseExperience = 0;					// 레벨 초기 경험치

        __int64 iData_Master =	// A
            (
                (
                    (__int64)9 + (__int64)iTotalLevel
                    )
                * (__int64)iTotalLevel
                * (__int64)iTotalLevel
                * (__int64)10
                )
            +
            (
                (
                    (__int64)9 + (__int64)iTOverLevel
                    )
                * (__int64)iTOverLevel
                * (__int64)iTOverLevel
                * (__int64)1000
                );

        iBaseExperience = (iData_Master - (__int64)3892250000) / (__int64)2;	// B

        // 레벨업 경험치
        double fNeedExp = (double)Master_Level_Data.lNext_MasterLevel_Experince - (double)iBaseExperience;

        // 현재 획득한 경험치
        double fExp = (double)Master_Level_Data.lMasterLevel_Experince - (double)iBaseExperience;

        swprintf(Buffer, GlobalText[3335], fExp / fNeedExp * 100.0);

        g_pRenderText->RenderText(466, 11, Buffer, 0, 0, 1, 0);
    }

    g_pRenderText->RenderText(154, 11, GlobalText[this->ClassNameTextIndex], 0, 0, 1, nullptr);

    g_pRenderText->SetTextColor(255, 155, 0, 0xFFu);

    swprintf(Buffer, GlobalText[this->CategoryTextIndex], this->CategoryPoint[0]);

    g_pRenderText->RenderText(92, 40, Buffer, 0, 0, RT3_SORT_CENTER, 0);

    swprintf(Buffer, GlobalText[this->CategoryTextIndex + 1], this->CategoryPoint[1]);

    g_pRenderText->RenderText(302, 40, Buffer, 0, 0, RT3_SORT_CENTER, 0);

    swprintf(Buffer, GlobalText[this->CategoryTextIndex + 2], this->CategoryPoint[2]);

    g_pRenderText->RenderText(513, 40, Buffer, 0, 0, RT3_SORT_CENTER, 0);
}

void SEASON3B::CNewUIMasterLevel::RenderIcon()
{
    int SKILL_ICON_WIDTH = 20;
    int SKILL_ICON_HEIGHT = 28;

    for (std::map<BYTE, _MASTER_SKILLTREE_DATA>::iterator it = this->map_masterData.begin(); it != this->map_masterData.end(); it++)
    {
        auto group = it->second.Group;
        auto skill = it->second.Skill;
        auto skillAttribute = &SkillAttribute[skill];
        int skillPoint = 0;
        int index = (it->second.Index - 1) % 4;
        BYTE rank = skillAttribute->SkillRank;

        int CalcX = (int)(index * 49.0f + this->categoryPos[group].x);
        int CalcY = (int)(this->categoryPos[group].y + (skillAttribute->SkillRank - 1) * 41.0f);

        auto sit = this->map_skilltreeinfo.find(skill);

        if (sit != this->map_skilltreeinfo.end())
        {
            skillPoint = sit->second.GetSkillPoint();
        }

        DWORD textColor;

        RenderImage(IMAGE_MASTER_INTERFACE + 4, CalcX, CalcY, 50, 38, 0, 0, 50.f / 64.f, 38.f / 64.f);

        if (!this->CheckParentSkill(it->second.RequireSkill[0], it->second.RequireSkill[1])
            || !this->CheckRankPoint(group, rank, skillPoint)
            || !this->CheckBeforeSkill(skill, skillPoint)
            || !g_csItemOption.IsNonWeaponSkillOrIsSkillEquipped(skill)
            )
        {
            textColor = RGBA(120, 120, 120, 255);

            g_pRenderText->SetTextColor(textColor);
            RenderImage(IMAGE_MASTER_INTERFACE + 3, CalcX + 8, CalcY + 5, SKILL_ICON_WIDTH, SKILL_ICON_HEIGHT, (20.f / 512.f) * (skillAttribute->Magic_Icon % 25), ((28.f / 512.f) * ((skillAttribute->Magic_Icon / 25))), 20.f / 512, 28.f / 512.f);
        }
        else
        {
            textColor = RGBA(255, 255, 255, 255);

            g_pRenderText->SetTextColor(textColor);

            RenderImage(IMAGE_MASTER_INTERFACE + 2, CalcX + 8, CalcY + 5, SKILL_ICON_WIDTH, SKILL_ICON_HEIGHT, (20.f / 512.f) * (skillAttribute->Magic_Icon % 25), ((28.f / 512.f) * ((skillAttribute->Magic_Icon / 25))), 20.f / 512.f, 28.f / 512.f);
        }

        if (it->second.ArrowDirection == 1)
            RenderImage(IMAGE_MASTER_INTERFACE + 6, CalcX + 8 + (SKILL_ICON_WIDTH)+2, CalcY + (SKILL_ICON_HEIGHT / 2), 28, 7, 0, 0, 28 / 32.f, 7 / 8.f);
        if (it->second.ArrowDirection == 2)
            RenderImage(IMAGE_MASTER_INTERFACE + 7, CalcX + 8 + (SKILL_ICON_WIDTH)+2, CalcY + (SKILL_ICON_HEIGHT / 2), 28, 7, 0, 0, 28 / 32.f, 7 / 8.f);
        if (it->second.ArrowDirection == 3)
            RenderImage(IMAGE_MASTER_INTERFACE + 8, CalcX + 8 + (SKILL_ICON_WIDTH / 2) - 3.5, CalcY + SKILL_ICON_HEIGHT + 7, 7, 12, 0, 0, 7 / 8.f, 12 / 16.f);
        if (it->second.ArrowDirection == 4)
            RenderImage(IMAGE_MASTER_INTERFACE + 9, CalcX + 8 + (SKILL_ICON_WIDTH / 2) - 3.5, CalcY + SKILL_ICON_HEIGHT + 7, 7, 52, 0, 0, 7 / 8.f, 12 / 16.f);
        if (it->second.ArrowDirection == 5)
            RenderImage(IMAGE_MASTER_INTERFACE + 10, CalcX, CalcY, 42, 31, 0, 0, 42 / 64.f, 31 / 32.f);
        if (it->second.ArrowDirection == 6)
            RenderImage(IMAGE_MASTER_INTERFACE + 11, CalcX, CalcY, 42, 31, 0, 0, 42 / 64.f, 31 / 32.f);
        if (it->second.ArrowDirection == 7)
            RenderImage(IMAGE_MASTER_INTERFACE + 12, CalcX + 8 + (SKILL_ICON_WIDTH / 2) - 1.5, CalcY + SKILL_ICON_HEIGHT + 8, 40, 28, 0, 0, 40 / 64.f, 28 / 32.f);
        if (it->second.ArrowDirection == 8)
            RenderImage(IMAGE_MASTER_INTERFACE + 13, CalcX, CalcY, 40, 28, 0, 0, 40 / 64.f, 28 / 32.f);

        g_pRenderText->RenderText(CalcX + 8 + 30, CalcY + 28 - 5, std::to_wstring(skillPoint).c_str());
    }

    this->RenderToolTip();
}

void SEASON3B::CNewUIMasterLevel::RenderToolTip()
{
    for (std::map<BYTE, _MASTER_SKILLTREE_DATA>::iterator it = this->map_masterData.begin(); it != this->map_masterData.end(); it++)
    {
        BYTE group = it->second.Group;

        auto Skill = it->second.Skill;

        SKILL_ATTRIBUTE* p = &SkillAttribute[Skill];

        if (p == nullptr)
        {
            break;
        }

        int index = (it->second.Index - 1) % 4;

        int CalcX = (int)(index * 49.0f + this->categoryPos[group].x);

        int CalcY = (int)(this->categoryPos[group].y + (p->SkillRank - 1) * 41.0f);

        if (SEASON3B::IsPress(VK_LBUTTON) == true || SEASON3B::CheckMouseIn(CalcX + 8, CalcY + 5, 20, 28) == false)
        {
            continue;
        }

        auto mtit = this->map_masterSkillToolTip.find(Skill);

        if (mtit == this->map_masterSkillToolTip.end())
        {
            return;
        }

        int skillPoint = 0;

        float skillValue = 0.0f;

        float skillNextValue = 0.0f;

        auto sit = this->map_skilltreeinfo.find(Skill);

        if (sit != this->map_skilltreeinfo.end())
        {
            skillPoint = sit->second.GetSkillPoint();

            skillValue = sit->second.GetSkillValue();

            skillNextValue = sit->second.GetSkillNextValue();
        }

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

        swprintf(TextList[lineCount], L"%s", p->Name);

        TextBold[lineCount] = true;

        lineCount++;

        swprintf(TextList[lineCount], mtit->second.Info1, p->SkillRank, skillPoint, it->second.MaxLevel);

        lineCount++;

        wchar_t buffer[512] = {};

        if (it->second.DefValue == -1.0f)
        {
            swprintf(buffer, mtit->second.Info2);
        }
        else
        {
            swprintf(buffer, mtit->second.Info2, skillPoint != 0 ? skillValue : it->second.DefValue);
        }

        lineCount = this->SetDivideString(buffer, 0, lineCount, 0, 0, true);

        if (skillPoint != 0 && skillPoint < it->second.MaxLevel)
        {
            swprintf(buffer, GlobalText[3328]);

            lineCount = this->SetDivideString(buffer, 0, lineCount, 4, 0, true);

            TextBold[lineCount] = 1;

            swprintf(buffer, mtit->second.Info2, skillNextValue);

            lineCount = this->SetDivideString(buffer, 0, lineCount, 0, 0, true);
        }

        if (skillPoint < it->second.MaxLevel)
        {
            swprintf(buffer, GlobalText[3329]);

            lineCount = this->SetDivideString(buffer, 0, lineCount, 1, 0, true);

            TextBold[lineCount] = 1;

            swprintf(buffer, mtit->second.Info3, it->second.RequiredPoints);

            if (it->second.RequiredPoints <= Master_Level_Data.nMLevelUpMPoint)
            {
                lineCount = this->SetDivideString(buffer, 0, lineCount, 0, 0, true);
            }
            else
            {
                lineCount = this->SetDivideString(buffer, 0, lineCount, 2, 0, true);
            }
        }

        int iTextColor = this->CheckBeforeSkill(Skill, skillPoint) == true ? 0 : 2;

        swprintf(buffer, mtit->second.Info4);

        lineCount = this->SetDivideString(buffer, 0, lineCount, iTextColor, 0, true);

        if (skillPoint < it->second.MaxLevel && p->SkillRank != 1)
        {
            iTextColor = this->CheckRankPoint(group, p->SkillRank, skillPoint) == true ? 0 : 2;

            swprintf(buffer, mtit->second.Info5);

            lineCount = this->SetDivideString(buffer, 0, lineCount, iTextColor, 0, true);

            auto RequireSkill1 = it->second.RequireSkill[0];
            auto RequireSkill2 = it->second.RequireSkill[1];

            if (RequireSkill1 != AT_SKILL_UNDEFINED)
            {
                auto rq1it = this->map_skilltreeinfo.find(RequireSkill1);

                if (rq1it != this->map_skilltreeinfo.end())
                {
                    iTextColor = rq1it->second.GetSkillValue() < 10 ? 2 : 0;
                }
                else
                {
                    iTextColor = 2;
                }

                swprintf(buffer, mtit->second.Info6);

                lineCount = this->SetDivideString(buffer, 0, lineCount, iTextColor, 0, true);
            }

            if (RequireSkill2 != AT_SKILL_UNDEFINED)
            {
                auto rq1it = this->map_skilltreeinfo.find(RequireSkill2);

                if (rq1it != this->map_skilltreeinfo.end())
                {
                    iTextColor = rq1it->second.GetSkillValue() < 10 ? 2 : 0;
                }
                else
                {
                    iTextColor = 2;
                }

                swprintf(buffer, mtit->second.Info7);

                lineCount = this->SetDivideString(buffer, 0, lineCount, iTextColor, 0, true);
            }
        }

        if (CalcY > 300)
        {

            RenderTipTextList(CalcX + 8, CalcY + 33, lineCount, 0, 3, STRP_BOTTOMCENTER, 1);
        }
        else
        {
            RenderTipTextList(CalcX + 8, CalcY + 33, lineCount, 0, 3, 0, 1);
        }

    }
}

bool SEASON3B::CNewUIMasterLevel::CheckMouse(int posx, int posy)
{
    POINT position[3] = { {185,65},{385,65},{585,65} };

    for (int i = 0; i < MAX_MASTER_SKILL_CATEGORY; i++)
    {
        if (SEASON3B::CheckMouseIn(position[i].x + this->PosX, this->ButtonY[i] + position[i].y + this->PosY, 15, 30) == true && this->ButtonX[i] == 0)
        {
            this->ButtonY[i] = 1;

            return false;
        }
    }

    return true;
}

bool SEASON3B::CNewUIMasterLevel::CheckBtn()
{
    int posX = 220;

    for (int i = 0; i < MAX_MASTER_SKILL_CATEGORY; i++)
    {
        if (this->ButtonX[i] == 1 && SEASON3B::IsRelease(VK_LBUTTON))
        {
            this->ButtonX[i] = 0;
            return false;
        }

        if (this->ButtonX[i] == 1)
        {
            this->ButtonY[i] = MouseY - 65;

            if (this->ButtonY[i] > posX)
            {
                this->ButtonY[i] = posX;
            }
            else if (this->ButtonY[i] <= 0)
            {
                this->ButtonY[i] = 0;
            }
        }
    }

    for (std::map<BYTE, _MASTER_SKILLTREE_DATA>::iterator it = this->map_masterData.begin(); it != this->map_masterData.end(); it++)
    {
        auto selectedSkill = it->second;
        
        switch (selectedSkill.Group)
        {
        case 0:
        case 1:
        case 2:
            this->CheckAttributeArea(selectedSkill);
            break;
        }
    }

    return true;
}

bool SEASON3B::CNewUIMasterLevel::CheckAttributeArea(const _MASTER_SKILLTREE_DATA skillData)// int group, int index, int reqPoint, unsigned int ReqSkill1, unsigned int ReqSkill2, unsigned int Skill, int defValue)
{
    if (skillData.Group < 0 || skillData.Group >= 3)
    {
        return false;
    }

    auto lpskill = &SkillAttribute[skillData.Skill];

    if (lpskill == nullptr)
    {
        return false;
    }

    int tindex = (skillData.Index - 1) % 4;

    int posX = (int)((double)this->categoryPos[skillData.Group].x + tindex * 49.0);

    int posY = (int)((double)this->categoryPos[skillData.Group].y + (lpskill->SkillRank - 1) * 41.0);

    if (!SEASON3B::IsPress(VK_LBUTTON) || SEASON3B::CheckMouseIn(posX + 8, posY + 5, 20, 28) == false)
    {
        return true;
    }

    auto it = this->map_skilltreeinfo.find(skillData.Skill);

    int skillPoint = 0;

    if (it != this->map_skilltreeinfo.end())
    {
        skillPoint = it->second.GetSkillPoint();
    }

    PlayBuffer(SOUND_CLICK01);

    if (!this->CheckSkillPoint(Master_Level_Data.nMLevelUpMPoint, skillData, skillPoint))
    {
        return true;
    }

    if (!g_csItemOption.IsNonWeaponSkillOrIsSkillEquipped(skillData.Skill))
    {
        SEASON3B::CreateOkMessageBox(GlobalText[3336]);
        return true;
    }

    if (!this->CheckParentSkill(skillData.RequireSkill[0], skillData.RequireSkill[1])
        || !this->CheckRankPoint(skillData.Group, lpskill->SkillRank, skillPoint)
        || !this->CheckBeforeSkill(skillData.Skill, skillPoint))
    {
        SEASON3B::CreateOkMessageBox(GlobalText[3327]);

        return true;
    }

    this->SetConsumePoint(skillData.RequiredPoints);

    this->SetCurSkillID(skillData.Skill);

    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CMaster_Level_Interface));

    MouseLButton = false;

    MouseLButtonPop = false;

    MouseLButtonPush = false;

    return true;
}

bool SEASON3B::CNewUIMasterLevel::CheckSkillPoint(WORD mLevelUpPoint, const _MASTER_SKILLTREE_DATA skillData, WORD skillPoint)
{

    if (skillPoint >= skillData.MaxLevel)
    {
        SEASON3B::CreateOkMessageBox(GlobalText[3326]);
        return false;
    }

    if (mLevelUpPoint >= skillData.RequiredPoints)
    {
        return true;
    }

    wchar_t Buffer[358] = {};

    swprintf(Buffer, GlobalText[3326], skillData.RequiredPoints - mLevelUpPoint);

    SEASON3B::CreateOkMessageBox(Buffer);

    return false;
}

bool SEASON3B::CNewUIMasterLevel::CheckParentSkill(ActionSkillType reqSkill1, ActionSkillType reqSkill2)
{
    if (reqSkill1 == 0)
    {
        return true;
    }

    auto it = this->map_skilltreeinfo.find(reqSkill1);

    if (it == this->map_skilltreeinfo.end())
    {
        return false;
    }

    if (it->second.GetSkillPoint() < 10)
    {
        return false;
    }

    if (reqSkill2 == 0)
    {
        return true;
    }

    it = this->map_skilltreeinfo.find(reqSkill2);

    if (it == this->map_skilltreeinfo.end())
    {
        return false;
    }

    if (it->second.GetSkillPoint() < 10)
    {
        return false;
    }

    return true;
}

bool SEASON3B::CNewUIMasterLevel::CheckRankPoint(BYTE group, BYTE rank, BYTE skillPoint)
{
    if (this->skillPoint[group][rank] < skillPoint)
    {
        this->skillPoint[group][rank] = skillPoint;
    }

    if (rank == 1)
    {
        return true;
    }

    return this->skillPoint[group][rank - 1] >= 10;	//PUEDE SER +2 SI HAY ALGUN ERROR
}

bool SEASON3B::CNewUIMasterLevel::CheckBeforeSkill(ActionSkillType skill, BYTE skillPoint)
{
    if (skillPoint != 0)
    {
        return true;
    }

    int Index = this->GetBeforeSkillID(skill);

    if (Index == 0)
    {
        return true;
    }

    SKILL_ATTRIBUTE* lpSkill = &SkillAttribute[Index];

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

int SEASON3B::CNewUIMasterLevel::GetBeforeSkillID(int index)
{
    return SkillAttribute[index].SkillBrand;
}

int SEASON3B::CNewUIMasterLevel::GetBaseSkillID(int index)
{
    if (index >= MAX_SKILLS)
    {
        return 0;
    }

    while (SkillAttribute[index].SkillBrand != 0 && SkillAttribute[index].SkillBrand != 75)
    {
        index = SkillAttribute[index].SkillBrand;
    }

    return index;
}

void SEASON3B::CNewUIMasterLevel::SkillUpgrade(int index, char point, float value, float nextvalue)
{
    auto it = this->map_masterData.find(index);
    if (it == this->map_masterData.end())
    {
        return;
    }

    auto realSkill = it->second.Skill;
    int oldPoints = 0;

    // First, remove old skill info if it exists
    auto oldSkillTreeInfo = this->map_skilltreeinfo.find(realSkill);
    if (oldSkillTreeInfo != this->map_skilltreeinfo.end())
    {
        oldPoints = oldSkillTreeInfo->second.GetSkillPoint();
        this->map_skilltreeinfo.erase(oldSkillTreeInfo);
    }

    // Then add the updated skill tree info
    CSkillTreeInfo skillTreeInfo;
    skillTreeInfo.SetSkillTreeInfo(point, value, nextvalue);
    this->map_skilltreeinfo.insert(std::pair(realSkill, skillTreeInfo));

    // And update the category points
    int addedPoints = point - oldPoints;
    this->CategoryPoint[it->second.Group] += addedPoints;

}

//bool SEASON3B::CNewUIMasterLevel::IsLearnSkill(int index)
//{
//    std::map<DWORD, CSkillTreeInfo>::iterator it = this->map_skilltreeinfo.find(index);
//
//    if (it == this->map_skilltreeinfo.end())
//    {
//        return false;
//    }
//
//    return true;
//}

float SEASON3B::CNewUIMasterLevel::GetSkillValue(ActionSkillType index)
{
    auto it = this->map_skilltreeinfo.find(index);

    if (it == this->map_skilltreeinfo.end())
    {
        return 0.0f;
    }

    return it->second.GetSkillValue();
}
/*
float SEASON3B::CNewUIMasterLevel::GetSkillCumulativeValue(int index, BYTE damage)
{
    if (!index || index < 300 || index > 608 || damage == 0 || damage >= 5)
    {
        return 0.0;
    }

    float value = 0.0;

    SKILL_ATTRIBUTE* lpSkill;

    for (int i = index; i != 75; i = lpSkill->SkillBrand)
    {
        if (i == 0)
        {
            break;
        }

        lpSkill = &SkillAttribute[i];

        if (damage == lpSkill->IsDamage)
        {
            value += this->GetSkillValue(i);
        }
    }

    return value;
}*/
/*
BYTE SEASON3B::CNewUIMasterLevel::GetSkillLevel(int index)
{
    std::map<DWORD, CSkillTreeInfo>::iterator it = this->map_skilltreeinfo.find(index);

    if (it == this->map_skilltreeinfo.end())
    {
        return 0;
    }

    return it->second.GetSkillPoint();
}*/

void SEASON3B::CNewUIMasterLevel::RegisterSkillInCharacterAttribute(ActionSkillType index)
{
    if (index < AT_SKILL_MASTER_BEGIN || index > AT_SKILL_MASTER_END)
    {
        return;
    }

    const auto skillAttribute = &SkillAttribute[index];

    if (skillAttribute->SkillUseType == SKILL_USE_TYPE_MASTERACTIVE)
    {
        for (int i = 0; i < MAX_MAGIC; i++)
        {
            if (skillAttribute->SkillBrand != CharacterAttribute->Skill[i])
            {
                continue;
            }

            CharacterAttribute->Skill[i] = index;

            return;
        }

        return;
    }

    for (int i = 0; i < MAX_MAGIC; i++)
    {
        if (CharacterAttribute->Skill[i] != AT_SKILL_UNDEFINED)
        {
            continue;
        }

        CharacterAttribute->Skill[i] = index;

        return;
    }
}

void SEASON3B::CNewUIMasterLevel::SetConsumePoint(BYTE ConsumePoint)
{
    this->ConsumePoint = ConsumePoint;
}

void SEASON3B::CNewUIMasterLevel::SetCurSkillID(int index)
{
    this->CurSkillID = index;
}

void SEASON3B::CNewUIMasterLevel::ClearSkillTreeData()
{
    if (!map_masterSkillToolTip.empty())
        this->map_masterData.clear();
}

void SEASON3B::CNewUIMasterLevel::ClearSkillTooltipData()
{
    if (!map_masterSkillToolTip.empty())
        this->map_masterSkillToolTip.clear();
}
