// ShadowVolume.h: interface for the CShadowVolume class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SHADOWVOLUME_H__8C7DEDBA_0557_4B98_AD53_900F41EAC8AD__INCLUDED_)
#define AFX_SHADOWVOLUME_H__8C7DEDBA_0557_4B98_AD53_900F41EAC8AD__INCLUDED_

#pragma once

#include "ZzzBMD.h"

typedef struct
{
    short	m_nVertexIndex[2];
    short	m_nMesh;
    short	m_nNormalIndex[2];
} St_Edges;

// DXP-09 9.1: trimmed to only the edge-list/silhouette machinery CSideHair actually uses
// (CSideHair::Create/Destroy fully reimplement everything else). The stencil-shadow queue
// system (InsertShadowVolume/RenderShadowVolumesAsFrame/ShadeWithShadowVolumes/
// RenderShadowToScreen) and the base Create/Destroy/GetReadyToCreate/GenerateSidePolygon/
// RenderAsFrame/RenderShadowVolume/Shade path were dead (superseded by CPlanarShadowShader,
// zero callers) and have been deleted.
class CShadowVolume
{
public:
    CShadowVolume();
    virtual ~CShadowVolume();

    void Clear(void);

protected:
    vec3_t* m_pVertices;
protected:
    vec3_t m_vLight;
    int m_iNumEdge;		// number of silhouette edges
    St_Edges* m_pEdges;	// silhouette edges
    void DeterminateSilhouette(short nMesh, vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES], short nNumTriangles, Triangle_t* pTriangles, bool Tga);
    void AddEdgeFast(short nV1, short nV2, short nMesh, int iTriangle, int Edge, Triangle_t* pTriangles);
};

#endif // !defined(AFX_SHADOWVOLUME_H__8C7DEDBA_0557_4B98_AD53_900F41EAC8AD__INCLUDED_)
