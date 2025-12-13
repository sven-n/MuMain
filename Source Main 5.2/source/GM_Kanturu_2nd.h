// GM_Kanturu_In.h: interface for the GM_Kanturu_In class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GM_KANTURU_IN_H__37F1E344_37B1_497A_9B04_84409C768A74__INCLUDED_)
#define AFX_GM_KANTURU_IN_H__37F1E344_37B1_497A_9B04_84409C768A74__INCLUDED_

#pragma once

#include "ZzzBMD.h"

// TODO
namespace M38Kanturu2nd
{
    // ĭ���� ������Ʈ ����
    bool Create_Kanturu2nd_Object(OBJECT* o);										// ������Ʈ ����
    bool Move_Kanturu2nd_Object(OBJECT* o);											// ������Ʈ ������Ʈ
    bool Render_Kanturu2nd_ObjectVisual(OBJECT* o, BMD* b);							// ������Ʈ ����Ʈ ȿ��
    bool Render_Kanturu2nd_ObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);			// ������Ʈ ������(���� ����)
    void Render_Kanturu2nd_AfterObjectMesh(OBJECT* o, BMD* b);

    // ĭ���系�� ���� ����
    CHARACTER* Create_Kanturu2nd_Monster(int iType, int PosX, int PosY, int Key);	// ���� ���� �Լ�
    bool	Set_CurrentAction_Kanturu2nd_Monster(CHARACTER* c, OBJECT* o);		// ���� ���� �׼� ����
    bool	AttackEffect_Kanturu2nd_Monster(CHARACTER* c, OBJECT* o, BMD* b);	// ���� ���� ����Ʈ
    bool	Move_Kanturu2nd_MonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);		// ���� ȿ�� ������Ʈ
    void	Move_Kanturu2nd_BlurEffect(CHARACTER* c, OBJECT* o, BMD* b);		// ���� ������ �ܻ� ó��
    bool	Render_Kanturu2nd_MonsterObjectMesh(OBJECT* o, BMD* b, int ExtraMon);	// ���� ������Ʈ ������
    bool	Render_Kanturu2nd_MonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);	// ���� ȿ�� ������

    // ĭ���系�� �� ����
    bool		Is_Kanturu2nd();						// ĭ���系�� ���ΰ�?
    bool		Is_Kanturu2nd_3rd();					// ĭ���系�ο� 3�� ���ΰ�?

    // ����
    void	Sound_Kanturu2nd_Object(OBJECT* o);		// ������Ʈ ����
    void	PlayBGM();
};

class CTrapCanon
{
public:
    CTrapCanon();
    ~CTrapCanon();

private:
    void Initialize();
    void Destroy();

public:
    void Open_TrapCanon();
    CHARACTER* Create_TrapCanon(int iPosX, int iPosY, int iKey);
    void Render_Object(OBJECT* o, BMD* b);
    void Render_Object_Visual(CHARACTER* c, OBJECT* o, BMD* b);
    void Render_AttackEffect(CHARACTER* c, OBJECT* o, BMD* b);
};

extern CTrapCanon g_TrapCanon;

#endif // !defined(AFX_GM_KANTURU_IN_H__37F1E344_37B1_497A_9B04_84409C768A74__INCLUDED_)
