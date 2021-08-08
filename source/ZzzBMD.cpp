///////////////////////////////////////////////////////////////////////////////
// BMD관련 함수
// Open, Save, Animation, Rendering 등등
// 뮤의 케릭터, 배경 오브젝트는 모두 BMD포맷으로 구성되어 있음
//
// *** 함수 레벨: 2
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "zzzlodterrain.h"
#include "ZzzTexture.h"
#include "ZzzAI.h"
#include "SMD.h"
#include "zzzEffect.h"
#include "MapManager.h"
#include "./Utilities/Log/muConsoleDebug.h"
//#include "FillPolygon.h"


#include "GMBattleCastle.h"

#include "UIMng.h"

#ifdef MR0
#include "EngineGate.h"
#endif //MR0

#include "CameraMove.h"

//BMD Models[MAX_MODELS];
BMD *Models;
BMD *ModelsDump;

vec4_t BoneQuaternion[MAX_BONES];
short  BoundingVertices[MAX_BONES];
vec3_t BoundingMin[MAX_BONES];
vec3_t BoundingMax[MAX_BONES];

float  BoneTransform[MAX_BONES][3][4];

vec3_t VertexTransform[MAX_MESH][MAX_VERTICES];
vec3_t NormalTransform[MAX_MESH][MAX_VERTICES];
float  IntensityTransform[MAX_MESH][MAX_VERTICES];
vec3_t LightTransform[MAX_MESH][MAX_VERTICES];
//float  ShadowCoordU[MAX_MESH][MAX_VERTICES];
//float  ShadowCoordV[MAX_MESH][MAX_VERTICES];


///////////////////////////////////////////////////////////////////////////////6244-6555

unsigned char ShadowBuffer[256*256];
int           ShadowBufferWidth  = 256;
int           ShadowBufferHeight = 256;

extern int  MouseX;
extern int  MouseY;
extern bool MouseLButton;

///////////////////////////////////////////////////////////////////////////////
// animation
///////////////////////////////////////////////////////////////////////////////

bool  StopMotion = false;
float ParentMatrix[3][4];

//////////////////////////////////////////////////////////////////////////
//  Light Vector
//////////////////////////////////////////////////////////////////////////
static vec3_t LightVector = { 0.f, -0.1f, -0.8f };
static vec3_t LightVector2 = { 0.f, -0.5f, -0.8f };

#ifdef MR0
void BMD::Animation(float (*BoneMatrix)[3][4],float AnimationFrame,float PriorFrame,unsigned char PriorAction,vec3_t Angle,vec3_t HeadAngle,bool Parent,bool Translate)
{
	if ( NumActions<=0 ) return;

	if(PriorAction >= NumActions) PriorAction = 0;
	if(CurrentAction >= NumActions)CurrentAction = 0;
	VectorCopy(Angle,BodyAngle);

 	CurrentAnimation = AnimationFrame;
	CurrentAnimationFrame = (int)AnimationFrame;
	float s1 = (CurrentAnimation - CurrentAnimationFrame);
	float s2 = 1.f-s1;
	int PriorAnimationFrame = (int)PriorFrame;
	if(NumActions > 0)
	{
		if(PriorAnimationFrame < 0 )
			PriorAnimationFrame = 0;
		if(CurrentAnimationFrame < 0 )
			CurrentAnimationFrame = 0;
		if(PriorAnimationFrame >= Actions[PriorAction].NumAnimationKeys)
			PriorAnimationFrame = 0;
		if(CurrentAnimationFrame >= Actions[CurrentAction].NumAnimationKeys)
			CurrentAnimationFrame = 0;
	}

	// bones
	for(int i=0;i<NumBones;i++)
	{
       	Bone_t *b = &Bones[i];
		if(b->Dummy)
		{
			continue;
		}
		BoneMatrix_t *bm1 = &b->BoneMatrixes[PriorAction  ];
		BoneMatrix_t *bm2 = &b->BoneMatrixes[CurrentAction];
		vec4_t q1,q2;

		if ( i==BoneHead )
		{
			vec3_t Angle1,Angle2;
			VectorCopy(bm1->Rotation[PriorAnimationFrame  ],Angle1);
			VectorCopy(bm2->Rotation[CurrentAnimationFrame],Angle2);

			float HeadAngleX = HeadAngle[0] / (180.f / Q_PI);
			float HeadAngleY = HeadAngle[1] / (180.f / Q_PI);
			Angle1[0] -= HeadAngleX;
			Angle2[0] -= HeadAngleX;
			Angle1[2] -= HeadAngleY;
			Angle2[2] -= HeadAngleY;
			AngleQuaternion( Angle1, q1 );
			AngleQuaternion( Angle2, q2 );
		}
		else
		{
			QuaternionCopy(bm1->Quaternion[PriorAnimationFrame  ],q1);
			QuaternionCopy(bm2->Quaternion[CurrentAnimationFrame],q2);
		}
		if (!QuaternionCompare( q1, q2 ))
		{
			QuaternionSlerp(q1 , q2, s1, BoneQuaternion[i] );
		}
		else
		{
			QuaternionCopy( q1, BoneQuaternion[i] );
		}

		float Matrix[3][4];
		QuaternionMatrix(BoneQuaternion[i],Matrix);
		float *Position1 = bm1->Position[PriorAnimationFrame];
		float *Position2 = bm2->Position[CurrentAnimationFrame];
		if(i==0 && (Actions[PriorAction].LockPositions || Actions[CurrentAction].LockPositions))
		{
			Matrix[0][3] = bm2->Position[0][0];
			Matrix[1][3] = bm2->Position[0][1];
			Matrix[2][3] = Position1[2]*s2+Position2[2]*s1+BodyHeight;
		}
		else
		{
			Matrix[0][3] = Position1[0]*s2+Position2[0]*s1;
			Matrix[1][3] = Position1[1]*s2+Position2[1]*s1;
			Matrix[2][3] = Position1[2]*s2+Position2[2]*s1;
		}

		if(b->Parent == -1)
		{
			if(!Parent)
			{
				AngleMatrix(BodyAngle,ParentMatrix);
				if(Translate)
				{
					for ( int y = 0; y < 3; ++y)
					{
						for ( int x = 0; x < 3; ++x)
						{
							ParentMatrix[y][x] *= BodyScale;
						}
					}

					ParentMatrix[0][3] = BodyOrigin[0];
					ParentMatrix[1][3] = BodyOrigin[1];
					ParentMatrix[2][3] = BodyOrigin[2];
				}
			}
			R_ConcatTransforms(ParentMatrix,Matrix,BoneMatrix[i]);
		} 
		else 
		{
			R_ConcatTransforms(BoneMatrix[b->Parent],Matrix,BoneMatrix[i]);
		}
	}
}
#else
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
void BMD::Animation(float (*BoneMatrix)[3][4],float AnimationFrame,float PriorFrame,unsigned short PriorAction,vec3_t Angle,vec3_t HeadAngle,bool Parent,bool Translate)
#else //PBG_ADD_NEWCHAR_MONK_ANI
void BMD::Animation(float (*BoneMatrix)[3][4],float AnimationFrame,float PriorFrame,unsigned char PriorAction,vec3_t Angle,vec3_t HeadAngle,bool Parent,bool Translate)
#endif //PBG_ADD_NEWCHAR_MONK_ANI
{
    if ( NumActions<=0 ) return;

    if(PriorAction >= NumActions) PriorAction = 0;
	if(CurrentAction >= NumActions)CurrentAction = 0;
	VectorCopy(Angle,BodyAngle);

 	CurrentAnimation = AnimationFrame;
	CurrentAnimationFrame = (int)AnimationFrame;
	float s1 = (CurrentAnimation - CurrentAnimationFrame);
	//if(StopMotion)
	//	s1 = (int)(s1*4)/4;
	float s2 = 1.f-s1;
	int PriorAnimationFrame = (int)PriorFrame;
	if(NumActions > 0)
	{
        if(PriorAnimationFrame < 0 )
            PriorAnimationFrame = 0;
        if(CurrentAnimationFrame < 0 )
            CurrentAnimationFrame = 0;
		if(PriorAnimationFrame >= Actions[PriorAction].NumAnimationKeys)
			PriorAnimationFrame = 0;
		if(CurrentAnimationFrame >= Actions[CurrentAction].NumAnimationKeys)
			CurrentAnimationFrame = 0;
	}

	// bones
	for(int i=0;i<NumBones;i++)
	{
       	Bone_t *b = &Bones[i];
		if(b->Dummy)
		{
			continue;
		}
		BoneMatrix_t *bm1 = &b->BoneMatrixes[PriorAction  ];
		BoneMatrix_t *bm2 = &b->BoneMatrixes[CurrentAction];
		vec4_t q1,q2;

		if ( i==BoneHead )
		{
			vec3_t Angle1,Angle2;
			VectorCopy(bm1->Rotation[PriorAnimationFrame  ],Angle1);
			VectorCopy(bm2->Rotation[CurrentAnimationFrame],Angle2);

            float HeadAngleX = HeadAngle[0] / (180.f / Q_PI);
            float HeadAngleY = HeadAngle[1] / (180.f / Q_PI);
			Angle1[0] -= HeadAngleX;
			Angle2[0] -= HeadAngleX;
			Angle1[2] -= HeadAngleY;
			Angle2[2] -= HeadAngleY;
			AngleQuaternion( Angle1, q1 );
			AngleQuaternion( Angle2, q2 );
		}
		else
		{
			QuaternionCopy(bm1->Quaternion[PriorAnimationFrame  ],q1);
			QuaternionCopy(bm2->Quaternion[CurrentAnimationFrame],q2);
		}
		if (!QuaternionCompare( q1, q2 ))
		{
			QuaternionSlerp(q1 , q2, s1, BoneQuaternion[i] );
		}
		else
		{
			QuaternionCopy( q1, BoneQuaternion[i] );
		}

		float Matrix[3][4];
		QuaternionMatrix(BoneQuaternion[i],Matrix);
		float *Position1 = bm1->Position[PriorAnimationFrame];
		float *Position2 = bm2->Position[CurrentAnimationFrame];

		if(i==0 && (Actions[PriorAction].LockPositions || Actions[CurrentAction].LockPositions))
		{
			Matrix[0][3] = bm2->Position[0][0];
			Matrix[1][3] = bm2->Position[0][1];
			Matrix[2][3] = Position1[2]*s2+Position2[2]*s1+BodyHeight;
		}
		else
		{
			Matrix[0][3] = Position1[0]*s2+Position2[0]*s1;
			Matrix[1][3] = Position1[1]*s2+Position2[1]*s1;
			Matrix[2][3] = Position1[2]*s2+Position2[2]*s1;
		}

		if(b->Parent == -1)
		{
			if(!Parent)
			{
				//memcpy(BoneMatrix[i],BoneMatrix,sizeof(float)*12);
				AngleMatrix(BodyAngle,ParentMatrix);
				if(Translate)
				{
					//ParentMatrix[0][0] *= BodyScale;
					//ParentMatrix[1][1] *= BodyScale;
					//ParentMatrix[2][2] *= BodyScale;
					for ( int y = 0; y < 3; ++y)
					{
						for ( int x = 0; x < 3; ++x)
						{
							ParentMatrix[y][x] *= BodyScale;
						}
					}

					ParentMatrix[0][3] = BodyOrigin[0];
					ParentMatrix[1][3] = BodyOrigin[1];
					ParentMatrix[2][3] = BodyOrigin[2];
				}
			}
			R_ConcatTransforms(ParentMatrix,Matrix,BoneMatrix[i]);
		} 
		else 
		{
			R_ConcatTransforms(BoneMatrix[b->Parent],Matrix,BoneMatrix[i]);
		}
	}
}

#endif //MR0

extern int  SceneFlag;
extern int EditFlag;

bool HighLight = true;
float BoneScale = 1.f;

#ifdef MR0
void BMD::Transform(float (*BoneMatrix)[3][4],vec3_t BoundingBoxMin,vec3_t BoundingBoxMax,OBB_t *OBB,bool Translate)
{
	if(NumMeshs == 0) 
	{
#ifdef LDS_MR0_MOD_WHENTRANFORM_BUILDOBB_ALLWAYS
		VectorCopy(BoundingBoxMin,OBB->StartPos);
		OBB->XAxis[0] = (BoundingBoxMax[0] - BoundingBoxMin[0]);
		OBB->YAxis[1] = (BoundingBoxMax[1] - BoundingBoxMin[1]);
		OBB->ZAxis[2] = (BoundingBoxMax[2] - BoundingBoxMin[2]);
		
		VectorAdd(OBB->StartPos,BodyOrigin,OBB->StartPos);
		OBB->XAxis[1] = 0.f;
		OBB->XAxis[2] = 0.f;
		OBB->YAxis[0] = 0.f;
		OBB->YAxis[2] = 0.f;
		OBB->ZAxis[0] = 0.f;
		OBB->ZAxis[1] = 0.f;
#endif // LDS_MR0_MOD_WHENTRANFORM_BUILDOBB_ALLWAYS

		return;
	}

	// 1-1. MR0 Renderer가 ON 경우 Shader를 통한 Transformation이 이루어 지도록 합니다.
#ifdef LDS_MR0_ADD_SPECIFICMODEL_DOANIMATIONPROCESSING
	if(false==m_bProcessAnimationTMUsingPassedMothod&&EngineGate::IsOn())
#else // LDS_MR0_ADD_SPECIFICMODEL_DOANIMATIONPROCESSING
	if(EngineGate::IsOn())
#endif // LDS_MR0_ADD_SPECIFICMODEL_DOANIMATIONPROCESSING
	{		
#ifdef LDS_MR0_MODIFY_TRANSFORMSCALE_FORSILHOUETTE
		m_fRequestScale = BoneScale;	// Sillouette 등 사용자 Scale 크기를 지정 합니다.
#endif // LDS_MR0_MODIFY_TRANSFORMSCALE_FORSILHOUETTE

		VectorCopy(BoundingBoxMin,OBB->StartPos);
		OBB->XAxis[0] = (BoundingBoxMax[0] - BoundingBoxMin[0]);
		OBB->YAxis[1] = (BoundingBoxMax[1] - BoundingBoxMin[1]);
		OBB->ZAxis[2] = (BoundingBoxMax[2] - BoundingBoxMin[2]);

		VectorAdd(OBB->StartPos,BodyOrigin,OBB->StartPos);
		OBB->XAxis[1] = 0.f;
		OBB->XAxis[2] = 0.f;
		OBB->YAxis[0] = 0.f;
		OBB->YAxis[2] = 0.f;
		OBB->ZAxis[0] = 0.f;
		OBB->ZAxis[1] = 0.f;

		g_pMeshMachine->SetTransOp(Translate);
		g_Renderer->Transform(this, BoneMatrix);
	}
	// 1-2. MR0 Renderer가 OFF 경우 Shader를 통한 Transformation이 이루어 지도록 합니다.
	else
	{
		// transform
		vec3_t LightPosition;

		if(LightEnable)
		{
     		vec3_t Position;

			float Matrix[3][4];
			if(HighLight)
			{
				Vector(1.3f,0.f,2.f,Position);
			}
			else if ( battleCastle::InBattleCastle() )
			{
				Vector ( 0.5f, -1.f, 1.f, Position );
				Vector ( 0.f, 0.f, -45.f, ShadowAngle );
			}
			else
			{
				Vector(0.f,-1.5f,0.f,Position);
			}

			AngleMatrix(ShadowAngle,Matrix);
			VectorIRotate(Position,Matrix,LightPosition);
		}
		vec3_t BoundingMin;
		vec3_t BoundingMax;
	#ifdef _DEBUG
	#else
		if(EditFlag==2)
	#endif
		{
			Vector( 999999.f, 999999.f, 999999.f,BoundingMin);
			Vector(-999999.f,-999999.f,-999999.f,BoundingMax);
		}
		for(int i=0;i<NumMeshs;i++)
		{
       		Mesh_t *m = &Meshs[i];
			for(int j=0;j<m->NumVertices;j++)
			{
				Vertex_t *v = &m->Vertices[j];
				float *vp = VertexTransform[i][j];
				if(BoneScale == 1.f)
				{
					VectorTransform(v->Position,BoneMatrix[v->Node],vp);
					if(Translate)
						VectorScale(vp,BodyScale,vp);
				}
				else
				{
					VectorRotate(v->Position,BoneMatrix[v->Node],vp);
					vp[0] = vp[0] * BoneScale + BoneMatrix[v->Node][0][3];
					vp[1] = vp[1] * BoneScale + BoneMatrix[v->Node][1][3];
					vp[2] = vp[2] * BoneScale + BoneMatrix[v->Node][2][3];
					if(Translate)
						VectorScale(vp,BodyScale,vp);
				}
	#ifdef _DEBUG
	#else
				if(EditFlag==2)
	#endif
				{
					for(int k=0;k<3;k++)
					{
						if(vp[k] < BoundingMin[k]) BoundingMin[k] = vp[k];
						if(vp[k] > BoundingMax[k]) BoundingMax[k] = vp[k];
					}
				}
				if(Translate)
					VectorAdd(vp,BodyOrigin,vp);
			}
			for(j=0;j<m->NumNormals;j++)
			{
				Normal_t *sn = &m->Normals[j];
				float    *tn = NormalTransform[i][j];
				VectorRotate(sn->Normal,BoneMatrix[sn->Node],tn);
				if(LightEnable)
				{
					float Luminosity;
						Luminosity = DotProduct(tn,LightPosition)*0.8f+0.4f;

					if(Luminosity < 0.2f) Luminosity = 0.2f;
					IntensityTransform[i][j] = Luminosity;
				}
			}
		}
		if(EditFlag==2)
		{
			VectorCopy(BoundingMin,OBB->StartPos);
			OBB->XAxis[0] = (BoundingMax[0] - BoundingMin[0]);
			OBB->YAxis[1] = (BoundingMax[1] - BoundingMin[1]);
			OBB->ZAxis[2] = (BoundingMax[2] - BoundingMin[2]);
		}
		else
		{
			VectorCopy(BoundingBoxMin,OBB->StartPos);
			OBB->XAxis[0] = (BoundingBoxMax[0] - BoundingBoxMin[0]);
			OBB->YAxis[1] = (BoundingBoxMax[1] - BoundingBoxMin[1]);
			OBB->ZAxis[2] = (BoundingBoxMax[2] - BoundingBoxMin[2]);
		}
		fTransformedSize = max( max( BoundingMax[0] - BoundingMin[0], BoundingMax[1] - BoundingMin[1]),
									BoundingMax[2] - BoundingMin[2]);
		//fTransformedSize *= 0.3f;
		VectorAdd(OBB->StartPos,BodyOrigin,OBB->StartPos);
		OBB->XAxis[1] = 0.f;
		OBB->XAxis[2] = 0.f;
		OBB->YAxis[0] = 0.f;
		OBB->YAxis[2] = 0.f;
		OBB->ZAxis[0] = 0.f;
		OBB->ZAxis[1] = 0.f;
	}
}

#else
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
void BMD::Transform(float (*BoneMatrix)[3][4],vec3_t BoundingBoxMin,vec3_t BoundingBoxMax,OBB_t *OBB,bool Translate, float _Scale)
#else //PBG_ADD_NEWCHAR_MONK_ITEM
void BMD::Transform(float (*BoneMatrix)[3][4],vec3_t BoundingBoxMin,vec3_t BoundingBoxMax,OBB_t *OBB,bool Translate)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
{
	// transform
	vec3_t LightPosition;

	if(LightEnable)
	{
     	vec3_t Position;

		float Matrix[3][4];
		if(HighLight)
		{
			Vector(1.3f,0.f,2.f,Position);
		}
        else if ( gMapManager.InBattleCastle() )
        {
            Vector ( 0.5f, -1.f, 1.f, Position );
            Vector ( 0.f, 0.f, -45.f, ShadowAngle );
        }
		else
		{
		    Vector(0.f,-1.5f,0.f,Position);
		}

		AngleMatrix(ShadowAngle,Matrix);
		VectorIRotate(Position,Matrix,LightPosition);
	}
	vec3_t BoundingMin;
	vec3_t BoundingMax;
#ifdef _DEBUG
#else
	if(EditFlag==2)
#endif
	{
		Vector( 999999.f, 999999.f, 999999.f,BoundingMin);
		Vector(-999999.f,-999999.f,-999999.f,BoundingMax);
	}
	for(int i=0;i<NumMeshs;i++)
	{
       	Mesh_t *m = &Meshs[i];
		for(int j=0;j<m->NumVertices;j++)
		{
			Vertex_t *v = &m->Vertices[j];
			float *vp = VertexTransform[i][j];

			if(BoneScale == 1.f)
			{
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
 				if(_Scale)
 				{
					vec3_t Position;
					VectorCopy(v->Position, Position);
					VectorScale(Position, _Scale, Position);
					VectorTransform(Position,BoneMatrix[v->Node],vp);
 				}
 				else
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
				VectorTransform(v->Position,BoneMatrix[v->Node],vp);
				if(Translate)
					VectorScale(vp,BodyScale,vp);
			}
			else
			{
				VectorRotate(v->Position,BoneMatrix[v->Node],vp);
				vp[0] = vp[0] * BoneScale + BoneMatrix[v->Node][0][3];
				vp[1] = vp[1] * BoneScale + BoneMatrix[v->Node][1][3];
				vp[2] = vp[2] * BoneScale + BoneMatrix[v->Node][2][3];
				if(Translate)
					VectorScale(vp,BodyScale,vp);
			}
#ifdef _DEBUG
#else
			if(EditFlag==2)
#endif
			{
				for(int k=0;k<3;k++)
				{
					if(vp[k] < BoundingMin[k]) BoundingMin[k] = vp[k];
					if(vp[k] > BoundingMax[k]) BoundingMax[k] = vp[k];
				}
			}
			if(Translate)
				VectorAdd(vp,BodyOrigin,vp);
		}

		for(int j=0;j<m->NumNormals;j++)
		{
			Normal_t *sn = &m->Normals[j];
			float    *tn = NormalTransform[i][j];
			VectorRotate(sn->Normal,BoneMatrix[sn->Node],tn);
			if(LightEnable)
			{
				float Luminosity;
					Luminosity = DotProduct(tn,LightPosition)*0.8f+0.4f;

				if(Luminosity < 0.2f) Luminosity = 0.2f;
				IntensityTransform[i][j] = Luminosity;
			}
		}
	}
	if(EditFlag==2)
	{
		VectorCopy(BoundingMin,OBB->StartPos);
		OBB->XAxis[0] = (BoundingMax[0] - BoundingMin[0]);
		OBB->YAxis[1] = (BoundingMax[1] - BoundingMin[1]);
		OBB->ZAxis[2] = (BoundingMax[2] - BoundingMin[2]);
	}
	else
	{
		VectorCopy(BoundingBoxMin,OBB->StartPos);
		OBB->XAxis[0] = (BoundingBoxMax[0] - BoundingBoxMin[0]);
		OBB->YAxis[1] = (BoundingBoxMax[1] - BoundingBoxMin[1]);
		OBB->ZAxis[2] = (BoundingBoxMax[2] - BoundingBoxMin[2]);
	}
	fTransformedSize = max( max( BoundingMax[0] - BoundingMin[0], BoundingMax[1] - BoundingMin[1]),
								BoundingMax[2] - BoundingMin[2]);
	//fTransformedSize *= 0.3f;
	VectorAdd(OBB->StartPos,BodyOrigin,OBB->StartPos);
	OBB->XAxis[1] = 0.f;
	OBB->XAxis[2] = 0.f;
	OBB->YAxis[0] = 0.f;
	OBB->YAxis[2] = 0.f;
	OBB->ZAxis[0] = 0.f;
	OBB->ZAxis[1] = 0.f;
}

#endif //MR0

#ifdef LDS_ADD_RENDERMESHEFFECT_FOR_VBO
// 모든BoneNodeMatrix를 받아서 현재 Model의 Vertex에 모두 곱한뒤 outVertexTransform__으로 반환. 
void BMD::TransformtoVertices(vec3_t (*outVertexTransform__)[MAX_VERTICES], float (*matBone)[3][4], bool Translate)
{
	for(int i=0;i<NumMeshs;i++)
	{
		Mesh_t *m = &Meshs[i];
		for(int j=0;j<m->NumVertices;j++)
		{
			Vertex_t *v = &m->Vertices[j];
			float *vp = outVertexTransform__[i][j];
			if(BoneScale == 1.f)
			{
				VectorTransform(v->Position,matBone[v->Node],vp);
				if(Translate)
					VectorScale(vp,BodyScale,vp);
			}
			else
			{
				VectorRotate(v->Position,matBone[v->Node],vp);
				vp[0] = vp[0] * BoneScale + matBone[v->Node][0][3];
				vp[1] = vp[1] * BoneScale + matBone[v->Node][1][3];
				vp[2] = vp[2] * BoneScale + matBone[v->Node][2][3];
				if(Translate)
					VectorScale(vp,BodyScale,vp);
			}

			if(Translate)
				VectorAdd(vp,BodyOrigin,vp);
		}
	}
}
#endif // LDS_ADD_RENDERMESHEFFECT_FOR_VBO

// vResultPosition = (BoneTransformMatrix * vRelativePosition) * BMD::BodyScale + vObjectPosition;
void BMD::TransformByObjectBone(vec3_t vResultPosition, OBJECT * pObject, int iBoneNumber, vec3_t vRelativePosition)
{
	if (iBoneNumber < 0 || iBoneNumber >= NumBones)
	{
		assert(!"본 인덱스 이상");
		return;
	}
	if (pObject == NULL)
	{
		assert(!"오브젝트 포인터 이상");
		return;
	}

	float (*TransformMatrix)[4];
	if (pObject->BoneTransform != NULL)
	{
		TransformMatrix = pObject->BoneTransform[iBoneNumber];
	}
	else
	{
		TransformMatrix = BoneTransform[iBoneNumber];	// 오브젝트에 BoneTransform이 설정되지 않은경우 전역변수 BoneTransform을 사용한다.
	}

	vec3_t vTemp;
	if (vRelativePosition == NULL)
	{
		vTemp[0] = TransformMatrix[0][3];	// VectorTransform(Position,Matrix,p);	Position이 0,0,0일때와같은 효과
		vTemp[1] = TransformMatrix[1][3];
		vTemp[2] = TransformMatrix[2][3];
	}
	else
	{
		VectorTransform(vRelativePosition, TransformMatrix, vTemp);
	}
	VectorScale(vTemp, BodyScale, vTemp);
	VectorAdd(vTemp, pObject->Position, vResultPosition);
}

void BMD::TransformByBoneMatrix(vec3_t vResultPosition, float (*BoneMatrix)[4], vec3_t vWorldPosition, vec3_t vRelativePosition)
{
	if (BoneMatrix == NULL)
	{
		assert(!"본 메트릭스 이상");
		return;
	}

	vec3_t vTemp;
	if (vRelativePosition == NULL)
	{
		vTemp[0] = BoneMatrix[0][3];	// VectorTransform(Position,Matrix,p);	Position이 0,0,0일때와같은 효과
		vTemp[1] = BoneMatrix[1][3];
		vTemp[2] = BoneMatrix[2][3];
	}
	else
	{
		VectorTransform(vRelativePosition, BoneMatrix, vTemp);
	}
	if (vWorldPosition != NULL)
	{
		VectorScale(vTemp, BodyScale, vTemp);
		VectorAdd(vTemp, vWorldPosition, vResultPosition);
	}
	else
	{
		VectorScale(vTemp, BodyScale, vResultPosition);
	}
}

void BMD::TransformPosition(float (*Matrix)[4],vec3_t Position,vec3_t WorldPosition,bool Translate)
{
	if(Translate)
	{
		vec3_t p;
		VectorTransform(Position,Matrix,p);
		VectorScale(p,BodyScale,p);
		VectorAdd(p,BodyOrigin,WorldPosition);
	}
	else
    	VectorTransform(Position,Matrix,WorldPosition);
}

void BMD::RotationPosition(float (*Matrix)[4],vec3_t Position,vec3_t WorldPosition)
{
	vec3_t p;
	VectorRotate(Position,Matrix,p);
	VectorScale(p,BodyScale,WorldPosition);
	for(int i=0;i<3;i++)
	{
		for(int j=0;j<4;j++)
		{
			ParentMatrix[i][j] = Matrix[i][j];
		}
	}
}

/*void BMD::TransformScale(float Scale,bool Link)
{
	for(int i=0;i<NumMeshs;i++)
	{
       	Mesh_t *m = &Meshs[i];
		for(int j=0;j<m->NumVertices;j++)
		{
			Vertex_t *v = &m->Vertices[j];
			float *vp = VertexTransform[i][j];
			vec3_t Position;
			VectorCopy(v->Position,Position);
			VectorScale(Position,Scale,Position);
			if(Link == false) 
				VectorTransform(Position,BoneMatrix[v->Node],vp);
			else
        		VectorTransform(Position,LinkBoneMatrix,vp);
			VectorScale(vp,BodyScale,vp);
			VectorAdd(vp,BodyOrigin,vp);
		}
	}
}*/

/*void BMD::TransformShadow()
{
	for(int i=0;i<NumMeshs;i++)
	{
       	Mesh_t *m = &Meshs[i];
		for(int j=0;j<m->NumVertices;j++)
		{
			Vertex_t *v = &m->Vertices[j];
			float *vp = VertexTransform[i][j];
			VectorTransform(v->Position,BoneMatrix[v->Node],vp);
			VectorScale(vp,BodyScale,vp);
			//vp[0] += (vp[2])*(vp[0]+8000.f)/(vp[2]-10000.f);
			vp[1] -= (vp[2])*(vp[1]+10000.f)/(vp[2]-8000.f);
			vp[2] = 0.f;
			VectorAdd(vp,BodyOrigin,vp);
		}
	}
}*/
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
bool BMD::PlayAnimation(float *AnimationFrame,float *PriorAnimationFrame,unsigned short *PriorAction,float Speed,vec3_t Origin,vec3_t Angle)
#else //PBG_ADD_NEWCHAR_MONK_ANI
bool BMD::PlayAnimation(float *AnimationFrame,float *PriorAnimationFrame,unsigned char *PriorAction,float Speed,vec3_t Origin,vec3_t Angle)
#endif //PBG_ADD_NEWCHAR_MONK_ANI
{
	bool Loop = true;

#ifdef YDG_FIX_GIGANTIC_STORM_CRASH
	if(AnimationFrame == NULL || PriorAnimationFrame == NULL || PriorAction == NULL || (NumActions > 0 && CurrentAction >= NumActions))
	{
#ifdef CONSOLE_DEBUG
//		g_ConsoleDebug->Write(MCD_ERROR, "PlayAnimation(NumActions:%d, CurrentAction:%d)", NumActions, CurrentAction);
#endif // CONSOLE_DEBUG
		return Loop;
	}
#endif // YDG_FIX_GIGANTIC_STORM_CRASH

	if(NumActions == 0 || Actions[CurrentAction].NumAnimationKeys <= 1) 
	{
		return Loop;
	}

	int Temp = (int)*AnimationFrame;
   	*AnimationFrame += Speed;
	if(Temp != (int)*AnimationFrame) 
	{
		*PriorAction = CurrentAction;
		*PriorAnimationFrame = (float)Temp;
	}
	if(*AnimationFrame <= 0.f)
	{
		*AnimationFrame += (float)Actions[CurrentAction].NumAnimationKeys-1.f;
	}
	
	if(Actions[CurrentAction].Loop)
	{
    	if(*AnimationFrame >= (float)Actions[CurrentAction].NumAnimationKeys)
		{
      		*AnimationFrame = (float)Actions[CurrentAction].NumAnimationKeys-0.01f;
    		Loop = false;
		}
	}
	else
	{
		int Key;
		if(Actions[CurrentAction].LockPositions)
			Key = Actions[CurrentAction].NumAnimationKeys-1;
		else
			Key = Actions[CurrentAction].NumAnimationKeys;

		float fTemp; 

		if(SceneFlag == 4)
		{
			fTemp = *AnimationFrame+2; // 케릭터 씬일때
		}
		else if(gMapManager.WorldActive == WD_39KANTURU_3RD && CurrentAction == MONSTER01_APEAR)
		{
			fTemp = *AnimationFrame+1;
		}
		else 
		{
			fTemp = *AnimationFrame;
		}

		if(fTemp >= (int)Key)  // 케릭터 씬 생성창에 나오는 케릭 에니메니션 조절을 위해(같은 동작이 여러번 반복되는 경우 발생)
		{
     		int Frame = (int)*AnimationFrame;
     		*AnimationFrame = (float)(Frame%(Key))+(*AnimationFrame-(float)Frame);
    		Loop = false;
		}	
	}
	CurrentAnimation = *AnimationFrame;
	CurrentAnimationFrame = (int)CurrentAnimation;

	return Loop;
}


#ifdef LDS_ADD_MODEL_ATTACH_SPECIFIC_NODE_

	// 자신을 object의 n번째 노드위치에 애니 하여 bmd의 모든 Vertex들에 반영.
#ifdef LDS_ADD_ANIMATIONTRANSFORMWITHMODEL_USINGGLOBALTM
void BMD::AnimationTransformWithAttachHighModel_usingGlobalTM( 
												OBJECT* oHighHierarchyModel, 
												BMD* bmdHighHierarchyModel, 
												int iBoneNumberHighHierarchyModel,
												vec3_t &vOutPosHighHiearachyModelBone,
												vec3_t *arrOutSetfAllBonePositions,
												bool bApplyTMtoVertices)
{
	if( NumBones < 1) return;
	if( NumBones > MAX_BONES ) return;
	
	vec34_t*	arrBonesTMLocal;
	
	vec34_t		tmBoneHierarchicalObject;
	
	vec3_t		Temp, v3Position;
	OBB_t		OBB;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	int			iBoneCount = NumBones;	
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	
	arrBonesTMLocal = new vec34_t[NumBones];
	Vector( 0.0f, 0.0f, 0.0f, Temp );
	
	memset( arrBonesTMLocal, 0, sizeof(vec34_t) * NumBones );
	memset( tmBoneHierarchicalObject, 0, sizeof(vec34_t) );
	
	// 1. 상위모델의본TM 을 HierarachicalObj로 복사.
	memcpy( tmBoneHierarchicalObject, oHighHierarchyModel->BoneTransform[iBoneNumberHighHierarchyModel], sizeof(vec34_t) );
	BodyScale = oHighHierarchyModel->Scale;		
	
	// 2. 상위모델의본Translation에 상위모델의Scale 반영. 
	tmBoneHierarchicalObject[0][3] = tmBoneHierarchicalObject[0][3] * BodyScale;
	tmBoneHierarchicalObject[1][3] = tmBoneHierarchicalObject[1][3] * BodyScale;
	tmBoneHierarchicalObject[2][3] = tmBoneHierarchicalObject[2][3] * BodyScale;
	
	// 3. OutParam : 상위 계층 모델의 해당 Bone 위치값
	if( NULL != vOutPosHighHiearachyModelBone )
	{
		Vector( tmBoneHierarchicalObject[0][3], tmBoneHierarchicalObject[1][3], tmBoneHierarchicalObject[2][3],
			vOutPosHighHiearachyModelBone );
	}
	
	// ㄴ. 상위 모델의 위치
	VectorCopy( oHighHierarchyModel->Position, v3Position );
	
	// ㄷ. Sword 자체 에니메이션 산출
	Animation( arrBonesTMLocal, 0, 0, 0, Temp, Temp, false, false );
	
	// ㄹ. (부모tm(scale적용) X 자체tm) + 부모위치
	for( int i_ = 0; i_ < NumBones; ++i_ )
	{
		R_ConcatTransforms( tmBoneHierarchicalObject, arrBonesTMLocal[i_], BoneTransform[i_] );
		
		// LocalTM+World(상위위치) => WorldTM
		BoneTransform[i_][0][3] = BoneTransform[i_][0][3] + v3Position[0];
		BoneTransform[i_][1][3] = BoneTransform[i_][1][3] + v3Position[1];
		BoneTransform[i_][2][3] = BoneTransform[i_][2][3] + v3Position[2];

		Vector( BoneTransform[i_][0][3], 
			BoneTransform[i_][1][3],
			BoneTransform[i_][2][3],
			arrOutSetfAllBonePositions[i_] );
	}
	
	// ㅁ. Vertex 결과 TM 적용.
	if(true == bApplyTMtoVertices)
	{
		Transform(BoneTransform, Temp, Temp, &OBB, false);
	}

	delete [] arrBonesTMLocal;
}
#endif // LDS_ADD_ANIMATIONTRANSFORMWITHMODEL_USINGGLOBALTM
						
void BMD::AnimationTransformWithAttachHighModel( 
						OBJECT* oHighHierarchyModel, 
						BMD* bmdHighHierarchyModel, 
						int iBoneNumberHighHierarchyModel,
						vec3_t &vOutPosHighHiearachyModelBone,
						vec3_t *arrOutSetfAllBonePositions )
{
	if( NumBones < 1) return;
	if( NumBones > MAX_BONES ) return;
	
	vec34_t*	arrBonesTMLocal;
	vec34_t*	arrBonesTMLocalResult;		// BoneTransform

	vec34_t		tmBoneHierarchicalObject;

	vec3_t		Temp, v3Position;
#ifndef LDS_MOD_ANIMATIONTRANSFORM_WITHHIGHMODEL		// #ifndef // 추후삭제
	OBB_t		OBB;
#endif // LDS_MOD_ANIMATIONTRANSFORM_WITHHIGHMODEL
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	int			iBoneCount = NumBones;	
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	
	arrBonesTMLocal = new vec34_t[NumBones];
	Vector( 0.0f, 0.0f, 0.0f, Temp );

	arrBonesTMLocalResult = new vec34_t[NumBones];

	memset( arrBonesTMLocalResult, 0, sizeof(vec34_t) * NumBones );
	memset( arrBonesTMLocal, 0, sizeof(vec34_t) * NumBones );

	memset( tmBoneHierarchicalObject, 0, sizeof(vec34_t) );
	
	// 1. 상위모델의본TM 을 HierarachicalObj로 복사.
	memcpy( tmBoneHierarchicalObject, oHighHierarchyModel->BoneTransform[iBoneNumberHighHierarchyModel], sizeof(vec34_t) );

	BodyScale = oHighHierarchyModel->Scale;		

	// 2. 상위모델의본Translation에 상위모델의Scale 반영. 
	tmBoneHierarchicalObject[0][3] = tmBoneHierarchicalObject[0][3] * BodyScale;
	tmBoneHierarchicalObject[1][3] = tmBoneHierarchicalObject[1][3] * BodyScale;
	tmBoneHierarchicalObject[2][3] = tmBoneHierarchicalObject[2][3] * BodyScale;

	// 3. OutParam : 상위 계층 모델의 해당 Bone 위치값
	if( NULL != vOutPosHighHiearachyModelBone )
	{
		Vector( tmBoneHierarchicalObject[0][3], tmBoneHierarchicalObject[1][3], tmBoneHierarchicalObject[2][3],
					vOutPosHighHiearachyModelBone );
	}
	
	// ㄴ. 상위 모델의 위치
	VectorCopy( oHighHierarchyModel->Position, v3Position );
	
	// ㄷ. Sword 자체 에니메이션 산출
	Animation( arrBonesTMLocal, 0, 0, 0, Temp, Temp, false, false );
	
	// ㄹ. (부모tm(scale적용) X 자체tm) + 부모위치
	for( int i_ = 0; i_ < NumBones; ++i_ )
	{
		// 상위_본의위치*자신_애니위치 => LocalTM
		R_ConcatTransforms( tmBoneHierarchicalObject, arrBonesTMLocal[i_], arrBonesTMLocalResult[i_] );

		// LocalTM+World(상위위치) => WorldTM
 		arrBonesTMLocalResult[i_][0][3] = arrBonesTMLocalResult[i_][0][3] + v3Position[0];
 		arrBonesTMLocalResult[i_][1][3] = arrBonesTMLocalResult[i_][1][3] + v3Position[1];
  		arrBonesTMLocalResult[i_][2][3] = arrBonesTMLocalResult[i_][2][3] + v3Position[2];

		// ㄹ_. Out Attach 하려는 상위 계층 모델의 해당 Bone 위치값
		Vector( arrBonesTMLocalResult[i_][0][3], 
			    arrBonesTMLocalResult[i_][1][3],
				arrBonesTMLocalResult[i_][2][3],
				arrOutSetfAllBonePositions[i_] );
	}
	
	// ㅁ. Vertex 결과 TM 적용.	
#ifndef LDS_MOD_ANIMATIONTRANSFORM_WITHHIGHMODEL		// #ifndef // 추후삭제
	Transform(arrBonesTMLocalResult, Temp, Temp, &OBB, false);
#endif // LDS_MOD_ANIMATIONTRANSFORM_WITHHIGHMODEL

	delete [] arrBonesTMLocalResult;
	delete [] arrBonesTMLocal;
}

// 외부object의 rts => RootRTS로 animate되어 각 Bone별 Position 값 array 반환
void BMD::AnimationTransformOnlySelf( vec3_t *arrOutSetfAllBonePositions, const OBJECT* oSelf )
{
	if( NumBones < 1) return;
	if( NumBones > MAX_BONES ) return;
	
	vec34_t*	arrBonesTMLocal;
	
	vec3_t		Temp;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	int			iBoneCount = NumBones;	
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	
	arrBonesTMLocal = new vec34_t[NumBones];
	Vector( 0.0f, 0.0f, 0.0f, Temp );
	
	memset( arrBonesTMLocal, 0, sizeof(vec34_t) * NumBones );

	// ㄷ. 자체 에니메이션 산출 : (외부 인자값=rotation), (내부인자값=BodyScale,Position)
	Animation( arrBonesTMLocal, oSelf->AnimationFrame,oSelf->PriorAnimationFrame,oSelf->PriorAction, (const_cast<OBJECT*>(oSelf))->Angle, Temp, false, true );
	
	// ㄹ. 자체 에니메이션 * SCALE * n번 본의 에니메이션 + n번 본의 위치
	for( int i_ = 0; i_ < NumBones; ++i_ )
	{				
		// ㄹ_. Out에 위치값 갱신.
		Vector( arrBonesTMLocal[i_][0][3], 
			arrBonesTMLocal[i_][1][3],
			arrBonesTMLocal[i_][2][3],
			arrOutSetfAllBonePositions[i_] );
	}
	
	// ㅁ. Vertex로 결과 갱신.
	//Transform(arrBonesTMLocal, Temp, Temp, &OBB, false);
	delete [] arrBonesTMLocal;
}


// 인자값 rts로 animate되어 각 Bone별 Position 값 array 반환
void BMD::AnimationTransformOnlySelf( vec3_t *arrOutSetfAllBonePositions, 
									  const vec3_t &v3Angle, 
									  const vec3_t &v3Position,
									  const float &fScale,
									  OBJECT* oRefAnimation,
									  const float fFrameArea,
									  const float fWeight)
{
	if( NumBones < 1) return;
	if( NumBones > MAX_BONES ) return;
	
	vec34_t*	arrBonesTMLocal;
	vec3_t		v3RootAngle, v3RootPosition;
	float		fRootScale;
	vec3_t		Temp;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	int			iBoneCount = NumBones;	
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

	fRootScale = const_cast<float&>(fScale);

	v3RootAngle[0] = v3Angle[0];
	v3RootAngle[1] = v3Angle[1];
	v3RootAngle[2] = v3Angle[2];

	v3RootPosition[0] = v3Position[0];
	v3RootPosition[1] = v3Position[1];
	v3RootPosition[2] = v3Position[2];
	
	arrBonesTMLocal = new vec34_t[NumBones];
	Vector( 0.0f, 0.0f, 0.0f, Temp );
	
	memset( arrBonesTMLocal, 0, sizeof(vec34_t) * NumBones );
	
	// ㄷ. Sword 자체 에니메이션 산출 : (외부 인자값=rotation), (내부인자값=BodyScale,Position)
	if( NULL == oRefAnimation )
	{
		Animation( arrBonesTMLocal, 0, 0, 0, v3RootAngle, Temp, false, true );
	}
	else
	{
		float			fAnimationFrame = oRefAnimation->AnimationFrame, 
						fPiriorAnimationFrame = oRefAnimation->PriorAnimationFrame;
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
		unsigned short	iPiriorAction = oRefAnimation->PriorAction;
#else //PBG_ADD_NEWCHAR_MONK_ANI
		unsigned char	iPiriorAction = oRefAnimation->PriorAction;
#endif //PBG_ADD_NEWCHAR_MONK_ANI

		if( fWeight >= 0.0f && fFrameArea > 0.0f )
		{
			float fAnimationFrameStart = fAnimationFrame - fFrameArea;
			float fAnimationFrameEnd = fAnimationFrame; 
			LInterpolationF( fAnimationFrame, fAnimationFrameStart, fAnimationFrameEnd, fWeight );
		}

		// 현재 설정된 AnimationFrame을 가져온다.
		Animation( arrBonesTMLocal, 
					fAnimationFrame,
					fPiriorAnimationFrame,
					iPiriorAction, 
					v3RootAngle, Temp, false, true );
	}
	
	// ㄹ. 자체 에니메이션 * SCALE * n번 본의 에니메이션 + n번 본의 위치
	vec3_t	v3RelatePos;
	Vector(1.0f, 1.0f, 1.0f, v3RelatePos);
	for( int i_ = 0; i_ < NumBones; ++i_ )
	{				
		// ㄹ_. Out에 위치값 갱신.
 		Vector( arrBonesTMLocal[i_][0][3], 
 			arrBonesTMLocal[i_][1][3],
 			arrBonesTMLocal[i_][2][3],
 			arrOutSetfAllBonePositions[i_] );
	}
	
	// ㅁ. Vertex로 결과 갱신.  
// 	OBB_t		OBB;
// 	Transform(arrBonesTMLocal, Temp, Temp, &OBB, false);
	delete [] arrBonesTMLocal;
}

#endif // LDS_ADD_MODEL_ATTACH_SPECIFIC_NODE_


#ifdef LDS_ADD_RENDERMESHEFFECT_FOR_VBO
// 외부object의 rts => RootRTS로 animate되어 각 Bone별 Position 값 array 반환
void BMD::AnimationTransformOutAllVertices(vec3_t (*outVertexTransform__)[MAX_VERTICES], const OBJECT& oSelf)
{
	if( NumBones < 1) return;
	if( NumBones > MAX_BONES ) return;
	
	vec34_t*	arrBonesTMLocal;
	
	vec3_t		Temp;
	int			iBoneCount = NumBones;	
	
	arrBonesTMLocal = new vec34_t[NumBones];
	Vector( 0.0f, 0.0f, 0.0f, Temp );
	
	memset( arrBonesTMLocal, 0, sizeof(vec34_t) * NumBones );
	
	// ㄷ. 자체 에니메이션 산출 : (외부 인자값=rotation), (내부인자값=BodyScale,Position)
	//Animation( arrBonesTMLocal, oSelf.AnimationFrame,oSelf.PriorAnimationFrame,oSelf.PriorAction, const_cast<OBJECT&>(oSelf).Angle, Temp, false, true );
	Animation( arrBonesTMLocal, oSelf.AnimationFrame,oSelf.PriorAnimationFrame,oSelf.PriorAction, const_cast<OBJECT&>(oSelf).Angle, Temp, false, false );
	
	// ㅂ. WorldBone arr 모두 vertex로 연산.
	//TransformtoVertices( outVertexTransform__, arrBonesTMLocal, false );
	TransformtoVertices( outVertexTransform__, arrBonesTMLocal, true );
	
	delete [] arrBonesTMLocal;
}
#endif // LDS_ADD_RENDERMESHEFFECT_FOR_VBO



vec3_t		g_vright;		// needs to be set to viewer's right in order for chrome to work
int			g_smodels_total = 1;				// cookie
float		g_chrome[MAX_VERTICES][2];	// texture coords for surface normals
int			g_chromeage[MAX_BONES];	// last time chrome vectors were updated
vec3_t		g_chromeup[MAX_BONES];		// chrome vector "up" in bone reference frames
vec3_t		g_chromeright[MAX_BONES];	// chrome vector "right" in bone reference frames

void BMD::Chrome(float *pchrome, int bone, vec3_t normal)
{
    Vector(0.f,0.f,1.f,g_vright);

	float n;

	//if (g_chromeage[bone] != g_smodels_total)
	{
		// calculate vectors from the viewer to the bone. This roughly adjusts for position
		vec3_t chromeupvec;		// g_chrome t vector in world reference frame
		vec3_t chromerightvec;	// g_chrome s vector in world reference frame
		vec3_t tmp;				// vector pointing at bone in world reference frame
		VectorScale( BodyOrigin, -1, tmp );
		//tmp[0] += BoneMatrix[bone][0][3];
		//tmp[1] += BoneMatrix[bone][1][3];
		//tmp[2] += BoneMatrix[bone][2][3];
		//tmp[0] += LinkBoneMatrix[0][3];
		//tmp[1] += LinkBoneMatrix[1][3];
		//tmp[2] += LinkBoneMatrix[2][3];
		VectorNormalize( tmp );
		CrossProduct( tmp, g_vright, chromeupvec );
		VectorNormalize( chromeupvec );
		CrossProduct( tmp, chromeupvec, chromerightvec );
		VectorNormalize( chromerightvec );

		//VectorIRotate( chromeupvec, BoneMatrix[bone], g_chromeup[bone] );
		//VectorIRotate( chromerightvec, BoneMatrix[bone], g_chromeright[bone] );
		//VectorIRotate( chromeupvec, LinkBoneMatrix, g_chromeup[bone] );
		//VectorIRotate( chromerightvec, LinkBoneMatrix, g_chromeright[bone] );

		g_chromeage[bone] = g_smodels_total;
	}

	// calc s coord
	n = DotProduct( normal, g_chromeright[bone] );
	pchrome[0] = (n + 1.f); // FIX: make this a float

	// calc t coord
	n = DotProduct( normal, g_chromeup[bone] );
	pchrome[1] = (n + 1.f); // FIX: make this a float
}

void BMD::Lighting(float *pLight, Light_t *lp, vec3_t Position, vec3_t Normal)
{
	vec3_t Light;
	VectorSubtract(lp->Position,Position,Light);
	float Length = sqrtf(Light[0]*Light[0] + Light[1]*Light[1] + Light[2]*Light[2]);

	float LightCos = (DotProduct(Normal,Light)/Length)*0.8f + 0.3f;
	if(Length > lp->Range) LightCos -= (Length-lp->Range)*0.01f;
	if(LightCos < 0.f) LightCos = 0.f;
	pLight[0] += LightCos * lp->Color[0];
	pLight[1] += LightCos * lp->Color[1];
	pLight[2] += LightCos * lp->Color[2];
}

///////////////////////////////////////////////////////////////////////////////
// light map
///////////////////////////////////////////////////////////////////////////////

#define AXIS_X  0
#define AXIS_Y  1
#define AXIS_Z  2

float SubPixel = 16.f;

void SmoothBitmap(int Width,int Height,unsigned char *Buffer)
{
	int RowStride = Width*3;
	for(int i=1;i<Height-1;i++)
	{
		for(int j=1;j<Width-1;j++)
		{
			int Index = (i*Width+j)*3;
			for(int k=0;k<3;k++)
			{
				Buffer[Index] = (Buffer[Index-RowStride-3]+Buffer[Index-RowStride]+Buffer[Index-RowStride+3]+
					Buffer[Index-3]+Buffer[Index+3]+
					Buffer[Index+RowStride-3]+Buffer[Index+RowStride]+Buffer[Index+RowStride+3])/8;
				Index ++;
			}
		}
	}
}

bool BMD::CollisionDetectLineToMesh(vec3_t Position,vec3_t Target,bool Collision,int Mesh,int Triangle)
{
	int i,j;
	for(i=0;i<NumMeshs;i++)
	{
       	Mesh_t *m = &Meshs[i];

		for(j=0;j<m->NumTriangles;j++)
		{
			if(i==Mesh && j==Triangle) continue;
			Triangle_t *tp = &m->Triangles[j];
			float *vp1 = VertexTransform[i][tp->VertexIndex[0]];
			float *vp2 = VertexTransform[i][tp->VertexIndex[1]];
			float *vp3 = VertexTransform[i][tp->VertexIndex[2]];
			float *vp4 = VertexTransform[i][tp->VertexIndex[3]];

			vec3_t Normal;
			FaceNormalize(vp1,vp2,vp3,Normal);
			bool success = CollisionDetectLineToFace(Position,Target,tp->Polygon,vp1,vp2,vp3,vp4,Normal,Collision);
			if(success == true) return true;
		}
	}
	return false;
}

void BMD::CreateLightMapSurface(Light_t *lp,Mesh_t *m,int i,int j,int MapWidth,int MapHeight,int MapWidthMax,int MapHeightMax,vec3_t BoundingMin,vec3_t BoundingMax,int Axis)
{
	int k,l;
	Triangle_t *tp = &m->Triangles[j];
	float *np = NormalTransform[i][tp->NormalIndex[0]];
	float *vp = VertexTransform[i][tp->VertexIndex[0]];
	float d = -DotProduct(vp,np);

	Bitmap_t *lmp = &LightMaps[NumLightMaps];
	if(lmp->Buffer == NULL)
	{
		lmp->Width = MapWidthMax;
		lmp->Height = MapHeightMax;
		int BufferBytes = lmp->Width*lmp->Height*3;
		lmp->Buffer = new unsigned char [BufferBytes];
		memset(lmp->Buffer,0,BufferBytes);
	}

	for(k=0;k<MapHeight;k++)
	{
		for(l=0;l<MapWidth;l++)
		{
			vec3_t p;
			Vector(0.f,0.f,0.f,p);
			switch(Axis)
			{
				case AXIS_Z:
					p[0] = BoundingMin[0]+l*SubPixel;
					p[1] = BoundingMin[1]+k*SubPixel;
					if(p[0] >= BoundingMax[0]) p[0] = BoundingMax[0];
					if(p[1] >= BoundingMax[1]) p[1] = BoundingMax[1];
					p[2] = (np[0]*p[0] + np[1]*p[1] + d) / -np[2];
					break;
				case AXIS_Y:
					p[0] = BoundingMin[0]+(float)l*SubPixel;
					p[2] = BoundingMin[2]+(float)k*SubPixel;
					if(p[0] >= BoundingMax[0]) p[0] = BoundingMax[0];
					if(p[2] >= BoundingMax[2]) p[2] = BoundingMax[2];
					p[1] = (np[0]*p[0] + np[2]*p[2] + d) / -np[1];
					break;
				case AXIS_X:
					p[2] = BoundingMin[2]+l*SubPixel;
					p[1] = BoundingMin[1]+k*SubPixel;
					if(p[2] >= BoundingMax[2]) p[2] = BoundingMax[2];
					if(p[1] >= BoundingMax[1]) p[1] = BoundingMax[1];
					p[0] = (np[2]*p[2] + np[1]*p[1] + d) / -np[0];
					break;
			}
       	    vec3_t Direction;
	        VectorSubtract(p,lp->Position,Direction);
			VectorNormalize(Direction);
			VectorSubtract(p,Direction,p);
			bool success = CollisionDetectLineToMesh(lp->Position,p,true,i,j);
			/*if(success == true)
			{
				DisableTexture();
				glBegin(GL_LINES);
				glColor3fv(lp->Color);
				glVertex3fv(p);
				glVertex3fv(lp->Position);
				glEnd();
			}*/
			if(success == false)
			{
               	unsigned char *Bitmap = &lmp->Buffer[(k*MapWidthMax+l)*3];
				vec3_t Light;
				Vector(0.f,0.f,0.f,Light);
				Lighting(Light,lp,p,np);
				for(int c=0;c<3;c++)
				{
					int Color;
					Color = Bitmap[c];
					Color += (unsigned char)(Light[c]*255.f);
					if(Color > 255) Color = 255;
					Bitmap[c] = Color;
				}
			}
		}
	}
}

void BMD::CreateLightMaps()
{
	/*if(LightMapEnable == true) return;

	int i,j,k,l;
	NumLightMaps = 0;
	for(i=0;i<NumMeshs;i++)
	{
       	Mesh_t *m = &Meshs[i];
		for(j=0;j<m->NumNormals;j++)
		{
			Normal_t *np = &m->Normals[j];
			VectorRotate(np->Normal,BoneTransform[np->Node],NormalTransform[i][j]);
		}
		for(j=0;j<m->NumTriangles;j++)
		{
			Triangle_t *tp = &m->Triangles[j];
			vec3_t BoundingMin,BoundingMax;
			Vector(99999.f,99999.f,99999.f,BoundingMin);
			Vector(-99999.f,-99999.f,-99999.f,BoundingMax);
			for(k=0;k<tp->Polygon;k++)
			{
				float *vp = VertexTransform[i][tp->VertexIndex[k]];
				for(l=0;l<3;l++)
				{
					if(vp[l] < BoundingMin[l]) BoundingMin[l] = vp[l];
					if(vp[l] > BoundingMax[l]) BoundingMax[l] = vp[l];
				}
			}

			float *np = NormalTransform[i][tp->NormalIndex[0]];
			float NormalX = absf(np[0]);
			float NormalY = absf(np[1]);
			float NormalZ = absf(np[2]);
			float NormalAxisMax = maxf(maxf(NormalX,NormalY),NormalZ);
			int Axis;
			int MapWidth;
			int MapHeight;
			if(NormalAxisMax == NormalZ)
			{
				Axis = AXIS_Z;
				MapWidth  = (int)((BoundingMax[0]-BoundingMin[0])/SubPixel)+1;
				MapHeight = (int)((BoundingMax[1]-BoundingMin[1])/SubPixel)+1;
			}
			else if(NormalAxisMax == NormalY)
			{
				Axis = AXIS_Y;
				MapWidth  = (int)((BoundingMax[0]-BoundingMin[0])/SubPixel)+1;
				MapHeight = (int)((BoundingMax[2]-BoundingMin[2])/SubPixel)+1;
			}
			else if(NormalAxisMax == NormalX)
			{
				Axis = AXIS_X;
				MapWidth  = (int)((BoundingMax[2]-BoundingMin[2])/SubPixel)+1;
				MapHeight = (int)((BoundingMax[1]-BoundingMin[1])/SubPixel)+1;
			}
			
			int MapWidthMax = 1;
			int MapHeightMax = 1;
			while(MapWidth >= MapWidthMax) MapWidthMax <<= 1;
			while(MapHeight >= MapHeightMax) MapHeightMax <<= 1;
			if(MapWidthMax  < 4) MapWidthMax  = 4;
			if(MapHeightMax < 4) MapHeightMax = 4;
			if(MapWidthMax  > 256) MapWidthMax  = 256;
			if(MapHeightMax > 256) MapHeightMax = 256;
			if(MapWidth  > MapWidthMax ) MapWidth  = MapWidthMax;
			if(MapHeight > MapHeightMax) MapHeight = MapHeightMax;
			float ScaleU = (float)MapWidthMax*SubPixel;
			float ScaleV = (float)MapHeightMax*SubPixel;

			for(k=0;k<tp->Polygon;k++)
			{
				float *vp = VertexTransform[i][tp->VertexIndex[k]];
    			if(NormalAxisMax == NormalZ)
				{
					tp->LightMapCoord[k].TexCoordU = (vp[0]-BoundingMin[0])/ScaleU;
					tp->LightMapCoord[k].TexCoordV = (vp[1]-BoundingMin[1])/ScaleV;
				}
    			else if(NormalAxisMax == NormalY)
				{
					tp->LightMapCoord[k].TexCoordU = (vp[0]-BoundingMin[0])/ScaleU;
					tp->LightMapCoord[k].TexCoordV = (vp[2]-BoundingMin[2])/ScaleV;
				}
    			else if(NormalAxisMax == NormalX)
				{
					tp->LightMapCoord[k].TexCoordU = (vp[2]-BoundingMin[2])/ScaleU;
					tp->LightMapCoord[k].TexCoordV = (vp[1]-BoundingMin[1])/ScaleV;
				}
			}
			Light_t Light;
			Light.Range = 300.f;
			Vector(200.f,0.f,200.f,Light.Position);
			Vector(40.f/255.f,80.f/255.f,124.f/255.f,Light.Color);
			CreateLightMapSurface(&Light,m,i,j,MapWidth,MapHeight,MapWidthMax,MapHeightMax,BoundingMin,BoundingMax,Axis);

			Light.Range = 400.f;
			Vector(0.f,0.f,200.f,Light.Position);
			Vector(70.f/255.f,40.f/255.f,10.f/255.f,Light.Color);
			Vector(150.f/255.f,130.f/255.f,100.f/255.f,Light.Color);
			CreateLightMapSurface(&Light,m,i,j,MapWidth,MapHeight,MapWidthMax,MapHeightMax,BoundingMin,BoundingMax,Axis);
         	
			tp->LightMapIndexes = NumLightMaps;
	        NumLightMaps++;
		}
	}*/
}

void BMD::BindLightMaps()
{
	if(LightMapEnable == true) return;

	for(int i=0;i<NumLightMaps;i++)
	{
		Bitmap_t *lmp = &LightMaps[i];
		if(lmp->Buffer != NULL) 
		{
			SmoothBitmap(lmp->Width,lmp->Height,lmp->Buffer);
			SmoothBitmap(lmp->Width,lmp->Height,lmp->Buffer);

			glBindTexture(GL_TEXTURE_2D,i+IndexLightMap);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_MODULATE);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,lmp->Width,lmp->Height,0,GL_RGB,GL_UNSIGNED_BYTE,lmp->Buffer);
		}
	}
	/*glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);*/
	LightMapEnable = true;
}

void BMD::ReleaseLightMaps()
{
	if(LightMapEnable == false) return;
	for(int i=0;i<NumLightMaps;i++)
	{
     	Bitmap_t *lmp = &LightMaps[i];
		if(lmp->Buffer != NULL) 
		{
			delete lmp->Buffer;
			lmp->Buffer = NULL;
		}
	}
    LightMapEnable = false;
}

///////////////////////////////////////////////////////////////////////////////
// render body
///////////////////////////////////////////////////////////////////////////////

void BMD::BeginRender(float Alpha)
{
	glPushMatrix();
    //glTranslatef(BodyOrigin[0],BodyOrigin[1],BodyOrigin[2]);
	//glScalef(BodyScale,BodyScale,BodyScale);
	//glRotatef(-90.f,0.f,0.f,1.f);
}

void BMD::EndRender()
{
    glPopMatrix();
}

#ifdef TEST_STREAM
float TexCoordStream[2048*2];
float ColorStream[2048*3];
float VertexStream[2048*3];
#endif// TEST_STREAM

extern float WorldTime;
extern int WaterTextureNumber;
extern int MoveSceneFrame;

#ifdef MR0
void BMD::RenderMesh(int i,int RenderFlag,float Alpha,int BlendMesh,float BlendMeshLight,float BlendMeshTexCoordU,float BlendMeshTexCoordV,int MeshTexture)
{
	if(i >= NumMeshs || i < 0 ) return;
	
	if(EngineGate::IsOn())
	{
		g_pMeshMachine->AddRender(this, i, RenderFlag, Alpha, BlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV, MeshTexture);
	}
	else
	{
		Mesh_t *m = &Meshs[i];
		if(m->NumTriangles == 0) return;

		float Wave = (int)WorldTime%10000 * 0.0001f;
		//텍스쳐
		int Texture = IndexTexture[m->Texture];
		if(Texture == BITMAP_HIDE)
			return;
		else if(Texture == BITMAP_SKIN)
		{
			if(HideSkin) return;
			Texture = BITMAP_SKIN+Skin;
		}
		else if(Texture == BITMAP_WATER)
		{
			Texture = BITMAP_WATER+WaterTextureNumber;
		}
		else  if( Texture==BITMAP_HAIR )
		{
			if(HideSkin) return;
			Texture = BITMAP_HAIR+(Skin-8);
		}
		if(MeshTexture != -1)
			Texture = MeshTexture;

		bool EnableWave = false;
		int streamMesh = StreamMesh;
		if ( m->m_csTScript!=NULL )
		{
			if ( m->m_csTScript->getStreamMesh() )
			{
				streamMesh = i;
			}
		}
		if((i==BlendMesh||i==streamMesh) && (BlendMeshTexCoordU!=0.f || BlendMeshTexCoordV!=0.f))
    		EnableWave = true;

		bool EnableLight = LightEnable;
		if(i==StreamMesh)
		{
			glColor3fv(BodyLight);
			EnableLight = false;
		}
		else if(EnableLight)
		{
			for(int j=0;j<m->NumNormals;j++)
			{
				VectorScale(BodyLight,IntensityTransform[i][j],LightTransform[i][j]);
			}
		}

		int Render = RenderFlag;
		if((RenderFlag&RENDER_COLOR) == RENDER_COLOR)
		{
    		Render = RENDER_COLOR;
       		if((RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
				EnableAlphaBlend();
			else if((RenderFlag&RENDER_DARK) == RENDER_DARK)
     			EnableAlphaBlendMinus();
			else
				DisableAlphaBlend();

			if ((RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
			{
				DisableDepthTest ();				
			}

			DisableTexture();
			if(Alpha >= 0.99f)
			{
				glColor3fv(BodyLight);
			}
			else
			{
				EnableAlphaTest();
				glColor4f(BodyLight[0],BodyLight[1],BodyLight[2],Alpha);
			}
 		}
		else if ( (RenderFlag&RENDER_CHROME)==RENDER_CHROME     || 
				  (RenderFlag&RENDER_CHROME2)==RENDER_CHROME2   ||
				  (RenderFlag&RENDER_CHROME3)==RENDER_CHROME3   ||
				  (RenderFlag&RENDER_CHROME4)==RENDER_CHROME4   ||
				  (RenderFlag&RENDER_CHROME5)==RENDER_CHROME5   ||
				  (RenderFlag&RENDER_CHROME6)==RENDER_CHROME6	||
				  (RenderFlag&RENDER_CHROME7)==RENDER_CHROME7	||
#ifdef PJH_NEW_CHROME
				  (RenderFlag&RENDER_CHROME8)==RENDER_CHROME8	||
#endif //PJH_NEW_CHROME
				  (RenderFlag&RENDER_METAL)==RENDER_METAL       ||
				  (RenderFlag&RENDER_OIL)==RENDER_OIL
				)
		{
			if ( m->m_csTScript!=NULL )
			{
				if ( m->m_csTScript->getNoneBlendMesh() ) return;
			}
			if(m->NoneBlendMesh )
				return;
   			Render = RENDER_CHROME;
			if ( (RenderFlag&RENDER_CHROME4)==RENDER_CHROME4 )
			{
				Render = RENDER_CHROME4;
			}
#ifdef PJH_NEW_CHROME
			if ( (RenderFlag&RENDER_CHROME8)==RENDER_CHROME8 )
			{
				Render = RENDER_CHROME8;
			}
#endif //PJH_NEW_CHROME
			if ( (RenderFlag&RENDER_OIL)==RENDER_OIL )
			{
				Render = RENDER_OIL;
			}

			float Wave2 = (int)WorldTime%5000 * 0.00024f - 0.4f;

			vec3_t L = { (float)(cos(WorldTime*0.001f)), (float)(sin(WorldTime*0.002f)), 1.f };
			for(int j=0;j<m->NumNormals;j++)
			{
				if ( j>MAX_VERTICES ) break;
				float *Normal = NormalTransform[i][j];

				if((RenderFlag&RENDER_CHROME2)==RENDER_CHROME2)
				{
					g_chrome[j][0] = (Normal[2]+Normal[0])*0.8f + Wave2*2.f;
					g_chrome[j][1] = (Normal[1]+Normal[0])*1.0f + Wave2*3.f;
				}
				else if((RenderFlag&RENDER_CHROME3)==RENDER_CHROME3)
				{
					g_chrome[j][0] = DotProduct ( Normal, LightVector );
					g_chrome[j][1] = 1.f-DotProduct ( Normal, LightVector );
				}
				else if((RenderFlag&RENDER_CHROME4)==RENDER_CHROME4)
				{
					g_chrome[j][0] = DotProduct ( Normal, L );
					g_chrome[j][1] = 1.f-DotProduct ( Normal, L );
					g_chrome[j][1] -= Normal[2]*0.5f + Wave*3.f;
					g_chrome[j][0] += Normal[1]*0.5f + L[1]*3.f;
				}
#ifdef PJH_NEW_CHROME
				else if((RenderFlag&RENDER_CHROME8)==RENDER_CHROME8)
				{
					g_chrome[j][0] = DotProduct ( Normal, L );
					g_chrome[j][1] = 1.f-DotProduct ( Normal, L );
					g_chrome[j][1] += Normal[2]*0.5f + Wave*3.f;
					g_chrome[j][0] += Normal[1]*0.5f + L[1]*3.f;
				}
#endif //PJH_NEW_CHROME
				else if((RenderFlag&RENDER_CHROME5)==RENDER_CHROME5)
				{
					g_chrome[j][0] = DotProduct ( Normal, L );
					g_chrome[j][1] = 1.f-DotProduct ( Normal, L );
					g_chrome[j][1] -= Normal[2]*2.5f + Wave*1.f;
					g_chrome[j][0] += Normal[1]*3.f + L[1]*5.f;
				}
				else if((RenderFlag&RENDER_CHROME6)==RENDER_CHROME6)
				{
					g_chrome[j][0] = (Normal[2]+Normal[0])*0.8f + Wave2*2.f;
					g_chrome[j][1] = (Normal[2]+Normal[0])*0.8f + Wave2*2.f;
				}
				else if((RenderFlag&RENDER_CHROME7)==RENDER_CHROME7)
				{
					g_chrome[j][0] = (Normal[2]+Normal[0])*0.8f + WorldTime * 0.00006f;
					g_chrome[j][1] = (Normal[2]+Normal[0])*0.8f + WorldTime * 0.00006f;
				}
				else if((RenderFlag&RENDER_OIL)==RENDER_OIL)
				{
					g_chrome[j][0] = Normal[0];
					g_chrome[j][1] = Normal[1];
				}
				else if((RenderFlag&RENDER_CHROME)==RENDER_CHROME)
				{
					g_chrome[j][0] = Normal[2]*0.5f + Wave;
					g_chrome[j][1] = Normal[1]*0.5f + Wave*2.f;
				}
				else
				{
					g_chrome[j][0] = Normal[2]*0.5f + 0.2f;
					g_chrome[j][1] = Normal[1]*0.5f + 0.5f;
				}
			}

			if ( (RenderFlag&RENDER_CHROME3)==RENDER_CHROME3
				|| (RenderFlag&RENDER_CHROME4)==RENDER_CHROME4
				|| (RenderFlag&RENDER_CHROME5)==RENDER_CHROME5
				|| (RenderFlag&RENDER_CHROME7)==RENDER_CHROME7
#ifdef PJH_NEW_CHROME
				|| (RenderFlag&RENDER_CHROME8)==RENDER_CHROME8
#endif //PJH_NEW_CHROME
				)
			{
				if ( Alpha < 0.99f)
				{
					BodyLight[0] *= Alpha; BodyLight[1] *= Alpha; BodyLight[2] *= Alpha;
				}
     			EnableAlphaBlend();
			}
			else if((RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
			{
				if ( Alpha < 0.99f)
				{
					BodyLight[0] *= Alpha; BodyLight[1] *= Alpha; BodyLight[2] *= Alpha;
				}
     			EnableAlphaBlend();
			}
			else if((RenderFlag&RENDER_DARK) == RENDER_DARK)
     			EnableAlphaBlendMinus();
     		else if((RenderFlag&RENDER_LIGHTMAP) == RENDER_LIGHTMAP)
				EnableLightMap();
			else if ( Alpha >= 0.99f)
			{
				DisableAlphaBlend();
			}
			else
			{
				EnableAlphaTest();
			}

			if ((RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
			{
				DisableDepthTest ();				
			}

			if((RenderFlag&RENDER_CHROME2)==RENDER_CHROME2 && MeshTexture==-1)
			{
				BindTexture(BITMAP_CHROME2);
			}
			else if((RenderFlag&RENDER_CHROME3)==RENDER_CHROME3 && MeshTexture==-1)
			{
				BindTexture(BITMAP_CHROME2);
			}
			else if((RenderFlag&RENDER_CHROME4)==RENDER_CHROME4 && MeshTexture==-1)
			{
				BindTexture(BITMAP_CHROME2);
			}
#ifdef PJH_NEW_CHROME
			else if((RenderFlag&RENDER_CHROME8)==RENDER_CHROME8 && MeshTexture==-1)
			{
				BindTexture(BITMAP_CHROME9);
			}
#endif //PJH_NEW_CHROME
			else if((RenderFlag&RENDER_CHROME6)==RENDER_CHROME6 && MeshTexture==-1)
			{
				BindTexture(BITMAP_CHROME6);
			}
			else if((RenderFlag&RENDER_CHROME)==RENDER_CHROME && MeshTexture==-1)
				BindTexture(BITMAP_CHROME);
			else if((RenderFlag&RENDER_METAL)==RENDER_METAL && MeshTexture==-1)
				BindTexture(BITMAP_SHINY);
			else
				BindTexture(Texture);
		}	
		else if(BlendMesh<=-2 || m->Texture == BlendMesh)
		{
    		Render = RENDER_TEXTURE;
   			BindTexture(Texture);
			if((RenderFlag&RENDER_DARK) == RENDER_DARK)
     			EnableAlphaBlendMinus();
			else
     			EnableAlphaBlend();

			if ((RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
			{
				DisableDepthTest ();				
			}

			glColor3f(BodyLight[0]*BlendMeshLight,BodyLight[1]*BlendMeshLight,BodyLight[2]*BlendMeshLight);
			EnableLight = false;
		}
		else if((RenderFlag&RENDER_TEXTURE) == RENDER_TEXTURE)
		{
    		Render = RENDER_TEXTURE;
			BindTexture(Texture);
			if((RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
     			EnableAlphaBlend();
			else if((RenderFlag&RENDER_DARK) == RENDER_DARK)
     			EnableAlphaBlendMinus();
			else if(Alpha<0.99f || Bitmaps[Texture].Components==4)
				EnableAlphaTest();
			else
				DisableAlphaBlend();

			if ((RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
			{
				DisableDepthTest ();				
			}
		}
		else if((RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
		{
			if(Bitmaps[Texture].Components==4 || m->Texture == BlendMesh)
				return;
			
    		Render = RENDER_BRIGHT;
			EnableAlphaBlend();
			DisableTexture();
			DisableDepthMask();

			if ((RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
			{
				DisableDepthTest ();				
			}
		}
		else
		{
    		Render = RENDER_TEXTURE;
		}

		// ver 1.0 (triangle)
		glBegin(GL_TRIANGLES);
		for(int j=0;j<m->NumTriangles;j++)
		{
			Triangle_t *tp = &m->Triangles[j];
			for(int k=0;k<tp->Polygon;k++)
			{
				int vi = tp->VertexIndex[k];  
				switch(Render)
				{

				case RENDER_TEXTURE:
					{
						TexCoord_t *texp = &m->TexCoords[tp->TexCoordIndex[k]];
						if(EnableWave)
						{
							glTexCoord2f(texp->TexCoordU+BlendMeshTexCoordU,texp->TexCoordV+BlendMeshTexCoordV);
						}
						else
						{
							glTexCoord2f(texp->TexCoordU,texp->TexCoordV);
						}
						
						if(EnableLight)
						{
							int ni = tp->NormalIndex[k];
		
							if(Alpha >= 0.99f)
							{
								glColor3fv(LightTransform[i][ni]);
							}
							else
							{
								float *Light = LightTransform[i][ni];
								glColor4f(Light[0],Light[1],Light[2],Alpha);
							}
						}
						break;
					}
				case RENDER_CHROME:
					{
						if(Alpha >= 0.99f)
							glColor3fv(BodyLight);
						else
							glColor4f(BodyLight[0],BodyLight[1],BodyLight[2],Alpha);
						int ni = tp->NormalIndex[k];  
						glTexCoord2f(g_chrome[ni][0],g_chrome[ni][1]);
						break;
					}
#ifdef PJH_NEW_CHROME
				case RENDER_CHROME8:
					{
						if(Alpha >= 0.99f)
							glColor3fv(BodyLight);
						else
							glColor4f(BodyLight[0],BodyLight[1],BodyLight[2],Alpha);
						int ni = tp->NormalIndex[k];  
						glTexCoord2f(g_chrome[ni][0]+BlendMeshTexCoordU,g_chrome[ni][1]+BlendMeshTexCoordV);
						break;
					}
#endif //PJH_NEW_CHROME
				case RENDER_CHROME4:
					{
						if(Alpha >= 0.99f)
							glColor3fv(BodyLight);
						else
							glColor4f(BodyLight[0],BodyLight[1],BodyLight[2],Alpha);
						int ni = tp->NormalIndex[k];  
						glTexCoord2f(g_chrome[ni][0]+BlendMeshTexCoordU,g_chrome[ni][1]+BlendMeshTexCoordV);
					}
					break;

				case RENDER_OIL:
					{
						if(Alpha >= 0.99f)
							glColor3fv(BodyLight);
						else
							glColor4f(BodyLight[0],BodyLight[1],BodyLight[2],Alpha);
						TexCoord_t *texp = &m->TexCoords[tp->TexCoordIndex[k]];
						int ni = tp->VertexIndex[k];  
						glTexCoord2f(g_chrome[ni][0]*texp->TexCoordU+BlendMeshTexCoordU,g_chrome[ni][1]*texp->TexCoordV+BlendMeshTexCoordV);
						break;
					}
				}

				if ( (RenderFlag&RENDER_SHADOWMAP)==RENDER_SHADOWMAP )
				{
					int vi = tp->VertexIndex[k];  
					vec3_t Position;
					VectorSubtract(VertexTransform[i][vi],BodyOrigin,Position);

					Position[0] += Position[2]*(Position[0]+2000.f)/(Position[2]-4000.f);
					Position[2] = 5.f;
                
					VectorAdd(Position,BodyOrigin,Position);
					glVertex3fv(Position);
				}
				else if((RenderFlag&RENDER_WAVE)==RENDER_WAVE)
				{
					float vPos[3];
					float fParam = ( float)( ( int)WorldTime + vi * 931)*0.007f;
					float fSin = sinf( fParam);
					float fCos = cosf( fParam);

					int ni = tp->NormalIndex[k];
					Normal_t *np = &m->Normals[ni];
					float *Normal = NormalTransform[i][ni];
					for ( int iCoord = 0; iCoord < 3; ++iCoord)
					{
						vPos[iCoord] = VertexTransform[i][vi][iCoord] + Normal[iCoord]*fSin*28.0f;
					}
					glVertex3fv(vPos);
				}
				else
				{
					glVertex3fv(VertexTransform[i][vi]);
				}
			}
		}
		glEnd();
	}
}

#else
void BMD::RenderMesh(int i,int RenderFlag,float Alpha,int BlendMesh,float BlendMeshLight,float BlendMeshTexCoordU,float BlendMeshTexCoordV,int MeshTexture)
{
    if ( i>=NumMeshs || i<0 ) return;

    Mesh_t *m = &Meshs[i];
	if(m->NumTriangles == 0) return;

	float Wave = (int)WorldTime%10000 * 0.0001f;
	//텍스쳐
	int Texture = IndexTexture[m->Texture];
    if(Texture == BITMAP_HIDE)
		return;
    else if(Texture == BITMAP_SKIN)
	{
		if(HideSkin) return;
		Texture = BITMAP_SKIN+Skin;
	}
	else if(Texture == BITMAP_WATER)
	{
	    Texture = BITMAP_WATER+WaterTextureNumber;
	}
    else  if( Texture==BITMAP_HAIR )
    {
		if(HideSkin) return;
        Texture = BITMAP_HAIR+(Skin-8);
    }
#ifdef YDG_ADD_DOPPELGANGER_MONSTER
// 	if (RenderFlag & RENDER_DOPPELGANGER)
// 	{
// 		Texture = BITMAP_CLOUD;
// 	}
#endif	// YDG_ADD_DOPPELGANGER_MONSTER
	if(MeshTexture != -1)
		Texture = MeshTexture;

	BITMAP_t* pBitmap = Bitmaps.GetTexture(Texture);

	bool EnableWave = false;
    int streamMesh = StreamMesh;
    if ( m->m_csTScript!=NULL )
    {
        if ( m->m_csTScript->getStreamMesh() )
        {
            streamMesh = i;
        }
    }
	if((i==BlendMesh||i==streamMesh) && (BlendMeshTexCoordU!=0.f || BlendMeshTexCoordV!=0.f))
    	EnableWave = true;

//	EnableWave = true;
	//박종훈...
	bool EnableLight = LightEnable;
	if(i==StreamMesh)
	{
		//vec3_t Light;
		//Vector(1.f,1.f,1.f,Light);
		glColor3fv(BodyLight);
		EnableLight = false;
	}
	else if(EnableLight)
	{
		for(int j=0;j<m->NumNormals;j++)
		{
			VectorScale(BodyLight,IntensityTransform[i][j],LightTransform[i][j]);
		}
	}

	int Render = RenderFlag;
	if((RenderFlag&RENDER_COLOR) == RENDER_COLOR)
	{
    	Render = RENDER_COLOR;
       	if((RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
			EnableAlphaBlend();
		else if((RenderFlag&RENDER_DARK) == RENDER_DARK)
     		EnableAlphaBlendMinus();
		else
			DisableAlphaBlend();

        if ((RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
        {
            DisableDepthTest ();				
        }

        DisableTexture();
		if(Alpha >= 0.99f)
        {
            glColor3fv(BodyLight);
        }
		else
        {
            EnableAlphaTest();
            glColor4f(BodyLight[0],BodyLight[1],BodyLight[2],Alpha);
        }
 	}
	else if ( (RenderFlag&RENDER_CHROME)==RENDER_CHROME     || 
              (RenderFlag&RENDER_CHROME2)==RENDER_CHROME2   ||
              (RenderFlag&RENDER_CHROME3)==RENDER_CHROME3   ||
              (RenderFlag&RENDER_CHROME4)==RENDER_CHROME4   ||
              (RenderFlag&RENDER_CHROME5)==RENDER_CHROME5   ||
			  (RenderFlag&RENDER_CHROME6)==RENDER_CHROME6	||
			  (RenderFlag&RENDER_CHROME7)==RENDER_CHROME7	||
#ifdef PJH_NEW_CHROME
			  (RenderFlag&RENDER_CHROME8)==RENDER_CHROME8	||
#endif //#ifdef PJH_NEW_CHROME
              (RenderFlag&RENDER_METAL)==RENDER_METAL       ||
              (RenderFlag&RENDER_OIL)==RENDER_OIL
            )
	{
		if ( m->m_csTScript!=NULL )
        {
            if ( m->m_csTScript->getNoneBlendMesh() ) return;
        }

		if(m->NoneBlendMesh )
			return;

   		Render = RENDER_CHROME;
        if ( (RenderFlag&RENDER_CHROME4)==RENDER_CHROME4 )
        {
            Render = RENDER_CHROME4;
        }
#ifdef PJH_NEW_CHROME
        if ( (RenderFlag&RENDER_CHROME8)==RENDER_CHROME8 )
        {
            Render = RENDER_CHROME8;
        }
#endif //PJH_NEW_CHROME
        if ( (RenderFlag&RENDER_OIL)==RENDER_OIL )
        {
            Render = RENDER_OIL;
        }

        float Wave2 = (int)WorldTime%5000 * 0.00024f - 0.4f;

        vec3_t L = { (float)(cos(WorldTime*0.001f)), (float)(sin(WorldTime*0.002f)), 1.f };
		for(int j=0;j<m->NumNormals;j++)
		{
            if ( j>MAX_VERTICES ) break;
			float *Normal = NormalTransform[i][j];

            if((RenderFlag&RENDER_CHROME2)==RENDER_CHROME2)
			{
				g_chrome[j][0] = (Normal[2]+Normal[0])*0.8f + Wave2*2.f;
				g_chrome[j][1] = (Normal[1]+Normal[0])*1.0f + Wave2*3.f;
			}
            else if((RenderFlag&RENDER_CHROME3)==RENDER_CHROME3)
            {
                g_chrome[j][0] = DotProduct ( Normal, LightVector );
                g_chrome[j][1] = 1.f-DotProduct ( Normal, LightVector );
            }
            else if((RenderFlag&RENDER_CHROME4)==RENDER_CHROME4)
            {
                g_chrome[j][0] = DotProduct ( Normal, L );
                g_chrome[j][1] = 1.f-DotProduct ( Normal, L );
				g_chrome[j][1] -= Normal[2]*0.5f + Wave*3.f;
				g_chrome[j][0] += Normal[1]*0.5f + L[1]*3.f;
            }
            else if((RenderFlag&RENDER_CHROME5)==RENDER_CHROME5)
            {
                g_chrome[j][0] = DotProduct ( Normal, L );
                g_chrome[j][1] = 1.f-DotProduct ( Normal, L );
				g_chrome[j][1] -= Normal[2]*2.5f + Wave*1.f;
				g_chrome[j][0] += Normal[1]*3.f + L[1]*5.f;
            }
            else if((RenderFlag&RENDER_CHROME6)==RENDER_CHROME6)
			{
				g_chrome[j][0] = (Normal[2]+Normal[0])*0.8f + Wave2*2.f;
				g_chrome[j][1] = (Normal[2]+Normal[0])*0.8f + Wave2*2.f;
			}
            else if((RenderFlag&RENDER_CHROME7)==RENDER_CHROME7)
			{
				g_chrome[j][0] = (Normal[2]+Normal[0])*0.8f + WorldTime * 0.00006f;
				g_chrome[j][1] = (Normal[2]+Normal[0])*0.8f + WorldTime * 0.00006f;
			}
#ifdef PJH_NEW_CHROME
			
            else if((RenderFlag&RENDER_CHROME8)==RENDER_CHROME8)
            {
				g_chrome[j][0] = Normal[0];
				g_chrome[j][1] = Normal[1];
            }
			
#endif //PJH_NEW_CHROME
            else if((RenderFlag&RENDER_OIL)==RENDER_OIL)
            {
				g_chrome[j][0] = Normal[0];
				g_chrome[j][1] = Normal[1];
            }
            else if((RenderFlag&RENDER_CHROME)==RENDER_CHROME)
			{
				g_chrome[j][0] = Normal[2]*0.5f + Wave;
				g_chrome[j][1] = Normal[1]*0.5f + Wave*2.f;
			}
			else
			{
				g_chrome[j][0] = Normal[2]*0.5f + 0.2f;
				g_chrome[j][1] = Normal[1]*0.5f + 0.5f;
			}
		}

        if ( (RenderFlag&RENDER_CHROME3)==RENDER_CHROME3
			|| (RenderFlag&RENDER_CHROME4)==RENDER_CHROME4
			|| (RenderFlag&RENDER_CHROME5)==RENDER_CHROME5
			|| (RenderFlag&RENDER_CHROME7)==RENDER_CHROME7
#ifdef PJH_NEW_CHROME
			|| (RenderFlag&RENDER_CHROME8)==RENDER_CHROME8
#endif //PJH_NEW_CHROME
			)
        {
			if ( Alpha < 0.99f)
			{
				BodyLight[0] *= Alpha; BodyLight[1] *= Alpha; BodyLight[2] *= Alpha;
			}
     		EnableAlphaBlend();
        }
        else if((RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
		{
			if ( Alpha < 0.99f)
			{
				BodyLight[0] *= Alpha; BodyLight[1] *= Alpha; BodyLight[2] *= Alpha;
			}
     		EnableAlphaBlend();
		}
		else if((RenderFlag&RENDER_DARK) == RENDER_DARK)
     		EnableAlphaBlendMinus();
     	else if((RenderFlag&RENDER_LIGHTMAP) == RENDER_LIGHTMAP)
            EnableLightMap();
		else if ( Alpha >= 0.99f)
		{
			DisableAlphaBlend();
		}
		else
		{
			EnableAlphaTest();
		}

        if ((RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
        {
            DisableDepthTest ();				
        }

        if((RenderFlag&RENDER_CHROME2)==RENDER_CHROME2 && MeshTexture==-1)
        {
			BindTexture(BITMAP_CHROME2);
        }
        else if((RenderFlag&RENDER_CHROME3)==RENDER_CHROME3 && MeshTexture==-1)
        {
			BindTexture(BITMAP_CHROME2);
        }
        else if((RenderFlag&RENDER_CHROME4)==RENDER_CHROME4 && MeshTexture==-1)
        {
			BindTexture(BITMAP_CHROME2);
        }
#ifdef PJH_NEW_CHROME
        else if((RenderFlag&RENDER_CHROME8)==RENDER_CHROME8 && MeshTexture==-1)
        {
			BindTexture(BITMAP_CHROME9);
        }
#endif //PJH_NEW_CHROME
        else if((RenderFlag&RENDER_CHROME6)==RENDER_CHROME6 && MeshTexture==-1)
        {
			BindTexture(BITMAP_CHROME6);
        }
        else if((RenderFlag&RENDER_CHROME)==RENDER_CHROME && MeshTexture==-1)
			BindTexture(BITMAP_CHROME);
		else if((RenderFlag&RENDER_METAL)==RENDER_METAL && MeshTexture==-1)
			BindTexture(BITMAP_SHINY);
		else
			BindTexture(Texture);
	}	
	else if(BlendMesh<=-2 || m->Texture == BlendMesh)
	{
    	Render = RENDER_TEXTURE;
   		BindTexture(Texture);
		if((RenderFlag&RENDER_DARK) == RENDER_DARK)
     		EnableAlphaBlendMinus();
		else
     		EnableAlphaBlend();

        if ((RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
        {
            DisableDepthTest ();				
        }

		glColor3f(BodyLight[0]*BlendMeshLight,BodyLight[1]*BlendMeshLight,BodyLight[2]*BlendMeshLight);
		//glColor3f(BlendMeshLight,BlendMeshLight,BlendMeshLight);
		EnableLight = false;
	}
	else if((RenderFlag&RENDER_TEXTURE) == RENDER_TEXTURE)
	{
    	Render = RENDER_TEXTURE;
		BindTexture(Texture);
		if((RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
		{
     		EnableAlphaBlend();
		}
		else if((RenderFlag&RENDER_DARK) == RENDER_DARK)
		{
     		EnableAlphaBlendMinus();
		}
		else if(Alpha<0.99f || pBitmap->Components==4)
		{
			EnableAlphaTest();
		}
		else
		{
			DisableAlphaBlend();
		}

        if ((RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
        {
            DisableDepthTest ();				
        }
	}
	else if((RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
	{
		if(pBitmap->Components==4 || m->Texture == BlendMesh)		
		{
			return;
		}
		
    	Render = RENDER_BRIGHT;
        EnableAlphaBlend();
        DisableTexture();
        DisableDepthMask();

        if ((RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
        {
            DisableDepthTest ();				
        }
	}
	else
	{
    	Render = RENDER_TEXTURE;
	}
#ifdef YDG_ADD_DOPPELGANGER_MONSTER
	if (RenderFlag & RENDER_DOPPELGANGER)
	{
		if (pBitmap->Components!=4)
		{
			EnableCullFace();
			EnableDepthMask();
		}
	}
#endif	// YDG_ADD_DOPPELGANGER_MONSTER

    // ver 1.0 (triangle)
	glBegin(GL_TRIANGLES);
	for(int j=0;j<m->NumTriangles;j++)
	{
		Triangle_t *tp = &m->Triangles[j];
		for(int k=0;k<tp->Polygon;k++)
		{
			int vi = tp->VertexIndex[k];  
			switch(Render)
			{

			case RENDER_TEXTURE:
				{
					TexCoord_t *texp = &m->TexCoords[tp->TexCoordIndex[k]];
					if(EnableWave)
					{
						glTexCoord2f(texp->TexCoordU+BlendMeshTexCoordU,texp->TexCoordV+BlendMeshTexCoordV);
					}
					else
					{
						glTexCoord2f(texp->TexCoordU,texp->TexCoordV);
					}
					
					if(EnableLight)
					{
						int ni = tp->NormalIndex[k];
	
						if(Alpha >= 0.99f)
						{
							glColor3fv(LightTransform[i][ni]);
						}
						else
						{
							float *Light = LightTransform[i][ni];
							glColor4f(Light[0],Light[1],Light[2],Alpha);
						}
					}
					break;
				}
			case RENDER_CHROME:
				{
					if(Alpha >= 0.99f)
						glColor3fv(BodyLight);
					else
						glColor4f(BodyLight[0],BodyLight[1],BodyLight[2],Alpha);
					int ni = tp->NormalIndex[k];  
					glTexCoord2f(g_chrome[ni][0],g_chrome[ni][1]);
					break;
				}
#ifdef PJH_NEW_CHROME
				case RENDER_CHROME8:
					{
						if(Alpha >= 0.99f)
							glColor3fv(BodyLight);
						else
							glColor4f(BodyLight[0],BodyLight[1],BodyLight[2],Alpha);
						int ni = tp->NormalIndex[k];  
//						glTexCoord2f(g_chrome[ni][0],g_chrome[ni][1]);
						glTexCoord2f(g_chrome[ni][0]+BlendMeshTexCoordU,g_chrome[ni][1]+BlendMeshTexCoordV);
						break;
					}
#endif //PJH_NEW_CHROME
            case RENDER_CHROME4:
				{
					if(Alpha >= 0.99f)
						glColor3fv(BodyLight);
					else
						glColor4f(BodyLight[0],BodyLight[1],BodyLight[2],Alpha);
					int ni = tp->NormalIndex[k];  
					glTexCoord2f(g_chrome[ni][0]+BlendMeshTexCoordU,g_chrome[ni][1]+BlendMeshTexCoordV);
//					glTexCoord2f(BlendMeshTexCoordU,BlendMeshTexCoordV);
				}
                break;

			case RENDER_OIL:
				{
					if(Alpha >= 0.99f)
						glColor3fv(BodyLight);
					else
						glColor4f(BodyLight[0],BodyLight[1],BodyLight[2],Alpha);
					TexCoord_t *texp = &m->TexCoords[tp->TexCoordIndex[k]];
					int ni = tp->VertexIndex[k];  
					glTexCoord2f(g_chrome[ni][0]*texp->TexCoordU+BlendMeshTexCoordU,g_chrome[ni][1]*texp->TexCoordV+BlendMeshTexCoordV);
					break;
				}
			}

            if ( (RenderFlag&RENDER_SHADOWMAP)==RENDER_SHADOWMAP )
            {
                int vi = tp->VertexIndex[k];  
                vec3_t Position;
                VectorSubtract(VertexTransform[i][vi],BodyOrigin,Position);

                Position[0] += Position[2]*(Position[0]+2000.f)/(Position[2]-4000.f);
                Position[2] = 5.f;
                
                VectorAdd(Position,BodyOrigin,Position);
                glVertex3fv(Position);
            }
			else if((RenderFlag&RENDER_WAVE)==RENDER_WAVE)
			{
				float vPos[3];
				float fParam = ( float)( ( int)WorldTime + vi * 931)*0.007f;
				float fSin = sinf( fParam);
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
				float fCos = cosf( fParam);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

				int ni = tp->NormalIndex[k];
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
				Normal_t *np = &m->Normals[ni];
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
				float *Normal = NormalTransform[i][ni];
				for ( int iCoord = 0; iCoord < 3; ++iCoord)
				{
					vPos[iCoord] = VertexTransform[i][vi][iCoord] + Normal[iCoord]*fSin*28.0f;
				}
				glVertex3fv(vPos);
			}
            else
			{
				glVertex3fv(VertexTransform[i][vi]);
			}
		}
	}
	glEnd();
}
#endif //MRO	- RenderMesh 함수 변형

void BMD::RenderMeshAlternative( int iRndExtFlag, int iParam, int i,int RenderFlag,float Alpha,int BlendMesh,float BlendMeshLight,float BlendMeshTexCoordU,float BlendMeshTexCoordV,int MeshTexture)
{
    if ( i>=NumMeshs || i<0 ) return;

    Mesh_t *m = &Meshs[i];
	if(m->NumTriangles == 0) return;
	float Wave = (int)WorldTime%10000 * 0.0001f;

	//텍스쳐
	int Texture = IndexTexture[m->Texture];
	if(Texture == BITMAP_HIDE)
		return;
	if(MeshTexture != -1)
		Texture = MeshTexture;

	BITMAP_t* pBitmap = Bitmaps.GetTexture(Texture);

	bool EnableWave = false;
    int streamMesh = StreamMesh;
    if ( m->m_csTScript!=NULL )
    {
        if ( m->m_csTScript->getStreamMesh() )
        {
            streamMesh = i;
        }
    }
	if((i==BlendMesh||i==streamMesh) && (BlendMeshTexCoordU!=0.f || BlendMeshTexCoordV!=0.f))
    	EnableWave = true;

	bool EnableLight = LightEnable;
	if(i==StreamMesh)
	{
		//vec3_t Light;
		//Vector(1.f,1.f,1.f,Light);
		glColor3fv(BodyLight);
		EnableLight = false;
	}
	else if(EnableLight)
	{
		for(int j=0;j<m->NumNormals;j++)
		{
			VectorScale(BodyLight,IntensityTransform[i][j],LightTransform[i][j]);
		}
	}

	int Render = RenderFlag;
	if((RenderFlag&RENDER_COLOR) == RENDER_COLOR)
	{
    	Render = RENDER_COLOR;
       	if((RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
			EnableAlphaBlend();
		else if((RenderFlag&RENDER_DARK) == RENDER_DARK)
     		EnableAlphaBlendMinus();
		else
			DisableAlphaBlend();

        if ((RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
        {
            DisableDepthTest ();				
        }

        DisableTexture();
		if(Alpha >= 0.99f)
        {
            glColor3fv(BodyLight);
        }
		else
        {
            EnableAlphaTest();
            glColor4f(BodyLight[0],BodyLight[1],BodyLight[2],Alpha);
        }
 	}
	else if ( (RenderFlag&RENDER_CHROME)==RENDER_CHROME     || 
              (RenderFlag&RENDER_CHROME2)==RENDER_CHROME2   ||
              (RenderFlag&RENDER_CHROME3)==RENDER_CHROME3   ||
              (RenderFlag&RENDER_CHROME4)==RENDER_CHROME4   ||
              (RenderFlag&RENDER_CHROME5)==RENDER_CHROME5   ||
				(RenderFlag&RENDER_CHROME7)==RENDER_CHROME7   ||
#ifdef PJH_NEW_CHROME
				(RenderFlag&RENDER_CHROME8)==RENDER_CHROME8   ||
#endif // PJH_NEW_CHROME
              (RenderFlag&RENDER_METAL)==RENDER_METAL       ||
              (RenderFlag&RENDER_OIL)==RENDER_OIL
            )
	{
		if ( m->m_csTScript!=NULL )
        {
            if ( m->m_csTScript->getNoneBlendMesh() ) return;
        }
		if(m->NoneBlendMesh )
			return;
   		Render = RENDER_CHROME;
        if ( (RenderFlag&RENDER_CHROME4)==RENDER_CHROME4 )
        {
            Render = RENDER_CHROME4;
        }
#ifdef PJH_NEW_CHROME
        if ( (RenderFlag&RENDER_CHROME8)==RENDER_CHROME8 )
        {
            Render = RENDER_CHROME8;
        }
#endif //PJH_NEW_CHROME
        float Wave2 = (int)WorldTime%5000 * 0.00024f - 0.4f;

        vec3_t L = { (float)(cos(WorldTime*0.001f)), (float)(sin(WorldTime*0.002f)), 1.f };
		for(int j=0;j<m->NumNormals;j++)
		{
            if ( j>MAX_VERTICES ) break;
			float *Normal = NormalTransform[i][j];

            if((RenderFlag&RENDER_CHROME2)==RENDER_CHROME2)
			{
				g_chrome[j][0] = (Normal[2]+Normal[0])*0.8f + Wave2*2.f;
				g_chrome[j][1] = (Normal[1]+Normal[0])*1.0f + Wave2*3.f;
			}
            else if((RenderFlag&RENDER_CHROME3)==RENDER_CHROME3)
            {
                g_chrome[j][0] = DotProduct ( Normal, LightVector );
                g_chrome[j][1] = 1.f-DotProduct ( Normal, LightVector );
            }
            else if((RenderFlag&RENDER_CHROME4)==RENDER_CHROME4)
            {
                g_chrome[j][0] = DotProduct ( Normal, L );
                g_chrome[j][1] = 1.f-DotProduct ( Normal, L );
				g_chrome[j][1] -= Normal[2]*0.5f + Wave*3.f;
				g_chrome[j][0] += Normal[1]*0.5f + L[1]*3.f;
            }
#ifdef PJH_NEW_CHROME
            else if((RenderFlag&RENDER_CHROME8)==RENDER_CHROME8)
            {
                g_chrome[j][0] = DotProduct ( Normal, L );
                g_chrome[j][1] = 1.f-DotProduct ( Normal, L );
				g_chrome[j][1] += Normal[2]*0.5f + Wave*3.f;
				g_chrome[j][0] += Normal[1]*0.5f + L[1]*3.f;
            }
#endif //#ifdef PJH_NEW_CHROME
            else if((RenderFlag&RENDER_CHROME5)==RENDER_CHROME5)
            {
                Vector ( 0.1f, -0.23f, 0.22f, LightVector2 );

                g_chrome[j][0] = ( DotProduct ( Normal, LightVector2 ) /*+ Normal[1] + LightVector2[1]*3.f */) / 1.08f;
                g_chrome[j][1] = ( 1.f-DotProduct ( Normal, LightVector2 ) /*- Normal[2]*0.5f + 3.f */) / 1.08f;
            }
            else if((RenderFlag&RENDER_CHROME6)==RENDER_CHROME6)
			{
				g_chrome[j][0] = (Normal[2]+Normal[0])*0.8f + Wave2*2.f;
				g_chrome[j][1] = (Normal[1]+Normal[0])*1.0f + Wave2*3.f;
			}
			else if((RenderFlag&RENDER_CHROME7)==RENDER_CHROME7)
			{
				Vector ( 0.1f, -0.23f, 0.22f, LightVector2 );

                g_chrome[j][0] = ( DotProduct ( Normal, LightVector2 ) ) / 1.08f;
                g_chrome[j][1] = ( 1.f-DotProduct ( Normal, LightVector2 ) ) / 1.08f;
			}
            else if((RenderFlag&RENDER_CHROME)==RENDER_CHROME)
			{
				g_chrome[j][0] = Normal[2]*0.5f + Wave;
				g_chrome[j][1] = Normal[1]*0.5f + Wave*2.f;
			}
			else
			{
				g_chrome[j][0] = Normal[2]*0.5f + 0.2f;
				g_chrome[j][1] = Normal[1]*0.5f + 0.5f;
			}
		}

        if ( (RenderFlag&RENDER_CHROME3)==RENDER_CHROME3
			|| (RenderFlag&RENDER_CHROME4)==RENDER_CHROME4
			|| (RenderFlag&RENDER_CHROME5)==RENDER_CHROME5
			|| (RenderFlag&RENDER_CHROME7)==RENDER_CHROME7 
#ifdef PJH_NEW_CHROME
			|| (RenderFlag&RENDER_CHROME8)==RENDER_CHROME8 
#endif //PJH_NEW_CHROME
		   )
        {
			if ( Alpha < 0.99f)
			{
				BodyLight[0] *= Alpha; BodyLight[1] *= Alpha; BodyLight[2] *= Alpha;
			}
     		EnableAlphaBlend();
        }
        else if((RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
		{
			if ( Alpha < 0.99f)
			{
				BodyLight[0] *= Alpha; BodyLight[1] *= Alpha; BodyLight[2] *= Alpha;
			}
     		EnableAlphaBlend();
		}
		else if((RenderFlag&RENDER_DARK) == RENDER_DARK)
     		EnableAlphaBlendMinus();
     	else if((RenderFlag&RENDER_LIGHTMAP) == RENDER_LIGHTMAP)
            EnableLightMap();
		else if ( Alpha >= 0.99f)
		{
			DisableAlphaBlend();
		}
		else
		{
			EnableAlphaTest();
		}

        if ((RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
        {
            DisableDepthTest ();				
        }

        if((RenderFlag&RENDER_CHROME2)==RENDER_CHROME2 && MeshTexture==-1)
        {
			BindTexture(BITMAP_CHROME2);
        }
        else if((RenderFlag&RENDER_CHROME3)==RENDER_CHROME3 && MeshTexture==-1)
        {
			BindTexture(BITMAP_CHROME2);
        }
        else if((RenderFlag&RENDER_CHROME4)==RENDER_CHROME4 && MeshTexture==-1)
        {
			BindTexture(BITMAP_CHROME2);
        }
#ifdef PJH_NEW_CHROME
        else if((RenderFlag&RENDER_CHROME8)==RENDER_CHROME8 && MeshTexture==-1)
        {
			BindTexture(BITMAP_CHROME9);
        }
#endif //PJH_NEW_CHROME
        else if((RenderFlag&RENDER_CHROME)==RENDER_CHROME && MeshTexture==-1)
			BindTexture(BITMAP_CHROME);
		else if((RenderFlag&RENDER_METAL)==RENDER_METAL && MeshTexture==-1)
			BindTexture(BITMAP_SHINY);
		else
			BindTexture(Texture);
	}	
	else if(BlendMesh<=-2 || m->Texture == BlendMesh)
	{
    	Render = RENDER_TEXTURE;
   		BindTexture(Texture);
		if((RenderFlag&RENDER_DARK) == RENDER_DARK)
     		EnableAlphaBlendMinus();
		else
     		EnableAlphaBlend();

        if ((RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
        {
            DisableDepthTest ();				
        }

		glColor3f(BodyLight[0]*BlendMeshLight,BodyLight[1]*BlendMeshLight,BodyLight[2]*BlendMeshLight);
		//glColor3f(BlendMeshLight,BlendMeshLight,BlendMeshLight);
		EnableLight = false;
	}
	else if((RenderFlag&RENDER_TEXTURE) == RENDER_TEXTURE)
	{
    	Render = RENDER_TEXTURE;
		BindTexture(Texture);
		if((RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
		{
     		EnableAlphaBlend();
		}
		else if((RenderFlag&RENDER_DARK) == RENDER_DARK)
		{
     		EnableAlphaBlendMinus();
		}
		else if(Alpha<0.99f || pBitmap->Components==4)
		{
			EnableAlphaTest();
		}
		else
		{
			DisableAlphaBlend();
		}

        if ((RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
        {
            DisableDepthTest ();				
        }
	}
	else if((RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
	{
		if(pBitmap->Components==4 || m->Texture == BlendMesh)
		{
			return;
		}
    	Render = RENDER_BRIGHT;
        EnableAlphaBlend();
        DisableTexture();
        DisableDepthMask();

        if ((RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
        {
            DisableDepthTest ();				
        }
	}
	else
	{
    	Render = RENDER_TEXTURE;
	}

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	int iFrame = MoveSceneFrame;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	
	// ver 1.0 (triangle)
	glBegin(GL_TRIANGLES);
	for(int j=0;j<m->NumTriangles;j++)
	{
		Triangle_t *tp = &m->Triangles[j];
		for(int k=0;k<tp->Polygon;k++)
		{
			int vi = tp->VertexIndex[k];  
			switch(Render)
			{
			case RENDER_TEXTURE:
				{
					TexCoord_t *texp = &m->TexCoords[tp->TexCoordIndex[k]];
					if(EnableWave)
						glTexCoord2f(texp->TexCoordU+BlendMeshTexCoordU,texp->TexCoordV+BlendMeshTexCoordV);
					else
						glTexCoord2f(texp->TexCoordU,texp->TexCoordV);
					if(EnableLight)
					{
						int ni = tp->NormalIndex[k];
						if(Alpha >= 0.99f)
						{
							glColor3fv(LightTransform[i][ni]);
						}
						else
						{
							float *Light = LightTransform[i][ni];
							glColor4f(Light[0],Light[1],Light[2],Alpha);
						}
					}
					break;
				}
			case RENDER_CHROME:
				{
					if(Alpha >= 0.99f)
						glColor3fv(BodyLight);
					else
						glColor4f(BodyLight[0],BodyLight[1],BodyLight[2],Alpha);
					int ni = tp->NormalIndex[k];  
					glTexCoord2f(g_chrome[ni][0],g_chrome[ni][1]);
					break;
				}
			}
			if ( (iRndExtFlag&RNDEXT_WAVE) )
			{
				float vPos[3];
				float fParam = ( float)( ( int)WorldTime + vi * 931)*0.007f;
				float fSin = sinf( fParam);
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
				float fCos = cosf( fParam);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

				int ni = tp->NormalIndex[k];
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
				Normal_t *np = &m->Normals[ni];
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
				float *Normal = NormalTransform[i][ni];
				for ( int iCoord = 0; iCoord < 3; ++iCoord)
				{
					vPos[iCoord] = VertexTransform[i][vi][iCoord] + Normal[iCoord]*fSin*28.0f;
				}
				glVertex3fv(vPos);
			}
			else
			{
				glVertex3fv(VertexTransform[i][vi]);
			}
		}
	}
	glEnd();
}


#ifdef LDS_ADD_RENDERMESHEFFECT_FOR_VBO
void BMD::RenderMeshEffect ( int i, int iType, int iSubType, vec3_t Angle, VOID* obj, const OBJECT* objSelf )
#else // LDS_ADD_RENDERMESHEFFECT_FOR_VBO
void BMD::RenderMeshEffect ( int i, int iType, int iSubType, vec3_t Angle, VOID* obj )
#endif // LDS_ADD_RENDERMESHEFFECT_FOR_VBO
{
    if ( i>=NumMeshs || i<0 ) return;

    Mesh_t *m = &Meshs[i];
	if(m->NumTriangles <= 0) return;

#ifdef LDS_ADD_RENDERMESHEFFECT_FOR_VBO
	if( objSelf == NULL ) return;

	this->BodyHeight     = 0.f;
	this->ContrastEnable = objSelf->ContrastEnable;
	this->BodyScale     = objSelf->Scale;
	this->CurrentAction = objSelf->CurrentAction;
	VectorCopy(objSelf->Position, this->BodyOrigin);
	
	// 모든 Animation TM 연산을 VertexTrasform으로 적용.
	AnimationTransformOutAllVertices( VertexTransform, *objSelf );
#endif // LDS_ADD_RENDERMESHEFFECT_FOR_VBO

    vec3_t angle, Light;
	int iEffectCount = 0;

    Vector ( 0.f, 0.f, 0.f, angle );
    Vector ( 1.f, 1.f, 1.f, Light );
	for(int j=0;j<m->NumTriangles;j++)
	{
		Triangle_t *tp = &m->Triangles[j];
		for(int k=0;k<tp->Polygon;k++)
		{
			int vi = tp->VertexIndex[k];  

            switch ( iType )
            {
            case MODEL_STONE_COFFIN:
                if ( iSubType==0 )
                {
                    if ( (rand()%2)==0 )
                    {
    			        CreateEffect ( MODEL_STONE_COFFIN+1,VertexTransform[i][vi],angle,Light);
                    }
                    if ( (rand()%10)==0 )
                    {
    			        CreateEffect ( MODEL_STONE_COFFIN,VertexTransform[i][vi],angle,Light);
                    }
                }
                else if ( iSubType==1 )
                {
    			    CreateEffect ( MODEL_STONE_COFFIN+1,VertexTransform[i][vi],angle,Light,2 );
                }
                else if ( iSubType==2 )
                {
    			    CreateEffect ( MODEL_STONE_COFFIN+1,VertexTransform[i][vi],angle,Light,3 );
                }
                else if ( iSubType==3 )
                {
    			    CreateEffect ( MODEL_STONE_COFFIN+rand()%2,VertexTransform[i][vi],angle,Light,4 );
                }
                break;
            case MODEL_GATE:
                if ( iSubType==1 )
                {
                    Vector ( 0.2f, 0.2f, 0.2f, Light );
                    if ( (rand()%5)==0 )
                    {
                        CreateEffect ( MODEL_GATE+1, VertexTransform[i][vi], angle, Light, 2 );//작은거.
                    }
                    if ( (rand()%10)==0 )
                    {
                        CreateEffect ( MODEL_GATE, VertexTransform[i][vi], angle, Light, 2 );//큰거.
                    }
                }
                else if ( iSubType==0 )
                {
                    Vector ( 0.2f, 0.2f, 0.2f, Light );
                    if ( (rand()%12)==0 )
                    {
                        CreateEffect ( MODEL_GATE+1,VertexTransform[i][vi],angle,Light);//작은거.
                    }
                    if ( (rand()%50)==0 )
                    {
                        CreateEffect ( MODEL_GATE,VertexTransform[i][vi],angle,Light);//큰거.
                    }
                }
                break;
            case MODEL_BIG_STONE_PART1:
                if ( (rand()%3)==0 )
                {
                    CreateEffect ( MODEL_BIG_STONE_PART1+rand()%2, VertexTransform[i][vi], angle, Light, 1 );
                }
                break;

            case MODEL_BIG_STONE_PART2:
                if ( (rand()%3)==0 )
                {
                    CreateEffect ( MODEL_BIG_STONE_PART1+rand()%2, VertexTransform[i][vi], angle, Light );
                }
                break;

            case MODEL_WALL_PART1:
                if ( (rand()%3)==0 )
                {
                    CreateEffect ( MODEL_WALL_PART1+rand()%2, VertexTransform[i][vi], angle, Light );
                }
                break;

            case MODEL_GATE_PART1:
                Vector ( 0.2f, 0.2f, 0.2f, Light );
                if ( (rand()%12)==0 )
                {
                    CreateEffect ( MODEL_GATE_PART1+1,VertexTransform[i][vi],angle,Light);//작은거.
                }
                if ( (rand()%40)==0 )
                {
                    CreateEffect ( MODEL_GATE_PART1,VertexTransform[i][vi],angle,Light);//큰거.
                }
                if ( (rand()%40)==0 )
                {
                    CreateEffect ( MODEL_GATE_PART1+2,VertexTransform[i][vi],angle,Light);//큰거.
                }
                break;
			case MODEL_GOLEM_STONE:
				if ( (rand()%45)==0 && iEffectCount < 20)
                {
					if(iSubType == 0) {	//. 불골렘
						CreateEffect ( MODEL_GOLEM_STONE, VertexTransform[i][vi], angle, Light);
					}
					else if(iSubType == 1) {	//. 독골렘
						CreateEffect ( MODEL_BIG_STONE_PART1, VertexTransform[i][vi], angle, Light ,2);
						CreateEffect ( MODEL_BIG_STONE_PART2, VertexTransform[i][vi], angle, Light ,2);
					}
					iEffectCount++;
                }
				break;
            case MODEL_SKIN_SHELL:
                if ( (rand()%8)==0 )
                {
        			CreateEffect ( MODEL_SKIN_SHELL,VertexTransform[i][vi],angle, Light, iSubType );
                }
                break;
            case BITMAP_LIGHT:
                Vector ( 0.08f, 0.08f, 0.08f, Light );
                if ( iSubType==0 )
                {
				    CreateSprite( BITMAP_LIGHT, VertexTransform[i][vi], BodyScale, Light, NULL );
                }
                else if ( iSubType==1 )
                {
                    Vector ( 1.f, 0.8f, 0.2f, Light );
                    if ( (j%22)==0 )
                    {
                        OBJECT* o = (OBJECT*)obj;

                        angle[0] = -(float)( rand()%90 );
                        angle[1] = 0.f;
                        angle[2] = Angle[2]+( float)( rand()%120-60 );
                        CreateJoint ( BITMAP_JOINT_SPIRIT, VertexTransform[i][vi], o->Position, angle, 13, o, 20.f, 0, 0 );
                    }
                }
				break;
            case BITMAP_BUBBLE:
                Vector ( 1.f, 1.f, 1.f, Light );
                if ( (rand()%30)==0 )
                {
                    CreateParticle ( BITMAP_BUBBLE, VertexTransform[i][vi], angle, Light, 2 );
                }
                break;
            }
		}
	}
}


#ifdef MR0
// HiddenMesh : 해당되는 메시를 그리지 않는다.	default : -1(다 그림)
void BMD::RenderBody(int Flag,float Alpha,int BlendMesh,float BlendMeshLight,float BlendMeshTexCoordU,float BlendMeshTexCoordV,int HiddenMesh,int Texture)
{
	if(NumMeshs == 0) return;
	if(!EngineGate::IsOn())
	{
		RenderBodyOld(Flag, Alpha, BlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV, HiddenMesh, Texture);
		return;
	}

	int iBlendMesh = BlendMesh;
	bool bNewMesh = false;
	TextureScript* pTexScript = NULL;
	if(NewMeshs.size()) bNewMesh = true;
	BeginRender(Alpha);
	for(int i=0;i<NumMeshs;i++)
	{
		iBlendMesh = BlendMesh;
		if(bNewMesh)
			pTexScript = NewMeshs[i].m_csTScript;
		else
			pTexScript = Meshs[i].m_csTScript;

        if( pTexScript!=NULL )
        {
            if( pTexScript->getHiddenMesh()==false && i!=HiddenMesh )
            {
                if( pTexScript->getBright() )
                {
                    iBlendMesh = i;
                }
    		    RenderMesh(i,Flag,Alpha,iBlendMesh,BlendMeshLight,BlendMeshTexCoordU,BlendMeshTexCoordV,Texture);

                //  그림자도 표시할 메쉬는 그림자를 표시한다.
                BYTE shadowType = pTexScript->getShadowMesh ();
                if ( shadowType==SHADOW_RENDER_COLOR )
                {
                    //  그림자 찍는다.
                    RenderMesh ( i, RENDER_COLOR|RENDER_SHADOWMAP, Alpha, iBlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV );
                }
                else if ( shadowType==SHADOW_RENDER_TEXTURE )
                {
                    //  그림자 찍는다.
                    RenderMesh ( i, RENDER_TEXTURE|RENDER_SHADOWMAP, Alpha, iBlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV );
                }
            }
        }
        else
        {
		    if(i != HiddenMesh)
		    {
    		    RenderMesh(i,Flag,Alpha,iBlendMesh,BlendMeshLight,BlendMeshTexCoordU,BlendMeshTexCoordV,Texture);
            }
        }
	}
	EndRender();	
}
void BMD::RenderBodyOld(int Flag,float Alpha,int BlendMesh,float BlendMeshLight,float BlendMeshTexCoordU,float BlendMeshTexCoordV,int HiddenMesh,int Texture)
{
	if(NumMeshs == 0) return;

	int iBlendMesh = BlendMesh;
	BeginRender(Alpha);
	if(!LightEnable)
	{
		if(Alpha >= 0.99f)
     		glColor3fv(BodyLight);
		else
			glColor4f(BodyLight[0],BodyLight[1],BodyLight[2],Alpha);
	}
	for(int i=0;i<NumMeshs;i++)
	{
		iBlendMesh = BlendMesh;

        Mesh_t *m = &Meshs[i];
        if( m->m_csTScript!=NULL )
        {
            if( m->m_csTScript->getHiddenMesh()==false && i!=HiddenMesh )
            {
                if( m->m_csTScript->getBright() )
                {
                    iBlendMesh = i;
                }
    		    RenderMesh(i,Flag,Alpha,iBlendMesh,BlendMeshLight,BlendMeshTexCoordU,BlendMeshTexCoordV,Texture);

                //  그림자도 표시할 메쉬는 그림자를 표시한다.
                BYTE shadowType = m->m_csTScript->getShadowMesh ();
                if ( shadowType==SHADOW_RENDER_COLOR )
                {
                    //  그림자 찍는다.
			        DisableAlphaBlend();
		            if ( Alpha >= 0.99f )
			            glColor3f( 0.f, 0.f, 0.f );
		            else
			            glColor4f( 0.f, 0.f, 0.f,Alpha);

                    RenderMesh ( i, RENDER_COLOR|RENDER_SHADOWMAP, Alpha, iBlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV );
			        glColor3f( 1.f, 1.f, 1.f );
                }
                else if ( shadowType==SHADOW_RENDER_TEXTURE )
                {
                    //  그림자 찍는다.
			        DisableAlphaBlend();
		            if ( Alpha >= 0.99f )
			            glColor3f( 0.f, 0.f, 0.f );
		            else
			            glColor4f( 0.f, 0.f, 0.f,Alpha);

                    RenderMesh ( i, RENDER_TEXTURE|RENDER_SHADOWMAP, Alpha, iBlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV );
			        glColor3f( 1.f, 1.f, 1.f );
                }
            }
        }
        else
        {
		    if(i != HiddenMesh)
		    {
    		    RenderMesh(i,Flag,Alpha,iBlendMesh,BlendMeshLight,BlendMeshTexCoordU,BlendMeshTexCoordV,Texture);
            }
        }
	}
	EndRender();
}

#else
void BMD::RenderBody(int Flag,float Alpha,int BlendMesh,float BlendMeshLight,float BlendMeshTexCoordU,float BlendMeshTexCoordV,int HiddenMesh,int Texture)
{
	if(NumMeshs == 0) return;

	int iBlendMesh = BlendMesh;
	BeginRender(Alpha);
	if(!LightEnable)
	{
		if(Alpha >= 0.99f)
     		glColor3fv(BodyLight);
		else
			glColor4f(BodyLight[0],BodyLight[1],BodyLight[2],Alpha);
	}
	for(int i=0;i<NumMeshs;i++)
	{
		iBlendMesh = BlendMesh;

        Mesh_t *m = &Meshs[i];
        if( m->m_csTScript!=NULL )
        {
            if( m->m_csTScript->getHiddenMesh()==false && i!=HiddenMesh )
            {
                if( m->m_csTScript->getBright() )
                {
                    iBlendMesh = i;
                }
    		    RenderMesh(i,Flag,Alpha,iBlendMesh,BlendMeshLight,BlendMeshTexCoordU,BlendMeshTexCoordV,Texture);

                //  그림자도 표시할 메쉬는 그림자를 표시한다.
                BYTE shadowType = m->m_csTScript->getShadowMesh ();
                if ( shadowType==SHADOW_RENDER_COLOR )
                {
                    //  그림자 찍는다.
			        DisableAlphaBlend();
		            if ( Alpha >= 0.99f )
			            glColor3f( 0.f, 0.f, 0.f );
		            else
			            glColor4f( 0.f, 0.f, 0.f,Alpha);

                    RenderMesh ( i, RENDER_COLOR|RENDER_SHADOWMAP, Alpha, iBlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV );
			        glColor3f( 1.f, 1.f, 1.f );
                }
                else if ( shadowType==SHADOW_RENDER_TEXTURE )
                {
                    //  그림자 찍는다.
			        DisableAlphaBlend();
		            if ( Alpha >= 0.99f )
			            glColor3f( 0.f, 0.f, 0.f );
		            else
			            glColor4f( 0.f, 0.f, 0.f,Alpha);

                    RenderMesh ( i, RENDER_TEXTURE|RENDER_SHADOWMAP, Alpha, iBlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV );
			        glColor3f( 1.f, 1.f, 1.f );
                }
            }
        }
        else
        {
		    if(i != HiddenMesh)
		    {
    		    RenderMesh(i,Flag,Alpha,iBlendMesh,BlendMeshLight,BlendMeshTexCoordU,BlendMeshTexCoordV,Texture);
            }
        }
	}
	EndRender();
}
#endif //MR0

void BMD::RenderBodyAlternative( int iRndExtFlag, int iParam, int Flag,float Alpha,int BlendMesh,float BlendMeshLight,float BlendMeshTexCoordU,float BlendMeshTexCoordV,int HiddenMesh,int Texture)
{
	if(NumMeshs == 0) return;

	BeginRender(Alpha);
	if(!LightEnable)
	{
		if(Alpha >= 0.99f)
     		glColor3fv(BodyLight);
		else
			glColor4f(BodyLight[0],BodyLight[1],BodyLight[2],Alpha);
	}
	for(int i=0;i<NumMeshs;i++)
	{
		if(i != HiddenMesh)
		{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
         	Mesh_t *m = &Meshs[i];
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
			RenderMeshAlternative(iRndExtFlag, iParam, i,Flag,Alpha,BlendMesh,BlendMeshLight,BlendMeshTexCoordU,BlendMeshTexCoordV,Texture);
		}
	}
	EndRender();
}

void BMD::RenderMeshTranslate(int i,int RenderFlag,float Alpha,int BlendMesh,float BlendMeshLight,float BlendMeshTexCoordU,float BlendMeshTexCoordV,int MeshTexture)
{
    if ( i>=NumMeshs || i<0 ) return;

    Mesh_t *m = &Meshs[i];
	if(m->NumTriangles == 0) return;
	float Wave = (int)WorldTime%10000 * 0.0001f;

	//텍스쳐
	int Texture = IndexTexture[m->Texture];
	if(Texture == BITMAP_HIDE)
		return;
    else if(Texture == BITMAP_SKIN)
	{
		if(HideSkin) return;
	    Texture = BITMAP_SKIN+Skin;
	}
	else if(Texture == BITMAP_WATER)
	{
	    Texture = BITMAP_WATER+WaterTextureNumber;
	}
	if(MeshTexture != -1)
		Texture = MeshTexture;

	BITMAP_t* pBitmap = Bitmaps.GetTexture(Texture);

	bool EnableWave = false;
    int streamMesh = StreamMesh;
    if ( m->m_csTScript!=NULL )
    {
        if ( m->m_csTScript->getStreamMesh() )
        {
            streamMesh = i;
        }
    }
	if((i==BlendMesh||i==streamMesh) && (BlendMeshTexCoordU!=0.f || BlendMeshTexCoordV!=0.f))
    	EnableWave = true;

	bool EnableLight = LightEnable;
	if(i==StreamMesh)
	{
		//vec3_t Light;
		//Vector(1.f,1.f,1.f,Light);
		glColor3fv(BodyLight);
		EnableLight = false;
	}
	else if(EnableLight)
	{
		for(int j=0;j<m->NumNormals;j++)
		{
			VectorScale(BodyLight,IntensityTransform[i][j],LightTransform[i][j]);
		}
	}

	int Render = RenderFlag;
	if((RenderFlag&RENDER_COLOR) == RENDER_COLOR)
	{
    	Render = RENDER_COLOR;
       	if((RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
			EnableAlphaBlend();
		else if((RenderFlag&RENDER_DARK) == RENDER_DARK)
     		EnableAlphaBlendMinus();
		else
			DisableAlphaBlend();
        DisableTexture();
		glColor3fv(BodyLight);
 	}
	else if((RenderFlag&RENDER_CHROME)==RENDER_CHROME
		|| (RenderFlag&RENDER_METAL)==RENDER_METAL
		|| (RenderFlag&RENDER_CHROME2)==RENDER_CHROME2 
		|| (RenderFlag&RENDER_CHROME6)==RENDER_CHROME6
		)
	{
		if ( m->m_csTScript!=NULL )
        {
            if ( m->m_csTScript->getNoneBlendMesh() ) return;
        }
		if(m->NoneBlendMesh )
			return;
   		Render = RENDER_CHROME;

        float Wave2 = (int)WorldTime%5000 * 0.00024f - 0.4f;

		for(int j=0;j<m->NumNormals;j++)
		{
//			Normal_t *np = &m->Normals[j];
            if ( j>MAX_VERTICES ) break;
			float *Normal = NormalTransform[i][j];

            if((RenderFlag&RENDER_CHROME2)==RENDER_CHROME2)
			{
				g_chrome[j][0] = (Normal[2]+Normal[0])*0.8f + Wave2*2.f;
				g_chrome[j][1] = (Normal[1]+Normal[0])*1.0f + Wave2*3.f;
			}
            else if((RenderFlag&RENDER_CHROME)==RENDER_CHROME)
			{
				g_chrome[j][0] = Normal[2]*0.5f + Wave;
				g_chrome[j][1] = Normal[1]*0.5f + Wave*2.f;
			}
			else if((RenderFlag&RENDER_CHROME6)==RENDER_CHROME6)
			{
				g_chrome[j][0] = (Normal[2]+Normal[0])*0.8f + Wave2*2.f;
				g_chrome[j][1] = (Normal[1]+Normal[0])*1.0f + Wave2*3.f;
			}	
			else
			{
				g_chrome[j][0] = Normal[2]*0.5f + 0.2f;
				g_chrome[j][1] = Normal[1]*0.5f + 0.5f;
			}
		}

		if((RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
     		EnableAlphaBlend();
		else if((RenderFlag&RENDER_DARK) == RENDER_DARK)
     		EnableAlphaBlendMinus();
     	else if((RenderFlag&RENDER_LIGHTMAP) == RENDER_LIGHTMAP)
            EnableLightMap();
		else
			DisableAlphaBlend();

        if((RenderFlag&RENDER_CHROME2)==RENDER_CHROME2 && MeshTexture==-1)
        {
			BindTexture(BITMAP_CHROME2);
        }
        else if((RenderFlag&RENDER_CHROME)==RENDER_CHROME && MeshTexture==-1)
			BindTexture(BITMAP_CHROME);
		else if((RenderFlag&RENDER_METAL)==RENDER_METAL && MeshTexture==-1)
			BindTexture(BITMAP_SHINY);
		else
			BindTexture(Texture);
	}	
	else if(BlendMesh<=-2 || m->Texture == BlendMesh)
	{
    	Render = RENDER_TEXTURE;
   		BindTexture(Texture);
		if((RenderFlag&RENDER_DARK) == RENDER_DARK)
     		EnableAlphaBlendMinus();
		else
     		EnableAlphaBlend();
		glColor3f(BodyLight[0]*BlendMeshLight,BodyLight[1]*BlendMeshLight,BodyLight[2]*BlendMeshLight);
		//glColor3f(BlendMeshLight,BlendMeshLight,BlendMeshLight);
		EnableLight = false;
	}
	else if((RenderFlag&RENDER_TEXTURE) == RENDER_TEXTURE)
	{
    	Render = RENDER_TEXTURE;
		BindTexture(Texture);
		if((RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
		{
     		EnableAlphaBlend();
		}
		else if((RenderFlag&RENDER_DARK) == RENDER_DARK)
		{
     		EnableAlphaBlendMinus();
		}
		else if(Alpha<0.99f || pBitmap->Components==4)
		{
			EnableAlphaTest();
		}
		else
		{
			DisableAlphaBlend();
		}
	}
	else if((RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
	{
		if(pBitmap->Components==4 || m->Texture == BlendMesh)
		{
			return;
		}
    	Render = RENDER_BRIGHT;
        EnableAlphaBlend();
        DisableTexture();
        DisableDepthMask();
	}
	else
	{
    	Render = RENDER_TEXTURE;
	}

#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int iFrame = MoveSceneFrame;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	
	// ver 1.0 (triangle)
#ifdef _DEBUG	// 임시
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	int iSelectPoly = iFrame%11;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
#endif
	glBegin(GL_TRIANGLES);
	for(int j=0;j<m->NumTriangles;j++)
	{
        vec3_t  pos;
		Triangle_t *tp = &m->Triangles[j];
		for(int k=0;k<tp->Polygon;k++)
		{
			int vi = tp->VertexIndex[k];  
			switch(Render)
			{
			case RENDER_TEXTURE:
				{
					TexCoord_t *texp = &m->TexCoords[tp->TexCoordIndex[k]];
					if(EnableWave)
						glTexCoord2f(texp->TexCoordU+BlendMeshTexCoordU,texp->TexCoordV+BlendMeshTexCoordV);
					else
						glTexCoord2f(texp->TexCoordU,texp->TexCoordV);
					if(EnableLight)
					{
						int ni = tp->NormalIndex[k];
						if(Alpha >= 0.99f)
						{
							glColor3fv(LightTransform[i][ni]);
						}
						else
						{
							float *Light = LightTransform[i][ni];
							glColor4f(Light[0],Light[1],Light[2],Alpha);
						}
					}
					break;
				}
			case RENDER_CHROME:
				{
					if(Alpha >= 0.99f)
						glColor3fv(BodyLight);
					else
						glColor4f(BodyLight[0],BodyLight[1],BodyLight[2],Alpha);
					int ni = tp->NormalIndex[k];  
					glTexCoord2f(g_chrome[ni][0],g_chrome[ni][1]);
					break;
				}
			}
// 테스트 코드 --->
#if defined TEST_TYPE1 || defined TEST_TYPE2 || defined TEST_TYPE3 || defined TEST_TYPE4
			if ( Render == RENDER_CHROME)
			{
				float vPos[3];
#ifdef TEST_TYPE1
				float fParam = ( float)( ( int)WorldTime + vi * 931)*0.007f;
#endif
#ifdef TEST_TYPE3
				float fParam = ( float)( ( int)WorldTime + vi * 931)*0.007f;
#endif
#ifdef TEST_TYPE4
				float fParam = ( float)( ( int)WorldTime + vi * 931)*0.007f;
#endif
#ifdef TEST_TYPE2
				float fParam = ( float)( ( int)WorldTime + vi * 931)*0.001f;
#endif
				float fSin = sinf( fParam);
				float fCos = cosf( fParam);

#ifdef TEST_TYPE1
				for ( int iCoord = 0; iCoord < 3; ++iCoord)
				{
					vPos[iCoord] = VertexTransform[i][vi][iCoord] + fSin*fTransformedSize*0.04f;
				}
#endif
#ifdef TEST_TYPE3
				int ni = tp->NormalIndex[k];
				Normal_t *np = &m->Normals[ni];
				float *Normal = NormalTransform[i][ni];
				for ( int iCoord = 0; iCoord < 3; ++iCoord)
				{
					vPos[iCoord] = VertexTransform[i][vi][iCoord] + Normal[iCoord]*fSin*fTransformedSize*0.04f;
					//vPos[iCoord] = VertexTransform[i][vi][iCoord] + Normal[iCoord]*(fSin+1.f)*fTransformedSize*0.02f;
				}
#endif
#ifdef TEST_TYPE2
				for ( int iCoord = 0; iCoord < 2; ++iCoord)
				{
					vPos[iCoord] = VertexTransform[i][vi][iCoord];
				}
				vPos[2] = VertexTransform[i][vi][2] + (fCos-.99f)*fTransformedSize*0.1f;
#endif

				glVertex3fv(vPos);
			}
			else
#endif //defined TEST_TYPE1 || defined TEST_TYPE2 || defined TEST_TYPE3 || defined TEST_TYPE4
// 테스트 코드 <---
			{
				VectorAdd(VertexTransform[i][vi],BodyOrigin,pos);
				glVertex3fv(pos);
			}
		}
	}
	glEnd();
}

void BMD::RenderBodyTranslate(int Flag,float Alpha,int BlendMesh,float BlendMeshLight,float BlendMeshTexCoordU,float BlendMeshTexCoordV,int HiddenMesh,int Texture)
{
	if(NumMeshs == 0) return;

	BeginRender(Alpha);
	if(!LightEnable)
	{
		if(Alpha >= 0.99f)
     		glColor3fv(BodyLight);
		else
			glColor4f(BodyLight[0],BodyLight[1],BodyLight[2],Alpha);
	}
	for(int i=0;i<NumMeshs;i++)
	{
		if(i != HiddenMesh)
		{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
         	Mesh_t *m = &Meshs[i];
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
			RenderMeshTranslate(i,Flag,Alpha,BlendMesh,BlendMeshLight,BlendMeshTexCoordU,BlendMeshTexCoordV,Texture);
		}
	}
	EndRender();
}

#ifdef MR0
void BMD::RenderBodyShadow(int BlendMesh,int HiddenMesh,int StartMeshNumber, int EndMeshNumber )
{
	if(NumMeshs == 0) return;
	if(!EngineGate::IsOn()) 
	{
		RenderBodyShadowOld(BlendMesh, HiddenMesh, StartMeshNumber, EndMeshNumber);
		return;
	}
	
    DisableTexture();
	DisableDepthMask();
	BeginRender(1.f);

    int startMesh = 0;
    int endMesh = NumMeshs;

    if ( StartMeshNumber!=-1 ) startMesh = StartMeshNumber;
    if ( EndMeshNumber!=-1 )   endMesh = EndMeshNumber;

    if ( battleCastle::InBattleCastle() )
		g_posLight[0] = 2500.f;


	Vector(0.f, 0.f, 0.f, BodyLight);
	
	for(int i=startMesh;i<endMesh;++i)
	{
		if(i != HiddenMesh)
		{
    		RenderMesh(i, RENDER_TEXTURE|RENDER_SHADOWMAP);
        }
	}
	EndRender();
	EnableDepthMask();
}

void BMD::RenderBodyShadowOld(int BlendMesh,int HiddenMesh,int StartMeshNumber, int EndMeshNumber )
{
	if(NumMeshs == 0) return;
	
    DisableTexture();
	DisableDepthMask();
	BeginRender(1.f);

    int startMesh = 0;
    int endMesh = NumMeshs;

    if ( StartMeshNumber!=-1 ) startMesh = StartMeshNumber;
    if ( EndMeshNumber!=-1 )   endMesh = EndMeshNumber;

    float sx = 2000.f;
    float sy = 4000.f;

    if ( battleCastle::InBattleCastle() )
    {
        sx = 2500.f;
        sy = 4000.f;
    }

	for(int i=startMesh;i<endMesh;i++)
	{
		if(i != HiddenMesh)
		{
			Mesh_t *m = &Meshs[i];
			if(m->NumTriangles > 0 && m->Texture != BlendMesh)
			{
				glBegin(GL_TRIANGLES);
				for(int j=0;j<m->NumTriangles;j++)
				{
					Triangle_t *tp = &m->Triangles[j];
					for(int k=0;k<tp->Polygon;k++)
					{
						int vi = tp->VertexIndex[k];  
						vec3_t Position;
						VectorSubtract(VertexTransform[i][vi],BodyOrigin,Position);
						Position[0] += Position[2]*(Position[0]+sx)/(Position[2]-sy);
						Position[2] = 5.f;
						VectorAdd(Position,BodyOrigin,Position);
						glVertex3fv(Position);
					}
				}
				glEnd();
			}
		}
	}
	EndRender();
	EnableDepthMask();
}

#else
void BMD::RenderBodyShadow(int BlendMesh,int HiddenMesh,int StartMeshNumber, int EndMeshNumber )
{
	if(NumMeshs == 0) return;

    DisableTexture();
	DisableDepthMask();
	BeginRender(1.f);

    int startMesh = 0;
    int endMesh = NumMeshs;

    if ( StartMeshNumber!=-1 ) startMesh = StartMeshNumber;
    if ( EndMeshNumber!=-1 )   endMesh = EndMeshNumber;

    float sx = 2000.f;
    float sy = 4000.f;

    if ( gMapManager.InBattleCastle() )
    {
        sx = 2500.f;
        sy = 4000.f;
    }

	for(int i=startMesh;i<endMesh;i++)
	{
		if(i != HiddenMesh)
		{
			Mesh_t *m = &Meshs[i];
			if(m->NumTriangles > 0 && m->Texture != BlendMesh)
			{
				glBegin(GL_TRIANGLES);
				for(int j=0;j<m->NumTriangles;j++)
				{
					Triangle_t *tp = &m->Triangles[j];
					for(int k=0;k<tp->Polygon;k++)
					{
						int vi = tp->VertexIndex[k];  
						vec3_t Position;
						VectorSubtract(VertexTransform[i][vi],BodyOrigin,Position);
						Position[0] += Position[2]*(Position[0]+sx)/(Position[2]-sy);
						Position[2] = 5.f;
						VectorAdd(Position,BodyOrigin,Position);
						glVertex3fv(Position);
					}
				}
				glEnd();
			}
		}
	}
	EndRender();
	EnableDepthMask();
}
#endif //MR0

void BMD::RenderObjectBoundingBox()
{
	DisableTexture();
	glPushMatrix();
    glTranslatef(BodyOrigin[0],BodyOrigin[1],BodyOrigin[2]);
	glScalef(BodyScale,BodyScale,BodyScale);
	for(int i=0;i<NumBones;i++)
	{
		Bone_t *b = &Bones[i];
		if(b->BoundingBox)
		{
			vec3_t BoundingVertices[8];
			for(int j=0;j<8;j++)
			{
				VectorTransform(b->BoundingVertices[j],BoneTransform[i],BoundingVertices[j]);
			}
			
			glBegin(GL_QUADS);
			//glBegin(GL_LINES);
			glColor3f(0.2f,0.2f,0.2f);
			glTexCoord2f( 1.0F, 1.0F); glVertex3fv(BoundingVertices[7]);
			glTexCoord2f( 1.0F, 0.0F); glVertex3fv(BoundingVertices[6]);
			glTexCoord2f( 0.0F, 0.0F); glVertex3fv(BoundingVertices[4]);
			glTexCoord2f( 0.0F, 1.0F); glVertex3fv(BoundingVertices[5]);
			
			glColor3f(0.2f,0.2f,0.2f);
			glTexCoord2f( 0.0F, 1.0F); glVertex3fv(BoundingVertices[0]);
			glTexCoord2f( 1.0F, 1.0F); glVertex3fv(BoundingVertices[2]);
			glTexCoord2f( 1.0F, 0.0F); glVertex3fv(BoundingVertices[3]);
			glTexCoord2f( 0.0F, 0.0F); glVertex3fv(BoundingVertices[1]);
			
			glColor3f(0.6f,0.6f,0.6f);
			glTexCoord2f( 1.0F, 1.0F); glVertex3fv(BoundingVertices[7]);
			glTexCoord2f( 1.0F, 0.0F); glVertex3fv(BoundingVertices[3]);
			glTexCoord2f( 0.0F, 0.0F); glVertex3fv(BoundingVertices[2]);
			glTexCoord2f( 0.0F, 1.0F); glVertex3fv(BoundingVertices[6]);
			
			glColor3f(0.6f,0.6f,0.6f);
			glTexCoord2f( 0.0F, 1.0F); glVertex3fv(BoundingVertices[0]);
			glTexCoord2f( 1.0F, 1.0F); glVertex3fv(BoundingVertices[1]);
			glTexCoord2f( 1.0F, 0.0F); glVertex3fv(BoundingVertices[5]);
			glTexCoord2f( 0.0F, 0.0F); glVertex3fv(BoundingVertices[4]);
			
			glColor3f(0.4f,0.4f,0.4f);
			glTexCoord2f( 1.0F, 1.0F); glVertex3fv(BoundingVertices[7]);
			glTexCoord2f( 1.0F, 0.0F); glVertex3fv(BoundingVertices[5]);
			glTexCoord2f( 0.0F, 0.0F); glVertex3fv(BoundingVertices[1]);
			glTexCoord2f( 0.0F, 1.0F); glVertex3fv(BoundingVertices[3]);
			
			glColor3f(0.4f,0.4f,0.4f);
			glTexCoord2f( 0.0F, 1.0F); glVertex3fv(BoundingVertices[0]);
			glTexCoord2f( 1.0F, 1.0F); glVertex3fv(BoundingVertices[4]);
			glTexCoord2f( 1.0F, 0.0F); glVertex3fv(BoundingVertices[6]);
			glTexCoord2f( 0.0F, 0.0F); glVertex3fv(BoundingVertices[2]);
			glEnd();
		}
	}
	glPopMatrix();
	DisableAlphaBlend();
}

void BMD::RenderBone(float (*BoneMatrix)[3][4])
{
	DisableTexture();
	glDepthFunc(GL_ALWAYS);
    glColor3f(0.8f,0.8f,0.2f);
	for(int i=0;i<NumBones;i++)
	{
		Bone_t *b = &Bones[i];
		if(!b->Dummy)
		{
          	BoneMatrix_t *bm = &b->BoneMatrixes[CurrentAction];
			int Parent = b->Parent;
			if(Parent > 0)
			{
				float Scale = 1.f;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNINGggj
         		Bone_t *p = &Bones[i];
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
				float dx = bm->Position[CurrentAnimationFrame][0];
				float dy = bm->Position[CurrentAnimationFrame][1];
				float dz = bm->Position[CurrentAnimationFrame][2];
				Scale = sqrtf(dx*dx+dy*dy+dz*dz)*0.05f;
				vec3_t Position[3];
				Vector(0.f,0.f,-Scale,Position[0]);
				Vector(0.f,0.f, Scale,Position[1]);
				Vector(0.f,0.f, 0.f  ,Position[2]);
				vec3_t BoneVertices[3];
				VectorTransform(Position[0],BoneMatrix[Parent],BoneVertices[0]);
				VectorTransform(Position[1],BoneMatrix[Parent],BoneVertices[1]);
				VectorTransform(Position[2],BoneMatrix[i     ],BoneVertices[2]);
				for(int j=0;j<3;j++)
				{
     				VectorMA(BodyOrigin,BodyScale,BoneVertices[j],BoneVertices[j]);
				}
				glBegin(GL_LINES);
				glVertex3fv(BoneVertices[0]);
				glVertex3fv(BoneVertices[1]);
				glEnd();
				glBegin(GL_LINES);
				glVertex3fv(BoneVertices[1]);
				glVertex3fv(BoneVertices[2]);
				glEnd();
				glBegin(GL_LINES);
				glVertex3fv(BoneVertices[2]);
				glVertex3fv(BoneVertices[0]);
				glEnd();
			}
		}
	}
	glDepthFunc(GL_LEQUAL);
}

///////////////////////////////////////////////////////////////////////////////
// projection shadow
///////////////////////////////////////////////////////////////////////////////

void BlurShadow()
{
	for(int i=1;i<ShadowBufferHeight-1;i++)
	{
      	unsigned char *ptr = &ShadowBuffer[i*ShadowBufferWidth];
		for(int j=1;j<ShadowBufferWidth-1;j++)
		{
			ptr[j] = (ptr[j-ShadowBufferWidth]+ptr[j+ShadowBufferWidth]+
				ptr[j-1]+ptr[j+1])>>2;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// util
///////////////////////////////////////////////////////////////////////////////

void BMD::Release()
{
#ifndef LDS_FIX_MEMORYLEAK_BMDWHICHBONECOUNTZERO	// #ifndef 정리시 삭제.
	if(NumBones == 0) return;
#endif // LDS_FIX_MEMORYLEAK_BMDWHICHBONECOUNTZERO

	for(int i=0;i<NumBones;i++)
	{
       	Bone_t *b = &Bones[i];

		if(!b->Dummy)
		{
			for(int j=0;j<NumActions;j++)
			{
				BoneMatrix_t *bm = &b->BoneMatrixes[j];
				delete []bm->Position;
				delete []bm->Rotation;
				delete []bm->Quaternion;
			}
			SAFE_DELETE_ARRAY(b->BoneMatrixes);
		}
	}

	for(int i=0;i<NumActions;i++)
	{
       	Action_t *a = &Actions[i];
     	if(a->LockPositions)
		{
     		delete []a->Positions;
		}
	}

	if(Meshs)
	{
		for(int i=0;i<NumMeshs;i++)
		{
       		Mesh_t *m = &Meshs[i];

			delete []m->Vertices;
			delete []m->Normals;
			delete []m->TexCoords;
			delete []m->Triangles;

			if ( m->m_csTScript)
			{
				delete m->m_csTScript;
				m->m_csTScript = NULL;
			}
			switch ( IndexTexture[m->Texture])
			{
			case BITMAP_SKIN:
				break;
			default:
				DeleteBitmap(IndexTexture[m->Texture]);
				break;
			}
		}
	}

	SAFE_DELETE_ARRAY(Meshs);
	SAFE_DELETE_ARRAY(Bones);
	SAFE_DELETE_ARRAY(Actions);
	SAFE_DELETE_ARRAY(Textures);
	SAFE_DELETE_ARRAY(IndexTexture);

	NumBones   = 0;
	NumActions = 0;
	NumMeshs   = 0;
	
#ifdef LDS_FIX_SETNULLALLOCVALUE_WHEN_BMDRELEASE
	m_bCompletedAlloc = false;
#endif // LDS_FIX_SETNULLALLOCVALUE_WHEN_BMDRELEASE

#ifdef MR0
	if(EngineGate::IsOn())
	{
		UnloadVBO();
		NewMeshs.clear();
	}
#endif //MR0
}

/*
void EdgeTriangleIndex(int i,int Edge,int Index1,int Index2,Triangle_t *Tri,int Number)
{
		Triangle_t *Tri1 = &Tri[i];
		if ( Tri1->EdgeTriangleIndex[Edge] != -1)
		{
			return;
		}
		int v1 = Tri1->VertexIndex[Index1];
		int v2 = Tri1->VertexIndex[Index2];
		for(int j=0;j<Number;j++)
		{
			if(i!=j)
			{
				Triangle_t *Tri2 = &Tri[j];
				for(int k1=0;k1<3;k1++)
				{
					int k2 = (k1+1)%3;
					if( ( v1 == Tri2->VertexIndex[k2]) && 
						( v2 == Tri2->VertexIndex[k1]))
					{
						if ( Tri2->EdgeTriangleIndex[k1] == -1)
						{
							Tri1->EdgeTriangleIndex[Edge] = j;
							Tri2->EdgeTriangleIndex[k1] = i;
							return;
						}
					}
				}
			}
		}
}

void EdgeTriangleIndex(Triangle_t *Tri,int Number)
{
	int i,j;
	for(i=0;i<Number;i++)
	{
		for(j=0;j<3;j++)
		{
			Tri[i].EdgeTriangleIndex[j] = -1;
		}
	}
	for(i=0;i<Number;i++)
	{
        EdgeTriangleIndex(i,0,0,1,Tri,Number);
        EdgeTriangleIndex(i,1,1,2,Tri,Number);
        EdgeTriangleIndex(i,2,2,0,Tri,Number);
	}
}
//*/

//#ifdef USE_SHADOWVOLUME

void BMD::FindNearTriangle( void)
{
	for( int iMesh=0; iMesh<NumMeshs; iMesh++)
	{
       	Mesh_t *m = &Meshs[iMesh];

		Triangle_t *pTriangle = m->Triangles;
		int iNumTriangles = m->NumTriangles;
		for ( int iTri = 0; iTri < iNumTriangles; ++iTri)
		{
			for ( int i = 0; i < 3; ++i)
			{
				pTriangle[iTri].EdgeTriangleIndex[i] = -1;
			}
		}
		for (int iTri = 0; iTri < iNumTriangles; ++iTri)
		{
#ifdef LDS_OPTIMIZE_FORLOADING
			// 각 Triangle 별로 Triangle이 보유한 각 3변에 인접한 Triangle들의 
			// Tri Index 값을 검색하여 설정 한다.
			FindTriangleForEdge( m, iTri);
#else // LDS_OPTIMIZE_FORLOADING
			FindTriangleForEdge( iMesh, iTri, 0);
			FindTriangleForEdge( iMesh, iTri, 1);
			FindTriangleForEdge( iMesh, iTri, 2);
#endif // LDS_OPTIMIZE_FORLOADING
		}
	}
}


#ifdef LDS_OPTIMIZE_FORLOADING
// 해당 arg1.Mesh의 arg2.IndexTriangle 로 해당 Triangle의 
// 각 3변이 맞닿는 Triangle들의 Tri Index들을 EdgeTriangleIndex[0-2] 로 구성한다.
void BMD::FindTriangleForEdge( Mesh_t* m, int iTriBasis )
{
	Triangle_t* pTriangle = m->Triangles;
	Triangle_t* pTriBasis = &pTriangle[iTriBasis];
	
	// 1. 이미 EdgeTriIndex 지정되어있으면 Return
	//if ( pTriBasis->EdgeTriangleIndex[0] != -1 && 
	//	pTriBasis->EdgeTriangleIndex[1] != -1 && 
	//	pTriBasis->EdgeTriangleIndex[2] != -1 )
	//{	// 이미 지정됨
	//	return;
	//}
	
	
	int iNumTriangles = m->NumTriangles;
	unsigned int uiFindCount = 0, uiMaxCount = 0;
	
	// 1. 현재 Triangle에서 이미 인접 Tri를 찾은 갯수를 센다.
	//	  이미 찾은 나머지 갯수만큼만 접근하여 인접 Triangle을 구성
	for( int iIdx = 0; iIdx < 3; ++iIdx )
	{
		(pTriBasis->EdgeTriangleIndex[iIdx] != -1)? ++uiMaxCount:0;
	}
	
	if( uiMaxCount < 1 ) return;
	
	// 2. 모든 각 Tri 별 접근을 하여 인접 TriIndex를 설정 한다.
	for ( int iTri = 0; iTri < iNumTriangles; ++iTri)
	{
		// 2-1. 현재 Tri와 같으면 안된다.
		if ( iTriBasis == iTri)
		{
			continue;
		}
		
		// 2-2. 비교할 현재 Triangle
		Triangle_t *pTri = &pTriangle[iTri];
		
		// 2-3. 현재 Triangle의 각 Index 에 접근 
		for( int iIndex11 = 0; iIndex11 < 3; ++iIndex11 )
		{
			// 2-3-1. 현재 Index의 다음 Index를 가져와 Edge 를 구성
			int iIndex12 = ( iIndex11 + 1) % 3;
			
			// 2-3-2. 비교할 Triangle의 각 Index에 접근
			for ( int iIndex21 = 0; iIndex21 < 3; ++iIndex21)
			{
				// 2-3-2-1. 비교할 Triangle의 현재 Vertex의 다음 순번 Vertex를 가져와 Edge 구성
				int iIndex22 = ( iIndex21 + 1) % 3;
				
				// 2-3-2-2. (1) 현재 Edge에 인접 Tri 설정이 안되어 있고,
				//			(2) 현재 Triangle Edge의 IndexVertex가 비교대상 Triangle의 Edge와 같다면 
				//			인접 Triangle로 설정
				if ( pTri->EdgeTriangleIndex[iIndex21] == -1 &&
					pTriBasis->VertexIndex[iIndex11] == pTri->VertexIndex[iIndex22] &&
					pTriBasis->VertexIndex[iIndex12] == pTri->VertexIndex[iIndex21])
				{
					
					pTriBasis->EdgeTriangleIndex[iIndex11] = iTri;
					pTri->EdgeTriangleIndex[iIndex21] = iTriBasis;
					
					// 2-3-2-2-1. 
					++uiFindCount;
				}
			}	
		}
		
		if( uiMaxCount <= uiFindCount ) return;
	}
}

#else // LDS_OPTIMIZE_FORLOADING
void BMD::FindTriangleForEdge( int iMesh, int iTri1, int iIndex11)
{
    if ( iMesh>=NumMeshs || iMesh<0 ) return;

	Mesh_t *m = &Meshs[iMesh];
	Triangle_t *pTriangle = m->Triangles;

	Triangle_t *pTri1 = &pTriangle[iTri1];
	if ( pTri1->EdgeTriangleIndex[iIndex11] != -1)
	{	// 이미 지정됨
		return;
	}

	int iNumTriangles = m->NumTriangles;
	for ( int iTri2 = 0; iTri2 < iNumTriangles; ++iTri2)
	{
		if ( iTri1 == iTri2)
		{
			continue;
		}

		Triangle_t *pTri2 = &pTriangle[iTri2];
		int iIndex12 = ( iIndex11 + 1) % 3;
		for ( int iIndex21 = 0; iIndex21 < 3; ++iIndex21)
		{
			int iIndex22 = ( iIndex21 + 1) % 3;
			if ( pTri2->EdgeTriangleIndex[iIndex21] == -1 &&
				pTri1->VertexIndex[iIndex11] == pTri2->VertexIndex[iIndex22] &&
				pTri1->VertexIndex[iIndex12] == pTri2->VertexIndex[iIndex21])
			{
				pTri1->EdgeTriangleIndex[iIndex11] = iTri2;
				pTri2->EdgeTriangleIndex[iIndex21] = iTri1;
				return;
			}
		}
	}
}
#endif // LDS_OPTIMIZE_FORLOADING


//#endif //USE_SHADOWVOLUME

bool BMD::Open(char *DirName,char *ModelFileName)
{
	char ModelName[64];
	strcpy(ModelName,DirName);
	strcat(ModelName,ModelFileName);
    FILE *fp = fopen(ModelName,"rb");
	if(fp == NULL)
	{
		/*char Text[256];
    	sprintf(Text,"%s file does not exist.",ModelFileName);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);*/
#ifdef _DEBUG
		/*char Text[256];
    	sprintf(Text,"%s 파일이 존재하지 않습니다.",ModelFileName);
		MessageBox(g_hWnd,Text,NULL,MB_OK);*/
		//SendMessage(g_hWnd,WM_DESTROY,0,0);
#endif
		return false;
	}
	fseek(fp,0,SEEK_END);
	int DataBytes = ftell(fp);
	fseek(fp,0,SEEK_SET);
    unsigned char *Data = new unsigned char[DataBytes];
	fread(Data,1,DataBytes,fp);
	fclose(fp);

	int Size;
	int DataPtr = 3;
	Version          = *((char *)(Data+DataPtr));DataPtr+=1;
	memcpy(Name,Data+DataPtr,32);DataPtr+=32;
	NumMeshs         = *((short *)(Data+DataPtr));DataPtr+=2;
	NumBones         = *((short *)(Data+DataPtr));DataPtr+=2;
	NumActions       = *((short *)(Data+DataPtr));DataPtr+=2;

	Meshs            = new Mesh_t    [max( 1, NumMeshs)  ];
	Bones            = new Bone_t    [max( 1, NumBones)  ];
	Actions          = new Action_t  [max( 1, NumActions)];
	Textures         = new Texture_t [max( 1, NumMeshs)  ];
	IndexTexture     = new GLuint    [max( 1, NumMeshs)  ];

	int i;
	for(i=0;i<NumMeshs;i++)
	{
       	Mesh_t *m = &Meshs[i];
		m->NumVertices      = *((short *)(Data+DataPtr));DataPtr+=2;
		m->NumNormals       = *((short *)(Data+DataPtr));DataPtr+=2;
		m->NumTexCoords     = *((short *)(Data+DataPtr));DataPtr+=2;
		m->NumTriangles     = *((short *)(Data+DataPtr));DataPtr+=2;
		m->Texture          = *((short *)(Data+DataPtr));DataPtr+=2;
        m->NoneBlendMesh    = false;
		//m->NumCommandBytes  = *((int   *)(Data+DataPtr));DataPtr+=4;
		m->Vertices  = new Vertex_t      [m->NumVertices ];
		m->Normals   = new Normal_t      [m->NumNormals  ];
		m->TexCoords = new TexCoord_t    [m->NumTexCoords];
		m->Triangles = new Triangle_t    [m->NumTriangles];
		//m->Commands  = new unsigned char [m->NumCommandBytes];
		Size = m->NumVertices  * sizeof(Vertex_t  );
		memcpy(m->Vertices ,Data+DataPtr,Size);DataPtr+=Size;
		Size = m->NumNormals   * sizeof(Normal_t  );
		memcpy(m->Normals  ,Data+DataPtr,Size);DataPtr+=Size;
		Size = m->NumTexCoords * sizeof(TexCoord_t);
		memcpy(m->TexCoords,Data+DataPtr,Size);DataPtr+=Size;
	    //Size = m->NumTriangles * sizeof(Triangle_t);
		//memcpy(m->Triangles,Data+DataPtr,Size);DataPtr+=Size;
		Size = sizeof(Triangle_t);
		int Size2 = sizeof(Triangle_t2);
		for(int j=0;j<m->NumTriangles;j++)
		{
	      	memcpy(&m->Triangles[j],Data+DataPtr,Size);DataPtr+=Size2;
		}
		//memcpy(m->Commands ,Data+DataPtr,m->NumCommandBytes);DataPtr+=m->NumCommandBytes;
		memcpy(Textures[i].FileName,Data+DataPtr,32);DataPtr+=32;

        TextureScriptParsing TSParsing;
                                                     
        if( TSParsing.parsingTScript(Textures[i].FileName) )
        {
            m->m_csTScript = new TextureScript;
            m->m_csTScript->setScript( (TextureScript&)TSParsing );
        }
        else
        {
            m->m_csTScript = NULL;
        }
	}
//#ifdef USE_SHADOWVOLUME
	/*for(i=0;i<NumMeshs;i++)
	{
		Mesh_t *m = &Meshs[i];
    	EdgeTriangleIndex(m->Triangles,m->NumTriangles);
	}*/
	FindNearTriangle();
//#endif
	for(i=0;i<NumActions;i++)
	{
       	Action_t *a = &Actions[i];
		a->Loop = false;
		a->NumAnimationKeys = *((short *)(Data+DataPtr));DataPtr+=2;
		a->LockPositions = *((bool *)(Data+DataPtr));DataPtr+=1;
     	if(a->LockPositions)
		{
          	a->Positions = new vec3_t [a->NumAnimationKeys];
			Size = a->NumAnimationKeys*sizeof(vec3_t);
			memcpy(a->Positions,Data+DataPtr,Size);DataPtr+=Size;
		}
	}
	for(i=0;i<NumBones;i++)
	{
       	Bone_t *b = &Bones[i];
		b->Dummy = *((char *)(Data+DataPtr));DataPtr+=1;
		if(!b->Dummy)
		{
			memcpy(b->Name,Data+DataPtr,32);DataPtr+=32;
			b->Parent = *((short *)(Data+DataPtr));DataPtr+=2;
			b->BoneMatrixes = new BoneMatrix_t [NumActions];
			for(int j=0;j<NumActions;j++)
			{
				BoneMatrix_t *bm = &b->BoneMatrixes[j];
				Size = Actions[j].NumAnimationKeys*sizeof(vec3_t);
				int NumAnimationKeys = Actions[j].NumAnimationKeys;
				bm->Position   = new vec3_t [NumAnimationKeys];
				bm->Rotation   = new vec3_t [NumAnimationKeys];
				bm->Quaternion = new vec4_t [NumAnimationKeys];
				memcpy(bm->Position,Data+DataPtr,Size);DataPtr+=Size;
				memcpy(bm->Rotation,Data+DataPtr,Size);DataPtr+=Size;
				for(int k=0;k<NumAnimationKeys;k++)
				{
         			AngleQuaternion(bm->Rotation[k],bm->Quaternion[k]);
				}
			}
		}
	}
    delete [] Data;
    Init(false);
	return true;
}

bool BMD::Save(char *DirName,char *ModelFileName)
{
	char ModelName[64];
	strcpy(ModelName,DirName);
	strcat(ModelName,ModelFileName);
    FILE *fp = fopen(ModelName,"wb");
    if(fp == NULL) return false;
	putc('B',fp);
	putc('M',fp);
	putc('D',fp);
	fwrite(&Version   ,1,1,fp);
	fwrite(Name       ,32,1,fp);
	fwrite(&NumMeshs  ,2,1,fp);
	fwrite(&NumBones  ,2,1,fp);
	fwrite(&NumActions,2,1,fp);
	int i;
	for(i=0;i<NumMeshs;i++)
	{
       	Mesh_t *m = &Meshs[i];
		fwrite(&m->NumVertices     ,2,1,fp);
		fwrite(&m->NumNormals      ,2,1,fp);
		fwrite(&m->NumTexCoords    ,2,1,fp);
		fwrite(&m->NumTriangles    ,2,1,fp);
		fwrite(&m->Texture         ,2,1,fp);
		//fwrite(&m->NumCommandBytes ,4,1,fp);
		fwrite(m->Vertices ,m->NumVertices *sizeof(Vertex_t  ),1,fp);
		fwrite(m->Normals  ,m->NumNormals  *sizeof(Normal_t  ),1,fp);
		fwrite(m->TexCoords,m->NumTexCoords*sizeof(TexCoord_t),1,fp);
		//fwrite(m->Triangles,m->NumTriangles*sizeof(Triangle_t),1,fp);
		for(int j=0;j<m->NumTriangles;j++)
		{
	       	fwrite(&m->Triangles[j],sizeof(Triangle_t2),1,fp);
		}
		//fwrite(m->Commands ,m->NumCommandBytes                ,1,fp);
		fwrite(Textures[i].FileName,32,1,fp);
	}
	for(i=0;i<NumActions;i++)
	{
       	Action_t *a = &Actions[i];
     	fwrite(&a->NumAnimationKeys,2,1,fp);
     	fwrite(&a->LockPositions,1,1,fp);
     	if(a->LockPositions)
		{
			fwrite(a->Positions,a->NumAnimationKeys*sizeof(vec3_t),1,fp);
		}
	}
	for(i=0;i<NumBones;i++)
	{
       	Bone_t *b = &Bones[i];
		fwrite(&b->Dummy,1,1,fp);
		if(!b->Dummy)
		{
			fwrite(b->Name,32,1,fp);
			fwrite(&b->Parent,2,1,fp);
			for(int j=0;j<NumActions;j++)
			{
				BoneMatrix_t *bm = &b->BoneMatrixes[j];
				fwrite(bm->Position,Actions[j].NumAnimationKeys*sizeof(vec3_t),1,fp);
				fwrite(bm->Rotation,Actions[j].NumAnimationKeys*sizeof(vec3_t),1,fp);
			}
		}
	}
	fclose(fp);
    return true;
}

#ifdef MR0

#ifdef LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
bool BMD::Open2(char *DirName,char *ModelFileName, bool bReAlloc)
#else // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
bool BMD::Open2(char *DirName,char *ModelFileName)
#endif // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
{
#ifdef LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
	// 이미 구성이 되어있는 상태라면
	if( true == m_bCompletedAlloc )
	{
		// 만약 재구성 TRUE이면, 
		if( true == bReAlloc )		
		{
			Release();
		}
		else
		{
			return true;
		}
	}
#endif // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL

	if(!EngineGate::IsOn())
	{
#ifdef LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
		 return Open2Old(DirName, ModelFileName, bReAlloc);
#else // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
		return Open2Old(DirName, ModelFileName);
#endif // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
	}

	char ModelName[64];
	strcpy(ModelName,DirName);
	strcat(ModelName,ModelFileName);
    FILE *fp = fopen(ModelName,"rb");
	if(fp == NULL)
	{
#ifdef LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
		m_bCompletedAlloc = false;
#endif // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
		return false;
	}
	fseek(fp,0,SEEK_END);
	int DataBytes = ftell(fp);
	fseek(fp,0,SEEK_SET);
    unsigned char *Data = new unsigned char[DataBytes];
	fread(Data,1,DataBytes,fp);
	fclose(fp);

	int Size;
	int DataPtr = 3;
	Version          = *((char *)(Data+DataPtr));
	DataPtr+=1;
	if ( Version >= 12)
	{	// 암호화 된거면
		// 암호화 풀기
		long lSize          = *((long *)(Data+DataPtr));DataPtr+=sizeof ( long);
		long lDecSize = MapFileDecrypt( NULL, Data+DataPtr, lSize);
		BYTE *pbyDec = new BYTE [lDecSize];
		MapFileDecrypt( pbyDec, Data+DataPtr, lSize);
		delete [] Data;
		Data = pbyDec;
		DataPtr = 0;
	}

	memcpy(Name,Data+DataPtr,32);DataPtr+=32;
	NumMeshs         = *((short *)(Data+DataPtr));DataPtr+=2;
	NumBones         = *((short *)(Data+DataPtr));DataPtr+=2;
	assert(NumBones <= MAX_BONES && "본 갯수가 200 넘었습니다.! 본 최대갯수가 200개입니다.");
	NumActions       = *((short *)(Data+DataPtr));DataPtr+=2;

	Meshs            = new Mesh_t    [max( 1, NumMeshs)  ];
	Bones            = new Bone_t    [max( 1, NumBones)  ];
	Actions          = new Action_t  [max( 1, NumActions)];
	Textures         = new Texture_t [max( 1, NumMeshs)  ];
	IndexTexture     = new GLuint    [max( 1, NumMeshs)  ];

	int i;
	
	for(i=0;i<NumMeshs;i++)
	{
       	Mesh_t *m = &Meshs[i];
		m->NumVertices      = *((short *)(Data+DataPtr));DataPtr+=2;
		m->NumNormals       = *((short *)(Data+DataPtr));DataPtr+=2;
		m->NumTexCoords     = *((short *)(Data+DataPtr));DataPtr+=2;

		if(Version >= 13)
		{
			m->NumVertexColors = *((short *)(Data+DataPtr));
			DataPtr+=2;
		}
		else
			m->NumVertexColors = 0;

		m->NumTriangles     = *((short *)(Data+DataPtr));DataPtr+=2;
		m->Texture          = *((short *)(Data+DataPtr));DataPtr+=2;
        m->NoneBlendMesh    = false;
		//m->NumCommandBytes  = *((int   *)(Data+DataPtr));DataPtr+=4;
		m->Vertices  = new Vertex_t      [m->NumVertices ];
		m->Normals   = new Normal_t      [m->NumNormals  ];
		m->TexCoords = new TexCoord_t    [m->NumTexCoords];
		m->Triangles = new Triangle_t    [m->NumTriangles];
		//m->Commands  = new unsigned char [m->NumCommandBytes];
		Size = m->NumVertices  * sizeof(Vertex_t  );
		memcpy(m->Vertices ,Data+DataPtr,Size);DataPtr+=Size;
		Size = m->NumNormals   * sizeof(Normal_t  );
		memcpy(m->Normals  ,Data+DataPtr,Size);DataPtr+=Size;
		Size = m->NumTexCoords * sizeof(TexCoord_t);
		memcpy(m->TexCoords,Data+DataPtr,Size);DataPtr+=Size;
		
		if(Version >= 13)
		{
			Size = m->NumVertexColors * sizeof(VertexColor_t);
			if(Size)
			{
				memcpy(m->VertexColors, Data+DataPtr, Size); 
				DataPtr+=Size;
			}
			else
				m->VertexColors = NULL;
		}
		else
			m->VertexColors = NULL;

	    //Size = m->NumTriangles * sizeof(Triangle_t);
		//memcpy(m->Triangles,Data+DataPtr,Size);DataPtr+=Size;
		Size = sizeof(Triangle_t);
		int Size2 = sizeof(Triangle_t2);
		for(int j=0;j<m->NumTriangles;j++)
		{
	      	memcpy(&m->Triangles[j],Data+DataPtr,Size);DataPtr+=Size2;
		}
		//memcpy(m->Commands ,Data+DataPtr,m->NumCommandBytes);DataPtr+=m->NumCommandBytes;
		memcpy(Textures[i].FileName,Data+DataPtr,32);DataPtr+=32;

        TextureScriptParsing TSParsing;
                                                     
        if( TSParsing.parsingTScript(Textures[i].FileName) )
        {
            m->m_csTScript = new TextureScript;
            m->m_csTScript->setScript( (TextureScript&)TSParsing );
        }
        else
        {
            m->m_csTScript = NULL;
        }
		//Read Vertex Animation
		
		//Read post-effect script
		if(Version >= 13)
		{
			WORD nScript = *((WORD *)(Data+DataPtr));DataPtr+=2;
			m->MeshEffectScript.reserve(nScript);
			
			for(j = 0; j < nScript; ++j )
			{
				m->MeshEffectScript.push_back(EffectScrip_t());
				EffectScrip_t* pNewScript = &m->MeshEffectScript[m->MeshEffectScript.size()-1];
				unsigned int isizeScript = sizeof(EffectScrip_t);
				memcpy(pNewScript, (EffectScrip_t* )(Data+DataPtr), isizeScript );
				DataPtr += isizeScript;

				//읽어 들였으면. 
				if((pNewScript->m_tShader == RENDER_TEXTURE) || (pNewScript->m_tShader == RENDER_COLOR))
				{
					pNewScript->m_iTex = -1;
				}
				else
				{
					BITMAP_t* pBitmap = Bitmaps.FindTexture(pNewScript->m_strTexName);
					pNewScript->m_iTex = pBitmap->TextureNumber;
				}
				
			}
		}

		//Read Additional Skin Infomation
		if(Version >= 14)
		{
			WORD nSubBone = *((WORD *)(Data+DataPtr));DataPtr+=2;
			m->MeshEffectScript.reserve(nSubBone);

			for(j = 0; j < nSubBone; ++j)
			{
				TempSkinInfo_t tInfo;
				WORD iVertexID = *((WORD *)(Data+DataPtr));DataPtr+=2;
				unsigned int iSizeInfo = sizeof(TempSkinInfo_t);
				memcpy(&tInfo, (TempSkinInfo_t* )(Data+DataPtr), iSizeInfo);
				DataPtr += iSizeInfo;
				
				m->MeshAdditionalSkinInfo[iVertexID] = tInfo;
			}
		}
	}
	FindNearTriangle();
	for(i=0;i<NumActions;i++)
	{
       	Action_t *a = &Actions[i];
		a->Loop = false;
		a->NumAnimationKeys = *((short *)(Data+DataPtr));DataPtr+=2;
		a->LockPositions = *((bool *)(Data+DataPtr));DataPtr+=1;
     	if(a->LockPositions)
		{
          	a->Positions = new vec3_t [a->NumAnimationKeys];
			Size = a->NumAnimationKeys*sizeof(vec3_t);
			memcpy(a->Positions,Data+DataPtr,Size);DataPtr+=Size;
		}
	}
	for(i=0;i<NumBones;i++)
	{
       	Bone_t *b = &Bones[i];
		b->Dummy = *((char *)(Data+DataPtr));DataPtr+=1;
		if(!b->Dummy)
		{
			memcpy(b->Name,Data+DataPtr,32);DataPtr+=32;
			b->Parent = *((short *)(Data+DataPtr));DataPtr+=2;
			b->BoneMatrixes = new BoneMatrix_t [NumActions];
			for(int j=0;j<NumActions;j++)
			{
				BoneMatrix_t *bm = &b->BoneMatrixes[j];
				Size = Actions[j].NumAnimationKeys*sizeof(vec3_t);
				int NumAnimationKeys = Actions[j].NumAnimationKeys;
				bm->Position   = new vec3_t [NumAnimationKeys];
				bm->Rotation   = new vec3_t [NumAnimationKeys];
				bm->Quaternion = new vec4_t [NumAnimationKeys];
				memcpy(bm->Position,Data+DataPtr,Size);DataPtr+=Size;
				memcpy(bm->Rotation,Data+DataPtr,Size);DataPtr+=Size;
				for(int k=0;k<NumAnimationKeys;k++)
				{
         			AngleQuaternion(bm->Rotation[k],bm->Quaternion[k]);
				}
			}
		}
	}
    delete [] Data;
    Init(false);

//////////////////////////////////////////////////////////////////////////
//VBO 버전
	Vector(1.f, 1.f, 1.f, BodyLight);

//빌보드 체크
	if(strstr(Name, "billboard_"))
		iBillType = 1;
	else if(strstr(Name, "bill_"))
		iBillType = 2;
	else
		iBillType = 0;

	if(strstr(Name, "nl_"))
		bOffLight = true;
	else
		bOffLight = false;

//////////////////////////////////////////////////////////////////////////
#ifdef LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
	m_bCompletedAlloc = true;
#endif // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
    return true;
}

bool BMD::Save2(char *DirName,char *ModelFileName)	// 암호화
{
	if(!EngineGate::IsOn())
	{
		return Save2Old(DirName, ModelFileName);
	}
	
	int i;
	char ModelName[64];
	strcpy(ModelName,DirName);
	strcat(ModelName,ModelFileName);
    FILE *fp = fopen(ModelName,"wb");
    if(fp == NULL) return false;
	putc('B',fp);
	putc('M',fp);
	putc('D',fp);
	Version = 13;

	//메쉬들 중에 한 개라도 Blended Mesh라면
	if(NumMeshs != 0)
	{
		for(i = 0; i < NumMeshs; ++i)
		{
			if(Meshs[i].MeshAdditionalSkinInfo.size())
			{
				Version = 14;
				break;
			}
		}
	}
/*
	12 : 암호화 있음
	13 : New Mesh
	14 : xmd (Blending)
*/
	fwrite(&Version   ,1,1,fp);

	// 여기부터 암호화
	BYTE *pbyBuffer = new BYTE [1024*1024*5];
	BYTE *pbyCur = pbyBuffer;
	memcpy( pbyCur, Name, 32); pbyCur += 32;
	memcpy( pbyCur, &NumMeshs, 2); pbyCur += 2;
	memcpy( pbyCur, &NumBones, 2); pbyCur += 2;
	memcpy( pbyCur, &NumActions, 2); pbyCur += 2;

	for(i=0;i<NumMeshs;i++)
	{
       	Mesh_t *m = &Meshs[i];
		memcpy( pbyCur, &m->NumVertices, 2); pbyCur += 2;
		memcpy( pbyCur, &m->NumNormals, 2); pbyCur += 2;
		memcpy( pbyCur, &m->NumTexCoords, 2); pbyCur += 2;

		if((Version >= 13) && (m->NumVertexColors))
		{
			memcpy( pbyCur, &m->NumVertexColors, 2); pbyCur += 2;
		}

		memcpy( pbyCur, &m->NumTriangles, 2); pbyCur += 2;
		memcpy( pbyCur, &m->Texture, 2); pbyCur += 2;
		//fwrite(&m->NumCommandBytes ,4,1,fp);
		memcpy( pbyCur, m->Vertices, m->NumVertices *sizeof(Vertex_t  )); pbyCur += m->NumVertices *sizeof(Vertex_t  );
		memcpy( pbyCur, m->Normals, m->NumNormals  *sizeof(Normal_t  )); pbyCur += m->NumNormals  *sizeof(Normal_t  );
		memcpy( pbyCur, m->TexCoords, m->NumTexCoords*sizeof(TexCoord_t)); pbyCur += m->NumTexCoords*sizeof(TexCoord_t);
		
		if((Version >= 13) && (m->NumVertexColors))
		{
			memcpy( pbyCur, m->VertexColors, 
				m->NumVertexColors*sizeof(VertexColor_t)); pbyCur += m->NumVertexColors*sizeof(VertexColor_t);
		}
		
		//fwrite(m->Triangles,m->NumTriangles*sizeof(Triangle_t),1,fp);
		for(int j=0;j<m->NumTriangles;j++)
		{
			memcpy( pbyCur, &m->Triangles[j],sizeof(Triangle_t2)); pbyCur += sizeof(Triangle_t2);
		}
		//fwrite(m->Commands ,m->NumCommandBytes                ,1,fp);
		memcpy( pbyCur, Textures[i].FileName,32); pbyCur += 32;

		if(Version >= 13)
		{
			unsigned int iSizeScript = sizeof(EffectScrip_t);
			if(NewMeshs.size())
			{
				VBOMesh* pNewMesh = &NewMeshs[i];
				WORD cntEffect = (WORD) pNewMesh->MeshEffectScript.size();
				memcpy( pbyCur, &cntEffect, 2); pbyCur += 2;
				for(j = 0; j < pNewMesh->MeshEffectScript.size(); ++j)
				{
					EffectScrip_t* pScript = &pNewMesh->MeshEffectScript[j];
					memcpy( pbyCur, pScript, iSizeScript);
					pbyCur += iSizeScript;
				}
			}
			else
			{
				WORD cntEffect = 1;
				EffectScrip_t	tScript;
				tScript.m_fAlpha = 1.f;
				tScript.m_iTex = -1;
				strcpy(tScript.m_strTexName, "");
				tScript.m_tShader = RENDER_TEXTURE;
				Vector(1.f, 1.f, 1.f, tScript.m_vColor3f);

				memcpy( pbyCur, &cntEffect, 2); pbyCur += 2;
				memcpy( pbyCur, &tScript, iSizeScript);
				pbyCur += iSizeScript;
			}
		}

		if(Version >= 14)
		{
			WORD cntBlendInfo = (WORD) m->MeshAdditionalSkinInfo.size();
			memcpy( pbyCur, &cntBlendInfo, 2); pbyCur += 2;
			TempSkinInfoMap_t::iterator mapIter = m->MeshAdditionalSkinInfo.begin();
			for(; mapIter != m->MeshAdditionalSkinInfo.end(); ++mapIter)
			{
				WORD iVertex = (WORD)mapIter->first;
				memcpy( pbyCur, &iVertex, 2); pbyCur += 2;
				TempSkinInfo_t* pInfo = &mapIter->second;
				unsigned int iSizeInfo = sizeof(TempSkinInfo_t);
				memcpy( pbyCur, pInfo, iSizeInfo);
				pbyCur += iSizeInfo;
			}
		}
	}
	for(i=0;i<NumActions;i++)
	{
       	Action_t *a = &Actions[i];
		memcpy( pbyCur, &a->NumAnimationKeys,2); pbyCur += 2;
		memcpy( pbyCur, &a->LockPositions,1); pbyCur += 1;
     	if(a->LockPositions && a->Positions )
		{
			memcpy( pbyCur, a->Positions,a->NumAnimationKeys*sizeof(vec3_t)); pbyCur += a->NumAnimationKeys*sizeof(vec3_t);
		}
	}
	for(i=0;i<NumBones;i++)
	{
       	Bone_t *b = &Bones[i];
		memcpy( pbyCur, &b->Dummy,1); pbyCur += 1;
		if(!b->Dummy)
		{
			memcpy( pbyCur, b->Name,32); pbyCur += 32;
			memcpy( pbyCur, &b->Parent,2); pbyCur += 2;
			for(int j=0;j<NumActions;j++)
			{
				BoneMatrix_t *bm = &b->BoneMatrixes[j];
				memcpy( pbyCur, bm->Position,Actions[j].NumAnimationKeys*sizeof(vec3_t)); pbyCur += Actions[j].NumAnimationKeys*sizeof(vec3_t);
				memcpy( pbyCur, bm->Rotation,Actions[j].NumAnimationKeys*sizeof(vec3_t)); pbyCur += Actions[j].NumAnimationKeys*sizeof(vec3_t);
			}
		}
	}

	long lSize = ( long)( pbyCur - pbyBuffer);
	long lEncSize = MapFileEncrypt( NULL, pbyBuffer, lSize);
	BYTE *pbyEnc = new BYTE [lEncSize];
	MapFileEncrypt( pbyEnc, pbyBuffer, lSize);
	fwrite(&lEncSize, sizeof ( long), 1, fp);
	fwrite(pbyEnc, lEncSize, 1, fp);
	fclose(fp);
	delete [] pbyBuffer;
	delete [] pbyEnc;
    return true;
}

#ifdef LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
bool BMD::Open2Old(char *DirName,char *ModelFileName, bool bReAlloc)
#else // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
bool BMD::Open2Old(char *DirName,char *ModelFileName)
#endif // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
{
#ifdef LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
	// 이미 구성이 되어있는 상태라면
	if( true == m_bCompletedAlloc )
	{
		// 만약 재구성 TRUE이면, 
		if( true == bReAlloc )		
		{
			// release
			Release();
		}
		else
		{
			return true;
		}
	}
#endif // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL

	char ModelName[64];
	strcpy(ModelName,DirName);
	strcat(ModelName,ModelFileName);
    FILE *fp = fopen(ModelName,"rb");
	if(fp == NULL)
	{
#ifdef _DEBUG
		/*char Text[256];
    	sprintf(Text,"%s 파일이 존재하지 않습니다.",ModelFileName);
		MessageBox(g_hWnd,Text,NULL,MB_OK);*/
		//SendMessage(g_hWnd,WM_DESTROY,0,0);
#endif

#ifdef LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
		m_bCompletedAlloc = false;
#endif // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
		return false;
	}
	fseek(fp,0,SEEK_END);
	int DataBytes = ftell(fp);
	fseek(fp,0,SEEK_SET);
    unsigned char *Data = new unsigned char[DataBytes];
	fread(Data,1,DataBytes,fp);
	fclose(fp);

	int Size;
	int DataPtr = 3;
	Version          = *((char *)(Data+DataPtr));DataPtr+=1;
	if ( Version == 12)
	{	// 암호화 된거면
		// 암호화 풀기
		long lSize          = *((long *)(Data+DataPtr));DataPtr+=sizeof ( long);
		long lDecSize = MapFileDecrypt( NULL, Data+DataPtr, lSize);
		BYTE *pbyDec = new BYTE [lDecSize];
		MapFileDecrypt( pbyDec, Data+DataPtr, lSize);
		delete [] Data;
		Data = pbyDec;
		DataPtr = 0;
	}

	memcpy(Name,Data+DataPtr,32);DataPtr+=32;
	NumMeshs         = *((short *)(Data+DataPtr));DataPtr+=2;
	NumBones         = *((short *)(Data+DataPtr));DataPtr+=2;
	NumActions       = *((short *)(Data+DataPtr));DataPtr+=2;

	Meshs            = new Mesh_t    [max( 1, NumMeshs)  ];
	Bones            = new Bone_t    [max( 1, NumBones)  ];
	Actions          = new Action_t  [max( 1, NumActions)];
	Textures         = new Texture_t [max( 1, NumMeshs)  ];
	IndexTexture     = new GLuint    [max( 1, NumMeshs)  ];

	int i;
	for(i=0;i<NumMeshs;i++)
	{
       	Mesh_t *m = &Meshs[i];
		m->NumVertices      = *((short *)(Data+DataPtr));DataPtr+=2;
		m->NumNormals       = *((short *)(Data+DataPtr));DataPtr+=2;
		m->NumTexCoords     = *((short *)(Data+DataPtr));DataPtr+=2;
		m->NumTriangles     = *((short *)(Data+DataPtr));DataPtr+=2;
		m->Texture          = *((short *)(Data+DataPtr));DataPtr+=2;
        m->NoneBlendMesh    = false;
		//m->NumCommandBytes  = *((int   *)(Data+DataPtr));DataPtr+=4;
		m->Vertices  = new Vertex_t      [m->NumVertices ];
		m->Normals   = new Normal_t      [m->NumNormals  ];
		m->TexCoords = new TexCoord_t    [m->NumTexCoords];
		m->Triangles = new Triangle_t    [m->NumTriangles];
		//m->Commands  = new unsigned char [m->NumCommandBytes];
		Size = m->NumVertices  * sizeof(Vertex_t  );
		memcpy(m->Vertices ,Data+DataPtr,Size);DataPtr+=Size;
		Size = m->NumNormals   * sizeof(Normal_t  );
		memcpy(m->Normals  ,Data+DataPtr,Size);DataPtr+=Size;
		Size = m->NumTexCoords * sizeof(TexCoord_t);
		memcpy(m->TexCoords,Data+DataPtr,Size);DataPtr+=Size;
	    //Size = m->NumTriangles * sizeof(Triangle_t);
		//memcpy(m->Triangles,Data+DataPtr,Size);DataPtr+=Size;
		Size = sizeof(Triangle_t);
		int Size2 = sizeof(Triangle_t2);

		for(int j=0;j<m->NumTriangles;j++)
		{
	      	memcpy(&m->Triangles[j],Data+DataPtr,Size);DataPtr+=Size2;
		}
		//memcpy(m->Commands ,Data+DataPtr,m->NumCommandBytes);DataPtr+=m->NumCommandBytes;
		memcpy(Textures[i].FileName,Data+DataPtr,32);DataPtr+=32;

        TextureScriptParsing TSParsing;
                                                     
        if( TSParsing.parsingTScript(Textures[i].FileName) )
        {
            m->m_csTScript = new TextureScript;
            m->m_csTScript->setScript( (TextureScript&)TSParsing );
        }
        else
        {
            m->m_csTScript = NULL;
        }
	}
//#ifdef USE_SHADOWVOLUME
	/*for(i=0;i<NumMeshs;i++)
	{
		Mesh_t *m = &Meshs[i];
    	EdgeTriangleIndex(m->Triangles,m->NumTriangles);
	}*/
	FindNearTriangle();
//#endif
	for(i=0;i<NumActions;i++)
	{
       	Action_t *a = &Actions[i];
		a->Loop = false;
		a->NumAnimationKeys = *((short *)(Data+DataPtr));DataPtr+=2;
		a->LockPositions = *((bool *)(Data+DataPtr));DataPtr+=1;
     	if(a->LockPositions)
		{
          	a->Positions = new vec3_t [a->NumAnimationKeys];
			Size = a->NumAnimationKeys*sizeof(vec3_t);
			memcpy(a->Positions,Data+DataPtr,Size);DataPtr+=Size;
		}
	}
	for(i=0;i<NumBones;i++)
	{
       	Bone_t *b = &Bones[i];
		b->Dummy = *((char *)(Data+DataPtr));DataPtr+=1;
		if(!b->Dummy)
		{
			memcpy(b->Name,Data+DataPtr,32);DataPtr+=32;
			b->Parent = *((short *)(Data+DataPtr));DataPtr+=2;
			b->BoneMatrixes = new BoneMatrix_t [NumActions];
			for(int j=0;j<NumActions;j++)
			{
				BoneMatrix_t *bm = &b->BoneMatrixes[j];
				Size = Actions[j].NumAnimationKeys*sizeof(vec3_t);
				int NumAnimationKeys = Actions[j].NumAnimationKeys;
				bm->Position   = new vec3_t [NumAnimationKeys];
				bm->Rotation   = new vec3_t [NumAnimationKeys];
				bm->Quaternion = new vec4_t [NumAnimationKeys];
				memcpy(bm->Position,Data+DataPtr,Size);DataPtr+=Size;
				memcpy(bm->Rotation,Data+DataPtr,Size);DataPtr+=Size;
				for(int k=0;k<NumAnimationKeys;k++)
				{
         			AngleQuaternion(bm->Rotation[k],bm->Quaternion[k]);
				}
			}
		}
	}


    delete [] Data;
    Init(false);

#ifdef LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
	m_bCompletedAlloc = true;
#endif // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
    return true;

/*
	if ( Version == 12)
        return  true;

    return false;
*/
}

bool BMD::Save2Old(char *DirName,char *ModelFileName)	// 암호화
{
	char ModelName[64];
	strcpy(ModelName,DirName);
	strcat(ModelName,ModelFileName);
    FILE *fp = fopen(ModelName,"wb");
    if(fp == NULL) return false;
	putc('B',fp);
	putc('M',fp);
	putc('D',fp);
	Version = 12;	// 암호화 전에는 10
	fwrite(&Version   ,1,1,fp);

	// 여기부터 암호화
	BYTE *pbyBuffer = new BYTE [1024*1024];
	BYTE *pbyCur = pbyBuffer;
	memcpy( pbyCur, Name, 32); pbyCur += 32;
	memcpy( pbyCur, &NumMeshs, 2); pbyCur += 2;
	memcpy( pbyCur, &NumBones, 2); pbyCur += 2;
	memcpy( pbyCur, &NumActions, 2); pbyCur += 2;

	int i;
	for(i=0;i<NumMeshs;i++)
	{
       	Mesh_t *m = &Meshs[i];
		memcpy( pbyCur, &m->NumVertices, 2); pbyCur += 2;
		memcpy( pbyCur, &m->NumNormals, 2); pbyCur += 2;
		memcpy( pbyCur, &m->NumTexCoords, 2); pbyCur += 2;
		memcpy( pbyCur, &m->NumTriangles, 2); pbyCur += 2;
		memcpy( pbyCur, &m->Texture, 2); pbyCur += 2;
		//fwrite(&m->NumCommandBytes ,4,1,fp);
		memcpy( pbyCur, m->Vertices, m->NumVertices *sizeof(Vertex_t  )); pbyCur += m->NumVertices *sizeof(Vertex_t  );
		memcpy( pbyCur, m->Normals, m->NumNormals  *sizeof(Normal_t  )); pbyCur += m->NumNormals  *sizeof(Normal_t  );
		memcpy( pbyCur, m->TexCoords, m->NumTexCoords*sizeof(TexCoord_t)); pbyCur += m->NumTexCoords*sizeof(TexCoord_t);
		//fwrite(m->Triangles,m->NumTriangles*sizeof(Triangle_t),1,fp);
		for(int j=0;j<m->NumTriangles;j++)
		{
			memcpy( pbyCur, &m->Triangles[j],sizeof(Triangle_t2)); pbyCur += sizeof(Triangle_t2);
		}
		//fwrite(m->Commands ,m->NumCommandBytes                ,1,fp);
		memcpy( pbyCur, Textures[i].FileName,32); pbyCur += 32;
	}
	for(i=0;i<NumActions;i++)
	{
       	Action_t *a = &Actions[i];
		memcpy( pbyCur, &a->NumAnimationKeys,2); pbyCur += 2;
		memcpy( pbyCur, &a->LockPositions,1); pbyCur += 1;
     	if(a->LockPositions)
		{
			memcpy( pbyCur, a->Positions,a->NumAnimationKeys*sizeof(vec3_t)); pbyCur += a->NumAnimationKeys*sizeof(vec3_t);
		}
	}
	for(i=0;i<NumBones;i++)
	{
       	Bone_t *b = &Bones[i];
		memcpy( pbyCur, &b->Dummy,1); pbyCur += 1;
		if(!b->Dummy)
		{
			memcpy( pbyCur, b->Name,32); pbyCur += 32;
			memcpy( pbyCur, &b->Parent,2); pbyCur += 2;
			for(int j=0;j<NumActions;j++)
			{
				BoneMatrix_t *bm = &b->BoneMatrixes[j];
				memcpy( pbyCur, bm->Position,Actions[j].NumAnimationKeys*sizeof(vec3_t)); pbyCur += Actions[j].NumAnimationKeys*sizeof(vec3_t);
				memcpy( pbyCur, bm->Rotation,Actions[j].NumAnimationKeys*sizeof(vec3_t)); pbyCur += Actions[j].NumAnimationKeys*sizeof(vec3_t);
			}
		}
	}
	long lSize = ( long)( pbyCur - pbyBuffer);
	long lEncSize = MapFileEncrypt( NULL, pbyBuffer, lSize);
	BYTE *pbyEnc = new BYTE [lEncSize];
	MapFileEncrypt( pbyEnc, pbyBuffer, lSize);
	fwrite(&lEncSize, sizeof ( long), 1, fp);
	fwrite(pbyEnc, lEncSize, 1, fp);
	fclose(fp);
	delete [] pbyBuffer;
	delete [] pbyEnc;
    return true;
}

#else // MR0

#ifdef LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
bool BMD::Open2(char *DirName,char *ModelFileName, bool bReAlloc)
#else // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
bool BMD::Open2(char *DirName,char *ModelFileName)
#endif // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
{
#ifdef DO_PROFILING_FOR_LOADING
	g_pProfilerForLoading->BeginUnit( EPROFILING_LOADING_ACCESSMODEL_OPEN2, PROFILING_LOADING_ACCESSMODEL_OPEN2 );
#endif // DO_PROFILING_FOR_LOADING
		
#ifdef DO_PROFILING_FOR_LOADING
	g_pProfilerForLoading->BeginUnit( EPROFILING_LOADING_ACCESSMODEL_OPEN2_PREOPEN2, PROFILING_LOADING_ACCESSMODEL_OPEN2_PREOPEN2 );
#endif // DO_PROFILING_FOR_LOADING


#ifdef LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
	// 이미 구성이 되어있는 상태라면
	if( true == m_bCompletedAlloc )
	{
		// 만약 재구성 TRUE이면, 
		if( true == bReAlloc )		
		{
			// release
			Release();
		}
		else
		{
			return true;
		}
	}
#endif // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
	
	char ModelName[64];
	strcpy(ModelName,DirName);
	strcat(ModelName,ModelFileName);
    FILE *fp = fopen(ModelName,"rb");
	if(fp == NULL)
	{
#ifdef _DEBUG
		// 모델이 없는 경우 출력.
		// 월드 로딩 시 각 월드별로 정크로딩되는 모델들의 경우 조차 수백개의 메세지가 출력 되는 문제로 주석 처리.
		//static unsigned int uiCountingMsgOut = 0;
		//if(uiCountingMsgOut<4)
		//{
		//	char Text[256];
		//	sprintf(Text,"%s 파일이 존재하지 않습니다.",ModelName);
		//	MessageBox(g_hWnd,Text,NULL,MB_OK);
		//	SendMessage(g_hWnd,WM_DESTROY,0,0);
		//}
		//uiCountingMsgOut++;
#endif

#ifdef DO_PROFILING_FOR_LOADING
		g_pProfilerForLoading->EndUnit( EPROFILING_LOADING_ACCESSMODEL_OPEN2_PREOPEN2 );
#endif // DO_PROFILING_FOR_LOADING
		
#ifdef DO_PROFILING_FOR_LOADING
		g_pProfilerForLoading->EndUnit( EPROFILING_LOADING_ACCESSMODEL_OPEN2 );
#endif // DO_PROFILING_FOR_LOADING

#ifdef LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
		m_bCompletedAlloc = false;
#endif // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
		return false;
	}

	fseek(fp,0,SEEK_END);
	int DataBytes = ftell(fp);
	fseek(fp,0,SEEK_SET);
    unsigned char *Data = new unsigned char[DataBytes];
	fread(Data,1,DataBytes,fp);
	fclose(fp);

	int Size;
	int DataPtr = 3;
	Version          = *((char *)(Data+DataPtr));DataPtr+=1;
	if ( Version == 12)
	{	// 암호화 된거면
		// 암호화 풀기
		long lSize          = *((long *)(Data+DataPtr));DataPtr+=sizeof ( long);
		long lDecSize = MapFileDecrypt( NULL, Data+DataPtr, lSize);
		BYTE *pbyDec = new BYTE [lDecSize];
		MapFileDecrypt( pbyDec, Data+DataPtr, lSize);
		delete [] Data;
		Data = pbyDec;
		DataPtr = 0;
	}

	memcpy(Name,Data+DataPtr,32);DataPtr+=32;

	NumMeshs         = *((short *)(Data+DataPtr));DataPtr+=2;
	NumBones         = *((short *)(Data+DataPtr));DataPtr+=2;
	assert(NumBones <= MAX_BONES && "본 갯수가 200 넘었습니다.! 본 최대갯수가 200개입니다.");
	NumActions       = *((short *)(Data+DataPtr));DataPtr+=2;

	Meshs            = new Mesh_t    [max( 1, NumMeshs)  ];
	Bones            = new Bone_t    [max( 1, NumBones)  ];
	Actions          = new Action_t  [max( 1, NumActions)];
	Textures         = new Texture_t [max( 1, NumMeshs)  ];
	IndexTexture     = new GLuint    [max( 1, NumMeshs)  ];
	
#ifdef DO_PROFILING_FOR_LOADING
	g_pProfilerForLoading->EndUnit( EPROFILING_LOADING_ACCESSMODEL_OPEN2_PREOPEN2 );
#endif // DO_PROFILING_FOR_LOADING

#ifdef DO_PROFILING_FOR_LOADING
	g_pProfilerForLoading->BeginUnit( EPROFILING_LOADING_ACCESSMODEL_OPEN2_MESHPARSING, PROFILING_LOADING_ACCESSMODEL_OPEN2_MESHPARSING );
#endif // DO_PROFILING_FOR_LOADING
	int i;
	for(i=0;i<NumMeshs;i++)
	{
       	Mesh_t *m = &Meshs[i];
		m->NumVertices      = *((short *)(Data+DataPtr));DataPtr+=2;
		m->NumNormals       = *((short *)(Data+DataPtr));DataPtr+=2;
		m->NumTexCoords     = *((short *)(Data+DataPtr));DataPtr+=2;
		m->NumTriangles     = *((short *)(Data+DataPtr));DataPtr+=2;
		m->Texture          = *((short *)(Data+DataPtr));DataPtr+=2;
        m->NoneBlendMesh    = false;
		//m->NumCommandBytes  = *((int   *)(Data+DataPtr));DataPtr+=4;
		m->Vertices  = new Vertex_t      [m->NumVertices ];
		m->Normals   = new Normal_t      [m->NumNormals  ];
		m->TexCoords = new TexCoord_t    [m->NumTexCoords];
		m->Triangles = new Triangle_t    [m->NumTriangles];
		//m->Commands  = new unsigned char [m->NumCommandBytes];
		Size = m->NumVertices  * sizeof(Vertex_t  );
		memcpy(m->Vertices ,Data+DataPtr,Size);DataPtr+=Size;
		Size = m->NumNormals   * sizeof(Normal_t  );
		memcpy(m->Normals  ,Data+DataPtr,Size);DataPtr+=Size;
		Size = m->NumTexCoords * sizeof(TexCoord_t);
		memcpy(m->TexCoords,Data+DataPtr,Size);DataPtr+=Size;
	    //Size = m->NumTriangles * sizeof(Triangle_t);
		//memcpy(m->Triangles,Data+DataPtr,Size);DataPtr+=Size;
		Size = sizeof(Triangle_t);
		int Size2 = sizeof(Triangle_t2);

		for(int j=0;j<m->NumTriangles;j++)
		{
	      	memcpy(&m->Triangles[j],Data+DataPtr,Size);DataPtr+=Size2;
		}
		//memcpy(m->Commands ,Data+DataPtr,m->NumCommandBytes);DataPtr+=m->NumCommandBytes;
		memcpy(Textures[i].FileName,Data+DataPtr,32);DataPtr+=32;

        TextureScriptParsing TSParsing;
                                                     
        if( TSParsing.parsingTScript(Textures[i].FileName) )
        {
            m->m_csTScript = new TextureScript;
            m->m_csTScript->setScript( (TextureScript&)TSParsing );
        }
        else
        {
            m->m_csTScript = NULL;
        }
	}

#ifdef DO_PROFILING_FOR_LOADING
	g_pProfilerForLoading->EndUnit( EPROFILING_LOADING_ACCESSMODEL_OPEN2_MESHPARSING );
#endif // DO_PROFILING_FOR_LOADING

//#ifdef USE_SHADOWVOLUME
	/*for(i=0;i<NumMeshs;i++)
	{
		Mesh_t *m = &Meshs[i];
    	EdgeTriangleIndex(m->Triangles,m->NumTriangles);
	}*/

// * define내용 : 만약 LDS_OPT_FORLOADING.. 선언되어 있으면, FindNearTriangle() 함수를
//				  #define USE_SHADOWVOLUME 하에 두고, 선언 않되어 있으면 기존것 대로 
//				  그냥 FindNearTriangle() 함수 호출 하게 한다.
#ifdef LDS_OPTIMIZE_FORLOADING_UNNECESSARY_SHADOWVOLUME
#ifdef USE_SHADOWVOLUME

#ifdef DO_PROFILING_FOR_LOADING
	g_pProfilerForLoading->BeginUnit( EPROFILING_LOADING_ACCESSMODEL_OPEN2_SETNEARTRIANGLE, PROFILING_LOADING_ACCESSMODEL_OPEN2_SETNEARTRIANGLE );
#endif // DO_PROFILING_FOR_LOADING
	FindNearTriangle();
#ifdef DO_PROFILING_FOR_LOADING
	g_pProfilerForLoading->EndUnit( EPROFILING_LOADING_ACCESSMODEL_OPEN2_SETNEARTRIANGLE );
#endif // DO_PROFILING_FOR_LOADING

#endif // USE_SHADOWVOLUME

#else  // LDS_OPTIMIZE_FORLOADING_UNNECESSARY_SHADOWVOLUME
	
	FindNearTriangle();

#endif // LDS_OPTIMIZE_FORLOADING_UNNECESSARY_SHADOWVOLUME


#ifdef DO_PROFILING_FOR_LOADING
	g_pProfilerForLoading->BeginUnit( EPROFILING_LOADING_ACCESSMODEL_OPEN2_ACTIONPARSING, PROFILING_LOADING_ACCESSMODEL_OPEN2_ACTIONPARSING );
#endif // DO_PROFILING_FOR_LOADING
	for(i=0;i<NumActions;i++)
	{
       	Action_t *a = &Actions[i];
		a->Loop = false;
		a->NumAnimationKeys = *((short *)(Data+DataPtr));DataPtr+=2;
		a->LockPositions = *((bool *)(Data+DataPtr));DataPtr+=1;
     	if(a->LockPositions)
		{
          	a->Positions = new vec3_t [a->NumAnimationKeys];
			Size = a->NumAnimationKeys*sizeof(vec3_t);
			memcpy(a->Positions,Data+DataPtr,Size);DataPtr+=Size;
		}
#ifdef LDS_ADD_EMPIRE_GUARDIAN
		else
		{
			a->Positions = NULL;
		}
#endif	// LDS_ADD_EMPIRE_GUARDIAN
	}

#ifdef DO_PROFILING_FOR_LOADING
	g_pProfilerForLoading->EndUnit( EPROFILING_LOADING_ACCESSMODEL_OPEN2_ACTIONPARSING );
#endif // DO_PROFILING_FOR_LOADING


#ifdef DO_PROFILING_FOR_LOADING
	g_pProfilerForLoading->BeginUnit( EPROFILING_LOADING_ACCESSMODEL_OPEN2_BONEANIPARSING, PROFILING_LOADING_ACCESSMODEL_OPEN2_BONEANIPARSING );
#endif // DO_PROFILING_FOR_LOADING
	for(i=0;i<NumBones;i++)
	{
       	Bone_t *b = &Bones[i];
		b->Dummy = *((char *)(Data+DataPtr));DataPtr+=1;
		if(!b->Dummy)
		{
			memcpy(b->Name,Data+DataPtr,32);DataPtr+=32;
			b->Parent = *((short *)(Data+DataPtr));DataPtr+=2;
			b->BoneMatrixes = new BoneMatrix_t [NumActions];
			for(int j=0;j<NumActions;j++)
			{
				BoneMatrix_t *bm = &b->BoneMatrixes[j];
				Size = Actions[j].NumAnimationKeys*sizeof(vec3_t);
				int NumAnimationKeys = Actions[j].NumAnimationKeys;
				bm->Position   = new vec3_t [NumAnimationKeys];
				bm->Rotation   = new vec3_t [NumAnimationKeys];
				bm->Quaternion = new vec4_t [NumAnimationKeys];
				memcpy(bm->Position,Data+DataPtr,Size);DataPtr+=Size;
				memcpy(bm->Rotation,Data+DataPtr,Size);DataPtr+=Size;
				for(int k=0;k<NumAnimationKeys;k++)
				{
         			AngleQuaternion(bm->Rotation[k],bm->Quaternion[k]);
				}
			}
		}
	}

#ifdef DO_PROFILING_FOR_LOADING
	g_pProfilerForLoading->EndUnit( EPROFILING_LOADING_ACCESSMODEL_OPEN2_BONEANIPARSING );
#endif // DO_PROFILING_FOR_LOADING


    delete [] Data;
    Init(false);


#ifdef DO_PROFILING_FOR_LOADING
	g_pProfilerForLoading->EndUnit( EPROFILING_LOADING_ACCESSMODEL_OPEN2 );
#endif // DO_PROFILING_FOR_LOADING

#ifdef LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
	m_bCompletedAlloc = true;
#endif // LDS_FIX_MEMORYLEAK_BMDOPEN2_DUMPMODEL
    return true;

/*
	if ( Version == 12)
        return  true;

    return false;
*/
}

bool BMD::Save2(char *DirName,char *ModelFileName)	// 암호화
{
	char ModelName[64];
	strcpy(ModelName,DirName);
	strcat(ModelName,ModelFileName);
    FILE *fp = fopen(ModelName,"wb");
    if(fp == NULL) return false;
	putc('B',fp);
	putc('M',fp);
	putc('D',fp);
	Version = 12;	// 암호화 전에는 10
	fwrite(&Version   ,1,1,fp);

	// 여기부터 암호화
	BYTE *pbyBuffer = new BYTE [1024*1024];
	BYTE *pbyCur = pbyBuffer;
	memcpy( pbyCur, Name, 32); pbyCur += 32;
	memcpy( pbyCur, &NumMeshs, 2); pbyCur += 2;
	memcpy( pbyCur, &NumBones, 2); pbyCur += 2;
	memcpy( pbyCur, &NumActions, 2); pbyCur += 2;

	int i;
	for(i=0;i<NumMeshs;i++)
	{
       	Mesh_t *m = &Meshs[i];
		memcpy( pbyCur, &m->NumVertices, 2); pbyCur += 2;
		memcpy( pbyCur, &m->NumNormals, 2); pbyCur += 2;
		memcpy( pbyCur, &m->NumTexCoords, 2); pbyCur += 2;
		memcpy( pbyCur, &m->NumTriangles, 2); pbyCur += 2;
		memcpy( pbyCur, &m->Texture, 2); pbyCur += 2;
		//fwrite(&m->NumCommandBytes ,4,1,fp);
		memcpy( pbyCur, m->Vertices, m->NumVertices *sizeof(Vertex_t  )); pbyCur += m->NumVertices *sizeof(Vertex_t  );
		memcpy( pbyCur, m->Normals, m->NumNormals  *sizeof(Normal_t  )); pbyCur += m->NumNormals  *sizeof(Normal_t  );
		memcpy( pbyCur, m->TexCoords, m->NumTexCoords*sizeof(TexCoord_t)); pbyCur += m->NumTexCoords*sizeof(TexCoord_t);
		//fwrite(m->Triangles,m->NumTriangles*sizeof(Triangle_t),1,fp);
		for(int j=0;j<m->NumTriangles;j++)
		{
			memcpy( pbyCur, &m->Triangles[j],sizeof(Triangle_t2)); pbyCur += sizeof(Triangle_t2);
		}
		//fwrite(m->Commands ,m->NumCommandBytes                ,1,fp);
		memcpy( pbyCur, Textures[i].FileName,32); pbyCur += 32;
	}
	for(i=0;i<NumActions;i++)
	{
       	Action_t *a = &Actions[i];
		memcpy( pbyCur, &a->NumAnimationKeys,2); pbyCur += 2;
		memcpy( pbyCur, &a->LockPositions,1); pbyCur += 1;
     	if(a->LockPositions)
		{
			memcpy( pbyCur, a->Positions,a->NumAnimationKeys*sizeof(vec3_t)); pbyCur += a->NumAnimationKeys*sizeof(vec3_t);
		}
	}
	for(i=0;i<NumBones;i++)
	{
       	Bone_t *b = &Bones[i];
		memcpy( pbyCur, &b->Dummy,1); pbyCur += 1;
		if(!b->Dummy)
		{
			memcpy( pbyCur, b->Name,32); pbyCur += 32;
			memcpy( pbyCur, &b->Parent,2); pbyCur += 2;
			for(int j=0;j<NumActions;j++)
			{
				BoneMatrix_t *bm = &b->BoneMatrixes[j];
				memcpy( pbyCur, bm->Position,Actions[j].NumAnimationKeys*sizeof(vec3_t)); pbyCur += Actions[j].NumAnimationKeys*sizeof(vec3_t);
				memcpy( pbyCur, bm->Rotation,Actions[j].NumAnimationKeys*sizeof(vec3_t)); pbyCur += Actions[j].NumAnimationKeys*sizeof(vec3_t);
			}
		}
	}
	long lSize = ( long)( pbyCur - pbyBuffer);
	long lEncSize = MapFileEncrypt( NULL, pbyBuffer, lSize);
	BYTE *pbyEnc = new BYTE [lEncSize];
	MapFileEncrypt( pbyEnc, pbyBuffer, lSize);
	fwrite(&lEncSize, sizeof ( long), 1, fp);
	fwrite(pbyEnc, lEncSize, 1, fp);
	fclose(fp);
	delete [] pbyBuffer;
	delete [] pbyEnc;
    return true;
}

#endif //MR0

void BMD::Init(bool Dummy)
{
	//for(i=0;i<NumActions;i++)
	//	Actions[i].Loop = false;
	if(Dummy)
	{
		int i;
		for(i=0;i<NumBones;i++)
		{
			Bone_t *b = &Bones[i];
			if(b->Name[0]=='D' && b->Name[1]=='u')
				b->Dummy = true;
			else
				b->Dummy = false;
		}
		/*for(i=0;i<NumMeshs;i++)
		{
			Mesh_t *m = &Meshs[i];
			for(int j=0;j<m->NumVertices;j++)
			{
				Vertex_t *v = &m->Vertices[j];
				if(v->Node != -1)
				{
					Bones[v->Node].Dummy = false;
				}
			}
		}*/
	}
    renderCount = 0;
	BoneHead = -1;
	StreamMesh = -1;
	CreateBoundingBox();
}

void BMD::CreateBoundingBox()
{
	for(int i=0;i<NumBones;i++)
	{
		for(int j=0;j<3;j++)
		{
			BoundingMin[i][j] = 9999.0;
			BoundingMax[i][j] = -9999.0;
		}
		BoundingVertices[i] = 0;
	}

	for(int i=0;i<NumMeshs;i++)
	{
       	Mesh_t *m = &Meshs[i];
		for(int j=0;j<m->NumVertices;j++)
		{
			Vertex_t *v = &m->Vertices[j];
			for(int k=0;k<3;k++)
			{
				if (v->Position[k] < BoundingMin[v->Node][k]) BoundingMin[v->Node][k] = v->Position[k];
				if (v->Position[k] > BoundingMax[v->Node][k]) BoundingMax[v->Node][k] = v->Position[k];
			}
			BoundingVertices[v->Node]++;
		}
	}
	for(int i=0;i<NumBones;i++)
	{
		Bone_t *b = &Bones[i];
		if(BoundingVertices[i]) 
			b->BoundingBox = true;
		else
			b->BoundingBox = false;
		Vector(BoundingMax[i][0],BoundingMax[i][1],BoundingMax[i][2],b->BoundingVertices[0]);
		Vector(BoundingMax[i][0],BoundingMax[i][1],BoundingMin[i][2],b->BoundingVertices[1]);
		Vector(BoundingMax[i][0],BoundingMin[i][1],BoundingMax[i][2],b->BoundingVertices[2]);
		Vector(BoundingMax[i][0],BoundingMin[i][1],BoundingMin[i][2],b->BoundingVertices[3]);
		Vector(BoundingMin[i][0],BoundingMax[i][1],BoundingMax[i][2],b->BoundingVertices[4]);
		Vector(BoundingMin[i][0],BoundingMax[i][1],BoundingMin[i][2],b->BoundingVertices[5]);
		Vector(BoundingMin[i][0],BoundingMin[i][1],BoundingMax[i][2],b->BoundingVertices[6]);
		Vector(BoundingMin[i][0],BoundingMin[i][1],BoundingMin[i][2],b->BoundingVertices[7]);
	}
}


#ifdef MR0

//////////////////////////////////////////////////////////////////////////
//새 버전의 메쉬로 재구성한다.
typedef struct _TempUnit
{
	short v, t, n, c;
	_TempUnit() : v(-1), t(-1), n(-1), c(-1) {}
	_TempUnit(short x, short y, short z, short w) : v(x), t(y), n(z), c(c) {}
}TempUnit;
typedef std::vector<TempUnit> TempVec;



// Convert된 VBOMesh의 Vertex및 Bone정보의 Validation 검증을 합니다.
#ifdef LDS_MR0_FORDEBUG_VERIFYCONVERTMESH
void BMD::ProcessVerification(MeshVector& CurrentVBOMeshContainer)
{
	unsigned int uiResultValidationTotal = VerifyTotalVBOMESHS( CurrentVBOMeshContainer );

	char	szValidation[512];
	UnValidateTypeText( uiResultValidationTotal, szValidation );

#ifdef KWAK_ADD_TRACE_FUNC
	__TraceF(TEXT("%s"), szValidation);
#endif // KWAK_ADD_TRACE_FUNC
}


unsigned int BMD::VerifyTotalVBOMESHS( MeshVector& CurrentVBOMeshContainer )
{
	MeshVector::iterator	iterV_ = 0;
	unsigned int uiResultValidationTotal = 0;

	for( iterV_ = CurrentVBOMeshContainer.begin(); iterV_ != CurrentVBOMeshContainer.end(); ++iterV_ )
	{
		VBOMesh& CurrentVBOMesh = (*iterV_);
		uiResultValidationTotal |= VerifyCurrentVBOMeshValidate( CurrentVBOMesh );
	}

	return uiResultValidationTotal;
}

unsigned int BMD::VerifyCurrentVBOMeshValidate( VBOMesh& CurrentVBOMesh )
{
	unsigned int uiResultValidation = 0;

	// Verify Vertex Buffer 
	UnitVector::iterator	iterV_ = 0;

	// 1. 모든 Vertex Buffer 접근.
	for( iterV_ = CurrentVBOMesh.VBuffer.begin(); iterV_ != CurrentVBOMesh.VBuffer.end(); ++iterV_ )
	{
		VertexUnit& CurrentVB = (*iterV_);

		CurrentVB.m_vPos[0];
		CurrentVB.m_vPos[1];
		CurrentVB.m_vPos[2];
		CurrentVB.m_vPos[3];
	}

	// 2. a. Verify Index Buffer
	UShortVector::iterator	iterI_	= 0;
	const unsigned int		uiVBMAX	= CurrentVBOMesh.VBuffer.size();
	const unsigned int		uiIBMAX = CurrentVBOMesh.IBuffer.size();

#ifdef KWAK_ADD_TRACE_FUNC
	__TraceF(TEXT(" ----------------------------------------------------------- HEAD 정보\n"));
	__TraceF(TEXT("\nVBuffer.size()=(%d)\n"), uiVBMAX);
	__TraceF(TEXT("IBuffer.size()=(%d)\n"), uiIBMAX);
#endif // KWAK_ADD_TRACE_FUNC

// 	unsigned int uiCounting = 0, uiCountingLineNext = 10;
// 
// 	sprintf( szDebugOutput, " ----------------------------------------------------------- IB 정보\n" );
// 	OutputDebugString( szDebugOutput );
// 	// 2. b.모든 버텍스의 인덱스가 인정되는 범위 내에 존재 여부.
// 	for( iterI_ = CurrentVBOMesh.IBuffer.begin(); iterI_ != CurrentVBOMesh.IBuffer.end(); ++iterI_ )
// 	{
// 		short shCurrentIB = (*iterI_);
// 
// 		// 2-1. 모든 버텍스의 인덱스들을 출력.
// 		if( true == bOutputRealTimeInfo )
// 		{
// 			char	szDebugOutput[512];
// 			
// 			sprintf( szDebugOutput, " %d/%d \t", shCurrentIB );
// 			OutputDebugString( szDebugOutput );
// 
// 			if( uiCountingLineNext < uiCounting++ )
// 			{
// 				uiCounting = 0;
// 				char	szDebugLineNext[16];
// 
// 				sprintf( szDebugLineNext, "\n" );
// 				OutputDebugString( szDebugLineNext );
// 			}
// 		}
// 
// 		// 현재 Index Buffer 값이 0 보다 작으면 문제가 있다고 결론.
// 		if( 0 > shCurrentIB || uiVBMAX < shCurrentIB)
// 		{
// 			uiResultValidation |= (unsigned int)UNVALIDATETYPE::UT_IB;
// 			break;
// 		}
// 	}

#ifdef KWAK_ADD_TRACE_FUNC
	__TraceF(TEXT("\n ----------------------------------------------------------- VB 정보\n"));
#endif // KWAK_ADD_TRACE_FUNC
	
	// 3. 모든 인덱스로 접근되는 모든 버텍스 출력.
	for( iterI_ = CurrentVBOMesh.IBuffer.begin(); iterI_ != CurrentVBOMesh.IBuffer.end(); ++iterI_ )
	{
		short shCurrentIB = (*iterI_);

		VertexUnit& CurrentVB = CurrentVBOMesh.VBuffer[shCurrentIB];

#ifdef KWAK_ADD_TRACE_FUNC
		// 2-1. 모든 버텍스의 인덱스들을 출력.
		__TraceF(TEXT("CurIB(%d) Pos(%f,%f,%f,%f)\n"), shCurrentIB, CurrentVB.m_vPos[0], CurrentVB.m_vPos[1], CurrentVB.m_vPos[2], CurrentVB.m_vPos[3]);
#endif // KWAK_ADD_TRACE_FUNC
	}

	return uiResultValidation;
}

void BMD::UnValidateTypeText( unsigned int uiUnvalidationType, char* szOutText )
{
	if(!szOutText) 
	{
		szOutText = 0;
		return;
	}

	unsigned int uiBuf = 0;

	if( (unsigned int)UT_SUCCESS == uiUnvalidationType ) 
	{
		uiBuf = uiBuf + sprintf( uiBuf + szOutText, " All Validated. " );
		return;
	}

	if( ((unsigned int)UT_IB | uiUnvalidationType) == (unsigned int)UT_IB )
	{
		uiBuf = uiBuf + sprintf( uiBuf + szOutText, " Unvalidate_IB. " );
	}
	
	if( ((unsigned int)UT_VB | uiUnvalidationType) == (unsigned int)UT_VB )
	{
		uiBuf = uiBuf + sprintf( uiBuf + szOutText, " Unvalidate_VB. " );
	}
	
	uiBuf = uiBuf + sprintf( uiBuf + szOutText, "\n" );
}

#endif // LDS_MR0_FORDEBUG_VERIFYCONVERTMESH


void BMD::ExtendVertice(Mesh_t* oM, VBOMesh* nM)
{
	int i, j =0;
	TempVec tbuf;

	//임시 버퍼 크기 확장 할당
	tbuf.reserve(oM->NumVertices*2);
	//일단 값을 다 채운다.
	for(i = 0; i < oM->NumVertices; ++i) tbuf.push_back(TempUnit(i, -1, -1, -1));
	//인덱스 루프
	for(i = 0; i < oM->NumTriangles; ++i)
	{
		Triangle_t* pTri = (Triangle_t*)&oM->Triangles[i];
		for(j = 0; j < 3; ++j)
		{
			//각 인덱스값을 얻고,
			short vIndex = pTri->VertexIndex[j];
			short tIndex = pTri->TexCoordIndex[j];
			short nIndex = pTri->NormalIndex[j];
//			short cIndex = pTri->m_ivIndexAdditional[j];

			//내용이 비었다. 
			if(tbuf[vIndex].t == -1)
			{
				tbuf[vIndex].t = tIndex;
				tbuf[vIndex].n = nIndex;
				nM->IBuffer.push_back(vIndex);
			}
			//같은 내용이다. 
			else if( (tbuf[vIndex].t == tIndex) && (tbuf[vIndex].n == nIndex) )
			{
				nM->IBuffer.push_back(vIndex);
			}
			//내용이 다르면 맨 끝에 추가하고 인덱스를 그곳으로 향하게 한다.  
			else
			{
				nM->IBuffer.push_back(tbuf.size());
				tbuf.push_back(TempUnit(vIndex, tIndex, nIndex, 0));		// 1. 실제로 그려질 IB들만 추가시킨다.
			}
		}
	}

	//VBO상 버택스들의 매핑 형태 규정
	bool bXMD = false;
	bool bVcolor = false;
	if(oM->NumVertexColors) bVcolor = true;
	if(oM->MeshAdditionalSkinInfo.size()) bXMD = true;
	if(!bVcolor && !bXMD) 
		nM->RealType = GL_C4F_N3F_V3F;
	else 
		nM->RealType = GL_T4F_C4F_N3F_V4F;

	//임시 버퍼의 내용을 바탕으로 값을 채운다.
	nM->VBuffer.reserve(tbuf.size());	//용량 할당
	for(i = 0; i < tbuf.size(); ++i)										// 2. 실 IB들 묶음 접근.
	{
		VertexUnit Nvertex;
		TempUnit& tp = tbuf[i];
		bool bfindbone = false;

		//위치값&본 등록
		short tBone = oM->Vertices[tp.v].Node;
		VectorCopy( oM->Vertices[tp.v].Position, Nvertex.m_vPos );
		for(j = 0; j < nM->BoneContainer.size(); ++j)
		{
			if(nM->BoneContainer[j] == tBone)
			{
				bfindbone = true;
				break;
			}
		}
		
		//현 메쉬에서 사용되고 있는 본의 값을 모두 누적시킨다.
		if(bfindbone)
			Nvertex.m_vPos[3] = j*3;
		else
		{
			nM->BoneContainer.push_back(tBone);
			Nvertex.m_vPos[3] = (nM->BoneContainer.size()-1)*3;
		}

		//텍스쳐 좌표 등록
		for(j = 0; j < 4; ++j) Nvertex.m_vTex[j] = 0.f;
		Nvertex.m_vTex[3] = 1.f;
		if(tp.t >= 0)
		{
			Nvertex.m_vTex[0] = oM->TexCoords[tp.t].TexCoordU;
			Nvertex.m_vTex[1] = oM->TexCoords[tp.t].TexCoordV;
		}
		
		//법선값 등록
		Vector(0.f, 0.f, 0.f, Nvertex.m_vNorm);
		if(tp.n >= 0)
			VectorCopy( oM->Normals[tp.n].Normal, Nvertex.m_vNorm );

		//라이트맵이면 버텍스컬러 사용치 않음
		if(bLightMap)
		{
			Nvertex.m_vCor[0] = (float)(oM->VertexColors[tp.c].m_Colors[0])/256.f;
			Nvertex.m_vCor[1] = (float)(oM->VertexColors[tp.c].m_Colors[1])/256.f;
			Nvertex.m_vCor[2] = Nvertex.m_vCor[3] = 1.f;
		}
		else
		{
			//버텍스컬러가 있으면 세팅
			if(bVcolor)
			{
				for(j = 0; j < 3; ++j)
					Nvertex.m_vCor[j] = (float)(oM->VertexColors[tp.c].m_Colors[j]) / 256.f;
			}
			else
			{	
				Vector(1.f, 1.f, 1.f, Nvertex.m_vCor);
			}
			Nvertex.m_vCor[3] = 1.f;
		}

		//만약 특정 버텍스 인덱스가 두번째 본의 정보를 갖고 있다면
		TempSkinInfoMap_t::iterator tSecondBoneIter = 
			oM->MeshAdditionalSkinInfo.find(tp.v);
		if(tSecondBoneIter != oM->MeshAdditionalSkinInfo.end())
		{
			TempSkinInfo_t& rSkinInfo = tSecondBoneIter->second;
			//웨이트 1,2번은 Texture 벡터의 z, w
			Nvertex.m_vTex[2] = rSkinInfo.m_fWeight0;
			Nvertex.m_vTex[3] = rSkinInfo.m_fWeight1;

			//본 컨테이너 작업을 하고, 본이 저장되는 위치는 컬러벡터의 w
			bfindbone = false;
			for(j = 0; j < nM->BoneContainer.size(); ++j)
			{
				if(nM->BoneContainer[j] == rSkinInfo.m_iBone1)
				{
					bfindbone = true;
					break;
				}
			}

			if(bfindbone)
				Nvertex.m_vCor[3] = j*3;
			else
			{
				nM->BoneContainer.push_back(rSkinInfo.m_iBone1);
				Nvertex.m_vCor[3] = (nM->BoneContainer.size()-1)*3;
			}
		}
		nM->VBuffer.push_back(Nvertex);
	}
	
	tbuf.clear();
}

#ifdef LDS_MR0_FORDEBUG_VERIFYCONVERTMESH
void BMD::ConvertMesh(bool bOutputRealtimeInfo)
#else // LDS_MR0_FORDEBUG_VERIFYCONVERTMESH
void BMD::ConvertMesh()
#endif // LDS_MR0_FORDEBUG_VERIFYCONVERTMESH
{
	int i, j = 0;
	if(NewMeshs.size())
		NewMeshs.clear();

#if defined(LDS_MR0_OUTPUT_GLERROR_INNER_GLRENDERER) && defined(_DEBUG)
	CheckGLError( __FILE__, __LINE__ );
#endif // defined(LDS_MR0_OUTPUT_GLERROR_INNER_GLRENDERER) && defined(_DEBUG)

	NewMeshs.reserve(NumMeshs);
	for(i = 0; i < NumMeshs; ++i)
	{
		VBOMesh nM;
		Mesh_t* pM = &Meshs[i];
		nM.NoneBlendMesh = pM->NoneBlendMesh;
		nM.Texture = pM->Texture;
		//인덱스는 2배로 확장
		nM.IBuffer.reserve(pM->NumTriangles*6);
		//본 컨테이너는 모델의 본 크기만큼 할당
		nM.BoneContainer.reserve(NumBones);
		//버텍스&인덱스 확장
		ExtendVertice(pM, &nM);
		//인덱스 버퍼 공간 정리
		nM.IBuffer.swap(UShortVector(nM.IBuffer));
		//본 컨테이너 공간 정리
		nM.BoneContainer.swap(ShortVector(nM.BoneContainer));
		
		//텍스쳐 스크립트 카피
		if(pM->m_csTScript)
		{
			nM.m_csTScript = new TextureScript(*(pM->m_csTScript));
		}

		//이펙트 스크립 카피
		nM.MeshEffectScript = pM->MeshEffectScript;

		NewMeshs.push_back(nM);
	}

#ifdef LDS_MR0_FORDEBUG_VERIFYCONVERTMESH
	if( true == bOutputRealtimeInfo )
	{
		ProcessVerification( NewMeshs );
	}
#endif // LDS_MR0_FORDEBUG_VERIFYCONVERTMESH
	
#if defined(LDS_MR0_OUTPUT_GLERROR_INNER_GLRENDERER) && defined(_DEBUG)
	CheckGLError( __FILE__, __LINE__ );
#endif // defined(LDS_MR0_OUTPUT_GLERROR_INNER_GLRENDERER) && defined(_DEBUG)
}

void PackBufferCNV(UnitVector& rVec, UnitVector2& rOut)
{
	VertexUnit2 oneUnit;
	for(UnitVector::iterator iter = rVec.begin(); iter != rVec.end(); ++iter)
	{
		VertexUnit& rSrcUnit = *iter;
		oneUnit.m_vPos[0] = rSrcUnit.m_vPos[0];
		oneUnit.m_vPos[1] = rSrcUnit.m_vPos[1];
		oneUnit.m_vPos[2] = rSrcUnit.m_vPos[2];

		VectorCopy(rSrcUnit.m_vNorm, oneUnit.m_vNorm);

		oneUnit.m_vTex[0] = rSrcUnit.m_vTex[0];
		oneUnit.m_vTex[1] = rSrcUnit.m_vTex[1];
		oneUnit.m_vTex[2] = 0.f;
		oneUnit.m_vTex[3] = rSrcUnit.m_vPos[3];

		rOut.push_back(oneUnit);
	}
}

void PackBufferTV(UnitVector& rVec, UnitVector3& rOut)
{
	VertexUnit3 oneUnit;
	for(UnitVector::iterator iter = rVec.begin(); iter != rVec.end(); ++iter)
	{
		VertexUnit& rSrcUnit = *iter;
		oneUnit.m_vPos[0] = rSrcUnit.m_vPos[0];
		oneUnit.m_vPos[1] = rSrcUnit.m_vPos[1];
		oneUnit.m_vPos[2] = rSrcUnit.m_vPos[2];
		oneUnit.m_vPos[3] = 1.f;

		oneUnit.m_vTex[0] = rSrcUnit.m_vTex[0];
		oneUnit.m_vTex[1] = rSrcUnit.m_vTex[1];
		oneUnit.m_vTex[2] = 0.f;
		oneUnit.m_vTex[3] = rSrcUnit.m_vPos[3];

		rOut.push_back(oneUnit);
	}
}

void BMD::UploadVBO( )
{
	
	//새 메쉬를 몽땅 VBO에 올린다.
	UnloadVBO();
	unsigned int MeshSize = NewMeshs.size();
	for(int i = 0; i < MeshSize; ++i)
	{
		VBOMesh& rM = NewMeshs[i];
		glGenBuffersARB(1, &rM.Vbo);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, rM.Vbo);

		//RealType이 기본타입이 아니라면 축약
		switch(rM.RealType)
		{
			case GL_T4F_C4F_N3F_V4F:
				{
					glBufferDataARB(GL_ARRAY_BUFFER_ARB, rM.VBuffer.size() * sizeof(VertexUnit), 
						&rM.VBuffer[0], GL_STATIC_DRAW_ARB );
				}
				break;
			case GL_C4F_N3F_V3F:
				{
					UnitVector2 vTmp;
					PackBufferCNV(rM.VBuffer, vTmp);
					glBufferDataARB(GL_ARRAY_BUFFER_ARB, vTmp.size() * sizeof(VertexUnit2), 
						&vTmp[0], GL_STATIC_DRAW_ARB );
				}
				break;
			case GL_T4F_V4F:
				{
					UnitVector3 vTmp;
					PackBufferTV(rM.VBuffer, vTmp);
					glBufferDataARB(GL_ARRAY_BUFFER_ARB, vTmp.size() * sizeof(VertexUnit3), 
						&vTmp[0], GL_STATIC_DRAW_ARB );
				}
				break;
		}

		//인덱스버퍼는 그대로 해도 상관 없다. 
		glGenBuffersARB(1, &rM.IdxVbo);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, rM.IdxVbo);
		glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, rM.IBuffer.size() * sizeof(short),
			&rM.IBuffer[0], GL_STATIC_DRAW_ARB );

		//다 날리고 로컬버퍼 앞 두 변수에 사이즈만 저장한다.
		unsigned int vsizeBackup = rM.VBuffer.size();
		unsigned int isizeBackup = rM.IBuffer.size();
		
		VertexUnit vBackup;
		vBackup.m_vTex[0] = (float)vsizeBackup;
		vBackup.m_vTex[1] = (float)isizeBackup;

		rM.VBuffer.clear();
		rM.VBuffer.reserve(1);
		rM.VBuffer.push_back(vBackup);
		
		rM.IBuffer.clear();
		rM.IBuffer.reserve(0);
	}
}

void BMD::UnloadVBO()
{
	//메쉬를 몽땅 내린다.
	unsigned int MeshSize = NewMeshs.size();
	for(int i = 0; i < MeshSize; ++i)
	{
		VBOMesh& rM = NewMeshs[i];
		unsigned int vSize, iSize = 0;
		vSize = (unsigned int) rM.VBuffer[0].m_vTex[0];
		iSize = (unsigned int) rM.VBuffer[0].m_vTex[1];

		if(rM.Vbo)
		{
			//VBO에 연결해서 로컬에 값을 카피해둔다.
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, rM.Vbo);
			char* pStart = (char* )glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_READ_ONLY);
			if(pStart)
			{
				rM.VBuffer.clear();
				rM.VBuffer.reserve(vSize);
				//하나씩 짤라서 카피한다. 
				switch(rM.RealType)
				{
					case GL_T4F_C4F_N3F_V4F:
						{
							for(int j = 0; j < vSize; ++j)
							{
								VertexUnit tUnit;
								char* pCur = pStart + sizeof(VertexUnit) * j;
								memcpy(&tUnit, pCur, sizeof(VertexUnit));
								rM.VBuffer.push_back(tUnit);
							}
						}
						break;

					case GL_C4F_N3F_V3F:
						{
							for(int j = 0; j < vSize; ++j)
							{
								VertexUnit tRstUnit;
								VertexUnit2 tUnit;
								char* pCur = pStart + sizeof(VertexUnit2) * j;
								memcpy(&tUnit, pCur, sizeof(VertexUnit2));

								VectorCopy(tUnit.m_vNorm, tRstUnit.m_vNorm);
								tRstUnit.m_vPos[0] = tUnit.m_vPos[0];
								tRstUnit.m_vPos[1] = tUnit.m_vPos[1];
								tRstUnit.m_vPos[2] = tUnit.m_vPos[2];
								tRstUnit.m_vPos[3] = tUnit.m_vTex[3];

								tRstUnit.m_vTex[0] = tUnit.m_vTex[0];
								tRstUnit.m_vTex[1] = tUnit.m_vTex[1];
								tRstUnit.m_vTex[2] = 0.f;
								tRstUnit.m_vTex[3] = 1.f;

								rM.VBuffer.push_back(tRstUnit);
							}
						}
						break;

					case GL_T4F_V4F:
						{
							for(int j = 0; j < vSize; ++j)
							{
								VertexUnit tRstUnit;
								VertexUnit3 tUnit;
								char* pCur = pStart + sizeof(VertexUnit3) * j;
								memcpy(&tUnit, pCur, sizeof(VertexUnit3));

								tRstUnit.m_vPos[0] = tUnit.m_vPos[0];
								tRstUnit.m_vPos[1] = tUnit.m_vPos[1];
								tRstUnit.m_vPos[2] = tUnit.m_vPos[2];
								tRstUnit.m_vPos[3] = tUnit.m_vTex[3];

								tRstUnit.m_vTex[0] = tUnit.m_vTex[0];
								tRstUnit.m_vTex[1] = tUnit.m_vTex[1];
								tRstUnit.m_vTex[2] = 0.f;
								tRstUnit.m_vTex[3] = 1.f;

								rM.VBuffer.push_back(tRstUnit);
							}
						}
						break;
				}
				
				// MR0_MODIFY : 종료시에 Null 접근 에러발생으로 
				//				하단에 정의된 unmap을 if(pStart) 내부로 옮김.
				glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
				
				//VBO를 날린다.
				glDeleteBuffersARB(1, &rM.Vbo);
				rM.Vbo = 0;
#if defined(LDS_MR0_OUTPUT_GLERROR_INNER_GLRENDERER) && defined(_DEBUG)
				CheckGLError( __FILE__, __LINE__ );
#endif // defined(LDS_MR0_OUTPUT_GLERROR_INNER_GLRENDERER) && defined(_DEBUG)
			}
			
 			rM.Vbo = 0;
		}

		if(rM.IdxVbo)
		{
			//VBO에 연결해서 로컬에 값을 카피해둔다.
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, rM.IdxVbo);
			char* pStart = (char* )glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, GL_READ_ONLY);
			if(pStart)
			{
#if defined(LDS_MR0_OUTPUT_GLERROR_INNER_GLRENDERER) && defined(_DEBUG)
				CheckGLError( __FILE__, __LINE__ );
#endif // defined(LDS_MR0_OUTPUT_GLERROR_INNER_GLRENDERER) && defined(_DEBUG)

				rM.IBuffer.clear();
				rM.IBuffer.reserve(iSize);
				for(int j = 0; j < iSize; ++j)
				{
					short tIdx;
					char* pCur = pStart + sizeof(short) * j;
					memcpy(&tIdx, pCur, sizeof(short));
					rM.IBuffer.push_back(tIdx);
				}

				// MR0_MODIFY : 종료시에 Null 접근 에러발생으로 
				//				하단에 정의된 unmap을 if(pStart) 내부로 옮김.
				glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);

				//VBO를 날린다.
				glDeleteBuffersARB(1, &rM.Vbo);
				rM.Vbo = 0;
#if defined(LDS_MR0_OUTPUT_GLERROR_INNER_GLRENDERER) && defined(_DEBUG)
				CheckGLError( __FILE__, __LINE__ );
#endif // defined(LDS_MR0_OUTPUT_GLERROR_INNER_GLRENDERER) && defined(_DEBUG)
			}

			if( rM.m_csTScript )
			{
				SAFE_DELETE( rM.m_csTScript );
			}
			
 			rM.IdxVbo = 0;
		}
	}
}

void BMD::DelOldMesh()
{
	for(int i=0;i<NumMeshs;i++)
	{
       	Mesh_t *m = &Meshs[i];
		SAFE_DELETE_ARRAY(m->Vertices);
		SAFE_DELETE_ARRAY(m->Normals);
		SAFE_DELETE_ARRAY(m->TexCoords);
		SAFE_DELETE_ARRAY(m->Triangles);
		if ( m->m_csTScript)
		{
			delete m->m_csTScript;
			m->m_csTScript = NULL;
		}
	}
//	SAFE_DELETE_ARRAY(Meshs);
}

#ifdef LDS_MR0_MODIFY_DIRECTLYAPPLY_SCALETM_INCLUDESHADER
bool _VBOMesh::RegisterBone( const float* BoneTransform )
{
	if(!BoneTransform) return false;
	int BoneNum = BoneContainer.size();
	
	if( BoneNum > (VPManager::GetMaxLocalParams() / 3)) return false; 	//너무 많다.

	for(int i = 0; i < BoneNum; ++i)
	{
		int iMatIdx = (BoneContainer[i])*12;
		for(int j = 0; j < 3; ++j)
		{
			const float* vTarget = BoneTransform + iMatIdx + j*4;
			
#ifdef LDS_MR0_FIXED_SHADERPARAM_WHICH_MODEL_BREAKED_DUETO_BONE
			VPManager::SetLocalParam4f(i*3+j + NUM_PRERESERVED_PARAM_GPUREGISTER,  
				*vTarget, 
				*(vTarget+1), 
				*(vTarget+2), 
				*(vTarget+3));
#else // LDS_MR0_FIXED_SHADERPARAM_WHICH_MODEL_BREAKED_DUETO_BONE
			VPManager::SetEnvParam4f(i*3+j, 
				*vTarget, 
				*(vTarget+1), 
				*(vTarget+2), 
				*(vTarget+3) );
#endif // LDS_MR0_FIXED_SHADERPARAM_WHICH_MODEL_BREAKED_DUETO_BONE
		}
	}
	
	return true;
}
#else // LDS_MR0_MODIFY_DIRECTLYAPPLY_SCALETM_INCLUDESHADER
#ifdef LDS_MR0_MODIFY_TRANSFORMSCALE_FORSILHOUETTE
bool _VBOMesh::RegisterBone(
							const float* BoneTransform, 
							bool bTrans, 
							vec3_t vTrans, 
							float fScale,
							bool bApplyRequestAbsoluteScale, 
							float fRequestAbsoluteScale )
#else // LDS_MR0_MODIFY_TRANSFORMSCALE_FORSILHOUETTE
bool _VBOMesh::RegisterBone(const float* BoneTransform, bool bTrans, vec3_t vTrans, float fScale)
#endif // LDS_MR0_MODIFY_TRANSFORMSCALE_FORSILHOUETTE
{
	if(!BoneTransform) return false;
	int BoneNum = BoneContainer.size();
	if( BoneNum > (VPManager::GetMaxLocalParams() / 3)) return false; 	//너무 많다.

	float	fResultScale = fScale;
	vec3_t	vTranslationResult;

	memset( vTranslationResult, 0, sizeof(vec3_t) );

	// 1. 
	if( true == bTrans )
	{
		fResultScale = fScale;
		VectorCopy( vTrans, vTranslationResult );
	}
	else
	{
		fResultScale = 1.f;
		Vector( 0.0f, 0.0f, 0.0f, vTranslationResult );
	}

#ifdef LDS_MR0_MODIFY_TRANSFORMSCALE_FORSILHOUETTE

	if( true == bApplyRequestAbsoluteScale )
	{
		// x,y,z,w 중 w 동차 좌표계에 곱해줄 Scale은 Sillouette 변환 스케일이 아닌 
		// 본래 해당 Object 의 고유 스케일이 필요 합니다.
		float fResultScalePre = fResultScale;	
		fResultScale = fRequestAbsoluteScale * fResultScale;


		for(int i = 0; i < BoneNum; ++i)
		{
			int iMatIdx = (BoneContainer[i])*12;
			for(int j = 0; j < 3; ++j)
			{
				const float* vTarget = BoneTransform + iMatIdx + j*4;
				float fTranslation = 0.f;
				
#ifdef LDS_MR0_FIXED_SHADERPARAM_WHICH_MODEL_BREAKED_DUETO_BONE
				VPManager::SetLocalParam4f(i*3+j + NUM_PRERESERVED_PARAM_GPUREGISTER,  
					*vTarget * fResultScale, 
					*(vTarget+1) * fResultScale, 
					*(vTarget+2) * fResultScale, 
					*(vTarget+3) * fResultScalePre + vTranslationResult[j] );
#else // LDS_MR0_FIXED_SHADERPARAM_WHICH_MODEL_BREAKED_DUETO_BONE
				VPManager::SetEnvParam4f(i*3+j, 
					*vTarget * fResultScale, 
					*(vTarget+1) * fResultScale, 
					*(vTarget+2) * fResultScale, 
					*(vTarget+3) * fResultScalePre + vTranslationResult[j] );
#endif // LDS_MR0_FIXED_SHADERPARAM_WHICH_MODEL_BREAKED_DUETO_BONE
			}
		}
 	}
 	else
 	{
 		for(int i = 0; i < BoneNum; ++i)
 		{
 			int iMatIdx = (BoneContainer[i])*12;
 			for(int j = 0; j < 3; ++j)
 			{
 				const float* vTarget = BoneTransform + iMatIdx + j*4;

#ifdef LDS_MR0_FIXED_SHADERPARAM_WHICH_MODEL_BREAKED_DUETO_BONE
				VPManager::SetLocalParam4f(i*3+j + NUM_PRERESERVED_PARAM_GPUREGISTER, 
					*vTarget * fResultScale, 
					*(vTarget+1) * fResultScale, 
					*(vTarget+2) * fResultScale, 
 					*(vTarget+3) * fResultScale + vTranslationResult[j] );
#else // LDS_MR0_FIXED_SHADERPARAM_WHICH_MODEL_BREAKED_DUETO_BONE
 				VPManager::SetEnvParam4f(i*3+j, 
 					*vTarget * fResultScale, 
 					*(vTarget+1) * fResultScale, 
 					*(vTarget+2) * fResultScale, 
 					*(vTarget+3) * fResultScale + vTranslationResult[j] );
#endif // LDS_MR0_FIXED_SHADERPARAM_WHICH_MODEL_BREAKED_DUETO_BONE
 			}
 		}
 	}

#else // LDS_MR0_MODIFY_TRANSFORMSCALE_FORSILHOUETTE
	
	for(int i = 0; i < BoneNum; ++i)
	{
		int iMatIdx = (BoneContainer[i])*12;
		for(int j = 0; j < 3; ++j)
		{
			const float* vTarget = BoneTransform + iMatIdx + j*4;
			
#ifdef LDS_MR0_FIXED_SHADERPARAM_WHICH_MODEL_BREAKED_DUETO_BONE
			VPManager::SetLocalParam4f(i*3+j + NUM_PRERESERVED_PARAM_GPUREGISTER, 
				*vTarget * fResultScale, 
				*(vTarget+1) * fResultScale, 
				*(vTarget+2) * fResultScale, 
				*(vTarget+3) * fResultScale + vTranslationResult[j]);
#else // LDS_MR0_FIXED_SHADERPARAM_WHICH_MODEL_BREAKED_DUETO_BONE
			VPManager::SetEnvParam4f(i*3+j, 
				*vTarget * fResultScale, 
				*(vTarget+1) * fResultScale, 
				*(vTarget+2) * fResultScale, 
				*(vTarget+3) * fResultScale + vTranslationResult[j]);
#endif // LDS_MR0_FIXED_SHADERPARAM_WHICH_MODEL_BREAKED_DUETO_BONE
		}
	}

#endif // LDS_MR0_MODIFY_TRANSFORMSCALE_FORSILHOUETTE
	
	return true;
}
#endif // LDS_MR0_MODIFY_DIRECTLYAPPLY_SCALETM_INCLUDESHADER

//위 함수처럼 구현해둔다.
bool _VBOMesh::RegisterBone(float (*BoneTransform)[3][4], bool bTrans, vec3_t vTrans, float fScale)
{
	if(!BoneTransform) return false;
	int BoneNum = BoneContainer.size();
	if( BoneNum > (VPManager::GetMaxLocalParams() / 3)) return false; 	//너무 많다.
	for(int i = 0; i < BoneNum; ++i)
	{
		VPManager::SetEnvParamMatrix(i*3,BoneTransform[BoneContainer[i]]);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////

#endif //MR0

#ifdef LDS_FIX_MEMORYLEAK_DESTRUCTORFORBMD
BMD::~BMD()
{	
	Release();
}
#endif // LDS_FIX_MEMORYLEAK_DESTRUCTORFORBMD

#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
void BMD::InterpolationTrans(float (*Mat1)[4], float (*TransMat2)[4], float _Scale)
{
	TransMat2[0][3] = TransMat2[0][3] - (TransMat2[0][3] - Mat1[0][3]) * (1-_Scale);
	TransMat2[1][3] = TransMat2[1][3] - (TransMat2[1][3] - Mat1[1][3]) * (1-_Scale);
	TransMat2[2][3] = TransMat2[2][3] - (TransMat2[2][3] - Mat1[2][3]) * (1-_Scale);
}
#endif //PBG_ADD_NEWCHAR_MONK_ITEM