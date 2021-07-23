// w_ClientInfoBuilder.h: interface for the ClientInfoBuilder class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_BUILDER

#if !defined(AFX_W_CLIENTINFOBUILDER_H__A61B2F84_D816_459C_BB92_0EB016635151__INCLUDED_)
#define AFX_W_CLIENTINFOBUILDER_H__A61B2F84_D816_459C_BB92_0EB016635151__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*
	클라이언트 빌더는 게임에 필요한 정보가 아닌 좀 더 큰 의미로 총괄적이고 더 큰 의미이다.
	예를 들어 게임에 옵션 등등...
*/

BoostSmartPointer( ClientInfoBuilder );
class ClientInfoBuilder : public Build
{
public:
	enum eType
	{
		eNone,
		eCount,
	};

public:
	static ClientInfoBuilderPtr Make();
	virtual ~ClientInfoBuilder();

private:
	void Initialize();
	void Destroy();

private:
	ClientInfoBuilder();

};

#endif // !defined(AFX_W_CLIENTINFOBUILDER_H__A61B2F84_D816_459C_BB92_0EB016635151__INCLUDED_)

#endif //NEW_USER_INTERFACE_BUILDER