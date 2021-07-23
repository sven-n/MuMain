#include "stdafx.h"

#ifdef MR0

//////////////////////////////////////////////////////////////////////////
// 
// RenderMachine.cpp : 쉐이더 타입별 렌더러 머신 정의 (VertexShaderRenderer, Pixel Shader Renderer)
//
//////////////////////////////////////////////////////////////////////////

#include "GLUtils.h"
#include "ZzzBMD.h"
#include "ZzzTexture.h"
#include "VPManager.h"
#include "TextureScript.h"

#include "RenderMachine.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"

CRenderMachine*	g_Renderer = NULL;

using namespace ModelManager;
using namespace VPManager;

//////////////////////////////////////////////////////////////////////////
// 디버깅 용도의 스크립트 출력. (행렬)
//////////////////////////////////////////////////////////////////////////
void PrintMatrix(float* p, FILE* fp)
{
	for(int i = 0; i < 3; ++i)
	{
		for(int j = 0; j < 4; ++j)
		{
			int idx = i*4+j;
			float* pCur = p;
			pCur += idx;
			fprintf(fp, "%f ", *pCur);
		}
		fprintf(fp, "\n");
	}
}

//////////////////////////////////////////////////////////////////////////
// 디버깅 용도의 스크립트 출력. (행렬)
//////////////////////////////////////////////////////////////////////////
void PrintBone(float* p, unsigned int iSize)
{
	FILE* fp = fopen("BonePrint.txt", "at");
	for(unsigned int i = 0; i < iSize; ++i)
	{
		float* pCur = p + i*12;
		fprintf(fp, "%d\n", i);
		PrintMatrix(pCur, fp);
		fprintf(fp, "\n");
	}
	fclose(fp);
}

//////////////////////////////////////////////////////////////////////////
//	CVBOShaderMachine
//	버텍스 쉐이더만 이용하는 버전
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Mesh단위를 렌더링 : 버텍스 쉐이더 렌더러 머신 
//////////////////////////////////////////////////////////////////////////
void CVBOShaderMachine::Render(ModelManager::MeshRender_t& r)
{
	BMD& rModel = *r.m_pSourceBMD;
	Mesh_t& rOldMesh = rModel.Meshs[r.m_iMesh];
	VBOMesh& rNewMesh = rModel.NewMeshs[r.m_iMesh];

	if(!rNewMesh.Vbo) return;

#ifdef _DEBUG
	CheckGLError( __FILE__, __LINE__ );
#endif // _DEBUG

	//쉐이더 설정
	if(r.m_bUseFixedVP)
		SelectVPSProgram(r.m_iVProgram);
	else
		SelectVProgram(r.m_iVProgram);

	//깊이버퍼 설정
	if(r.m_iFlag & RENDER_NODEPTH) DisableDepthTest();
	else EnableDepthTest();

	//텍스쳐 선택
	if(r.m_iFlag & RENDER_COLOR) 
	{
		DisableTexture();
	}
	else
	{
		glEnable(GL_TEXTURE_2D);
		BindTexture(r.m_iTex);
	}

	//블렌드
#ifdef LDS_MR0_MODIFY_MISSRENDERSHADOW_MOSTMESH
	if(r.m_iFlag & RENDER_LIGHTMAP)
	{
		EnableLightMap();
	}
	else if(r.m_iFlag & RENDER_DARK)
	{
		EnableAlphaBlendMinus();
	}
	else if(r.m_iFlag & RENDER_BRIGHT)
	{
		EnableAlphaBlend();
	}
	// SHADOWMAP인경우에는 Render Statement부분에 해줄것이 없습니다.
//  else if(r.m_iFlag & RENDER_SHADOWMAP)
//  {
//  }
	else if(r.m_fAlpha < 0.99f)
	{
		// 중요 : LDS_MR0_MODIFY_MISSRENDERSHADOW_MOSTMESH 작업으로
		//		대부분은 MR0 AddRender 처리시에 BITMAP_HIDE경우 바로 반환하여 이곳 Render로 넘어올 일이 없으나,
		//		RENDER_SHADOW의 경우는 BITMAP_HIDE라 하더라도 무조건 Render 처리를 합니다.
		EnableAlphaTest();	
	}
	else if( BITMAP_HIDE != r.m_iTex )
	{
		if(Bitmaps[r.m_iTex].Components == 4)
		{
			EnableAlphaTest();
		}
		else
		{
			DisableAlphaBlend();
		}
	}
	else
	{
		DisableAlphaBlend();
	}

#else // LDS_MR0_MODIFY_MISSRENDERSHADOW_MOSTMESH

	if(r.m_iFlag & RENDER_LIGHTMAP)
	{
		EnableLightMap();
	}
	else if(r.m_iFlag & RENDER_DARK)
	{
		EnableAlphaBlendMinus();
	}
	else if(r.m_iFlag & RENDER_BRIGHT)
	{
		EnableAlphaBlend();
	}
	else if(r.m_fAlpha < 0.99f || (Bitmaps[r.m_iTex].Components == 4))
	{
		EnableAlphaTest();	
	}
	else
	{
		DisableAlphaBlend();
	}

#endif // LDS_MR0_MODIFY_MISSRENDERSHADOW_MOSTMESH
	
	// ********************* Shader Parameter 입력 시작 부분. Parameter 1번-7번 // 8번 이후는 Bone TM

	//7. 파라미터 1번 - 7번 (스케일, 모델컬러, 라이트 정보 등등..)
	//상수 등록 : 0번과 5번은 정해진 값 -> 기존 버전과 호환맞추기 위해

	SetLocalParam4f(0, r.m_matConstants[0].x, r.m_matConstants[0].y, r.m_matConstants[0].z, r.m_matConstants[0].w);
	if(r.m_bLight) 
		SetLocalParam4f(5, r.m_matConstants[1].x, r.m_matConstants[1].y, r.m_matConstants[1].z, r.m_matConstants[1].w);

	//1~4번을 그 뒤로
	for(unsigned int i = 2; (i < 5 && i < r.m_iNumConst); ++i)
	{	
		SetLocalParam4f(i-1, r.m_matConstants[i].x, r.m_matConstants[i].y, 
			r.m_matConstants[i].z, r.m_matConstants[i].w);
	}

	bool bApplyRequestAbsoluteScale = false;
	float fScaleRequest = 0.0f, fScaleBais = 0.0f;
	fScaleBais		= r.m_pSourceBMD->BodyScale;
	fScaleRequest	= r.m_pSourceBMD->m_fRequestScale;
	
	if( fScaleRequest != 1.0f && fScaleRequest != 0.0f )
	{
		bApplyRequestAbsoluteScale = true;
	}
	else
	{
		bApplyRequestAbsoluteScale = false;
	}

	
#ifdef LDS_MR0_MODIFY_DIRECTLYAPPLY_SCALETM_INCLUDESHADER
	//7-1. 6번, 7번은 ModelScale 값으로 설정.	// ScaleX, ScaleY, ScaleZ
	vec4_t		v4Scale, v4Translation;		// Translation[3]값(w)는 어떤 값이 들어가도 상관없다. Shader에서 Constant하게 처리.
	vec3_t		v3TranslationStandard;
	float		fScaleStandard = 0.0f;

	//7-2. 쉐이더로 넘길 Translation[0,1,2,3]과 Scale[0,1,2,3]의 기준값 설정.
	VectorCopy( r.m_pSourceBMD->BodyOrigin, v3TranslationStandard );
 	if( true == g_pMeshMachine->GetTransOp() )	// bTrans == true
 	{
		fScaleStandard = r.m_pSourceBMD->BodyScale;
		VectorCopy( v3TranslationStandard, v4Translation );
 	}
	else 
	{
		fScaleStandard = 1.0f;
		v4Translation[0] = v4Translation[1] = v4Translation[2] = v4Translation[3] = 0.0f;
	}

	//7-3. 쉐이더로 넘길 Translation[0,1,2,3]과 Scale[0,1,2,3]의 추가값 설정.
	if( true == bApplyRequestAbsoluteScale )
	{
		// x,y,z,w 중 w 동차 좌표계에 곱해줄 Scale은 Sillouette 변환 스케일이 아닌 
		// 본래 해당 Object 의 고유 스케일이 필요 합니다.
		float fResultScalePre = fScaleStandard;
		fScaleStandard = fScaleRequest * fScaleStandard;

		v4Scale[0] = fScaleStandard;
		v4Scale[1] = fScaleStandard;
		v4Scale[2] = fScaleStandard;
		v4Scale[3] = fResultScalePre;
	}
	else
	{
		v4Scale[0] = fScaleStandard;
		v4Scale[1] = fScaleStandard;
		v4Scale[2] = fScaleStandard;
		v4Scale[3] = fScaleStandard;		
	}
	
 	SetLocalParam4f(6, 
		v4Scale[0], v4Scale[1], v4Scale[2], v4Scale[3]);
 	SetLocalParam4f(7, 
		v4Translation[0], v4Translation[1], v4Translation[2], v4Translation[3]);

		
	//그 뒤는 8번 뒤에.
	if(r.m_matConstants.size() > 8)
	{
		for(i = 8; i < r.m_matConstants.size(); ++i)
			SetLocalParam4f(i, r.m_matConstants[i].x, r.m_matConstants[i].y, 
			r.m_matConstants[i].z, r.m_matConstants[i].w);
	}
	//그 뒤로는 본 등록
#else // LDS_MR0_MODIFY_DIRECTLYAPPLY_SCALETM_INCLUDESHADER
	//그 뒤는 5번 뒤에.
	if(r.m_matConstants.size() > 6)
	{
		for(i = 6; i < r.m_matConstants.size(); ++i)
			SetLocalParam4f(i, r.m_matConstants[i].x, r.m_matConstants[i].y, 
			r.m_matConstants[i].z, r.m_matConstants[i].w);
	}
	//그 뒤로는 본 등록
#endif // LDS_MR0_MODIFY_DIRECTLYAPPLY_SCALETM_INCLUDESHADER

// 	// For TESTDEBUG
// 	// ===============================================================================================
// 	{
// 		const bool bOutputDebuginfo = true;
// 		if( true == bOutputDebuginfo )
// 		{
// 			if( rModel.m_iBMDSeqID == 4965 )		// 전설의 
// 			{
// 				int		iBreakPoint = 0;
// 				static int iCounting = 0;
// 				
// 				char	szOutput[1024];
// 				int		iBuf_ = 0;
// 				
// 				iBuf_ = iBuf_ + sprintf( szOutput+iBuf_, "\t%d.두번째RenderFlag(%d)/bLight(%d)/Tex(%d)/iVProgram(shader)(%d)", 
// 					++iCounting,r.m_iFlag,r.m_bLight,r.m_iTex, r.m_iVProgram );
// 				iBuf_ = iBuf_ + sprintf( szOutput+iBuf_, "\n" );
// 				iBuf_ = iBuf_ + sprintf( szOutput+iBuf_, "\t\t" );
// 				
// 				// Shader parameter 0
// 				iBuf_ = iBuf_ + sprintf( szOutput + iBuf_,
// 					" <%d>SetLocalParam4f(%1.3f,%1.3f,%1.3f,%1.3f)," , 
// 					i-1, r.m_matConstants[0].x, r.m_matConstants[0].y, 
// 					r.m_matConstants[0].z, r.m_matConstants[0].w);
// 				
// 				// Shader parameter 1
// 				iBuf_ = iBuf_ + sprintf( szOutput + iBuf_,
// 					" <%d>SetLocalParam4f(%1.3f,%1.3f,%1.3f,%1.3f)," , 
// 					5, r.m_matConstants[1].x, r.m_matConstants[1].y, 
// 					r.m_matConstants[1].z, r.m_matConstants[1].w);
// 				
// 				// shader parameter 2 ~ 4
// 				for(unsigned int i = 2; (i < 5 && i < r.m_iNumConst); ++i)
// 				{
// 					iBuf_ = iBuf_ + sprintf( szOutput + iBuf_,
// 						"<%d>SetLocalParam4f(%1.3f,%1.3f,%1.3f,%1.3f)," , i-1, r.m_matConstants[i].x, r.m_matConstants[i].y, 
// 						r.m_matConstants[i].z, r.m_matConstants[i].w);
// 				}
// 				iBuf_ = iBuf_ + sprintf( szOutput+iBuf_, "\n" );
// 				
// 				OutputDebugString( szOutput );
// 			}
// 		}
// 	}
// 	// ===============================================================================================
// 	// For TESTDEBUG

	//8. BoneAnimationTM을 쉐이더 파라미터 로 저장하고, 파라미터 8번 부터 70개 가량은 BoneTM 전용
#ifdef LDS_MR0_MODIFY_DIRECTLYAPPLY_SCALETM_INCLUDESHADER
	rNewMesh.RegisterBone(m_pMat);
#else // LDS_MR0_MODIFY_DIRECTLYAPPLY_SCALETM_INCLUDESHADER
	#ifdef LDS_MR0_MODIFY_TRANSFORMSCALE_FORSILHOUETTE

		rNewMesh.RegisterBone( m_pMat, 
							g_pMeshMachine->GetTransOp(), 
							r.m_pSourceBMD->BodyOrigin, 
							r.m_pSourceBMD->BodyScale, 
							bApplyRequestAbsoluteScale, 
							fScaleRequest );

	#else // LDS_MR0_MODIFY_TRANSFORMSCALE_FORSILHOUETTE
		rNewMesh.RegisterBone(m_pMat, g_pMeshMachine->GetTransOp(), r.m_pSourceBMD->BodyOrigin, r.m_pSourceBMD->BodyScale);
	#endif // LDS_MR0_MODIFY_TRANSFORMSCALE_FORSILHOUETTE
#endif // LDS_MR0_MODIFY_DIRECTLYAPPLY_SCALETM_INCLUDESHADER

#ifdef _DEBUG
		CheckGLError( __FILE__, __LINE__ );
#endif // _DEBUG

	//9. 마지막으로 실제 GPU에게 렌더링을 실제 요청 한다.
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, rNewMesh.Vbo);
	glInterleavedArrays(rNewMesh.RealType, 0, NULL);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, rNewMesh.IdxVbo);
	short IndexSize = (short)rNewMesh.VBuffer[0].m_vTex[1];
	glDrawRangeElements(GL_TRIANGLES, 0, IndexSize-1, IndexSize, GL_UNSIGNED_SHORT, NULL);

#ifdef _DEBUG
	CheckGLError( __FILE__, __LINE__ );
#endif // _DEBUG
}

//////////////////////////////////////////////////////////////////////////
// 에니메이션 결과 World TM : 전역적으로 한개 모델만 설정.
//////////////////////////////////////////////////////////////////////////
void CVBOShaderMachine::Transform(BMD* m, float (*BoneMatrix)[3][4])
{
	m_pMat = BoneMatrix[0][0];
}

//////////////////////////////////////////////////////////////////////////
//	CVBOPixelShaderMachine : 미사용 (추후 구현)
//	픽쉘 쉐이더까지 모두 이용하는 버전
//////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////
//	COriginMachine : 미사용 (추후 구현)
//	낮은 사양 호환  // 드라이버가 쉐이더 지원 하지 않는경우 
//////////////////////////////////////////////////////////////////////////

//  Light Vector
static vec3_t LightVector = { 0.f, -0.1f, -0.8f };
static vec3_t LightVector2 = { 0.f, -0.5f, -0.8f };

void COriginMachine::Render(MeshRender_t& rPatch)
{
/*
	ModelInfo& meshInfo = ModelManager::GetModelInfo(model->m_ID);
	RenderInfo& mInfo = meshInfo.m_RenderInfo;
    if (mInfo.m_MeshIndex >= model->NumMeshs || mInfo.m_MeshIndex < 0 ) return;

    Mesh_t *m = &model->Meshs[mInfo.m_MeshIndex];
	if(m->NumTriangles == 0) return;

	float Wave = (int)WorldTime % 10000 * 0.0001f;

	//텍스쳐
	int Texture = model->IndexTexture[m->Texture];
    if(Texture == BITMAP_HIDE && !(mInfo.m_RenderFlag&RENDER_SHADOWMAP)) return;
	if( mInfo.m_PriorTex != -1 )
		Texture = mInfo.m_PriorTex;

	if( Texture < 0)
		glDisable(GL_TEXTURE_2D);
	else
		glEnable(GL_TEXTURE_2D);

	if( mInfo.m_Alpha < 0.99f)
		glColor4f(model->BodyLight[0], model->BodyLight[1], model->BodyLight[2], mInfo.m_Alpha);
	else
		glColor3f(model->BodyLight[0], model->BodyLight[1], model->BodyLight[2]);

	bool EnableWave = false;
    int streamMesh = model->StreamMesh;
    if ( m->m_csTScript!=NULL )
    {
        if ( m->m_csTScript->getStreamMesh() )
        {
            streamMesh = mInfo.m_MeshIndex;
        }
    }
	if((mInfo.m_MeshIndex==mInfo.m_IdxBlendMesh ||mInfo.m_MeshIndex==streamMesh) 
		&& (mInfo.m_BlendMeshUV[0]!=0.f || mInfo.m_BlendMeshUV[1]!=0.f))
    	EnableWave = true;

	bool EnableLight = model->LightEnable;
	if(mInfo.m_MeshIndex==model->StreamMesh)
	{
		glColor3fv(model->BodyLight);
		EnableLight = false;
	}
	else if(EnableLight)
	{
		for(int j=0;j<m->NumNormals;j++)
		{
			VectorScale(model->BodyLight,IntensityTransform[mInfo.m_MeshIndex][j],LightTransform[mInfo.m_MeshIndex][j]);
		}
	}

	int Render = mInfo.m_RenderFlag;
	if((Render&RENDER_COLOR) == RENDER_COLOR)
	{
    	Render = RENDER_COLOR;
       	if((mInfo.m_RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
			EnableAlphaBlend();
		else if((mInfo.m_RenderFlag&RENDER_DARK) == RENDER_DARK)
     		EnableAlphaBlendMinus();
		else
			DisableAlphaBlend();

        if ((mInfo.m_RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
        {
            DisableDepthTest ();				
        }

        DisableTexture();
		if(mInfo.m_Alpha >= 0.99f)
        {
            glColor3fv(model->BodyLight);
        }
		else
        {
            EnableAlphaTest();
			glColor4f(model->BodyLight[0],model->BodyLight[1],model->BodyLight[2],mInfo.m_Alpha);
        }
 	}
	else if ( (Render&RENDER_CHROME)==RENDER_CHROME     || 
              (Render&RENDER_CHROME2)==RENDER_CHROME2   ||
              (Render&RENDER_CHROME3)==RENDER_CHROME3   ||
              (Render&RENDER_CHROME4)==RENDER_CHROME4   ||
              (Render&RENDER_CHROME5)==RENDER_CHROME5   ||
			  (Render&RENDER_CHROME6)==RENDER_CHROME6	||
              (Render&RENDER_METAL)==RENDER_METAL       ||
              (Render&RENDER_OIL)==RENDER_OIL
            )
	{
		if ( m->m_csTScript!=NULL )
        {
            if ( m->m_csTScript->getNoneBlendMesh() ) return;
        }
		if(m->NoneBlendMesh )
			return;
   		Render = RENDER_CHROME;
        if ( (mInfo.m_RenderFlag&RENDER_CHROME4)==RENDER_CHROME4 )
        {
            Render = RENDER_CHROME4;
        }
        if ( (mInfo.m_RenderFlag&RENDER_OIL)==RENDER_OIL )
        {
            Render = RENDER_OIL;
        }
        float Wave2 = (int)WorldTime%5000 * 0.00024f - 0.4f;
        vec3_t L = { (float)(cos(WorldTime*0.001f)), (float)(sin(WorldTime*0.002f)), 1.f };
		for(int j=0;j<m->NumNormals;j++)
		{
            if (j > MAX_VERTICES ) break;
			float *Normal = NormalTransform[mInfo.m_MeshIndex][j];

            if((mInfo.m_RenderFlag&RENDER_CHROME2)==RENDER_CHROME2)
			{
				g_chrome[j][0] = (Normal[2]+Normal[0])*0.8f + Wave2*2.f;
				g_chrome[j][1] = (Normal[1]+Normal[0])*1.0f + Wave2*3.f;
			}
            else if((mInfo.m_RenderFlag&RENDER_CHROME3)==RENDER_CHROME3)
            {
                g_chrome[j][0] = DotProduct ( Normal, LightVector );
                g_chrome[j][1] = 1.f-DotProduct ( Normal, LightVector );
            }
            else if((mInfo.m_RenderFlag&RENDER_CHROME4)==RENDER_CHROME4)
            {
                g_chrome[j][0] = DotProduct ( Normal, L );
                g_chrome[j][1] = 1.f-DotProduct ( Normal, L );
				g_chrome[j][1] -= Normal[2]*0.5f + Wave*3.f;
				g_chrome[j][0] += Normal[1]*0.5f + L[1]*3.f;
            }
            else if((mInfo.m_RenderFlag&RENDER_CHROME5)==RENDER_CHROME5)
            {
                g_chrome[j][0] = DotProduct ( Normal, L );
                g_chrome[j][1] = 1.f-DotProduct ( Normal, L );
				g_chrome[j][1] -= Normal[2]*2.5f + Wave*1.f;
				g_chrome[j][0] += Normal[1]*3.f + L[1]*5.f;
            }

            else if((mInfo.m_RenderFlag&RENDER_CHROME6)==RENDER_CHROME6)
			{
				g_chrome[j][0] = (Normal[2]+Normal[0])*0.8f + Wave2*2.f;
				g_chrome[j][1] = (Normal[2]+Normal[0])*0.8f + Wave2*2.f;
			}

            else if((mInfo.m_RenderFlag&RENDER_OIL)==RENDER_OIL)
            {
				g_chrome[j][0] = Normal[0];
				g_chrome[j][1] = Normal[1];
            }
            else if((mInfo.m_RenderFlag&RENDER_CHROME)==RENDER_CHROME)
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

        if ( (mInfo.m_RenderFlag&RENDER_CHROME3)==RENDER_CHROME3 || (mInfo.m_RenderFlag&RENDER_CHROME4)==RENDER_CHROME4 
			|| (mInfo.m_RenderFlag&RENDER_CHROME5)==RENDER_CHROME5 )
        {
			if ( mInfo.m_Alpha < 0.99f)
			{
				model->BodyLight[0] *= mInfo.m_Alpha; 
				model->BodyLight[1] *= mInfo.m_Alpha; 
				model->BodyLight[2] *= mInfo.m_Alpha;
			}
     		EnableAlphaBlend();
        }
        else if((mInfo.m_RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
		{
			if ( mInfo.m_Alpha < 0.99f)
			{
				model->BodyLight[0] *= mInfo.m_Alpha; 
				model->BodyLight[1] *= mInfo.m_Alpha; 
				model->BodyLight[2] *= mInfo.m_Alpha;
			}
     		EnableAlphaBlend();
		}
		else if((mInfo.m_RenderFlag&RENDER_DARK) == RENDER_DARK)
     		EnableAlphaBlendMinus();
     	else if((mInfo.m_RenderFlag&RENDER_LIGHTMAP) == RENDER_LIGHTMAP)
            EnableLightMap();
		else if ( mInfo.m_Alpha >= 0.99f)
		{
			DisableAlphaBlend();
		}
		else
		{
			EnableAlphaTest();
		}

        if ((mInfo.m_RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
        {
            DisableDepthTest ();				
        }

        if((mInfo.m_RenderFlag&RENDER_CHROME2)==RENDER_CHROME2 && mInfo.m_PriorTex==-1)
        {
			BindTexture(BITMAP_CHROME2);
        }
        else if((mInfo.m_RenderFlag&RENDER_CHROME3)==RENDER_CHROME3 && mInfo.m_PriorTex==-1)
        {
			BindTexture(BITMAP_CHROME2);
        }
        else if((mInfo.m_RenderFlag&RENDER_CHROME4)==RENDER_CHROME4 && mInfo.m_PriorTex==-1)
        {
			BindTexture(BITMAP_CHROME2);
        }	
        else if((mInfo.m_RenderFlag&RENDER_CHROME6)==RENDER_CHROME6 && mInfo.m_PriorTex==-1)
        {
			BindTexture(BITMAP_CHROME6);
        }
        else if((mInfo.m_RenderFlag&RENDER_CHROME)==RENDER_CHROME && mInfo.m_PriorTex==-1)
			BindTexture(BITMAP_CHROME);
		else if((mInfo.m_RenderFlag&RENDER_METAL)==RENDER_METAL && mInfo.m_PriorTex==-1)
			BindTexture(BITMAP_SHINY);
		else
			BindTexture(Texture);
	}
	else if(mInfo.m_IdxBlendMesh <=-2 || m->Texture == mInfo.m_IdxBlendMesh)
	{
    	Render = RENDER_TEXTURE;
   		BindTexture(Texture);
		if((mInfo.m_RenderFlag&RENDER_DARK) == RENDER_DARK)
     		EnableAlphaBlendMinus();
		else
     		EnableAlphaBlend();

        if ((mInfo.m_RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
        {
            DisableDepthTest ();				
        }

		glColor3f(model->BodyLight[0]*mInfo.m_BlendMeshLight,
			model->BodyLight[1]*mInfo.m_BlendMeshLight,
			model->BodyLight[2]*mInfo.m_BlendMeshLight);
		EnableLight = false;
	}
	else if((mInfo.m_RenderFlag&RENDER_TEXTURE) == RENDER_TEXTURE)
	{
    	Render = RENDER_TEXTURE;
		BindTexture(Texture);
		if((mInfo.m_RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
     		EnableAlphaBlend();
		else if((mInfo.m_RenderFlag&RENDER_DARK) == RENDER_DARK)
     		EnableAlphaBlendMinus();
		else if(mInfo.m_Alpha<0.99f || Bitmaps[Texture].Components==4)
			EnableAlphaTest();
		else
			DisableAlphaBlend();

        if ((mInfo.m_RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
        {
            DisableDepthTest ();				
        }
	}
	else if((mInfo.m_RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
	{
		if(Bitmaps[Texture].Components==4 || m->Texture == mInfo.m_IdxBlendMesh)
			return;
    	Render = RENDER_BRIGHT;
        EnableAlphaBlend();
        DisableTexture();
        DisableDepthMask();

        if ((mInfo.m_RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
        {
            DisableDepthTest ();				
        }
	}
	else if((mInfo.m_RenderFlag&RENDER_SHADOWMAP) == RENDER_SHADOWMAP)
	{
		DisableAlphaBlend();
		if ( mInfo.m_Alpha >= 0.99f )
			glColor3f( 0.f, 0.f, 0.f );
		else
			glColor4f( 0.f, 0.f, 0.f,mInfo.m_Alpha);

		DisableTexture();
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
						glTexCoord2f(texp->TexCoordU+mInfo.m_BlendMeshUV[0],texp->TexCoordV+mInfo.m_BlendMeshUV[1]);
					else
						glTexCoord2f(texp->TexCoordU,texp->TexCoordV);
					if(EnableLight)
					{
						int ni = tp->NormalIndex[k];
						if(mInfo.m_Alpha >= 0.99f)
						{
							glColor3fv(LightTransform[mInfo.m_MeshIndex][ni]);
						}
						else
						{
							float *Light = LightTransform[mInfo.m_MeshIndex][ni];
							glColor4f(Light[0],Light[1],Light[2],mInfo.m_Alpha);
						}
					}
					break;
				}
			case RENDER_CHROME:
				{
					if(mInfo.m_Alpha >= 0.99f)
						glColor3fv(model->BodyLight);
					else
						glColor4f(model->BodyLight[0],model->BodyLight[1],model->BodyLight[2],mInfo.m_Alpha);
					int ni = tp->NormalIndex[k];  
					glTexCoord2f(g_chrome[ni][0],g_chrome[ni][1]);
					break;
				}
            case RENDER_CHROME4:
				{
					if(mInfo.m_Alpha >= 0.99f)
						glColor3fv(model->BodyLight);
					else
						glColor4f(model->BodyLight[0],model->BodyLight[1],model->BodyLight[2],mInfo.m_Alpha);
					int ni = tp->NormalIndex[k];  
					glTexCoord2f(g_chrome[ni][0]+mInfo.m_BlendMeshUV[0],g_chrome[ni][1]+mInfo.m_BlendMeshUV[1]);
				}
                break;

			case RENDER_OIL:
				{
					if(mInfo.m_Alpha >= 0.99f)
						glColor3fv(model->BodyLight);
					else
						glColor4f(model->BodyLight[0],model->BodyLight[1],model->BodyLight[2],mInfo.m_Alpha);
					TexCoord_t *texp = &m->TexCoords[tp->TexCoordIndex[k]];
					int ni = tp->VertexIndex[k];  
					glTexCoord2f(g_chrome[ni][0]*texp->TexCoordU+mInfo.m_BlendMeshUV[0],g_chrome[ni][1]*texp->TexCoordV+mInfo.m_BlendMeshUV[1]);
					break;
				}
			}

            if ((mInfo.m_RenderFlag&RENDER_SHADOWMAP)==RENDER_SHADOWMAP )
            {
                int vi = tp->VertexIndex[k];  
                vec3_t Position;
                VectorSubtract(VertexTransform[mInfo.m_MeshIndex][vi],model->BodyOrigin,Position);

                Position[0] += Position[2]*(Position[0]+2000.f)/(Position[2]-4000.f);
                Position[2] = 5.f;
                
                VectorAdd(Position,model->BodyOrigin,Position);
                glVertex3fv(Position);
            }
			else if((mInfo.m_RenderFlag&RENDER_WAVE_EXT)==RENDER_WAVE_EXT)
			{
				float vPos[3];
				float fParam = ( float)( ( int)WorldTime + vi * 931)*0.007f;
				float fSin = sinf( fParam);
				float fCos = cosf( fParam);

				int ni = tp->NormalIndex[k];
				Normal_t *np = &m->Normals[ni];
				float *Normal = NormalTransform[mInfo.m_MeshIndex][ni];
				for ( int iCoord = 0; iCoord < 3; ++iCoord)
				{
					vPos[iCoord] = VertexTransform[mInfo.m_MeshIndex][vi][iCoord] + Normal[iCoord]*fSin*28.0f;
				}
				glVertex3fv(vPos);
			}
            else
			{
				glVertex3fv(VertexTransform[mInfo.m_MeshIndex][vi]);
			}
		}
	}

	//렌더 설정 복구
	glColor3f(1.f, 1.f, 1.f);
	EnableAlphaTest();
	glEnd();
*/
}

extern float BoneScale;
void COriginMachine::Transform(BMD* model, float (*BoneMatrix)[3][4])
{
/*
	ModelInfo& meshInfo = ModelManager::GetModelInfo(model->m_ID);
	TransInfo& mInfo = meshInfo.m_TransInfo;

	vec3_t LightPosition;
	if(model->LightEnable)
	{
     	vec3_t Position;
		float Matrix[3][4];
		if(mInfo.m_bHighLight)
		{
     		Vector(1.3f,0.f,2.f,Position);
		}
		else
		{
		    Vector(0.f,-1.5f,0.f,Position);
		}

		AngleMatrix(model->ShadowAngle,Matrix);
		VectorIRotate(Position,Matrix,LightPosition);
	}
	vec3_t BoundingMin;
	vec3_t BoundingMax;
	for(int i=0;i<model->NumMeshs;i++)
	{
       	Mesh_t *m = &model->Meshs[i];
		for(int j=0;j<m->NumVertices;j++)
		{
			Vertex_t *v = &m->Vertices[j];
			float *vp = VertexTransform[i][j];
			if(BoneScale == 1.f)
			{
				VectorTransform(v->Position,BoneMatrix[v->Node],vp);
				if(mInfo.m_bTrans)
					VectorScale(vp,model->BodyScale,vp);
			}
			else
			{
				VectorRotate(v->Position,BoneMatrix[v->Node],vp);
				vp[0] = vp[0] * BoneScale + BoneMatrix[v->Node][0][3];
				vp[1] = vp[1] * BoneScale + BoneMatrix[v->Node][1][3];
				vp[2] = vp[2] * BoneScale + BoneMatrix[v->Node][2][3];
				if(mInfo.m_bTrans)
					VectorScale(vp,model->BodyScale,vp);
			}
			for(int k=0;k<3;k++)
			{
				if(vp[k] < BoundingMin[k]) BoundingMin[k] = vp[k];
				if(vp[k] > BoundingMax[k]) BoundingMax[k] = vp[k];
			}
			if(mInfo.m_bTrans)
				VectorAdd(vp,model->BodyOrigin,vp);
		}

		for(j=0;j<m->NumNormals;j++)
		{
			Normal_t *sn = &m->Normals[j];
			float    *tn = NormalTransform[i][j];
			VectorRotate(sn->Normal,BoneMatrix[sn->Node],tn);
			if(model->LightEnable)
			{
				float Luminosity = DotProduct(tn,LightPosition)*0.8f+0.4f;
				if(Luminosity < 0.2f) Luminosity = 0.2f;
				IntensityTransform[i][j] = Luminosity;
			}
		}
	}
*/
}
//////////////////////////////////////////////////////////////////////////


#endif //MR0