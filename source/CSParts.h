//////////////////////////////////////////////////////////////////////////
//  
//  CSParts.h
//  
//  내  용 : 캐릭터에 연결되는 오브젝트 처리.
//  
//  날  짜 : 2004/09 06
//
//  작성자 : 조 규 하.
//  
//////////////////////////////////////////////////////////////////////////
#ifndef __CSPARTS_H__
#define __CSPARTS_H__

//////////////////////////////////////////////////////////////////////////
//  INCLUDE.
//////////////////////////////////////////////////////////////////////////
#include "zzzBmd.h"
#include "zzzinfomation.h"
#include "zzzobject.h"
#include "zzzcharacter.h"

//////////////////////////////////////////////////////////////////////////
//  CLASS
//////////////////////////////////////////////////////////////////////////
class CSIPartsMDL
{
protected :
    OBJECT  m_pObj;             //  모델 타입.
    int     m_iBoneNumber;      //  연결될 번 번호.
    vec3_t  m_vOffset;          //  offset.

public :
    CSIPartsMDL ( void ) : m_iBoneNumber( -1 ) {};
    virtual void    IRender ( CHARACTER* c ) = 0;
	inline OBJECT* GetObject() { return &m_pObj; }
};


//  기본적인 모델 연결.
class CSParts : public CSIPartsMDL
{
private :

public :
    CSParts ( int Type, int BoneNumber, bool bBillBoard=false, float x=0.f, float y=0.f, float z=0.f, float ax=0.f, float ay=0.f, float az=0.f );
    virtual void    IRender ( CHARACTER* c );
};


//  프로그램적인 애니메이션 되는 모델.
class CSAnimationParts : public CSIPartsMDL
{
private :

public :
    CSAnimationParts ( int Type, int BoneNumber, bool bBillBoard=false, float x=0.f, float y=0.f, float z=0.f, float ax=0.f, float ay=0.f, float az=0.f );
    void    Animation ( CHARACTER* c );
    virtual void    IRender ( CHARACTER* c );
};


//  2D이미지를 연결한다.
class CSParts2D : public CSIPartsMDL
{
public :
    CSParts2D ( int Type, int SubType, int BoneNumber, float x=0.f, float y=0.f, float z=0.f );
    virtual void    IRender ( CHARACTER* c );
};

//////////////////////////////////////////////////////////////////////////
//  FUNCTION.
//////////////////////////////////////////////////////////////////////////
void    CreatePartsFactory ( CHARACTER* c );
void    RenderParts ( CHARACTER* c );
void    DeleteParts ( CHARACTER* c );


#endif// __CSPARTS_H__

