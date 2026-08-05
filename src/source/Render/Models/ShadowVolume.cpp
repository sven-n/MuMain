//////////////////////////////////////////////////////////////////////
// ShadowVolume.cpp: implementation of the CShadowVolume class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "ShadowVolume.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Render/Textures/ZzzTexture.h"
#include "Core/Utilities/BaseCls.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Core/ImmediateRenderer.h"
#include "Render/Shaders/PassthroughShader.h"
#include "Render/Core/RenderConfig.h"

CShadowVolume::CShadowVolume()
{
    Clear();
}

CShadowVolume::~CShadowVolume()
{
}

void CShadowVolume::Clear(void)
{
    m_pVertices = NULL;
    m_iNumEdge = 0;
    m_pEdges = NULL;
}

void CShadowVolume::AddEdgeFast(short nV1, short nV2, short nMesh, int iTriangle, int Edge, Triangle_t* pTriangles)
{
    Triangle_t* pTriangle = &pTriangles[iTriangle];
    short EdgeTriangleIndex = pTriangle->EdgeTriangleIndex[Edge];
    if (EdgeTriangleIndex == -1 || pTriangles[EdgeTriangleIndex].Front == false)
    {
        m_pEdges[m_iNumEdge].m_nVertexIndex[0] = nV1;
        m_pEdges[m_iNumEdge].m_nVertexIndex[1] = nV2;
        m_pEdges[m_iNumEdge].m_nMesh = nMesh;
        m_pEdges[m_iNumEdge].m_nNormalIndex[0] = pTriangle->NormalIndex[Edge];
        m_pEdges[m_iNumEdge].m_nNormalIndex[1] = pTriangle->NormalIndex[(Edge + 1) % 3];
        m_iNumEdge++;
    }
}

void CShadowVolume::DeterminateSilhouette(short nMesh, vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES], short nNumTriangles, Triangle_t* pTriangles, bool Tga)
{
    for (int iTriangle = 0; iTriangle < nNumTriangles; ++iTriangle)
    {
        Triangle_t* pTriangle = &pTriangles[iTriangle];
        short* pnVertexIndex = pTriangle->VertexIndex;

        vec3_t* pVertex[3];
        pVertex[0] = &ppVertexTransformed[nMesh][pnVertexIndex[0]];
        pVertex[1] = &ppVertexTransformed[nMesh][pnVertexIndex[1]];
        pVertex[2] = &ppVertexTransformed[nMesh][pnVertexIndex[2]];
        vec3_t Normal;
        FaceNormalize(*pVertex[0], *pVertex[1], *pVertex[2], Normal);
        if (DotProduct(Normal, m_vLight) <= 0.f)
            pTriangle->Front = true;
        else
            pTriangle->Front = false;
    }

    for (int iTriangle = 0; iTriangle < nNumTriangles; ++iTriangle)
    {
        Triangle_t* pTriangle = &pTriangles[iTriangle];

        if (pTriangle->Front)
        {
            short* pnVertexIndex = pTriangle->VertexIndex;
            AddEdgeFast(pnVertexIndex[0], pnVertexIndex[1], nMesh, iTriangle, 0, pTriangles);
            AddEdgeFast(pnVertexIndex[1], pnVertexIndex[2], nMesh, iTriangle, 1, pTriangles);
            AddEdgeFast(pnVertexIndex[2], pnVertexIndex[0], nMesh, iTriangle, 2, pTriangles);
        }
    }
}
