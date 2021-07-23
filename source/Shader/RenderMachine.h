#ifndef __RENDER_MACHINE_H__
#define __RENDER_MACHINE_H__

//////////////////////////////////////////////////////////////////////////
// 
// RenderMachine.h : 쉐이더 타입별 렌더러 머신 정의 (VertexShaderRenderer, Pixel Shader Renderer)
//
//////////////////////////////////////////////////////////////////////////

#ifdef MR0

#include "ModelManager.h"
#include "VPManager.h"

using namespace ModelManager;

class BMD;

//////////////////////////////////////////////////////////////////////////
// Immediate Mode상에 현재 Render Mesh 정의
//////////////////////////////////////////////////////////////////////////
class CRenderMachine
{
public:
	typedef enum _RENDERER_TYPE
	{
		TYPE_RDT_IMMEDIATE = 0,		//기존
		TYPE_RDT_VBO,				//VBO만 사용
		TYPE_RDT_VBOVS,				//VBO에 버텍스 쉐이더 결합
		TYPE_RDT_VBOVSPS,			//VBO와 버텍스/픽쉘세이더 양쪽다 사용

		TYPE_RDT_ALL
	}RENDERER_TYPE;

	typedef enum _RENDERER_SUBTYPE
	{
		TYPE_RDST_IMMEDIATE = 0,	//버퍼에 있는 녀석들을 즉시 그린다.
		TYPE_RDST_BATCH,			//따로 여러개의 렌더 콜을 쌓는다.
		TYPE_RDST_ALL

	}RENDERER_SUBTYPE;

	RENDERER_TYPE		m_eType;
	RENDERER_SUBTYPE	m_eSubType;

	virtual void Render(ModelManager::MeshRender_t&) = 0;		// 렌더 메쉬 개체를 실제 렌더링 요청.
	virtual void Transform(BMD*, float (*BoneMatrix)[3][4]) = 0;

	//유틸리티 함수
	bool IsBatchRenderer() { return (m_eSubType == TYPE_RDST_BATCH); }
	bool IsUsePixelShader() { return (m_eType >= TYPE_RDT_VBOVSPS); }
};

extern CRenderMachine*	g_Renderer;

//////////////////////////////////////////////////////////////////////////
// 렌더 머신 : 버텍스 쉐이더 버전
//////////////////////////////////////////////////////////////////////////
class CVBOShaderMachine : public CRenderMachine
{
private:
	float*			m_pMat;

public:
	CVBOShaderMachine() : m_pMat(NULL) {m_eType = TYPE_RDT_VBOVS; m_eSubType = TYPE_RDST_IMMEDIATE;}
	~CVBOShaderMachine() { }

	void Render(ModelManager::MeshRender_t&);
	void Transform(BMD* m, float (*BoneMatrix)[3][4]);
};


//////////////////////////////////////////////////////////////////////////
// 렌더 머신 : 픽셀 쉐이더 버전
//////////////////////////////////////////////////////////////////////////
class CVBOPixelShaderMachine : public CRenderMachine
{
private:

public:
	CVBOPixelShaderMachine() {m_eType = TYPE_RDT_VBOVSPS; m_eSubType = TYPE_RDST_BATCH;}
	~CVBOPixelShaderMachine() { }

	void Render(ModelManager::MeshRender_t&) {}
	void Transform(BMD* m, float (*BoneMatrix)[3][4]) { return; }
};

//////////////////////////////////////////////////////////////////////////
// 렌더 머신 : 현재 미사용
//////////////////////////////////////////////////////////////////////////
class CVBOMachine : public CRenderMachine
{
public:
	CVBOMachine() {m_eType = TYPE_RDT_VBO; m_eSubType = TYPE_RDST_IMMEDIATE;}

	virtual void Render(ModelManager::MeshRender_t&);
	virtual void Transform(BMD* m, float (*BoneMatrix)[3][4]);
};

//////////////////////////////////////////////////////////////////////////
// 렌더 머신 : Original ( 낮은 사양 호환 )
//////////////////////////////////////////////////////////////////////////
class COriginMachine : public CRenderMachine
{
public:
	COriginMachine() {m_eType = TYPE_RDT_IMMEDIATE; m_eSubType = TYPE_RDST_IMMEDIATE;}
	~COriginMachine() { }	

	virtual void Render(ModelManager::MeshRender_t&);
	virtual void Transform(BMD* m, float (*BoneMatrix)[3][4]);
};

#endif // MR0

#endif //__RENDER_MACHINE_H__