// SideHair.cpp: implementation of the CSideHair
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
#include "SideHair.h"
#include "ZzzCharacter.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSideHair::CSideHair()
{
    CShadowVolume();
}

CSideHair::~CSideHair()
{
}

void CSideHair::Create(vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES], BMD* b, OBJECT* o, bool SkipTga)
{
    VectorSubtract(Hero->Object.Position, CameraPosition, m_vLight);
    VectorNormalize(m_vLight);

    if (o->Alpha < 0.01f)
    {
        return;
    }
    short nHiddenMesh = o->HiddenMesh;
    short nBlendMesh = o->BlendMesh;
    if (nHiddenMesh == -2 || nBlendMesh == -2)
    {
        return;
    }

    int iNumTriangles = 0;
    for (int i = 1; i < 2; ++i)
    {
        if (nHiddenMesh == i || nBlendMesh == i)
        {
            continue;
        }
        if (Bitmaps[b->IndexTexture[i]].Components == 4)
        {
            if (SkipTga) continue;
        }
        iNumTriangles += b->Meshs[i].NumTriangles;
    }
    m_iNumEdge = 0;
    m_pEdges = new St_Edges[iNumTriangles * 3];

    for (short i = 1; i < 2; ++i)
    {
        if (nHiddenMesh == i || nBlendMesh == i)
        {
            continue;
        }

        bool Tga = false;
        if (Bitmaps[b->IndexTexture[i]].Components == 4)
        {
            Tga = true;
            if (SkipTga) continue;
        }
        DeterminateSilhouette(i, ppVertexTransformed, b->Meshs[i].NumTriangles, b->Meshs[i].Triangles, Tga);
    }
}

void CSideHair::Destroy(void)
{
    delete[] m_pEdges;
    delete[] m_pVertices;
}

void CSideHair::Render(vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES], vec3_t ppLightTransformed[MAX_MESH][MAX_VERTICES])
{
    for (int i = 0; i < m_iNumEdge; ++i)
    {
        RenderLine(ppVertexTransformed[m_pEdges[i].m_nMesh][m_pEdges[i].m_nVertexIndex[0]],
            ppVertexTransformed[m_pEdges[i].m_nMesh][m_pEdges[i].m_nVertexIndex[1]],
            ppLightTransformed[m_pEdges[i].m_nMesh][m_pEdges[i].m_nNormalIndex[0]],
            ppLightTransformed[m_pEdges[i].m_nMesh][m_pEdges[i].m_nNormalIndex[1]]);
    }
}

void CSideHair::RenderLine(vec3_t v1, vec3_t v2, vec3_t c1, vec3_t c2)
{
    vec3_t p1, p2, d;

    glColor3f(1.f, 1.f, 1.f);
    VectorSubtract(v2, v1, d);
    const float fLength = VectorLength(d);
    float fTextureMove = 0.0f;
    fTextureMove = (50.0f - fLength) * 0.5f / 50.0f;

    VectorCopy(v1, p1);
    VectorCopy(v2, p2);
    VectorSubtract(p2, p1, d);
    VectorScale(d, 0.1f, d);
    VectorSubtract(p1, d, p1);
    VectorAdd(p2, d, p2);

    float fTextureV = (float)(rand() % 100) * 0.01f;
    glColor3f(1.f, 1.f, 1.f);
    BindTexture(BITMAP_ROBE + 4);
    EnableAlphaBlendMinus();
    //EnableAlphaTest();
    //g_OpenglLib.DisableTexture();
    //g_OpenglLib.Disable(GL_CULL_FACE);
    /*glBegin(GL_QUADS);
    glTexCoord2f(0.f,0.f+fTextureMove);glVertex3f(p1[0]-Scale,p1[1],p1[2]);
    glTexCoord2f(0.f,1.f-fTextureMove);glVertex3f(p2[0]-Scale,p2[1],p2[2]);
    glTexCoord2f(1.f,1.f-fTextureMove);glVertex3f(p2[0]+Scale,p2[1],p2[2]);
    glTexCoord2f(1.f,0.f+fTextureMove);glVertex3f(p1[0]+Scale,p1[1],p1[2]);
    glEnd();
    glBegin(GL_QUADS);
    glTexCoord2f(0.f,0.f+fTextureMove);glVertex3f(p1[0],p1[1]-Scale,p1[2]);
    glTexCoord2f(0.f,1.f-fTextureMove);glVertex3f(p2[0],p2[1]-Scale,p2[2]);
    glTexCoord2f(1.f,1.f-fTextureMove);glVertex3f(p2[0],p2[1]+Scale,p2[2]);
    glTexCoord2f(1.f,0.f+fTextureMove);glVertex3f(p1[0],p1[1]+Scale,p1[2]);
    glEnd();
    glBegin(GL_QUADS);
    glTexCoord2f(0.f,0.f+fTextureMove);glVertex3f(p1[0],p1[1],p1[2]-Scale);
    glTexCoord2f(0.f,1.f-fTextureMove);glVertex3f(p2[0],p2[1],p2[2]-Scale);
    glTexCoord2f(1.f,1.f-fTextureMove);glVertex3f(p2[0],p2[1],p2[2]+Scale);
    glTexCoord2f(1.f,0.f+fTextureMove);glVertex3f(p1[0],p1[1],p1[2]+Scale);
    glEnd();*/
    vec3_t vOrtho;
    CrossProduct(m_vLight, d, vOrtho);
    VectorNormalize(vOrtho);
    VectorScale(vOrtho, 10.f, vOrtho);
    glBegin(GL_QUADS);
    //glColor3fv( c1);
    glTexCoord2f(0.f, 0.f + fTextureMove + fTextureV); glVertex3f(p1[0] - vOrtho[0], p1[1] - vOrtho[1], p1[2] - vOrtho[2]);
    //glColor3fv( c2);
    glTexCoord2f(0.f, 1.f - fTextureMove + fTextureV); glVertex3f(p2[0] - vOrtho[0], p2[1] - vOrtho[1], p2[2] - vOrtho[2]);
    glTexCoord2f(1.f, 1.f - fTextureMove + fTextureV); glVertex3f(p2[0] + vOrtho[0], p2[1] + vOrtho[1], p2[2] + vOrtho[2]);
    //glColor3fv( c1);
    glTexCoord2f(1.f, 0.f + fTextureMove + fTextureV); glVertex3f(p1[0] + vOrtho[0], p1[1] + vOrtho[1], p1[2] + vOrtho[2]);
    glEnd();
    //g_OpenglLib.Enable(GL_CULL_FACE);
}