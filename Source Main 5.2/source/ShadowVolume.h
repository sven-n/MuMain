// ShadowVolume.h: interface for the CShadowVolume class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHADOWVOLUME_H__8C7DEDBA_0557_4B98_AD53_900F41EAC8AD__INCLUDED_)
#define AFX_SHADOWVOLUME_H__8C7DEDBA_0557_4B98_AD53_900F41EAC8AD__INCLUDED_

#pragma once

#include "ZzzBmd.h"

typedef struct
{
    short	m_nVertexIndex[2];
    short	m_nMesh;
    short	m_nNormalIndex[2];
} St_Edges;

class CShadowVolume
{
public:
    CShadowVolume();
    virtual ~CShadowVolume();

    void Clear(void);

    // a) 최종 생성된 섀도우 볼륨 정보
protected:
    short	m_nNumVertices;	// 점 개수
    vec3_t* m_pVertices;	// 점들
protected:
    BOOL GetReadyToCreate(vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES], BMD* b, OBJECT* o, bool SkipTga = true);	// 생성
public:
    virtual void Create(vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES], BMD* b, OBJECT* o, bool SkipTga = true);	// 생성
    virtual void Destroy(void);	// 제거
    void RenderAsFrame(void);	// 섀도우 볼륨을 frame 으로 그리기
    void Shade(void);	// 버퍼에 그림자 그리기

    // b) 중간 과정
protected:
    vec3_t m_vLight;	// 빛
    int m_iNumEdge;		// 가장자리 개수
    St_Edges* m_pEdges;	// 가장자리
    void DeterminateSilhouette(short nMesh, vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES], short nNumTriangles, Triangle_t* pTriangles, bool Tga);	// Mesh 별 가장자리 따기
    void AddEdge(short nV1, short nV2, short nMesh);	// 가장자리 추가
    void AddEdgeFast(short nV1, short nV2, short nMesh, int iTriangle, int Edge, Triangle_t* pTriangles);	// 가장자리 추가
    void GenerateSidePolygon(vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES]);	// 가장자리를 이용한 폴리곤 생성

    // c) 표현
protected:
    void RenderShadowVolume(void);	// 섀도우 볼륨을 지정된 방식으로 그리기
};

void InsertShadowVolume(CShadowVolume* psv);
void RenderShadowVolumesAsFrame(void);
void ShadeWithShadowVolumes(void);
void RenderShadowToScreen(void);

//#endif //USE_SHADOWVOLUME

#endif // !defined(AFX_SHADOWVOLUME_H__8C7DEDBA_0557_4B98_AD53_900F41EAC8AD__INCLUDED_)
