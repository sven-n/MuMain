#ifndef __ZZZBMD_H__
#define __ZZZBMD_H__

#include <vector>
#include <cstdint>
#include "Render/Sprites/TextureScript.h"

extern const float (*g_pActiveBoneTransform)[3][4];
// Bumped by every SetActiveBoneTransform() call — lets BoneUBO detect "the pointer is
// the same address as last upload, but the content underneath it has changed" (e.g. a
// stack-local BoneTransform array reused at the same call depth for a different linked
// item, or the shared global BoneTransform[] array reused across different objects).
// Pointer identity alone is NOT safe to cache on in this codebase — see the call sites.
extern unsigned int g_BoneTransformVersion;
void SetActiveBoneTransform(const float (*ptr)[3][4]);

#define MAX_BONES    200
#define MAX_MESH     50
#define MAX_VERTICES 15000

#define RENDER_COLOR        0x00000001
#define RENDER_TEXTURE      0x00000002
#define RENDER_CHROME       0x00000004
#define RENDER_METAL        0x00000008
#define RENDER_LIGHTMAP     0x00000010
#define RENDER_SHADOWMAP    0x00000020
#define RENDER_BRIGHT       0x00000040
#define RENDER_DARK         0x00000080
#define RENDER_EXTRA        0x00000100
#define RENDER_CHROME2      0x00000200
#define RENDER_WAVE			0x00000400 
#define RENDER_CHROME3      0x00000800
#define RENDER_CHROME4      0x00001000
#define RENDER_NODEPTH      0x00002000
#define RENDER_CHROME5      0x00004000
#define RENDER_OIL          0x00008000
#define RENDER_CHROME6      0x00010000
#define RENDER_CHROME7      0x00020000
#define RENDER_DOPPELGANGER 0x00040000
#define RENDER_WAVE_EXT		0x10000000
#define RENDER_BYSCRIPT		0x80000000
#define RNDEXT_WAVE			1
#define RNDEXT_OIL          2
#define RNDEXT_RISE			4

#define MAX_MONSTER_SOUND   10

typedef struct
{
    vec3_t Position;
    vec3_t Color;
    float  Range;
} Light_t;

typedef struct
{
    vec3_t* Position;
    vec3_t* Rotation;
    vec4_t* Quaternion;
} BoneMatrix_t;

typedef struct
{
    char         Name[32];
    short        Parent;
    char         Dummy;
    BoneMatrix_t* BoneMatrixes;
    char         BoundingBox;
    vec3_t       BoundingVertices[8];
} Bone_t;

typedef struct
{
    char FileName[32];
} Texture_t;

typedef struct
{
    unsigned char Width;
    unsigned char Height;
    unsigned char* Buffer;
} Bitmap_t;

typedef struct
{
    short  Node;
    vec3_t Position;
} Vertex_t;

typedef struct
{
    short  Node;
    vec3_t Normal;
    short  BindVertex;
} Normal_t;

typedef struct
{
    float TexCoordU;
    float TexCoordV;
} TexCoord_t;

typedef struct
{
    BYTE m_Colors[3];	//0~255 RGB
} VertexColor_t;

typedef struct
{
    char       Polygon;
    short      VertexIndex[4];
    short      NormalIndex[4];
    short      TexCoordIndex[4];
    short      EdgeTriangleIndex[4];
    bool       Front;
} Triangle_t;

typedef struct
{
    char       Polygon;
    short      VertexIndex[4];
    short      NormalIndex[4];
    short      TexCoordIndex[4];
    TexCoord_t LightMapCoord[4]; //ver1.2
    short      LightMapIndexes; //ver1.2
} Triangle_t2;

typedef struct
{
    bool          Loop;
    float         PlaySpeed;
    short         NumAnimationKeys;
    bool          LockPositions;
    vec3_t* Positions;
} Action_t;

typedef struct _Triangle_t3 : public Triangle_t
{
    short	   m_ivIndexAdditional[4];
} Triangle_t3;

typedef struct _Mesh_t
{
    bool          NoneBlendMesh;
    short         Texture;
    short         NumVertices;
    short         NumNormals;
    short         NumTexCoords;
    short		  NumVertexColors;	//ver1.3
    short         NumTriangles;
    int           NumCommandBytes; //ver1.1
    Vertex_t* Vertices;
    Normal_t* Normals;
    TexCoord_t* TexCoords;
    VertexColor_t* VertexColors;	//ver1.3
    Triangle_t* Triangles;
    unsigned char* Commands; //ver1.1

    TextureScript* m_csTScript;

    _Mesh_t()
    {
        Vertices = NULL;
        Normals = NULL;
        Triangles = NULL;
        Commands = NULL;
        m_csTScript = NULL;

        NumVertices = NumNormals = NumTexCoords =
            NumVertexColors = NumTriangles = 0;
    }
} Mesh_t;

class BMD
{
public:
    char          Name[64];
    char          Version;
    short         NumBones;
    short         NumMeshs;
    short         NumActions;
    Mesh_t* Meshs;
    Bone_t* Bones;
    Action_t* Actions;
    Texture_t* Textures;
    GLuint* IndexTexture;

    short         NumLightMaps;  //ver1.2
    short         IndexLightMap; //ver1.2
    Bitmap_t* LightMaps;    //ver1.2

    bool          LightEnable;
    bool          ContrastEnable;
    vec3_t        BodyLight;
    int           BoneHead;

    int           BoneFoot[4];
    float         BodyScale;
    vec3_t        BodyOrigin;
    vec3_t        BodyAngle;
    float         BodyHeight;
    char          StreamMesh;
    vec3_t        ShadowAngle;
    char          Skin;
    bool          HideSkin;
    float         Velocity;
    unsigned short CurrentAction;
    unsigned short PriorAction;
    float         CurrentAnimation;
    short         CurrentAnimationFrame;
    short         Sounds[MAX_MONSTER_SOUND];
    int           renderCount;
    float		  fTransformedSize;

    unsigned int		m_iBMDSeqID;
    bool				bLightMap;
    bool				bOffLight;
    char				iBillType;

    bool				m_bCompletedAlloc;

    GLuint m_VAO_Static;
    GLuint m_VBO_Dynamic; // Interleaved: Pos (3) | UV (2) | Color (4) = 9 floats per corner
    GLuint m_EBO;

    GLuint m_VAO_StaticGPU;   // VAO for GPU skinning path
    GLuint m_VBO_StaticGeom;  // Static Interleaved VBO: RestPos(3) | UV(2) | RestNormal(3) | BoneIndex(1)
    GLuint m_VBO_Color;       // Pre-allocated dynamic color buffer for GPU skinning path
    bool   m_HasStaticGPUVBO;

    // True once UploadStaticVBOs() has built the per-corner geometry arrays for this model
    // (both m_VAO_StaticGPU's GPU-skin buffer AND the CPU dynamic-VBO fallback's
    // m_MeshIndexCount/m_MeshIndexOffset bookkeeping are ready).
    bool m_StaticGeomReady;

    float (*m_pCurrentBoneTransform)[3][4]; // Active bone matrix palette stored during Transform()

    int* m_MeshUVOffset;
    int* m_MeshIndexOffset;
    int* m_MeshIndexCount;

    bool m_DynBufsReady;
    bool m_LastTranslate; // Set by Transform(): true=Translate mode (BodyOrigin/BodyScale shift world pos),
                          // false=non-Translate mode (bone matrices already encode world position)
    vec3_t m_LastLightPosition; // Set by Transform() when LightEnable: the per-body light direction
                                // vector (HighLight/BattleCastle/default variants), consumed by
                                // RenderMesh()'s GPU-skinned draw to compute per-vertex lighting
                                // in-shader instead of replaying IntensityTransform on the CPU (DXP-20).

    // DXP-20 increment 4: lazy CPU-skin materialization. TransformCheap() stashes the skin
    // request and bumps m_SkinStamp instead of eagerly filling VertexTransform/NormalTransform/
    // IntensityTransform; EnsureCpuVertices()/EnsureCpuNormals() run the deferred work on first
    // read per mesh per Transform() call. See DXP-20-inc4-plan.md.
    // mutable: several consumers (e.g. AddMeshShadowTriangles()) are const methods that only read
    // game state but still need to trigger materialization -- this is cache state, not object
    // identity, same rationale as any other lazy-init pattern behind a const accessor.
    mutable uint32_t m_SkinStamp;                 // matches g_SkinStampCounter iff this BMD owns the scratch arrays right now
    mutable bool     m_CpuVertsReady[MAX_MESH];   // per-mesh: VertexTransform[mesh] materialized under m_SkinStamp
    mutable bool     m_CpuNormalsReady[MAX_MESH]; // per-mesh: NormalTransform/IntensityTransform[mesh] materialized under m_SkinStamp
    float    m_LastSkinScale;             // Transform()'s `_Scale` arg, stashed for deferred materialization
    float    m_LastBoneScale;             // global BoneScale AT TransformCheap() time (callers mutate the global right after)

    std::vector<float> m_Staging; // Pre-allocated CPU staging buffer (size = flatOffset * 9)

    void UploadStaticVBOs();
    void UploadDynamicBuffers(int meshIndex, float alpha = 1.0f, int renderFlags = 0);
    void ReleaseDynamicVBOs();
    void ReleaseStaticVBOs();

    BMD() : NumBones(0), NumActions(0), NumMeshs(0),
        Meshs(NULL), Bones(NULL), Actions(NULL), Textures(NULL), IndexTexture(NULL),
        m_VAO_Static(0), m_VBO_Dynamic(0), m_EBO(0),
        m_VAO_StaticGPU(0), m_VBO_StaticGeom(0), m_VBO_Color(0), m_HasStaticGPUVBO(false),
        m_StaticGeomReady(false),
        m_pCurrentBoneTransform(nullptr),
        m_MeshUVOffset(nullptr), m_MeshIndexOffset(nullptr), m_MeshIndexCount(nullptr),
        m_DynBufsReady(false), m_LastTranslate(false),
        m_SkinStamp(0), m_LastSkinScale(0.f), m_LastBoneScale(1.f)
    {
        m_LastLightPosition[0] = m_LastLightPosition[1] = m_LastLightPosition[2] = 0.f;
        for (int _i = 0; _i < MAX_MESH; _i++)
        {
            m_CpuVertsReady[_i] = false;
            m_CpuNormalsReady[_i] = false;
        }
        LightEnable = false;
        ContrastEnable = false;
        HideSkin = false;
        bLightMap = false;
        iBillType = -1;
        bOffLight = false;
        m_bCompletedAlloc = false;
    }

    ~BMD();
    //utility
    void Init(bool Dummy);
    bool Open2(const wchar_t* DirName, const wchar_t* FileName, bool bReAlloc = true);
    bool Save2(wchar_t* DirName, wchar_t* FileName);
    void Release();
    void CreateBoundingBox();

    bool PlayAnimation(float* AnimationFrame, float* PriorAnimationFrame, unsigned short* PriorAction, float Speed, vec3_t Origin, vec3_t Angle);
    // BoneMatrixCapacity: element count of the caller's BoneTransform buffer. The DXP-24 stale-slot
    // fill writes identity into [NumBones, BoneMatrixCapacity) so UBO-uploaded full-size palettes
    // never carry a previous model's matrices; callers passing a buffer smaller than MAX_BONES
    // (e.g. the internal new vec34_t[NumBones] scratch arrays) MUST pass their real capacity.
    void Animation(float(*BoneTransform)[3][4], float AnimationFrame, float PriorAnimationFrame, unsigned short PriorAction, vec3_t Angle, vec3_t HeadAngle, bool Parent = false, bool Translate = true, const float (*ExtParentMatrix)[4] = nullptr, short CurrentAction = -1, int BoneMatrixCapacity = MAX_BONES);
    void InterpolationTrans(float(*Mat1)[4], float(*TransMat2)[4], float _Scale);
    void Transform(float(*BoneMatrix)[3][4], vec3_t BoundingBoxMin, vec3_t BoundingBoxMax, OBB_t* OBB, bool Translate = false, float _Scale = 0.0f);
    // DXP-20 increment 2: the state-stash + OBB-from-args portion of Transform(), without the
    // expensive per-mesh vertex/normal loop. Only equivalent to Transform() when EditFlag != 2 and
    // not a _DEBUG build -- those need the vertex-loop-derived OBB Transform() itself still computes.
    void TransformCheap(float(*BoneMatrix)[3][4], vec3_t BoundingBoxMin, vec3_t BoundingBoxMax, OBB_t* OBB, bool Translate = false, float _Scale = 0.0f);
    // DXP-20 increment 2: skins ONE vertex from BoneMatrix, replicating Transform()'s per-vertex
    // math exactly (position only -- no normals, lighting, or bounding box). `_Scale`/`Translate`
    // must match what the caller would have passed to Transform(); BoneScale/BodyScale/BodyOrigin
    // are read as the same ambient globals/members Transform() itself reads.
    void SkinVertex(int mesh, int vertexIndex, float(*BoneMatrix)[3][4], bool Translate, float _Scale, vec3_t out) const;
    // Batch form: skins every vertex of `mesh` into the corresponding slice of the global
    // VertexTransform[mesh] array -- the same side effect Transform()'s loop has for that one mesh,
    // without touching any other mesh or computing normals/lighting.
    void SkinVertices(int mesh, float(*BoneMatrix)[3][4], bool Translate, float _Scale) const;
    // DXP-20 increment 4: materializes VertexTransform[mesh] (mesh == -1: all meshes) from the
    // skin request stashed by the most recent TransformCheap()/Transform() call, if not already
    // materialized under the current stamp. Idempotent -- safe to call from every consumer site.
    void EnsureCpuVertices(int mesh = -1) const;
    // Same as EnsureCpuVertices, but for NormalTransform/IntensityTransform[mesh] (needs LightEnable
    // + m_LastLightPosition, both stashed by TransformCheap()).
    void EnsureCpuNormals(int mesh = -1) const;
    // For consumers (cloth sim write-back) that populate VertexTransform[mesh] themselves, outside
    // the normal skin path -- marks it "ready" under the current stamp so a later EnsureCpuVertices
    // doesn't re-skin and clobber the externally-written data.
    void MarkCpuVerticesExternallyWritten(int mesh) const;
    void TransformByObjectBone(vec3_t vResultPosition, OBJECT* pObject, int iBoneNumber, vec3_t vRelativePosition = NULL);
    void TransformByBoneMatrix(vec3_t vResultPosition, float(*BoneMatrix)[4], vec3_t vWorldPosition = NULL, vec3_t vRelativePosition = NULL);
    void TransformPosition(float(*Matrix)[4], vec3_t Position, vec3_t WorldPosition, bool Translate = false);
    void RotationPosition(float(*Matrix)[4], vec3_t Position, vec3_t WorldPosition);

public:
    void AnimationTransformWithAttachHighModel_usingGlobalTM(
        OBJECT* oHighHierarchyModel,
        BMD* bmdHighHierarchyModel,
        int iBoneNumberHighHierarchyModel,
        vec3_t& vOutPosHighHiearachyModelBone,
        vec3_t* arrOutSetfAllBonePositions,
        bool bApplyTMtoVertices);

    void AnimationTransformWithAttachHighModel(
        OBJECT* oHighHierarchyModel,
        BMD* bmdHighHierarchyModel,
        int iBoneNumberHighHierarchyModel,
        vec3_t& vOutPosHighHiearachyModelBone,
        vec3_t* arrOutSetfAllBonePositions);

    void AnimationTransformOnlySelf(vec3_t* arrOutSetfAllBonePositions,
        const OBJECT* oSelf);

    void AnimationTransformOnlySelf(vec3_t* arrOutSetfAllBonePositions,
        const vec3_t& v3Angle,
        const vec3_t& v3Position,
        const float& fScale,
        OBJECT* oRefAnimation = NULL,
        const float fFrameArea = -1.0f,
        const float fWeight = -1.0f);

    void Lighting(float*, Light_t*, vec3_t, vec3_t);
    void Chrome(float*, int, vec3_t);

    //render
    void BeginRender(float);
    void EndRender();

    void RenderMeshEffect(int i, int iType, int iSubType = 0, vec3_t Angle = 0, VOID* obj = NULL);

    void RenderMesh(int meshIndex, int renderFlags, float alpha = 1.f, int blendMeshIndex = -1, float blendMeshAlpha = 1.f, float blendMeshTextureCoordU = 0.f, float blendMeshTextureCoordV = 0.f, int textureIndex = -1);
    void BeginRenderCoinHeap();
    int AddToCoinHeap(int coinIndex, int target_vertex_index);
    void EndRenderCoinHeap(int coinCount);

    void RenderBody(int RenderFlag, float Alpha = 1.f, int BlendMesh = -1, float BlendMeshLight = 1.f, float BlendMeshTexCoordU = 0.f, float BlendMeshTexCoordV = 0.f, int HiddenMesh = -1, int Texture = -1);
    void RenderBodyShadow(int blendMesh = -1, int hiddenMesh = -1, int startMeshNumber = -1, int endMeshNumber = -1, void* pClothes = nullptr, int clothesCount = 0);

    void SetBodyLight(vec3_t right) { VectorCopy(right, BodyLight); }

    bool LightMapEnable;
    bool CollisionDetectLineToMesh(vec3_t, vec3_t, bool Collision = true, int Mesh = -1, int Triangle = -1);
    void CreateLightMapSurface(Light_t*, Mesh_t*, int, int, int, int, int, int, vec3_t, vec3_t, int);
    void CreateLightMaps();
    void BindLightMaps();
    void ReleaseLightMaps();

    //#ifdef USE_SHADOWVOLUME
    void FindNearTriangle(void);

    void FindTriangleForEdge(int iMesh, int iTri, int iIndex11);
    //#endif //USE_SHADOWVOLUME
private:
    BMD(const BMD& b);

    // DXP-20 increment 4: claims ownership of the shared scratch arrays (VertexTransform etc.)
    // for this BMD if another BMD's TransformCheap()/claim ran since ours -- clears both ready
    // arrays so stale per-mesh flags from before the eviction can't be mistaken for fresh data.
    // No-op if this BMD already owns the scratch under the current stamp.
    void ClaimSkinStamp() const;

    void AddClothesShadowTriangles(void* pClothes, int clothesCount, float sx, float sy) const;
    void AddMeshShadowTriangles(int blendMesh, int hiddenMesh, int startMesh, int endMesh, float sx, float sy) const;
};

extern BMD* Models;
extern BMD* ModelsDump;
extern float BoneTransform[MAX_BONES][3][4];
extern vec3_t VertexTransform[MAX_MESH][MAX_VERTICES];
extern vec3_t NormalTransform[MAX_MESH][MAX_VERTICES];
extern vec3_t LightTransform[MAX_MESH][MAX_VERTICES];
extern float  IntensityTransform[MAX_MESH][MAX_VERTICES];
extern float g_chrome[MAX_VERTICES][2];

#endif
