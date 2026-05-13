#define NODE_MAX     200
#define TIME_MAX     100
#define TRIANGLE_MAX 15000
#define VERTEX_MAX   10000

#define TEXTURE_MAX  100
#define MESH_MAX     100

#define REFERENCE_FRAME     0
#define SKELETAL_ANIMATION  1

typedef struct
{
    char  Name[32];
    short Parent;
} Node_t;

typedef struct
{
    vec3_t Position[NODE_MAX];
    vec3_t Rotation[NODE_MAX];
} Skeleton_t;

typedef struct
{
    short      NodeNum;
    Node_t     Node[NODE_MAX];
    Skeleton_t Skeleton;
} NodeGroup_t;

typedef struct
{
    short      TimeNum;
    Skeleton_t Skeleton[TIME_MAX];
} SkeletonGroup_t;

typedef struct
{
    short  Node;
    vec3_t Position;
    vec3_t Normal;
    float  TexCoordU;
    float  TexCoordV;
} SMDVertex_t;

typedef struct
{
    short       TriangleNum;
    char        TextureName[TRIANGLE_MAX][32];
    SMDVertex_t Vertex[TRIANGLE_MAX][3];
} TriangleGroup_t;

typedef struct
{
    short      Texture;
    short      VertexNum;
    short      NormalNum;
    short      TexCoordNum;
    short      TriangleNum;
    Vertex_t   Vertex[VERTEX_MAX];
    Normal_t   Normal[VERTEX_MAX];
    TexCoord_t TexCoord[VERTEX_MAX];

    char       Polygon[TRIANGLE_MAX];
    short      VertexList[TRIANGLE_MAX][4];
    short      NormalList[TRIANGLE_MAX][4];
    short      TexCoordList[TRIANGLE_MAX][4];
} SMDMesh_t;

typedef struct
{
    short       MeshNum;
    Texture_t   Texture[TEXTURE_MAX];
    SMDMesh_t   Mesh[MESH_MAX];
} SMDMeshGroup_t;

extern NodeGroup_t     NodeGroup;
extern SkeletonGroup_t SkeletonGroup;
extern TriangleGroup_t TriangleGroup;
extern SMDMeshGroup_t  MeshGroup;

bool OpenSMDModel(int ID, wchar_t* FileName, int Actions = 1, bool Flip = false);
bool OpenSMDAnimation(int ID, wchar_t* FileName, bool LockPosition = false);
void OpenBMD(int ID, wchar_t* FileName);
void SaveBMD(int ID, wchar_t* FileName);
