
#pragma once
#ifdef KJH_ADD_INGAMESHOP_UI_SYSTEM
#include "UI/Dialogs/MessageBox.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "GameShop/IgsDialogModel.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"

#include <RmlUi/Core/Types.h>

namespace Rml
{
class ElementDocument;
}

using namespace SEASON3B;
using namespace mu::ui::window;

class CMsgBoxIGSSendGift : public CMessageBoxBase
{
public:
    enum
    {
        IGS_WINDOW_WIDTH = 640,
        IGS_WINDOW_HEIGHT = 429,
        IGS_FRAME_WIDTH = 210,
        IGS_FRAME_HEIGHT = 267,
        IGS_TEXT_NOTICE_WIDTH = 170,
    };

public:
    CMsgBoxIGSSendGift();
    ~CMsgBoxIGSSendGift();

    bool Create(float fPriority = 3.f);
    void Release();

    bool Update();
    bool Render();
    Rml::Element* GetPanel() const override
    {
        return m_RmlView.Document() != nullptr ? m_RmlView.Document()->GetElementById("panel") : nullptr;
    }

    void Initialize(int iPackageSeq, int iDisplaySeq, int iPriceSeq, DWORD wItemCode, int iCashType, const wchar_t* pszName,
                    const wchar_t* pszPrice, const wchar_t* pszPeriod);

    static CALLBACK_RESULT OKButtonDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);
    static CALLBACK_RESULT CancelButtonDown(class CMessageBoxBase* pOwner, const leaf::xstreambuf& xParam);

private:
    void SetAddCallbackFunc();


    void ChangeInputBoxFocus();


    void InitInputBox();

private:
    // igs_send_gift.rml: the dialog, with the recipient and the message as its fields.
    struct SendGiftRmlModel
    {
        float textPx = 0.f;
        Rml::String recipient;
        Rml::String message;
        Rml::String title, recipientLabel, messageLabel;
        std::vector<Rml::String> itemLines;
        std::vector<Rml::String> noticeLines;
        std::vector<GameShop::DialogButton> buttons;
        std::vector<Rml::String> debugLines;
    };
    void BindRmlModel(Rml::DataModelConstructor& c, SendGiftRmlModel& model);
    UI::RmlBridge::ThemedView<SendGiftRmlModel> m_RmlView{"igs_send_gift",
        [this](Rml::DataModelConstructor& c, SendGiftRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/igs_send_gift.rml"}}};

    void SyncRmlModel();
    bool FieldHasFocus(const char* id) const;
    int m_PressedButton = -1;
    // Where the dialog stands on the stage (UI::RmlBridge::PlaceOnStage()).
    UI::RmlBridge::SlotPlacement m_Placement;

    int m_iPackageSeq;
    int m_iDisplaySeq;
    int m_iPriceSeq;
    DWORD m_wItemCode;
    int m_iCashType;

    wchar_t m_szID[MAX_USERNAME_SIZE + 1];
    wchar_t m_szMessage[MAX_GIFT_MESSAGE_SIZE];

    wchar_t m_szName[MAX_TEXT_LENGTH];
    wchar_t m_szPrice[MAX_TEXT_LENGTH];
    wchar_t m_szPeriod[MAX_TEXT_LENGTH];

    wchar_t m_szNotice[NUM_LINE_CMB][MAX_TEXT_LENGTH];

    int m_iNumNoticeLine;
};

class CMsgBoxIGSSendGiftLayout : public TMsgBoxLayout<CMsgBoxIGSSendGift>
{
public:
    bool SetLayout();
};

#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM
