// w_Module.h: interface for the Module class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_CLIENTSYSTEM

#if !defined(AFX_W_MODULE_H__B04EED84_4D7E_4C20_B81F_3F1FA7190880__INCLUDED_)
#define AFX_W_MODULE_H__B04EED84_4D7E_4C20_B81F_3F1FA7190880__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Module  
{
public:
	enum eType
	{
		eNone,
		eMessageBox,
		//Game
		eGame_Party,
		eGame_Buff,
		//Shop
		eShop_Top,
		eShop_Left,
		eShop_Middle,
		eShop_Right,

		eCount,
	};
public:
	Module( eType type );
	virtual ~Module();

public:
	const eType GetType() const;

public:
	eType			m_Type;
};

inline
const Module::eType Module::GetType() const
{
	return m_Type;
}

#endif // !defined(AFX_W_MODULE_H__B04EED84_4D7E_4C20_B81F_3F1FA7190880__INCLUDED_)

#endif //NEW_USER_INTERFACE_CLIENTSYSTEM
