//*****************************************************************************
// File: GM_kanturu_1st.h
//
// Desc: ĭ���� 1��(�ܺ�) ��, ����.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#ifndef _GM_KANTURU_1ST_H_
#define _GM_KANTURU_1ST_H_

namespace M37Kanturu1st
{
    bool IsKanturu1st();						// ĭ���� �ܺ� ���ΰ�?

    // ������Ʈ ����
    bool CreateKanturu1stObject(OBJECT* pObject);
    bool MoveKanturu1stObject(OBJECT* pObject);
    bool RenderKanturu1stObjectVisual(OBJECT* pObject, BMD* pModel);
    bool RenderKanturu1stObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);	// ������Ʈ ����(���� ����)
    void RenderKanturu1stAfterObjectMesh(OBJECT* o, BMD* b);				// ������ ������Ʈ ���� ����.

    // ���� ����
    CHARACTER* CreateKanturu1stMonster(int iType, int PosX, int PosY, int Key);	// ���� ����
    bool SetCurrentActionKanturu1stMonster(CHARACTER* c, OBJECT* o);		// ���� ���� �׼� ����
    bool AttackEffectKanturu1stMonster(CHARACTER* c, OBJECT* o, BMD* b);	// ���� ���� ����Ʈ
    bool MoveKanturu1stMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);		// ���� ȿ�� ������Ʈ
    void MoveKanturu1stBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);		// ���� ������ �ܻ� ó��
    bool RenderKanturu1stMonsterObjectMesh(OBJECT* o, BMD* b, int ExtraMon);	// ���� ������Ʈ ������
    bool RenderKanturu1stMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);	// ���� ȿ�� ������
};

#endif	// _GM_KANTURU_1ST_H_