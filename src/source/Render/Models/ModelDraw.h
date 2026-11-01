#pragma once

#include <cstdint>

class BMD;

// The seam between game code and how a model is drawn. BMD::RenderMesh describes one mesh draw
// as a MeshDraw and hands it to the active ModelDrawer; the default drawer draws it as the game
// always has (BMD::DrawMesh). Another drawer can change how draws are carried out without touching
// the code that asks for them. The Classic and Enhanced looks are not drawers: both go through the
// same drawer, and the renderer applies the active look to each draw.
namespace Render::ModelDrawing
{
// How a model's vertices are skinned for a draw: what its last Transform() set up.
struct ModelSkin
{
    // MAX_BONES bone matrices; nullptr when the model was not transformed.
    const float (*bones)[3][4] = nullptr;
    // g_BoneTransformVersion when the bones were set: GPU palette uploads key on it.
    std::uint32_t boneVersion = 0;
    float boneScale = 1.f;
    // The rest-pose scale Transform() was given; 0 for none.
    float restPoseScale = 0.f;
    // Whether the bones already place the body (BMD::Transform's Translate).
    bool translate = false;
    // The model light vector GPU-skinned lighting uses.
    float lightDirection[3] = {};
};

// One mesh draw: the call's arguments and the model state the draw reads.
struct MeshDraw
{
    // The model's data: meshes and textures.
    const BMD* model = nullptr;
    int mesh = 0;
    int flags = 0;
    float alpha = 1.f;
    int blendMesh = -1;
    float blendMeshAlpha = 1.f;
    float blendMeshU = 0.f;
    float blendMeshV = 0.f;
    // A texture instead of the mesh's own; -1 for none.
    int texture = -1;

    // The drawer may change it: RenderMesh copies it back to the model (see there).
    float bodyLight[3] = {};
    float bodyOrigin[3] = {};
    float bodyScale = 1.f;
    bool lightEnable = false;
    bool hideSkin = false;
    char streamMesh = -1;
    // The LOD of a model file to draw (0: the BMD's meshes).
    int lod = 0;
    ModelSkin skin;
};

// One model's shadow projected on the ground: the call's arguments and the model state the draw reads.
struct ShadowDraw
{
    const BMD* model = nullptr;
    int blendMesh = -1;
    int hiddenMesh = -1;
    // -1 for the first and past the last mesh.
    int startMesh = -1;
    int endMesh = -1;
    // The cloth simulation's CPhysicsCloth array; when set, the cloth casts the shadow instead of the meshes.
    void* clothes = nullptr;
    int clothesCount = 0;
    float alpha = 1.f;

    float bodyOrigin[3] = {};
    float bodyScale = 1.f;
    // The LOD of a model file whose meshes cast the shadow (0: the BMD's meshes).
    int lod = 0;
    ModelSkin skin;
};

class ModelDrawer
{
public:
    virtual ~ModelDrawer() = default;
    virtual void DrawMesh(MeshDraw& draw) = 0;
    virtual void DrawShadow(const ShadowDraw& draw) = 0;
};

[[nodiscard]] ModelDrawer& ActiveModelDrawer();
// nullptr restores the default drawer. The drawer must outlive its use.
void SetModelDrawer(ModelDrawer* drawer);
} // namespace Render::ModelDrawing
