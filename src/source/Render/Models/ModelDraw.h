#pragma once

class BMD;

// The seam between game code and how a model is drawn. BMD::RenderMesh describes one mesh draw
// as a MeshDraw and hands it to the active ModelDrawer; the default drawer draws it as the game
// always has (BMD::DrawMesh). Another drawer can change how draws are carried out without touching
// the code that asks for them. The Classic and Enhanced looks are not drawers: both go through the
// same drawer, and the renderer applies the active look to each draw.
namespace Render::ModelDrawing
{
// One mesh draw: the call's arguments and the model state the draw reads.
struct MeshDraw
{
    // The model's data: meshes, textures, the skinning set up by its last Transform().
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
