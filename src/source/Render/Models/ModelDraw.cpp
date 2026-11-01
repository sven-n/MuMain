#include "stdafx.h"

#include "Render/Models/ModelDraw.h"

#include "Render/Models/ZzzBMD.h"

namespace Render::ModelDrawing
{
namespace
{
class DefaultModelDrawer final : public ModelDrawer
{
public:
    void DrawMesh(MeshDraw& draw) override
    {
        draw.model->DrawMesh(draw);
    }

    void DrawShadow(const ShadowDraw& draw) override
    {
        draw.model->DrawShadow(draw);
    }
};

DefaultModelDrawer s_default;
ModelDrawer* s_active = &s_default;
} // namespace

ModelDrawer& ActiveModelDrawer()
{
    return *s_active;
}

void SetModelDrawer(ModelDrawer* drawer)
{
    s_active = drawer ? drawer : &s_default;
}
} // namespace Render::ModelDrawing
