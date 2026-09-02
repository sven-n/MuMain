// ShadowVolume.h: interface for the CShadowVolume class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHADOWVOLUME_H__8C7DEDBA_0557_4B98_AD53_900F41EAC8AD__INCLUDED_)
#define AFX_SHADOWVOLUME_H__8C7DEDBA_0557_4B98_AD53_900F41EAC8AD__INCLUDED_

#pragma once

#include "ZzzBMD.h"

typedef struct
{
    short m_nVertexIndex[2];
    short m_nMesh;
    short m_nNormalIndex[2];
} St_Edges;

class CShadowVolume
{
public:
    CShadowVolume();
    virtual ~CShadowVolume();

    void Clear(void);

    // a) ���� ������ ������ ���� ����
protected:
    short m_nNumVertices; // �� ����
    vec3_t* m_pVertices;  // ����
protected:
    BOOL GetReadyToCreate(vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES], BMD* b, OBJECT* o,
                          bool SkipTga = true); // ����
public:
    virtual void Create(vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES], BMD* b, OBJECT* o,
                        bool SkipTga = true); // ����
    virtual void Destroy(void);               // ����
    void RenderAsFrame(void);                 // ������ ������ frame ���� �׸���
    void Shade(void);                         // ���ۿ� �׸��� �׸���

    // b) �߰� ����
protected:
    vec3_t m_vLight;    // ��
    int m_iNumEdge;     // �����ڸ� ����
    St_Edges* m_pEdges; // �����ڸ�
    void DeterminateSilhouette(short nMesh, vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES], short nNumTriangles,
                               Triangle_t* pTriangles, bool Tga); // Mesh �� �����ڸ� ����
    void AddEdge(short nV1, short nV2, short nMesh);              // �����ڸ� �߰�
    void AddEdgeFast(short nV1, short nV2, short nMesh, int iTriangle, int Edge, Triangle_t* pTriangles); // �����ڸ� �߰�
    void GenerateSidePolygon(vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES]); // �����ڸ��� �̿��� ������ ����

    // c) ǥ��
protected:
    void RenderShadowVolume(
        void); // ������ ������ ������ ������� �׸���
};

void InsertShadowVolume(CShadowVolume* psv);
void RenderShadowVolumesAsFrame(void);
void ShadeWithShadowVolumes(void);
void RenderShadowToScreen(void);

// #endif //USE_SHADOWVOLUME

#endif // !defined(AFX_SHADOWVOLUME_H__8C7DEDBA_0557_4B98_AD53_900F41EAC8AD__INCLUDED_)
