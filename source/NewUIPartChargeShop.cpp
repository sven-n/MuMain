// NewUIPartChargeShop.cpp: implementation of the CNewUIPartChargeShop class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIPartChargeShop.h"
#include "NewUISystem.h"

#include "DSPlaySound.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_SHELL

using namespace SEASON3B;

CNewUIPartChargeShop::CNewUIPartChargeShop() : m_pNewUIMng( NULL )
{
	TheClientSystem().ChangeTask( Task::eGame );
}

CNewUIPartChargeShop::~CNewUIPartChargeShop()
{
	Release();
}

bool CNewUIPartChargeShop::Create(CNewUIManager* pNewUIMng, int x, int y)
{
	if (NULL == pNewUIMng) 
	{ 
		return false;
	}
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_PARTCHARGE_SHOP, this);

	Show(false);
	
	return true;
}

void CNewUIPartChargeShop::Release()
{
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj( this );
		m_pNewUIMng = NULL;
	}
}

void CNewUIPartChargeShop::OpeningProcess()
{
	PlayBuffer(SOUND_CLICK01);
}

void CNewUIPartChargeShop::ClosingProcess()
{
	PlayBuffer(SOUND_CLICK01);
}

bool CNewUIPartChargeShop::UpdateMouseEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PARTCHARGE_SHOP))
	{
		TheUISystem().Process();
		return false;
	}

	return true;
}

bool CNewUIPartChargeShop::UpdateKeyEvent()
{
	if(g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PARTCHARGE_SHOP) == false)
		return true;
	
	if(SEASON3B::IsPress(VK_ESCAPE) == true)
	{
		SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
		
		if( !TheUISystem().IsFrame( UIMESSAGEBOXFRAME_NAME ) )
		{
			TheShopServerProxy().SetShopOut();
			PlayBuffer(SOUND_CLICK01);
			return false;
		}
	}

	return true;
}

bool CNewUIPartChargeShop::Update()
{
	return true;	
}

bool CNewUIPartChargeShop::Render()
{
	TheUISystem().Drew();
	BeginBitmap();
	EnableAlphaTest();

	return true;
}

float CNewUIPartChargeShop::GetLayerDepth()		// 10.08f
{
	return 10.08f;
}

bool CNewUIPartChargeShop::IsVisible() const
{
	return CNewUIObj::IsVisible();
}

#endif //NEW_USER_INTERFACE_SHELL
