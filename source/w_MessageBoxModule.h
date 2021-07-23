// w_MessageBoxModule.h: interface for the MessageBoxModule class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_CLIENTSYSTEM

#if !defined(AFX_W_MESSAGEBOXMODULE_H__5FCDC925_B524_4156_8388_5DE4AA38E75A__INCLUDED_)
#define AFX_W_MESSAGEBOXMODULE_H__5FCDC925_B524_4156_8388_5DE4AA38E75A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

BoostSmartPointer( MessageBoxModule );
class MessageBoxModule : public Module
{
public:
	static MessageBoxModulePtr Make( const MESSAGEBOXDATA& messageboxdata );
	virtual ~MessageBoxModule();

private:
    void OnOkButtonClick( ui::UIWnd& wnd );
    void OnCloseButtonClick( ui::UIWnd& wnd );
    void OnYesButtonClick( ui::UIWnd& wnd );
    void OnNoButtonClick( ui::UIWnd& wnd );

private:
	void Initialize( const MESSAGEBOXDATA& messageboxdata, weak_ptr<MessageBoxModule> uiHandler );
	void Destroy();
	MessageBoxModule();

private:
	weak_ptr<MESSAGEBOXHANDLER>			m_Handler;
    int									m_ID;
};

#endif // !defined(AFX_W_MESSAGEBOXMODULE_H__5FCDC925_B524_4156_8388_5DE4AA38E75A__INCLUDED_)

#endif //NEW_USER_INTERFACE_CLIENTSYSTEM