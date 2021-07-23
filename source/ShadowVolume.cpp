// ShadowVolume.cpp: implementation of the CShadowVolume class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBmd.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ShadowVolume.h"
#include "ZzzLodTerrain.h"
#include "zzzTexture.h"
#include "BaseCls.h"


/*
- 남은 과제

1. Animation 이 없는 mesh 는 한번 계산한 것을 계속 쓴다.
2. 하드웨어가 스텐실 지원하는지 체크하는 함수
3. stencil 버퍼가 구현되지 않는 곳에서의 처리(소프트웨어로 구현)

4. 두개의 Mesh 가 공유하는 vertex 및 edge 에 대한 처리(해결)
5. Edge 따기 더 빠르게(해결)
6. Tga 더 빠르게 거르기(해결)
7. ShadowVolume 이 깨지는 것 처리(해결)

*/


//#ifdef USE_SHADOWVOLUME


// 임시
#include "ZzzCharacter.h"


CQueue<CShadowVolume*> m_qSV;

void InsertShadowVolume( CShadowVolume *psv)
{
	m_qSV.Insert( psv);
}

void RenderShadowVolumesAsFrame( void)
{
	glPolygonMode( GL_FRONT, GL_LINE);
	glDepthMask( true);
	DisableAlphaBlend();
	DisableTexture();
	vec3_t vLight = { 0.4f, 0.f, 0.f};
	glColor3fv( vLight);

	while ( m_qSV.GetCount() > 0)
	{
		CShadowVolume *psv = m_qSV.Remove();
		psv->RenderAsFrame();
		psv->Destroy();
		delete psv;
	}

	glPolygonMode( GL_FRONT, GL_FILL);
}

//스텐실버퍼에 쉐도우볼륨 그림
void ShadeWithShadowVolumes( void)
{
	DisableAlphaBlend();

	DisableDepthMask();
	glEnable( GL_STENCIL_TEST);

	// 1. Stencil 버퍼에 그리기
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glStencilFunc( GL_ALWAYS, 0xFFFFFFFF, 0xFFFFFFFF);

	while ( m_qSV.GetCount() > 0)
	{
		CShadowVolume *psv = m_qSV.Remove();
		psv->Shade();
		psv->Destroy();
		delete psv;
	}

	// 3. 복구
	glFrontFace( GL_CCW);
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable( GL_STENCIL_TEST);
	EnableDepthMask();
}

//최종적으로 스텐실버퍼를 화면에 쫙 그림
void RenderShadowToScreen( void)
{
	DisableDepthTest();
	DisableDepthMask();
	glEnable( GL_STENCIL_TEST);

	glStencilFunc( GL_LEQUAL, 0x1, 0xFFFFFFFF);
	glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP);

	glDepthFunc( GL_ALWAYS);

	EnableAlphaBlendMinus();
	DisableTexture();
	vec3_t vLight = { .7f,0.7f,0.5f};
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	float fAlpha = 1.0f;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

	// 임시로 바닥 뿌리기
	//RenderTerrainAlphaBitmap(BITMAP_HIDE, Hero->Object.Position[0],Hero->Object.Position[1],20.f,20.f,vLight,0.f,fAlpha);
	float p[4][2];
	float Width  = ( float)WindowWidth;
	float Height = ( float)WindowHeight;
	p[0][0] = 0.f      ;p[0][1] = 0.f;
	p[1][0] = 0.f      ;p[1][1] = Height;
	p[2][0] = 0.f+Width;p[2][1] = Height;
	p[3][0] = 0.f+Width;p[3][1] = 0.f;
	//BeginBitmap();
	glBegin(GL_TRIANGLE_FAN);
	glColor3fv(vLight);
	for(int i=0;i<4;i++)
	{
		glVertex2f(p[i][0],p[i][1]);
	}
	glEnd();
	//EndBitmap();

	// 3. 복구
	//EnableTexture();
	glDepthFunc( GL_LESS);
	glDisable( GL_STENCIL_TEST);
	EnableDepthMask();
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CShadowVolume::CShadowVolume()
{
	Clear();
}

CShadowVolume::~CShadowVolume()
{

}

void CShadowVolume::Clear( void)
{
	m_nNumVertices = 0;
	m_pVertices = NULL;
	m_iNumEdge = 0;
	m_pEdges = NULL;
}

BOOL CShadowVolume::GetReadyToCreate( vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES], BMD *b, OBJECT *o, bool SkipTga)
{
	if ( o->Alpha < 0.01f)
	{
		return ( FALSE);
	}
	short nHiddenMesh = o->HiddenMesh;
	short nBlendMesh = o->BlendMesh;
	if ( nHiddenMesh == -2 || nBlendMesh == -2)
	{
		return ( FALSE);
	}

	// 1. 테두리를 위한 공간 할당
	int iNumTriangles = 0;
	for ( int i = 0; i < b->NumMeshs; ++i)
	{
		if ( nHiddenMesh == i || nBlendMesh == i)
		{
			continue;
		}
		// Tga 그림은 통과 시킴
		if(Bitmaps[b->IndexTexture[i]].Components == 4)
		{
     		if(SkipTga) continue;
		}
		iNumTriangles += max( 0, b->Meshs[i].NumTriangles);
	}
	m_iNumEdge = 0;
	m_pEdges = new St_Edges[iNumTriangles*3];
	// 2. Mesh 별 테두리 따기
#ifdef _VS2008PORTING
	for ( int i = 0; i < b->NumMeshs; ++i)
#else // _VS2008PORTING
	for ( i = 0; i < b->NumMeshs; ++i)
#endif // _VS2008PORTING
	{
		if ( nHiddenMesh == i || nBlendMesh == i)
		{
			continue;
		}
		// Tga 그림은 통과 시킴
		bool Tga = false;
		if(Bitmaps[b->IndexTexture[i]].Components == 4)
		{
			Tga = true;
     		if(SkipTga) continue;
		}
		DeterminateSilhouette( i, ppVertexTransformed, b->Meshs[i].NumTriangles, b->Meshs[i].Triangles,Tga);
	}

	return ( TRUE);
}

void CShadowVolume::Create( vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES], BMD *b, OBJECT *o, bool SkipTga)
{
	// 0. 빛 방향 정하기
	m_vLight[0] = -1.f; m_vLight[1] = 0.03f;m_vLight[2] = -1.f;
	VectorNormalize( m_vLight);

	// 1-2. 테두리를 위한 공간 할당, Mesh 별 테두리 따기
	if ( !GetReadyToCreate( ppVertexTransformed, b, o, SkipTga))
	{
		return;
	}

	// 3. ShadowVolume 생성
	GenerateSidePolygon( ppVertexTransformed);
	delete [] m_pEdges;
}

void CShadowVolume::Destroy( void)
{
	if ( m_pVertices)
	{
		delete [] m_pVertices;
	}
}

void CShadowVolume::AddEdge( short nV1, short nV2, short nMesh)
{
	for ( int i = 0; i < m_iNumEdge; ++i)
	{	// 같은 것 찾기
		if ( m_pEdges[i].m_nMesh == nMesh)
		{	// 같은 메쉬면 인덱스
			if ( m_pEdges[i].m_nVertexIndex[0] == nV2 && m_pEdges[i].m_nVertexIndex[1] == nV1)
			{
				if ( m_iNumEdge > 1)
				{	// 제거
					m_pEdges[i].m_nVertexIndex[0] = m_pEdges[m_iNumEdge - 1].m_nVertexIndex[0];
					m_pEdges[i].m_nVertexIndex[1] = m_pEdges[m_iNumEdge - 1].m_nVertexIndex[1];
					m_pEdges[i].m_nMesh = m_pEdges[m_iNumEdge - 1].m_nMesh;
     				m_iNumEdge--;
				}
				return;
			}
		}
	}

	// 추가
	m_pEdges[m_iNumEdge].m_nVertexIndex[0] = nV1;
	m_pEdges[m_iNumEdge].m_nVertexIndex[1] = nV2;
	m_pEdges[m_iNumEdge].m_nMesh = nMesh;
	m_iNumEdge++;
}

void CShadowVolume::AddEdgeFast( short nV1, short nV2, short nMesh, int iTriangle, int Edge, Triangle_t *pTriangles)
{
	Triangle_t *pTriangle = &pTriangles[iTriangle];
	short EdgeTriangleIndex = pTriangle->EdgeTriangleIndex[Edge];
	if(EdgeTriangleIndex==-1 || pTriangles[EdgeTriangleIndex].Front==false)
	{
		// 추가
		m_pEdges[m_iNumEdge].m_nVertexIndex[0] = nV1;
		m_pEdges[m_iNumEdge].m_nVertexIndex[1] = nV2;
		m_pEdges[m_iNumEdge].m_nMesh = nMesh;
		m_pEdges[m_iNumEdge].m_nNormalIndex[0] = pTriangle->NormalIndex[Edge];
		m_pEdges[m_iNumEdge].m_nNormalIndex[1] = pTriangle->NormalIndex[(Edge+1)%3];
		m_iNumEdge++;
	}
}

//쉐도우볼륨 외곽 따내기
void CShadowVolume::DeterminateSilhouette( short nMesh, vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES], short nNumTriangles, Triangle_t *pTriangles, bool Tga)
{
	for ( int iTriangle = 0; iTriangle < nNumTriangles; ++iTriangle)
	{
		Triangle_t *pTriangle = &pTriangles[iTriangle];
		short *pnVertexIndex = pTriangle->VertexIndex;

		vec3_t *pVertex[3];
		pVertex[0] = &ppVertexTransformed[nMesh][pnVertexIndex[0]];
		pVertex[1] = &ppVertexTransformed[nMesh][pnVertexIndex[1]];
		pVertex[2] = &ppVertexTransformed[nMesh][pnVertexIndex[2]];
		vec3_t Normal;
		FaceNormalize( *pVertex[0], *pVertex[1], *pVertex[2], Normal);
		if (DotProduct( Normal, m_vLight) <= 0.f)//앞면
			pTriangle->Front = true;
		else
			pTriangle->Front = false;
	}
#ifdef _VS2008PORTING
	for (int iTriangle = 0; iTriangle < nNumTriangles; ++iTriangle)
#else // _VS2008PORTING
	for (iTriangle = 0; iTriangle < nNumTriangles; ++iTriangle)
#endif // _VS2008PORTING
	{
		Triangle_t *pTriangle = &pTriangles[iTriangle];

		if(pTriangle->Front)
		{
        	short *pnVertexIndex = pTriangle->VertexIndex;
			//AddEdge( pnVertexIndex[0], pnVertexIndex[1], nMesh);
			//AddEdge( pnVertexIndex[1], pnVertexIndex[2], nMesh);
			//AddEdge( pnVertexIndex[2], pnVertexIndex[0], nMesh);
			AddEdgeFast( pnVertexIndex[0], pnVertexIndex[1], nMesh, iTriangle, 0, pTriangles);
			AddEdgeFast( pnVertexIndex[1], pnVertexIndex[2], nMesh, iTriangle, 1, pTriangles);
			AddEdgeFast( pnVertexIndex[2], pnVertexIndex[0], nMesh, iTriangle, 2, pTriangles);
		}
	}
}

#define GROUND_HEIGHT 22.5f

void CShadowVolume::GenerateSidePolygon( vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES])
{
	m_nNumVertices = 0;
	m_pVertices = new vec3_t[m_iNumEdge * 6];

	// 옆면 만들기
	vec3_t Vertex[4];
	for ( int i = 0; i < m_iNumEdge; ++i)
	{
		VectorCopy(ppVertexTransformed[m_pEdges[i].m_nMesh][m_pEdges[i].m_nVertexIndex[0]], Vertex[0]);
		VectorCopy(ppVertexTransformed[m_pEdges[i].m_nMesh][m_pEdges[i].m_nVertexIndex[1]], Vertex[1]);
		//float fLength = ( max( GROUND_HEIGHT, max( Vertex[0][2], Vertex[1][2])) / -m_vLight[2]);
		float fLength = ( max( GROUND_HEIGHT, Vertex[0][2]) / -m_vLight[2]);
		VectorMA( Vertex[0], fLength, m_vLight, Vertex[2]);
		fLength = ( max( GROUND_HEIGHT, Vertex[1][2]) / -m_vLight[2]);
		VectorMA( Vertex[1], fLength, m_vLight, Vertex[3]);

		// 추가
		VectorCopy( Vertex[0], m_pVertices[m_nNumVertices]);m_nNumVertices++;
		VectorCopy( Vertex[2], m_pVertices[m_nNumVertices]);m_nNumVertices++;
		VectorCopy( Vertex[1], m_pVertices[m_nNumVertices]);m_nNumVertices++;
		VectorCopy( Vertex[1], m_pVertices[m_nNumVertices]);m_nNumVertices++;
		VectorCopy( Vertex[2], m_pVertices[m_nNumVertices]);m_nNumVertices++;
		VectorCopy( Vertex[3], m_pVertices[m_nNumVertices]);m_nNumVertices++;
	}
}

void CShadowVolume::RenderAsFrame( void)
{
	RenderShadowVolume();
}

void CShadowVolume::RenderShadowVolume( void)
{
	glBegin(GL_TRIANGLES);

	for ( int i = 0; i < m_nNumVertices; ++i)
	{
		glVertex3fv(m_pVertices[i]);
	}

	glEnd();
}

void CShadowVolume::Shade( void)
{
	// 1. 앞면
	glFrontFace( GL_CCW);
	glStencilOp( GL_KEEP, GL_KEEP, GL_INCR);
	RenderShadowVolume();

	// 2. 뒷면
	glFrontFace( GL_CW);
	glStencilOp( GL_KEEP, GL_KEEP, GL_DECR);
	RenderShadowVolume();
}


//#endif //#ifdef USE_SHADOWVOLUME
