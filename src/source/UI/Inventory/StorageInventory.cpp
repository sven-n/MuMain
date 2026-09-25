//*****************************************************************************
// File: NewUIStorageInventory.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UI/Inventory/StorageInventory.h"
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"
#include "UI/Core/WindowSystem.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "Engine/Object/ZzzInventory.h"
#include "UI/Inventory/MyInventory.h"
#include "Scenes/SceneCore.h" // g_iLengthAuthorityCode -- the WEBZEN password field's maxLength below

// RmlUi migration -- see this class's header comment.
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/Scaling/UITransform.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
    // Plain 4-digit Mode::NumericKeypad PIN verify. Two call sites: SendRequestItemToMyInven's
    // locked-vault guard below, and the insufficient-storage-gold zen-payment onPrimary further
    // down in this file.
    void ShowVaultPinVerifyDialog()
    {
        GenericDialogConfig cfg;
        cfg.showCancel = true;
        cfg.lines = {
            { I18N::Game::PasswordVerification, false },
            { I18N::Game::Choose4DigitsForPassword, false },
        };
        cfg.input = GenericDialogConfig::InputField{};
        cfg.input->mode = GenericDialogConfig::InputField::Mode::NumericKeypad;
        cfg.input->maxLength = 4;
        cfg.tallPanel = true; // the on-screen digit pad doesn't comfortably fit the default panel height
        cfg.onPrimary = []
        {
            const std::wstring strText = g_pGenericConfirmDialog->GetInputText();
            if (strText.size() < 4)
            {
                g_pGenericConfirmDialog->KeepOpen();
                return;
            }
            SocketClient->ToGameServer()->SendUnlockVault((WORD)_wtoi(strText.c_str()));
        };
        cfg.onSecondary = []
        {
            if (g_pPickedItem)
                g_pPickedItem->ShowPickedItem();
            if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_STORAGE))
                g_pStorageInventory->SetItemAutoMove(false);
        };
        g_pGenericConfirmDialog->Show(std::move(cfg));
    }

    // PIN re-entry confirm step of the vault-lock flow. Only ever reached from
    // ShowStorageLockPinDialog()'s own onPrimary below. `firstPin` is the PIN just typed in the
    // first step, captured by value.
    void ShowStorageLockPinConfirmDialog(std::wstring firstPin)
    {
        GenericDialogConfig cfg;
        cfg.showCancel = true;
        cfg.lines = {
            { I18N::Game::VerifyNewPassword, false },
            { I18N::Game::EnterPasswordAgain, false },
        };
        cfg.input = GenericDialogConfig::InputField{};
        cfg.input->mode = GenericDialogConfig::InputField::Mode::NumericKeypad;
        cfg.input->maxLength = 4;
        cfg.tallPanel = true; // the on-screen digit pad doesn't comfortably fit the default panel height
        cfg.onPrimary = [firstPin]
        {
            const std::wstring strText = g_pGenericConfirmDialog->GetInputText();
            if (strText.size() < 4)
            {
                g_pGenericConfirmDialog->KeepOpen();
                return;
            }
            if (strText[0] == strText[1] && strText[1] == strText[2] && strText[2] == strText[3])
            {
                mu::ui::window::CreateOkMessageBox(I18N::Game::ItIsNotAllowedToUseSame4Numbers);
                return;
            }
            if (strText != firstPin)
            {
                mu::ui::window::CreateOkMessageBox(I18N::Game::PasswordIsIncorrect);
                return;
            }

            // Masked (bIsPassword=true), non-numeric-restricted Mode::Text WEBZEN.COM password
            // entry. The 4-digit PIN just confirmed above has no equivalent field on
            // GenericDialogConfig -- captured directly in the closure instead of a
            // SetPassword()/GetPassword() round-trip.
            const WORD wInputNumber = (WORD)_wtoi(strText.c_str());
            GenericDialogConfig pwCfg;
            pwCfg.showCancel = true;
            pwCfg.lines = {
                { I18N::Game::EnterYourWEBZENCOMPassword, false },
                { I18N::Game::EnterYourWEBZENCOMPassword697, false },
            };
            pwCfg.input = GenericDialogConfig::InputField{};
            pwCfg.input->mode = GenericDialogConfig::InputField::Mode::Text;
            pwCfg.input->maxLength = g_iLengthAuthorityCode;
            pwCfg.input->masked = true;
            pwCfg.onPrimary = [wInputNumber]
            {
                const std::wstring strPassword = g_pGenericConfirmDialog->GetInputText();
                if (strPassword.empty())
                {
                    g_pGenericConfirmDialog->KeepOpen();
                    return;
                }
                SocketClient->ToGameServer()->SendSetVaultPin(wInputNumber, MU_C16(strPassword.c_str()));
            };
            g_pGenericConfirmDialog->Show(std::move(pwCfg));
        };
        g_pGenericConfirmDialog->Show(std::move(cfg));
    }

    // First step of the vault-lock flow (choose a new 4-digit PIN). One call site:
    // storage_lock_click's !m_bLock branch below.
    void ShowStorageLockPinDialog()
    {
        GenericDialogConfig cfg;
        cfg.showCancel = true;
        cfg.lines = {
            { I18N::Game::ChooseNewPassword, false },
            { I18N::Game::Choose4DigitsForPassword, false },
        };
        cfg.input = GenericDialogConfig::InputField{};
        cfg.input->mode = GenericDialogConfig::InputField::Mode::NumericKeypad;
        cfg.input->maxLength = 4;
        cfg.tallPanel = true; // the on-screen digit pad doesn't comfortably fit the default panel height
        cfg.onPrimary = []
        {
            const std::wstring strText = g_pGenericConfirmDialog->GetInputText();
            if (strText.size() < 4)
            {
                g_pGenericConfirmDialog->KeepOpen();
                return;
            }
            if (strText[0] == strText[1] && strText[1] == strText[2] && strText[2] == strText[3])
            {
                mu::ui::window::CreateOkMessageBox(I18N::Game::ItIsNotAllowedToUseSame4Numbers);
                return;
            }
            ShowStorageLockPinConfirmDialog(strText);
        };
        g_pGenericConfirmDialog->Show(std::move(cfg));
    }
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CStorageInventory::CStorageInventory()
{
    m_pNewUIMng = nullptr;
    m_pNewInventoryCtrl = nullptr;
    m_Pos.x = m_Pos.y = 0;
    m_nBackupSourceInvenIndex = -1;
}

CStorageInventory::~CStorageInventory()
{
    Release();
}

bool CStorageInventory::Create(CManager* pNewUIMng, int x, int y)
{
    if (nullptr == pNewUIMng || nullptr == g_pNewUI3DRenderMng
        || nullptr == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(INTERFACE_STORAGE, this);

    m_pNewInventoryCtrl = new CInventoryCtrl;
    if (false == m_pNewInventoryCtrl->Create(STORAGE_TYPE::VAULT, g_pNewUI3DRenderMng, g_pNewItemMng, this, x + 15, y + 36, 8, 15))
    {
        SAFE_DELETE(m_pNewInventoryCtrl);
        return false;
    }

    SetPos(x, y);

    m_bLock = false;
    SetItemAutoMove(false);
    InitBackupItemInfo();

    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}

void CStorageInventory::BuildRmlUi()
{
    // Guarded so the document/model are created once, even if Create() re-runs on resolution change.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "storage",
            [this](Rml::DataModelConstructor& c, StorageRmlModel& model)
            {
                c.Bind("root_x", &model.rootX);
                c.Bind("root_y", &model.rootY);
                c.Bind("root_scale", &model.rootScale);

                c.Bind("title", &model.title);
                c.Bind("title_locked", &model.titleLocked);

                c.Bind("zen_text", &model.zenText);
                c.Bind("zen_color", &model.zenColor);
                c.Bind("fee_label", &model.feeLabel);
                c.Bind("fee_value", &model.feeValue);

                c.Bind("expand_visible", &model.expandVisible);
                c.Bind("expand_tooltip", &model.expandTooltip);

                c.Bind("storage_locked", &model.storageLocked);

                c.Bind("insert_tooltip", &model.insertTooltip);
                c.Bind("take_tooltip", &model.takeTooltip);
                c.Bind("lock_tooltip", &model.lockTooltip);

                c.BindEventCallback("storage_insert_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                    {
                        mu::ui::window::GenericDialogConfig cfg;
                        cfg.showCancel = true;
                        cfg.lines = { { I18N::Game::EnterTheAmountOfZenYouWouldLikeToDeposit, false } };
                        cfg.input = mu::ui::window::GenericDialogConfig::InputField{};
                        cfg.input->mode = mu::ui::window::GenericDialogConfig::InputField::Mode::Text;
                        cfg.input->maxLength = 8;
                        cfg.input->numericOnly = true;
                        cfg.onPrimary = []
                        {
                            const std::wstring strText = mu::ui::window::g_pGenericConfirmDialog->GetInputText();
                            const int iInputZen = strText.empty() ? 0 : _wtoi(strText.c_str());
                            if (iInputZen == 0)
                            {
                                mu::ui::window::g_pGenericConfirmDialog->KeepOpen();
                                return;
                            }
                            if (iInputZen <= (int)CharacterMachine->Gold)
                            {
                                SocketClient->ToGameServer()->SendVaultMoveMoneyRequest(
                                    VaultMoneyMoveDirection::InventoryToVault, iInputZen);
                            }
                            else
                            {
                                mu::ui::window::CreateOkMessageBox(I18N::Game::YouAreShortOfZen);
                            }
                        };
                        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
                    });
                c.BindEventCallback("storage_take_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                    {
                        // The insufficient-storage-gold branch chains into
                        // ShowVaultPinVerifyDialog() (this file).
                        mu::ui::window::GenericDialogConfig cfg;
                        cfg.showCancel = true;
                        cfg.lines = { { I18N::Game::EnterTheAmountOfZenYouWouldLikeToWithdraw, false } };
                        cfg.input = mu::ui::window::GenericDialogConfig::InputField{};
                        cfg.input->mode = mu::ui::window::GenericDialogConfig::InputField::Mode::Text;
                        cfg.input->maxLength = 8;
                        cfg.input->numericOnly = true;
                        cfg.onPrimary = []
                        {
                            const std::wstring strText = mu::ui::window::g_pGenericConfirmDialog->GetInputText();
                            const int iInputZen = strText.empty() ? 0 : _wtoi(strText.c_str());
                            if (iInputZen == 0)
                            {
                                mu::ui::window::g_pGenericConfirmDialog->KeepOpen();
                                return;
                            }
                            if (iInputZen <= CharacterMachine->StorageGold
                                && CharacterMachine->Gold + iInputZen <= 2000000000)
                            {
                                if (!g_pStorageInventory->IsStorageLocked() || g_pStorageInventory->IsCorrectPassword())
                                {
                                    SocketClient->ToGameServer()->SendVaultMoveMoneyRequest(
                                        VaultMoneyMoveDirection::VaultToInventory, iInputZen);
                                }
                                else
                                {
                                    g_pStorageInventory->SetBackupTakeZen(iInputZen);
                                    ShowVaultPinVerifyDialog();
                                }
                            }
                            else if (CharacterMachine->Gold + iInputZen > 2000000000)
                            {
                                // Silent no-op, matching native -- still closes.
                            }
                            else
                            {
                                mu::ui::window::CreateOkMessageBox(I18N::Game::YouAreShortOfZen);
                            }
                        };
                        mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
                    });
                c.BindEventCallback("storage_lock_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                    {
                        if (m_bLock)
                        {
                            mu::ui::window::GenericDialogConfig cfg;
                            cfg.showCancel = true;
                            cfg.lines = {
                                { I18N::Game::WarehouseLockUnlock, false },
                                { I18N::Game::EnterYourWEBZENCOMPassword697, false },
                            };
                            cfg.input = mu::ui::window::GenericDialogConfig::InputField{};
                            cfg.input->mode = mu::ui::window::GenericDialogConfig::InputField::Mode::Text;
                            cfg.input->maxLength = g_iLengthAuthorityCode;
                            cfg.input->masked = true;
                            cfg.onPrimary = []
                            {
                                const std::wstring strText = mu::ui::window::g_pGenericConfirmDialog->GetInputText();
                                if (strText.empty())
                                {
                                    mu::ui::window::g_pGenericConfirmDialog->KeepOpen();
                                    return;
                                }
                                SocketClient->ToGameServer()->SendRemoveVaultPin(MU_C16(strText.c_str()));
                            };
                            mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
                        }
                        else
                            ShowStorageLockPinDialog();
                    });
                c.BindEventCallback("storage_expand_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                    {
                        if (CharacterAttribute->IsVaultExtended > 0)
                            g_pNewUISystem->Toggle(INTERFACE_STORAGE_EXT);
                    });
            });

        if (modelCreated)
            m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/storage.rml");

        // Frame background panel uses the background context -- see StorageBgRmlModel (StorageInventory.h).
        if (Rml::Context* bgContext = RmlUiRuntime::Instance().GetBackgroundContext())
        {
            const bool bgModelCreated = m_BgRmlBinder.Create(bgContext, "storage_bg",
                [](Rml::DataModelConstructor& c, StorageBgRmlModel& model)
                {
                    c.Bind("root_x", &model.rootX);
                    c.Bind("root_y", &model.rootY);
                    c.Bind("root_scale", &model.rootScale);
                });
            if (bgModelCreated)
            {
                // Starts hidden -- CreateBackgroundDocument() no longer Show()s eagerly (see its
                // own comment, RmlTheme.h); SyncRmlModel() below is what shows/hides it.
                m_pRmlBgDoc = UI::RmlBridge::CreateBackgroundDocument("Data/Interface/RmlUi/storage_bg.rml");
            }
        }

        // Not Show()n here -- m_pRmlDoc's visibility follows this window's own Show()/Hide() via
        // SyncRmlModel(), not an eager Show() at Create() time.
    }
}

void CStorageInventory::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return; // never opened -- BuildRmlUi() will simply pick up the new theme whenever it first is

    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    if (m_pRmlBgDoc)
    {
        if (Rml::Context* bgContext = RmlUiRuntime::Instance().GetBackgroundContext())
        {
            m_BgRmlBinder.Destroy(bgContext);
            bgContext->UnloadDocument(m_pRmlBgDoc);
        }
        m_pRmlBgDoc = nullptr;
    }

    BuildRmlUi();
    // Next frame's Update()/SyncRmlModel() self-corrects live state/visibility for both docs.
}

void CStorageInventory::Release()
{
    SAFE_DELETE(m_pNewInventoryCtrl);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        UI::RmlBridge::UnregisterForThemeReload(this);
        m_pNewUIMng = nullptr;
    }
}

void CStorageInventory::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
    if (m_pNewInventoryCtrl)
    {
        m_pNewInventoryCtrl->SetPos(x + 15, y + 36);
    }
}

bool CStorageInventory::UpdateMouseEvent()
{
    if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->UpdateMouseEvent())
        return false;

    ProcessInventoryCtrl();

    if (ProcessBtns())
        return false;

    // #panel's own live RCSS size is the source of truth -- STORAGE_WIDTH/HEIGHT only cover the
    // first frame after Create()/Show(true)/ReloadRmlTheme(), before RmlUi's next layout pass.
    float panelWidth = STORAGE_WIDTH;
    float panelHeight = STORAGE_HEIGHT;
    UI::RmlBridge::RefreshLogicalPanelSize(m_pRmlDoc, "panel", panelWidth, panelHeight);
    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, static_cast<int>(panelWidth), static_cast<int>(panelHeight)).Contains(MouseX, MouseY))
    {
        if (IsPress(VK_RBUTTON))
        {
            MouseRButton = false;
            MouseRButtonPop = false;
            MouseRButtonPush = false;
            return false;
        }

        if (!IsNone(VK_LBUTTON))
        {
            return false;
        }
    }

    return true;
}

bool CStorageInventory::UpdateKeyEvent()
{
    if (!g_pNewUISystem->IsVisible(INTERFACE_STORAGE))
    {
        return true;
    }

    if (IsPress(VK_ESCAPE) == true)
    {
        g_pNewUISystem->Hide(INTERFACE_STORAGE);
        PlayBuffer(SOUND_CLICK01);
        return false;
    }

    if (CharacterAttribute->IsVaultExtended > 0 && IsPress('H'))
    {
        g_pNewUISystem->Toggle(INTERFACE_STORAGE_EXT);
        PlayBuffer(SOUND_CLICK01);
        return false;
    }
    return true;
}

bool CStorageInventory::Update()
{
    if (m_pNewInventoryCtrl && !m_pNewInventoryCtrl->Update())
        return false;

    SyncRmlModel();
    return true;
}

bool CStorageInventory::Render()
{
    EnableAlphaTest();

    // Frame background panel is RmlUi, routed through the background context (see
    // StorageBgRmlModel), painted by CManager::Render()'s centralized RenderBackgroundLayer() call
    // before this window's own Render()/Render3D() run.
    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->Render();

    DisableAlphaBlend();

    return true;
}

void CStorageInventory::SyncRmlModel()
{
    if (m_pRmlBgDoc)
    {
        UI::RmlBridge::SyncRootTransform(m_BgRmlBinder, m_Pos);

        // RenderBackgroundLayer() renders whatever's shown in the shared background context
        // regardless of caller, so this Hide()/Show() is what keeps the bg panel hidden when closed.
        if (IsVisible()) m_pRmlBgDoc->Show(); else m_pRmlBgDoc->Hide();
    }

    if (!m_pRmlDoc) return;
    if (IsVisible()) m_pRmlDoc->Show(); else m_pRmlDoc->Hide();

    UI::RmlBridge::SyncRootTransform(m_RmlBinder, m_Pos);

    auto syncBool = [this](bool StorageRmlModel::* field, const char* boundName, bool value)
    {
        if (m_RmlBinder.GetModel().*field != value) { m_RmlBinder.GetModel().*field = value; m_RmlBinder.MarkDirty(boundName); }
    };
    auto syncText = [this](Rml::String StorageRmlModel::* field, const char* boundName, const Rml::String& value)
    {
        if (m_RmlBinder.GetModel().*field != value) { m_RmlBinder.GetModel().*field = value; m_RmlBinder.MarkDirty(boundName); }
    };
    auto syncWide = [&](Rml::String StorageRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        syncText(field, boundName, StringUtils::WideToNarrow(text));
    };

    // "Storage (open/close)" title text, red when locked.
    wchar_t titleBuf[128];
    mu_swprintf(titleBuf, L"%ls (%ls)", I18N::Game::Storage, I18N::Game::Lookup(m_bLock ? 241 : 240));
    syncWide(&StorageRmlModel::title, "title", titleBuf);
    syncBool(&StorageRmlModel::titleLocked, "title_locked", m_bLock);

    const int nZen = CharacterMachine->StorageGold;
    wchar_t zenBuf[256] = { 0, };
    ConvertGold(nZen, zenBuf);
    syncWide(&StorageRmlModel::zenText, "zen_text", zenBuf);

    // getGoldColor() packs (A<<24)+(R<<16)+(G<<8)+B -- unpack into an rgba() CSS string, same
    // technique as CMyInventory's gold_color (legacy theme only binds this; modern uses a fixed
    // warm-gold color, same reasoning as my_inventory.rml's #gold_text).
    const unsigned int zenArgb = getGoldColor(nZen);
    char zenColorBuf[32];
    snprintf(zenColorBuf, sizeof(zenColorBuf), "rgba(%u,%u,%u,%u)",
        (zenArgb >> 16) & 0xFF, (zenArgb >> 8) & 0xFF, zenArgb & 0xFF, (zenArgb >> 24) & 0xFF);
    syncText(&StorageRmlModel::zenColor, "zen_color", Rml::String(zenColorBuf));

    syncWide(&StorageRmlModel::feeLabel, "fee_label", I18N::Game::StorageFee);

    const __int64 iTotalLevel = (__int64)CharacterAttribute->Level + Master_Level_Data.nMLevel;
    int nFee = int(double(iTotalLevel) * double(iTotalLevel) * 0.04);
    nFee += m_bLock ? int(CharacterAttribute->Level) * 2 : 0;
    nFee = std::max<int>(1, nFee);
    if (nFee >= 1000)
        nFee = nFee / 100 * 100;
    else if (nFee >= 100)
        nFee = nFee / 10 * 10;
    wchar_t feeBuf[256] = { 0, };
    ConvertGold(nFee, feeBuf);
    syncWide(&StorageRmlModel::feeValue, "fee_value", feeBuf);

    syncBool(&StorageRmlModel::expandVisible, "expand_visible", CharacterAttribute->IsVaultExtended > 0);
    syncWide(&StorageRmlModel::expandTooltip, "expand_tooltip", I18N::Game::OpeningAnExpandedVaultH);

    syncBool(&StorageRmlModel::storageLocked, "storage_locked", m_bLock);

    syncWide(&StorageRmlModel::insertTooltip, "insert_tooltip", I18N::Game::Deposit);
    syncWide(&StorageRmlModel::takeTooltip, "take_tooltip", I18N::Game::Withdraw);
    syncWide(&StorageRmlModel::lockTooltip, "lock_tooltip", I18N::Game::WarehouseLockUnlock);
}

float CStorageInventory::GetLayerDepth()
{
    return 2.2f;
}

CInventoryCtrl* CStorageInventory::GetInventoryCtrl() const
{
    return m_pNewInventoryCtrl;
}

void CStorageInventory::LockStorage(bool bLock)
{
    m_bLock = bLock;
}

bool CStorageInventory::ProcessClosing()
{
    if (EquipmentItem)
        return false;

    CInventoryCtrl::BackupPickedItem();
    DeleteAllItems();
    SocketClient->ToGameServer()->SendVaultClosed();
    return true;
}

bool CStorageInventory::InsertItem(int nIndex, std::span<const BYTE> pbyItemPacket)
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->AddItem(nIndex, pbyItemPacket);

    return false;
}

void CStorageInventory::DeleteAllItems()
{
    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->RemoveAllItems();
}

void CStorageInventory::ProcessInventoryCtrl()
{
    if (nullptr == m_pNewInventoryCtrl)
    {
        return;
    }

    CPickedItem* pPickedItem = CInventoryCtrl::GetPickedItem();
    if (pPickedItem)
    {
        ITEM* pItemObj = pPickedItem->GetItem();
        if (nullptr == pItemObj)
        {
            return;
        }

        if (IsPress(VK_LBUTTON) || IsRelease(VK_LBUTTON))
        {
            const int nDstIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);

            if (nDstIndex >= 0 && m_pNewInventoryCtrl->CanMove(nDstIndex, pItemObj))
            {
                const int nSrcIndex = pPickedItem->GetSourceLinealPos();
                const auto sourceStorageType = pPickedItem->GetSourceStorageType();
                const auto targetStorageType = m_pNewInventoryCtrl->GetStorageType();
                SendRequestEquipmentItem(sourceStorageType, nSrcIndex,
                    pItemObj, targetStorageType, nDstIndex);
            }
        }
        else
        {
            if (::IsStoreBan(pItemObj))
            {
                m_pNewInventoryCtrl->SetSquareColorNormal(1.0f, 0.0f, 0.0f);
            }
            else
            {
                m_pNewInventoryCtrl->SetSquareColorNormal(0.1f, 0.4f, 0.8f);
            }
        }
    }
    else if (IsPress(VK_RBUTTON))
    {
        ProcessStorageItemAutoMove();
    }
}

void CStorageInventory::ProcessStorageItemAutoMove()
{
    if (g_pPickedItem)
        if (g_pPickedItem->GetItem())
            return;

    if (IsItemAutoMove())
        return;

    ITEM* pItemObj = m_pNewInventoryCtrl->FindItemAtPt(MouseX, MouseY);
    if (pItemObj)
    {
        int nDstIndex = g_pMyInventory->FindEmptySlotIncludingExtensions(pItemObj);
        if (-1 != nDstIndex)
        {
            SetItemAutoMove(true);

            int nSrcIndex
                = pItemObj->y * m_pNewInventoryCtrl->GetNumberOfColumn()
                + pItemObj->x;
            SendRequestItemToMyInven(pItemObj, nSrcIndex, nDstIndex);

            PlayBuffer(SOUND_GET_ITEM01);
        }
    }
}

bool CStorageInventory::ProcessMyInvenItemAutoMove(CInventoryCtrl* sourceCtrl)
{
    if (g_pPickedItem && g_pPickedItem->GetItem())
    {
        return false;
    }

    if (IsItemAutoMove())
    {
        return false;
    }

    if (sourceCtrl == nullptr)
    {
        sourceCtrl = g_pMyInventory->GetInventoryCtrl();
    }

    if (sourceCtrl == nullptr)
    {
        return false;
    }

    if (const auto pItemObj = sourceCtrl->FindItemAtPt(MouseX, MouseY))
    {
        if (pItemObj->Type == ITEM_WIZARDS_RING)
            return false;

        const int emptySlotIndex = FindEmptySlot(pItemObj);
        if (-1 != emptySlotIndex)
        {
            const int nSrcIndex = sourceCtrl->GetIndexByItem(pItemObj);
            if (nSrcIndex < 0)
            {
                return false;
            }

            SetItemAutoMove(true, nSrcIndex);
            SendRequestItemToStorage(pItemObj, nSrcIndex, emptySlotIndex);
            PlayBuffer(SOUND_GET_ITEM01);
            return true;
        }
    }

    return false;
}

void CStorageInventory::SendRequestItemToMyInven(ITEM* pItemObj, int nStorageIndex, int nInvenIndex)
{
    if (!IsStorageLocked() || IsCorrectPassword())
    {
        SendRequestEquipmentItem(STORAGE_TYPE::VAULT, nStorageIndex,
            pItemObj, STORAGE_TYPE::INVENTORY, nInvenIndex);
    }
    else
    {
        SetBackupInvenIndex(nInvenIndex);
        if (!IsItemAutoMove())
            g_pPickedItem->HidePickedItem();

        ShowVaultPinVerifyDialog();
    }
}

void CStorageInventory::SendRequestItemToStorage(ITEM* pItemObj, int nInvenIndex, int nStorageIndex)
{
    if (IsStoreBan(pItemObj))
    {
#ifdef KJH_PBG_ADD_INGAMESHOP_SYSTEM
        // MessageBox
        CreateOkMessageBoxWithTitle(I18N::Game::Error, I18N::Game::TheseItemsCannotBeStoredInTheInventory);
#endif // KJH_PBG_ADD_INGAMESHOP_SYSTEM

        g_pSystemLogBox->AddText(I18N::Game::TheseItemsCannotBeStoredInTheInventory, TYPE_ERROR_MESSAGE);
        CInventoryCtrl::BackupPickedItem();

        if (IsItemAutoMove())
            SetItemAutoMove(false);
    }
    else
    {
        SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, nInvenIndex,
            pItemObj, STORAGE_TYPE::VAULT, nStorageIndex);
    }
}

bool CStorageInventory::ProcessBtns()
{
    // Top-right corner close "X" (shared frame): hides + swallows the click. The 4 real buttons
    // are handled by RmlUi's data-event-click (see Create()).
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, INTERFACE_STORAGE))
        return true;

    return false;
}

void CStorageInventory::SetItemAutoMove(bool bItemAutoMove, int nSourceInvenIndex)
{
    m_bItemAutoMove = bItemAutoMove;

    if (bItemAutoMove)
    {
        m_nBackupMouseX = MouseX;
        m_nBackupMouseY = MouseY;
        m_nBackupSourceInvenIndex = nSourceInvenIndex;
    }
    else
    {
        m_nBackupMouseX = m_nBackupMouseY = 0;
        m_nBackupSourceInvenIndex = -1;
    }
}

void CStorageInventory::InitBackupItemInfo()
{
    m_bTakeZen = false;
    m_nBackupTakeZen = 0;
    m_nBackupInvenIndex = -1;
    m_nBackupSourceInvenIndex = -1;
}

void CStorageInventory::SetBackupTakeZen(int nZen)
{
    m_bTakeZen = true;
    m_nBackupTakeZen = nZen;
}

void CStorageInventory::SetBackupInvenIndex(int nInvenIndex)
{
    m_bTakeZen = false;
    m_nBackupInvenIndex = nInvenIndex;
}

int CStorageInventory::FindEmptySlot(ITEM* pItemObj)
{
    if (pItemObj == nullptr)
        return -1;

    ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItemObj->Type];

    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->FindEmptySlot(pItemAttr->Width, pItemAttr->Height);

    return -1;
}

void CStorageInventory::ProcessToReceiveStorageStatus(BYTE byStatus)
{
    switch (byStatus)
    {
    case 0:
        LockStorage(false);
        SetCorrectPassword(false);
        break;

    case 1:
        LockStorage(true);
        SetCorrectPassword(false);
        break;

    case 10:
        CreateOkMessageBox(I18N::Game::IncorrectPassword);
        CInventoryCtrl::BackupPickedItem();
        ProcessStorageItemAutoMoveFailure();
        break;

    case 11:
        CreateOkMessageBox(I18N::Game::InventoryIsAlreadyLocked);
        break;

    case 12:
        if (IsStorageLocked() && !IsCorrectPassword())
        {
            if (m_bTakeZen)
            {
                SocketClient->ToGameServer()->SendVaultMoveMoneyRequest(VaultMoneyMoveDirection::VaultToInventory, GetBackupTakeZen());
                InitBackupItemInfo();
            }
            else
            {
                ITEM* pItemObj;
                int nStorageIndex;

                if (IsItemAutoMove())
                {
                    pItemObj = m_pNewInventoryCtrl->FindItemAtPt(
                        m_nBackupMouseX, m_nBackupMouseY);
                    nStorageIndex
                        = pItemObj->y * m_pNewInventoryCtrl->GetNumberOfColumn()
                        + pItemObj->x;
                }
                else
                {
                    nStorageIndex = g_pPickedItem->GetSourceLinealPos();
                    pItemObj = g_pPickedItem->GetItem();
                }

                SendRequestEquipmentItem(
                    STORAGE_TYPE::VAULT, nStorageIndex,
                    pItemObj, STORAGE_TYPE::INVENTORY, GetBackupInvenIndex());

                InitBackupItemInfo();
            }
        }
        LockStorage(true);
        SetCorrectPassword(true);
        break;

    case 13:
        CreateOkMessageBox(I18N::Game::ThePasswordYouHaveEnteredIsIncorrect);
        break;
    }
}

void CStorageInventory::ProcessToReceiveStorageItems(int nIndex, std::span<const BYTE> pbyItemPacket)
{
    CInventoryCtrl::DeletePickedItem();

    if (nIndex >= 0 && nIndex < (m_pNewInventoryCtrl->GetNumberOfColumn()
        * m_pNewInventoryCtrl->GetNumberOfRow()))
    {
        if (IsItemAutoMove())
        {
            if (m_nBackupSourceInvenIndex >= MAX_EQUIPMENT_INDEX && m_nBackupSourceInvenIndex < MAX_MY_INVENTORY_INDEX)
            {
                g_pMyInventory->DeleteItem(m_nBackupSourceInvenIndex);
            }
            else if (m_nBackupSourceInvenIndex >= MAX_MY_INVENTORY_INDEX && m_nBackupSourceInvenIndex < MAX_MY_INVENTORY_EX_INDEX)
            {
                g_pMyInventoryExt->DeleteItem(m_nBackupSourceInvenIndex);
            }
            else
            {
                CInventoryCtrl* pMyInvenCtrl = g_pMyInventory->GetInventoryCtrl();
                ITEM* pItemObj = pMyInvenCtrl->FindItemAtPt(m_nBackupMouseX, m_nBackupMouseY);
                g_pMyInventory->GetInventoryCtrl()->RemoveItem(pItemObj);
            }

            SetItemAutoMove(false);
        }

        InsertItem(nIndex, pbyItemPacket);
    }
}

void CStorageInventory::ProcessStorageItemAutoMoveSuccess()
{
    if (!IsVisible())
        return;

    if (IsItemAutoMove())
    {
        ITEM* pItemObj = m_pNewInventoryCtrl->FindItemAtPt(m_nBackupMouseX, m_nBackupMouseY);
        m_pNewInventoryCtrl->RemoveItem(pItemObj);

        SetItemAutoMove(false);
    }
}

void CStorageInventory::ProcessStorageItemAutoMoveFailure()
{
    if (!IsVisible())
        return;

    InitBackupItemInfo();
    SetItemAutoMove(false);
}

int CStorageInventory::GetPointedItemIndex()
{
    return m_pNewInventoryCtrl->GetPointedSquareIndex();
}
