#include "stdafx.h"
#include "NewUIInventoryExtension.h"

#include "NewUISystem.h"


using namespace SEASON3B;

CNewUIInventoryExtension::CNewUIInventoryExtension()
{
    Init();
}

CNewUIInventoryExtension::~CNewUIInventoryExtension()
{
    Release();
}

void CNewUIInventoryExtension::Init()
{
    m_pNewUIMng = nullptr;
    m_Pos.x = m_Pos.y = 0;
}

bool CNewUIInventoryExtension::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (nullptr == pNewUIMng || nullptr == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(INTERFACE_INVENTORY_EXT, this);

    // we have to create all 4 boxes here already, and just handle the ones
    // which are available to the character...
    int i = 0;
    for (auto& m_extension : m_extensions)
    {
        m_extension = new CNewUIInventoryCtrl();

        const int indexOffset = MAX_MY_INVENTORY_INDEX + i * MAX_INVENTORY_EXT_ONE;
        if (false == m_extension->Create(STORAGE_TYPE::INVENTORY, g_pNewUI3DRenderMng, g_pNewItemMng, this, x + 15, y + 45 + HEIGHT_PER_EXT * i, COLUMN_INVENTORY, ROW_INVENTORY_EXT, indexOffset))
        {
            SAFE_DELETE(m_extension);
            return false;
        }

        if (m_extension)
        {
            m_extension->SetToolTipType(TOOLTIP_TYPE_INVENTORY);
        }

        i++;
    }

    SetPos(x, y);
    LoadImages();
    SetButtonInfo();
    Show(false);

    return true;
}

void CNewUIInventoryExtension::Release()
{
    UnloadImages();

    for (auto extension : m_extensions)
    {
        if (extension)
        {
            SAFE_DELETE(extension);
        }
    }

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = nullptr;
    }
}

void CNewUIInventoryExtension::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
    m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + 391, 36, 29);
}

bool CNewUIInventoryExtension::UpdateMouseEvent()
{
    for (int i = 0; i < CharacterAttribute->InventoryExtensions; i++)
    {
        if (const auto m_extension = m_extensions[i])
        {
            if (!m_extension->UpdateMouseEvent())
            {
                return false;
            }

            if (InventoryProcess())
            {
                return false;
            }
        }
    }

    if (m_BtnExit.UpdateMouseEvent())
    {
        g_pNewUISystem->Hide(INTERFACE_INVENTORY_EXT);
        return false;
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y, WIDTH, HEIGHT))
    {
        if (IsPress(VK_RBUTTON))
        {
            MouseRButton = false;
            MouseRButtonPop = false;
            MouseRButtonPush = false;
            return false;
        }

        if (IsNone(VK_LBUTTON) == false)
        {
            return false;
        }
    }

    return true;
}

bool CNewUIInventoryExtension::InventoryProcess()
{
    if (!CheckMouseIn(m_Pos.x, m_Pos.y, WIDTH, HEIGHT))
    {
        return false;
    }

    int targetExtensionIndex = 0;
    CNewUIInventoryCtrl* target = nullptr;
    for (auto* extension : m_extensions)
    {
        if (extension->CheckPtInRect(MouseX, MouseY))
        {
            return g_pMyInventory->HandleInventoryActions(extension);
        }

        targetExtensionIndex++;
    }

    return false;
}

bool CNewUIInventoryExtension::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(INTERFACE_INVENTORY_EXT) == false)
    {
        return true;
    }

    return true;
}

bool CNewUIInventoryExtension::Update()
{
    for (int i = 0; i < CharacterAttribute->InventoryExtensions; i++)
    {
        if (const auto& extension = m_extensions[i])
        {
            if (extension && !extension->Update())
            {
                return false;
            }
        }
    }

    return true;
}

bool CNewUIInventoryExtension::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderTexts();

    for (int i = 0; i < CharacterAttribute->InventoryExtensions; i++)
    {
        if (const auto& m_extension = m_extensions[i])
        {
            m_extension->Render();
        }
    }

    m_BtnExit.Render();

    DisableAlphaBlend();
    return true;
}

void CNewUIInventoryExtension::RenderFrame() const
{
    const auto x = static_cast<float>(m_Pos.x);
    const auto y = static_cast<float>(m_Pos.y);

    RenderImage(IMAGE_NPCSHOP_BACK, x, y, WIDTH, HEIGHT);
    RenderImage(IMAGE_NPCSHOP_TOP, x, y, WIDTH, 64.f);
    RenderImage(IMAGE_NPCSHOP_LEFT, x, y + 64, 21.f, 320.f);
    RenderImage(IMAGE_NPCSHOP_RIGHT, x + WIDTH - 21, y + 64, 21.f, 320.f);
    RenderImage(IMAGE_NPCSHOP_BOTTOM, x, y + 429 - 45, WIDTH, 45.f);

    for (int i = MAX_INVENTORY_EXT_COUNT - 1; i >= CharacterAttribute->InventoryExtensions; --i)
    {
        RenderImage(IMAGE_EXTENSION_TABLE, x + 11, y + 42 + i * HEIGHT_PER_EXT, 173, HEIGHT_PER_EXT);
        RenderImage(IMAGE_EXTENSION_EMPTY, x + 15, y + 45 + i * HEIGHT_PER_EXT, 161, HEIGHT_PER_EXT - (EXT_BORDER * 2));
        RenderImage(static_cast<GLuint>(IMAGE_EXTENSION_NO1 + i), x + 85, y + 71 + i * HEIGHT_PER_EXT, 25, 28);
    }
}

void CNewUIInventoryExtension::RenderTexts() const
{
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetTextColor(220, 220, 220, 255);

    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 12, GlobalText[3323], WIDTH, 0, RT3_SORT_CENTER);
}

float CNewUIInventoryExtension::GetLayerDepth()
{
    return 4.55;
}

void CNewUIInventoryExtension::SetButtonInfo()
{
    m_BtnExit.ChangeButtonImgState(true, IMAGE_INVENTORY_EXIT_BTN, false);
    m_BtnExit.ChangeToolTipText(GlobalText[1002], true);
}

void CNewUIInventoryExtension::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_NPCSHOP_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back04.tga", IMAGE_NPCSHOP_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_NPCSHOP_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_NPCSHOP_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_NPCSHOP_BOTTOM, GL_LINEAR);

    LoadBitmap(L"Interface\\newui_item_add_marking_non.jpg", IMAGE_EXTENSION_EMPTY, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_add_table.tga", IMAGE_EXTENSION_TABLE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_add_marking_no01.tga", IMAGE_EXTENSION_NO1, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_add_marking_no02.tga", IMAGE_EXTENSION_NO2, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_add_marking_no03.tga", IMAGE_EXTENSION_NO3, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_add_marking_no04.tga", IMAGE_EXTENSION_NO4, GL_LINEAR);
}

void CNewUIInventoryExtension::UnloadImages()
{
    DeleteBitmap(IMAGE_NPCSHOP_BACK);
    DeleteBitmap(IMAGE_NPCSHOP_TOP);
    DeleteBitmap(IMAGE_NPCSHOP_LEFT);
    DeleteBitmap(IMAGE_NPCSHOP_LEFT);
    DeleteBitmap(IMAGE_NPCSHOP_BOTTOM);

    DeleteBitmap(IMAGE_EXTENSION_EMPTY);
    DeleteBitmap(IMAGE_EXTENSION_TABLE);
    DeleteBitmap(IMAGE_EXTENSION_NO1);
    DeleteBitmap(IMAGE_EXTENSION_NO2);
    DeleteBitmap(IMAGE_EXTENSION_NO3);
    DeleteBitmap(IMAGE_EXTENSION_NO4);
}

CNewUIInventoryCtrl* CNewUIInventoryExtension::TryGetExtensionByInventoryIndex(int iIndex) const
{
    const auto index = iIndex - MAX_MY_INVENTORY_INDEX;
    const auto extensionIndex = index / MAX_INVENTORY_EXT_ONE;
    if (extensionIndex >= 0 && extensionIndex < MAX_INVENTORY_EXT_COUNT)
    {
        return m_extensions[extensionIndex];
    }

    return nullptr;
}

ITEM* CNewUIInventoryExtension::FindItem(int iIndex) const
{
    if (const auto& extension = TryGetExtensionByInventoryIndex(iIndex))
    {
        return extension->FindItem(iIndex);
    }
    return nullptr;
}

bool CNewUIInventoryExtension::InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket) const
{
    if (const auto& extension = TryGetExtensionByInventoryIndex(iIndex))
    {
        extension->AddItem(iIndex, pbyItemPacket);
    }

    return false;
}

void CNewUIInventoryExtension::DeleteItem(int iIndex) const
{
    if (const auto& extension = TryGetExtensionByInventoryIndex(iIndex))
    {
        if (const auto& pItem = extension->FindItem(iIndex))
        {
            extension->RemoveItem(pItem);
            return;
        }

        if (const auto pPickedItem = CNewUIInventoryCtrl::GetPickedItem())
        {
            if (GetOwnerOf(pPickedItem)
                && pPickedItem->GetSourceLinealPos() == iIndex)
            {
                CNewUIInventoryCtrl::DeletePickedItem();
            }
        }
    }
}

void CNewUIInventoryExtension::DeleteAllItems() const
{
    for (auto* extension : m_extensions)
    {
        if (extension)
        {
            extension->RemoveAllItems();
        }
    }
}

CNewUIInventoryCtrl* CNewUIInventoryExtension::GetOwnerOf(const CNewUIPickedItem* pPickedItem) const
{
    if (!pPickedItem)
    {
        return nullptr;
    }

    const auto* ownerOfItem = pPickedItem->GetOwnerInventory();

    for (auto* extension : m_extensions)
    {
        if (extension == ownerOfItem)
        {
            return extension;
        }
    }

    return nullptr;
}