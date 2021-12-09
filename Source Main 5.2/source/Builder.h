// Builder.h: interface for the Builder class.
//
//////////////////////////////////////////////////////////////////////

#ifdef INFO_BUILDER

#if !defined(AFX_BUILDER_H__35C2431A_4627_4EDA_B60F_828C8C795357__INCLUDED_)
#define AFX_BUILDER_H__35C2431A_4627_4EDA_B60F_828C8C795357__INCLUDED_

#include "Singleton.h"
#include "interface.h"

#pragma once

namespace info 
{

class Builder : public Singleton<Builder>
{
public:
	Builder();
	virtual ~Builder();

private:
	void InitBuilder();
	void Clear();

public:
	InfoFile* QueryInfo( InfoTextType type );

private:
	BoostSmart_Ptr(InfoFile) MakeInfo( InfoTextType type );

private:
	typedef std::map< InfoTextType, BoostSmart_Ptr(InfoFile) > InfoMAP;
	
private:
	InfoMAP			m_InfoMap;
	bool			m_isTestServer;
	char			m_Language[32];
};

};

#define g_BuilderINFO info::Builder::GetSingleton ()

template<typename T>
T& TheInfoData( InfoTextType type ) {
	return dynamic_cast<T&>(g_BuilderINFO.QueryInfo(type));
}

#endif // !defined(AFX_BUILDER_H__35C2431A_4627_4EDA_B60F_828C8C795357__INCLUDED_)

#endif //INFO_BUILDER