///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <ctype.h>
#include "ZzzBMD.h"
#include "SMD.h"
//#include "Triangle2Strip.h"

struct
{
	float  m[3][4];
	float  im[3][4];
	vec3_t WorldOrg;
} BoneFixup[NODE_MAX];

void FixupSMD()
{
	Skeleton_t  *s = &NodeGroup.Skeleton;
	NodeGroup_t *ng = &NodeGroup;
	for(int i=0;i<ng->NodeNum;i++)
	{
       	Node_t *n = &ng->Node[i];

		vec3_t Angle;
		// convert to degrees
		Angle[0] = s->Rotation[i][0] * (180.f / Q_PI);
		Angle[1] = s->Rotation[i][1] * (180.f / Q_PI);
		Angle[2] = s->Rotation[i][2] * (180.f / Q_PI);

		if (n->Parent == -1) 
		{
			// scale the done pos.
			// calc rotational matrices
			AngleMatrix(Angle,BoneFixup[i].m);
			AngleIMatrix(Angle,BoneFixup[i].im);
			VectorCopy(s->Position[i],BoneFixup[i].WorldOrg);
		}
		else 
		{
			float m[3][4];
			// calc compound rotational matrices
			// FIXME : Hey, it's orthogical so inv(A) == transpose(A)
			AngleMatrix(Angle,m);
			R_ConcatTransforms(BoneFixup[n->Parent].m,m,BoneFixup[i].m);
			AngleIMatrix(Angle,m);
			R_ConcatTransforms(m,BoneFixup[n->Parent].im,BoneFixup[i].im);

			vec3_t p;
			// calc true world coord.
			VectorTransform(s->Position[i],BoneFixup[n->Parent].m,p);
			VectorAdd(p,BoneFixup[n->Parent].WorldOrg,BoneFixup[i].WorldOrg);
		}
	}

    TriangleGroup_t *tg = &TriangleGroup;

	for(int i=0;i<tg->TriangleNum;i++)
	{
		for(int j=0;j<3;j++)
		{
			SMDVertex_t *v = &tg->Vertex[i][j];
			vec3_t p;
			// move vertex position to object space.
			VectorSubtract(v->Position,BoneFixup[v->Node].WorldOrg,p);
			VectorTransform(p,BoneFixup[v->Node].im,v->Position);

			// move normal to object space.
			VectorCopy(v->Normal,p);
			VectorTransform(p,BoneFixup[v->Node].im,v->Normal);
			VectorNormalize(v->Normal);
		}
	}

	SMDMeshGroup_t *mg = &MeshGroup;
	mg->MeshNum = 0;

	for(int i=0;i<MESH_MAX;i++)
	{
      	SMDMesh_t *m = &mg->Mesh[i];
		m->VertexNum = 0;
		m->NormalNum = 0;
		m->TexCoordNum = 0;
		m->TriangleNum = 0;
	}

	for(int i=0;i<tg->TriangleNum;i++)
	{
		int MeshNum = 0;
		for(int k=0;k<mg->MeshNum;k++)
		{
			if(strcmp(tg->TextureName[i],mg->Texture[k].FileName)==NULL)
			{
				MeshNum = k;
				break;
			}
		}

		if( MeshNum == 0 )
		{
			MeshNum = mg->MeshNum;
			mg->Mesh[MeshNum].Texture = mg->MeshNum;
			strcpy(mg->Texture[MeshNum].FileName,tg->TextureName[i]);
			mg->MeshNum++;
		}

		SMDMesh_t *m = &mg->Mesh[MeshNum];

		for(int j=0;j<3;j++)
		{
     		SMDVertex_t *v = &tg->Vertex[i][j];
			int k = m->VertexNum-1;
			if(k >= 0)
			{
				for(k; k>=0; k--)
				{
					Vertex_t *v2 = &m->Vertex[k];
					if(//v->Node == v2->Node && 
						v->Position[0] == v2->Position[0] && 
						v->Position[1] == v2->Position[1] && 
						v->Position[2] == v2->Position[2])
					{
						m->VertexList[m->TriangleNum][j] = k;
						break;
					}
				}
			}
			if(k == -1)
			{
				m->VertexList[m->TriangleNum][j] = m->VertexNum;
				m->Vertex[m->VertexNum].Node     = tg->Vertex[i][j].Node;
				m->Vertex[m->VertexNum].Position[0] = tg->Vertex[i][j].Position[0];
				m->Vertex[m->VertexNum].Position[1] = tg->Vertex[i][j].Position[1];
				m->Vertex[m->VertexNum].Position[2] = tg->Vertex[i][j].Position[2];
				m->VertexNum++;
			}
			k = m->NormalNum-1;
			if(k >= 0)
			{
				for(k; k>=0; k--)
				{
					Normal_t *n = &m->Normal[k];
					if(//v->Node == n->Node && 
						v->Normal[0] == n->Normal[0] && 
						v->Normal[1] == n->Normal[1] && 
						v->Normal[2] == n->Normal[2])
					{
						m->NormalList[m->TriangleNum][j] = k;
						break;
					}
				}
			}
			if(k == -1)
			{
				m->NormalList[m->TriangleNum][j]   = m->NormalNum;
				m->Normal[m->NormalNum].Node       = tg->Vertex[i][j].Node;
				m->Normal[m->NormalNum].Normal[0]  = tg->Vertex[i][j].Normal[0];
				m->Normal[m->NormalNum].Normal[1]  = tg->Vertex[i][j].Normal[1];
				m->Normal[m->NormalNum].Normal[2]  = tg->Vertex[i][j].Normal[2];
				m->Normal[m->NormalNum].BindVertex = m->VertexList[m->TriangleNum][j];
				m->NormalNum++;
			}
			k = m->TexCoordNum-1;
			if(k >= 0)
			{
				for(k; k>=0; k--)
				{
					TexCoord_t *t = &m->TexCoord[k];
					if(v->TexCoordU == t->TexCoordU && 
						v->TexCoordV == t->TexCoordV)
					{
						m->TexCoordList[m->TriangleNum][j] = k;
						break;
					}
				}
			}
			if(k == -1)
			{
				m->TexCoordList[m->TriangleNum][j] = m->TexCoordNum;
				m->TexCoord[m->TexCoordNum].TexCoordU = tg->Vertex[i][j].TexCoordU;
				m->TexCoord[m->TexCoordNum].TexCoordV = tg->Vertex[i][j].TexCoordV;
				m->TexCoordNum++;
			}
		}

		/*if(m->TriangleNum > 0)
		{
			int Count = 0;
			bool Test1[3] = {false,false,false};
			bool Test2[3] = {false,false,false};
			for(k=0;k<3;k++)
			{
				for(int l=0;l<3;l++)
				{
					if(m->VertexList[m->TriangleNum-1][k] == m->VertexList[m->TriangleNum][l])
					{
						Test1[k] = true;
						Test2[l] = true;
						Count ++;
						break;
					}
				}
			}
			if(m->NormalList[m->TriangleNum-1][0] == m->NormalList[m->TriangleNum][0] &&
			   m->NormalList[m->TriangleNum-1][1] == m->NormalList[m->TriangleNum][1] &&
			   m->NormalList[m->TriangleNum-1][2] == m->NormalList[m->TriangleNum][2])
			   Count ++;
			if(Count >= 3)
			{
				int New;
				for(k=0;k<3;k++)
				{
					if(Test2[k] == false) New = k;
				}
				if(Test1[2] == false)
				{
					m->Polygon[m->TriangleNum] = 3;
					m->TriangleNum++;
					continue;
					for(k=3;k>=2;k--)
					{
						m->VertexList  [m->TriangleNum-1][k] = m->VertexList  [m->TriangleNum-1][k-1];
						m->NormalList  [m->TriangleNum-1][k] = m->NormalList  [m->TriangleNum-1][k-1];
						m->TexCoordList[m->TriangleNum-1][k] = m->TexCoordList[m->TriangleNum-1][k-1];
					}
					m->VertexList  [m->TriangleNum-1][1] = m->VertexList  [m->TriangleNum][New];
					m->NormalList  [m->TriangleNum-1][1] = m->NormalList  [m->TriangleNum][New];
					m->TexCoordList[m->TriangleNum-1][1] = m->TexCoordList[m->TriangleNum][New];
				}
				if(Test1[0] == false)
				{
					for(k=3;k>=3;k--)
					{
						m->VertexList  [m->TriangleNum-1][k] = m->VertexList  [m->TriangleNum-1][k-1];
						m->NormalList  [m->TriangleNum-1][k] = m->NormalList  [m->TriangleNum-1][k-1];
						m->TexCoordList[m->TriangleNum-1][k] = m->TexCoordList[m->TriangleNum-1][k-1];
					}
					m->VertexList  [m->TriangleNum-1][2] = m->VertexList  [m->TriangleNum][New];
					m->NormalList  [m->TriangleNum-1][2] = m->NormalList  [m->TriangleNum][New];
					m->TexCoordList[m->TriangleNum-1][2] = m->TexCoordList[m->TriangleNum][New];
				}
				if(Test1[1] == false)
				{
					m->VertexList  [m->TriangleNum-1][3] = m->VertexList  [m->TriangleNum][New];
					m->NormalList  [m->TriangleNum-1][3] = m->NormalList  [m->TriangleNum][New];
					m->TexCoordList[m->TriangleNum-1][3] = m->TexCoordList[m->TriangleNum][New];
				}
				m->Polygon[m->TriangleNum-1] = 4;
				continue;
			}
		}*/
		m->Polygon[m->TriangleNum] = 3;
		m->TriangleNum++;
	}
}

//s_mesh_t      mesh;
//int           NumCommandBytes[MESH_MAX];
//unsigned char *CommandData[MESH_MAX];

void Triangle2Strip()
{
	/*SMDMeshGroup_t *mg = &MeshGroup;
	for(int i=0;i<mg->MeshNum;i++)
	{
     	SMDMesh_t *m = &mg->Mesh[i];
		mesh.numtris = m->TriangleNum;
		mesh.triangle = new s_trianglevert_t [mesh.numtris][3];
		for(int j=0;j<mesh.numtris;j++)
		{
			for(int k=0;k<3;k++)
			{
				mesh.triangle[j][k].vertindex = m->VertexList[j][k];
				mesh.triangle[j][k].normindex = m->NormalList[j][k];
				mesh.triangle[j][k].texindex  = m->TexCoordList[j][k];
			}
		}
		unsigned char *pCmdSrc;
		NumCommandBytes[i] = BuildTris(mesh.triangle,&mesh,&pCmdSrc);
		CommandData[i] = new unsigned char [NumCommandBytes[i]];
		memcpy(CommandData[i],pCmdSrc,NumCommandBytes[i]);

		delete []mesh.triangle;
	}*/
}

void SMD2BMDModel(int ID,int Actions)
{
	int i,j;
	BMD *bmd = &Models[ID];

	bmd->NumBones         = NodeGroup.NodeNum;
	bmd->NumMeshs         = MeshGroup.MeshNum;
	bmd->Bones            = new Bone_t    [bmd->NumBones];
	bmd->Meshs            = new Mesh_t    [bmd->NumMeshs];
	bmd->Textures         = new Texture_t [bmd->NumMeshs];
	bmd->IndexTexture     = new GLuint    [bmd->NumMeshs];
	bmd->NumLightMaps = 0;
	for(i=0;i<bmd->NumMeshs;i++)
	{
		SMDMesh_t *sm = &MeshGroup.Mesh[i];
		Mesh_t    *m  = &bmd->Meshs[i];

        m->NoneBlendMesh = false;
		m->Texture = sm->Texture;

		m->NumVertices = sm->VertexNum;
		m->Vertices    = new Vertex_t [m->NumVertices];
		memcpy(m->Vertices,sm->Vertex,sizeof(Vertex_t)*m->NumVertices);
		
		m->NumNormals = sm->NormalNum;
		m->Normals    = new Normal_t [m->NumNormals];
		memcpy(m->Normals,sm->Normal,sizeof(Normal_t)*m->NumNormals);
		
		m->NumTexCoords = sm->TexCoordNum;
		m->TexCoords    = new TexCoord_t [m->NumTexCoords];
		memcpy(m->TexCoords,sm->TexCoord,sizeof(TexCoord_t)*m->NumTexCoords);
        memcpy(bmd->Textures[i].FileName,MeshGroup.Texture[i].FileName,32);

        TextureScriptParsing TSParsing;
                                                         
        if( TSParsing.parsingTScript(bmd->Textures[i].FileName) )
        {
            m->m_csTScript = new TextureScript;
            m->m_csTScript->setScript( (TextureScript&)TSParsing );
        }
        else
        {
            m->m_csTScript = NULL;
        }

		// ver 1.0 (triangle)
		m->NumTriangles = sm->TriangleNum;
		m->Triangles = new Triangle_t [m->NumTriangles];
		for(j=0;j<m->NumTriangles;j++)
		{
			Triangle_t *tp = &m->Triangles[j];
			tp->Polygon = sm->Polygon[j];
			for(int k=0;k<tp->Polygon;k++)
			{
				tp->VertexIndex  [k] = sm->VertexList  [j][k];
				tp->NormalIndex  [k] = sm->NormalList  [j][k];
				tp->TexCoordIndex[k] = sm->TexCoordList[j][k];
			}
		}
		// ver 1.1 (strip)
		//m->NumCommandBytes = NumCommandBytes[i];
		//m->Commands = new unsigned char [m->NumCommandBytes];
		//memcpy(m->Commands,CommandData[i],NumCommandBytes[i]);
        //delete CommandData[i];

     	// ver 1.2
		//bmd->NumLightMaps += m->NumTriangles;
	}

	//lightmap
    //bmd->LightMaps = new Bitmap_t [bmd->NumLightMaps];
	//for(i=0;i<bmd->NumLightMaps;i++)
	//{
    //    bmd->LightMaps[i].Buffer = NULL;
	//}
    bmd->Actions = new Action_t [Actions];
	for(i=0;i<bmd->NumBones;i++)
	{
       	Node_t *n = &NodeGroup.Node[i];
		Bone_t *b = &bmd->Bones[i];

		strcpy(b->Name, n->Name);
		b->Parent   = n->Parent;

		bmd->Bones[i].BoneMatrixes = new BoneMatrix_t [Actions];
	}

//	bmd->NumActions = Actions;		//. used : 2003/10/1 soyaviper
	bmd->Init(true);
	bmd->NumActions = 0;
}

void SMD2BMDAnimation(int ID,bool LockPosition)
{
	int i,j;
	BMD *bmd = &Models[ID];

	for(i=0;i<bmd->NumBones;i++)
	{
		Bone_t *b = &bmd->Bones[i];

		//if(!b->Dummy)
		{
			BoneMatrix_t *bm = &b->BoneMatrixes[bmd->NumActions];
			bm->Position   = new vec3_t [SkeletonGroup.TimeNum];
			bm->Rotation   = new vec3_t [SkeletonGroup.TimeNum];
			bm->Quaternion = new vec4_t [SkeletonGroup.TimeNum];
			for(j=0;j<SkeletonGroup.TimeNum;j++)
			{
				VectorCopy(SkeletonGroup.Skeleton[j].Position[i],bm->Position[j]);
				VectorCopy(SkeletonGroup.Skeleton[j].Rotation[i],bm->Rotation[j]);
      			AngleQuaternion(bm->Rotation[j],bm->Quaternion[j]);
			}
		}
	}

	//action
	Action_t *a = &bmd->Actions[bmd->NumActions];
	a->Loop             = false;
	a->LockPositions    = LockPosition;
	a->NumAnimationKeys = SkeletonGroup.TimeNum;
	a->PlaySpeed        = 0.3f;

	a->Positions = new vec3_t [a->NumAnimationKeys];
	Bone_t *b = &bmd->Bones[0];
	for(i=0;i<a->NumAnimationKeys;i++)
	{
		BoneMatrix_t *bm = &b->BoneMatrixes[bmd->NumActions];
		j = i+1;
		if(j > a->NumAnimationKeys-1) j = a->NumAnimationKeys-1;
		VectorSubtract(bm->Position[j],bm->Position[i],a->Positions[i]);
	}

	bmd->NumActions++;
}