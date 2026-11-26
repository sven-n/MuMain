#include "stdafx.h"

#include "doctest.h"

// The live preview of things on bones (FX1.7c, design section 7a, L51 to L54): the marks of the effect lists that the
// game's draw and move loops ask, and the Style Editor's views creating their items' things in the game's lists.

#ifdef _EDITOR

#include "LookRecorder.h"
#include "../gamelogic/ShippedLooksScope.h"

#include "Core/Globals/_TextureIndex.h"
#include "Core/Globals/_define.h"
#include "Data/GameData/ItemData/ItemModelSlots.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Render/Drawing/DrawCallLog.h"
#include "Render/Effects/EffectDraw.h"
#include "Render/Effects/PreviewSlots.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Items/ItemEffects.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Scenes/SceneCommon.h"
#include "Scenes/SceneCore.h"
#include "UI/EffectBrowser/EffectPreviewCamera.h"
#include "UI/EffectBrowser/EffectPreviewScene.h"
#include "UI/ItemTools/ItemMeshPicking.h"
#include "UI/StyleEditor/BoneMarks.h"
#include "UI/StyleEditor/BoneThingsPreview.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <vector>

namespace
{
using Render::Effects::PreviewSlots::List;
namespace PreviewSlots = Render::Effects::PreviewSlots;

// Counts the sprites and lightning quads the draw loops hand it.
class CountingDrawer final : public Render::EffectDrawing::EffectDrawer
{
public:
    int sprites = 0;
    int quads = 0;

    void SetState(const Render::EffectDrawing::EffectState&) override {}
    void DrawSprite(const Render::EffectDrawing::SpriteDraw&) override
    {
        ++sprites;
    }
    void DrawQuads(const Render::EffectDrawing::QuadsDraw&) override
    {
        ++quads;
    }
    void DrawDecal(const Render::EffectDrawing::DecalDraw&) override {}
    void DrawCircle(const Render::EffectDrawing::CircleDraw&) override {}
    void DrawPlane(const Render::EffectDrawing::PlaneDraw&) override {}
};

// The draw loops draw into `drawer` and the renderer is a log while it lives.
class DrawScope
{
public:
    explicit DrawScope(CountingDrawer& drawer)
    {
        Render::EffectDrawing::SetEffectDrawer(&drawer);
        mu::SetRendererOverride(&m_log);
    }
    ~DrawScope()
    {
        mu::SetRendererOverride(nullptr);
        Render::EffectDrawing::SetEffectDrawer(nullptr);
    }
    DrawScope(const DrawScope&) = delete;
    DrawScope& operator=(const DrawScope&) = delete;

private:
    Render::Drawing::DrawCallLog m_log;
};

void ClearLists()
{
    for (int i = 0; i < MAX_SPRITES; ++i)
    {
        Sprites[i].Live = false;
    }
    for (int i = 0; i < MAX_PARTICLES; ++i)
    {
        Particles[i].Live = false;
    }
    for (int i = 0; i < MAX_JOINTS; ++i)
    {
        Joints[i].Live = false;
    }
    for (int i = 0; i < MAX_EFFECTS; ++i)
    {
        Effects[i].Live = false;
    }
    PreviewSlots::Clear();
}

// The world the Style Editor's views create in: a character standing in the main scene.
class PreviewWorld
{
public:
    PreviewWorld()
        : m_character(std::make_unique<CHARACTER>()), m_hero(Hero), m_scene(SceneFlag), m_loading(LoadingWorld),
          m_render(EnableMainRender), m_factor(FPS_ANIMATION_FACTOR)
    {
        m_character->Object.Live = true;
        Vector(13120.0f, 12480.0f, 140.0f, m_character->Object.Position);
        Hero = m_character.get();
        SceneFlag = MAIN_SCENE;
        LoadingWorld = 0;
        EnableMainRender = true;
        FPS_ANIMATION_FACTOR = 1.0f;
        ClearLists();
    }
    ~PreviewWorld()
    {
        ClearLists();
        Hero = m_hero;
        SceneFlag = m_scene;
        LoadingWorld = m_loading;
        EnableMainRender = m_render;
        FPS_ANIMATION_FACTOR = m_factor;
    }
    PreviewWorld(const PreviewWorld&) = delete;
    PreviewWorld& operator=(const PreviewWorld&) = delete;

    const float* Place() const
    {
        return m_character->Object.Position;
    }

private:
    std::unique_ptr<CHARACTER> m_character;
    CHARACTER* m_hero;
    EGameScene m_scene;
    int m_loading;
    bool m_render;
    float m_factor;
};

std::vector<int> LiveSprites()
{
    std::vector<int> live;
    for (int i = 0; i < MAX_SPRITES; ++i)
    {
        if (Sprites[i].Live)
        {
            live.push_back(i);
        }
    }
    return live;
}

std::vector<int> LiveJoints()
{
    std::vector<int> live;
    for (int i = 0; i < MAX_JOINTS; ++i)
    {
        if (Joints[i].Live)
        {
            live.push_back(i);
        }
    }
    return live;
}

float DistanceTo(const float* a, const float* b)
{
    const float dx = a[0] - b[0];
    const float dy = a[1] - b[1];
    const float dz = a[2] - b[2];
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

// What view 0 shows of `item`: the effect its model runs.
MuEditor::Looks::BoneThingsShown Shown(int item, MuEditor::Effects::PreviewItemPose pose,
                                       std::optional<double> stoppedAt = std::nullopt)
{
    MuEditor::Looks::BoneThingsShown shown;
    shown.request.showOn = MuEditor::Effects::PreviewShowOn::Item;
    shown.request.itemType = item;
    shown.request.itemPose = pose;
    shown.effect = Render::Items::ItemEffects::EffectOf(MuEditor::Effects::PreviewItemModelsOf(shown.request).model);
    shown.stoppedAt = stoppedAt;
    return shown;
}

// One frame of the game for the views, in the game's order: the sprites of the last frame cleared, the lightning moved
// (lightning that lives a frame ends), then the views' update. A view's draw comes after, in the editor's render.
void Frame(MuEditor::Looks::BoneThingsPreview& preview)
{
    ClearSprites();
    MoveJoints();
    preview.Update();
}

// The live joints marked for a view.
int JointsOf(int view)
{
    int count = 0;
    for (int i = 0; i < MAX_JOINTS; ++i)
    {
        if (Joints[i].Live && PreviewSlots::ViewOf(List::Joint, i) == view + 1)
        {
            ++count;
        }
    }
    return count;
}
} // namespace

TEST_CASE("The draw loops draw the game's slots and a preview view's only in its capture, and a stopped view's don't "
          "move (design L51, L54) [looks][preview]")
{
    ClearLists();
    vec3_t place{0.0f, 0.0f, 0.0f};
    vec3_t light{1.0f, 1.0f, 1.0f};
    const int game = CreateSprite(BITMAP_LIGHT, place, 1.0f, light, nullptr);
    const int previewed = CreateSprite(BITMAP_LIGHT, place, 1.0f, light, nullptr);
    REQUIRE(previewed != game);
    PreviewSlots::Mark(List::Sprite, previewed, 2);
    CHECK(PreviewSlots::ViewOf(List::Sprite, previewed) == 2);
    CHECK(PreviewSlots::ViewOf(List::Sprite, game) == 0);
    // Slots past those ever marked are the game's.
    CHECK(PreviewSlots::ViewOf(List::Joint, MAX_JOINTS - 1) == 0);

    CountingDrawer world;
    {
        const DrawScope draw(world);
        BeginSpriteDraws();
        RenderSprites();
    }
    CHECK(world.sprites == 1);
    CHECK_FALSE(PreviewSlots::IsCapturing());

    CountingDrawer view;
    {
        const DrawScope draw(view);
        const PreviewSlots::CaptureScope capture(2);
        CHECK(PreviewSlots::IsCapturing());
        CHECK(PreviewSlots::Draws(List::Sprite, previewed));
        CHECK_FALSE(PreviewSlots::Draws(List::Sprite, game));
        BeginSpriteDraws();
        RenderSprites();
    }
    CHECK(view.sprites == 1);
    CHECK_FALSE(PreviewSlots::IsCapturing());

    // Another view's capture draws neither.
    CountingDrawer other;
    {
        const DrawScope draw(other);
        const PreviewSlots::CaptureScope capture(1);
        BeginSpriteDraws();
        RenderSprites();
    }
    CHECK(other.sprites == 0);

    CHECK(PreviewSlots::Moves(List::Sprite, previewed));
    PreviewSlots::SetStopped(2, true);
    CHECK_FALSE(PreviewSlots::Moves(List::Sprite, previewed));
    CHECK(PreviewSlots::Moves(List::Sprite, game));
    // The game itself never stops.
    PreviewSlots::SetStopped(0, true);
    CHECK(PreviewSlots::Moves(List::Sprite, game));
    PreviewSlots::SetStopped(2, false);
    CHECK(PreviewSlots::Moves(List::Sprite, previewed));

    PreviewSlots::Clear();
    CHECK(PreviewSlots::ViewOf(List::Sprite, previewed) == 0);
    ClearLists();
}

TEST_CASE("A Style Editor view creates its item's things on bones at the character, in the game's lists and marked "
          "for it, draws only those, and removes them when it changes or is no longer drawn (design L51 to L53) "
          "[looks][preview]")
{
    const ShippedLooksScope looks;
    REQUIRE(looks.IsComplete());
    const std::vector<int> items = LookRecorder::ItemsUsing("wingsOfDarkness", Data::Looks::LookKind::Effect);
    REQUIRE_FALSE(items.empty());
    LookRecorder::ShippedItems shipped;
    shipped.Open(items);
    const int item = items.front();
    const PreviewWorld world;
    auto preview = std::make_unique<MuEditor::Looks::BoneThingsPreview>();
    MuEditor::Effects::EffectPreviewCamera camera;
    camera.Frame({0.0f, 0.0f, 0.0f}, 100.0f);

    const MuEditor::Looks::BoneThingsShown inventory = Shown(item, MuEditor::Effects::PreviewItemPose::Inventory);
    REQUIRE(inventory.effect != nullptr);
    // Nothing is created before the view was drawn.
    Frame(*preview);
    CHECK(LiveSprites().empty());

    CountingDrawer firstDraw;
    {
        const DrawScope draw(firstDraw);
        preview->Draw(0, inventory, camera);
    }
    // It had nothing to draw yet.
    CHECK(firstDraw.sprites == 0);
    Frame(*preview);
    const std::vector<int> sprites = LiveSprites();
    REQUIRE_FALSE(sprites.empty());
    for (const int sprite : sprites)
    {
        CAPTURE(sprite);
        CHECK(PreviewSlots::ViewOf(List::Sprite, sprite) == 1);
        CHECK_FALSE(PreviewSlots::Draws(List::Sprite, sprite));
        CHECK(Sprites[sprite].Owner != nullptr);
        // The copy of the item stands at the character, the wings' bones within a few hundred units of it.
        CHECK(DistanceTo(Sprites[sprite].Position, world.Place()) < 400.0f);
    }

    // The world draws none of them; the view draws them all, after its item.
    CountingDrawer worldDraw;
    {
        const DrawScope draw(worldDraw);
        BeginSpriteDraws();
        RenderSprites();
    }
    CHECK(worldDraw.sprites == 0);
    CountingDrawer viewDraw;
    {
        const DrawScope draw(viewDraw);
        preview->Draw(0, inventory, camera);
    }
    CHECK(viewDraw.sprites == static_cast<int>(sprites.size()));

    // Its lightning comes by chance; it is kept and marked like the sprites.
    std::vector<int> joints;
    for (int frame = 0; frame < 400 && joints.empty(); ++frame)
    {
        preview->Draw(0, inventory, camera);
        Frame(*preview);
        joints = LiveJoints();
    }
    REQUIRE_FALSE(joints.empty());
    for (const int joint : joints)
    {
        CHECK(PreviewSlots::ViewOf(List::Joint, joint) == 1);
    }
    CHECK(preview->GetCounts(0).joints == static_cast<int>(joints.size()));

    SUBCASE("While the time is stopped, the game moves none of its things and it creates only its sprites (L54)")
    {
        const MuEditor::Looks::BoneThingsShown stopped =
            Shown(item, MuEditor::Effects::PreviewItemPose::Inventory, 2500.0);
        const float lifeTime = Joints[joints.front()].LifeTime;
        // Ticked in the view's draw: the next move already leaves the lightning, which lives a frame, as it is.
        for (int frame = 0; frame < 50; ++frame)
        {
            preview->Draw(0, stopped, camera);
            Frame(*preview);
        }
        CHECK(Joints[joints.front()].LifeTime == lifeTime);
        CHECK(LiveJoints() == joints);
        CHECK(preview->GetCounts(0).joints == static_cast<int>(joints.size()));
        CHECK_FALSE(LiveSprites().empty());
        // And the view still draws it.
        CountingDrawer stoppedDraw;
        {
            const DrawScope draw(stoppedDraw);
            preview->Draw(0, stopped, camera);
        }
        CHECK(stoppedDraw.quads == static_cast<int>(joints.size()));
        // Time runs again: the next move moves it on.
        preview->Draw(0, inventory, camera);
        CHECK(PreviewSlots::Moves(List::Joint, joints.front()));
    }
    SUBCASE("Another pose of the item removes what the view made, before it makes it anew")
    {
        // With the time stopped the old lightning would stay: the view makes its effect once anew, to stop on.
        preview->Draw(0, Shown(item, MuEditor::Effects::PreviewItemPose::Ground, 2500.0), camera);
        Frame(*preview);
        CHECK(LiveJoints().size() == joints.size());
        CHECK(preview->GetCounts(0).joints == static_cast<int>(joints.size()));
        CHECK(JointsOf(0) == static_cast<int>(joints.size()));
        CHECK_FALSE(LiveSprites().empty());
    }
    SUBCASE("A view that is no longer drawn loses its things")
    {
        for (int frame = 0; frame < MuEditor::Looks::BoneThingsPreview::UpdatesWithoutDraw; ++frame)
        {
            Frame(*preview);
        }
        CHECK(LiveSprites().empty());
        CHECK(LiveJoints().empty());
        CHECK(preview->GetCounts(0).joints == 0);
        CHECK(PreviewSlots::ViewOf(List::Joint, joints.front()) == 0);
    }
    SUBCASE("The world being cleared removes them")
    {
        preview->OnWorldClearing();
        CHECK(LiveJoints().empty());
        CHECK(preview->GetCounts(0).joints == 0);
    }
    SUBCASE("Outside the main scene the view creates nothing")
    {
        const EGameScene scene = SceneFlag;
        SceneFlag = CHARACTER_SCENE;
        preview->Draw(0, inventory, camera);
        Frame(*preview);
        CHECK(LiveSprites().empty());
        CHECK(LiveJoints().empty());
        SceneFlag = scene;
    }
    preview->RemoveAll();
}

TEST_CASE("Each view keeps its own lightning, and the game's lightning of the same type in slots a view left stays "
          "the game's (design L52) [looks][preview]")
{
    const ShippedLooksScope looks;
    REQUIRE(looks.IsComplete());
    const std::vector<int> items = LookRecorder::ItemsUsing("wingsOfDarkness", Data::Looks::LookKind::Effect);
    REQUIRE_FALSE(items.empty());
    LookRecorder::ShippedItems shipped;
    shipped.Open(items);
    const PreviewWorld world;
    auto preview = std::make_unique<MuEditor::Looks::BoneThingsPreview>();
    MuEditor::Effects::EffectPreviewCamera camera;
    camera.Frame({0.0f, 0.0f, 0.0f}, 100.0f);
    const MuEditor::Looks::BoneThingsShown shown = Shown(items.front(), MuEditor::Effects::PreviewItemPose::Inventory);
    constexpr int Views = MuEditor::Looks::BoneThingsPreview::MaxViews;
    const auto drawAll = [&]
    {
        for (int view = 0; view < Views; ++view)
        {
            preview->Draw(view, shown, camera);
        }
    };
    drawAll();
    Frame(*preview);
    drawAll();
    // The Wings of Darkness' lightning lives a frame: each view makes the same anew every frame.
    const int perView = JointsOf(0);
    REQUIRE(perView > 0);
    for (int view = 0; view < Views; ++view)
    {
        CHECK(JointsOf(view) == perView);
    }

    // The game's own lightning takes the first free slots, of the same type and SubType as the views' (the
    // character's Wings of Darkness): created after the views' lightning of the last frame ended, in slots the views
    // had. They stay the game's: no view takes them for its own, draws them or removes them.
    ClearSprites();
    MoveJoints();
    std::vector<int> gameJoints;
    for (int i = 0; i < perView; ++i)
    {
        vec3_t from{100.0f, 100.0f, 0.0f};
        vec3_t to{200.0f, 100.0f, 0.0f};
        vec3_t angle{0.0f, 0.0f, 0.0f};
        CreateJoint(BITMAP_JOINT_THUNDER, from, to, angle, 14, nullptr, 1.0f);
    }
    for (int i = 0; i < MAX_JOINTS; ++i)
    {
        if (Joints[i].Live)
        {
            gameJoints.push_back(i);
        }
    }
    REQUIRE(static_cast<int>(gameJoints.size()) == perView);
    preview->Update();
    for (int view = 0; view < Views; ++view)
    {
        CAPTURE(view);
        CHECK(JointsOf(view) == perView);
        CHECK(preview->GetCounts(view).joints == perView);
    }
    for (const int joint : gameJoints)
    {
        CAPTURE(joint);
        CHECK(Joints[joint].Live);
        CHECK(PreviewSlots::ViewOf(List::Joint, joint) == 0);
    }
    // Removing the views' things leaves the game's.
    preview->RemoveAll();
    for (const int joint : gameJoints)
    {
        CHECK(Joints[joint].Live);
    }
    for (const int joint : gameJoints)
    {
        Joints[joint].Live = false;
    }
}

TEST_CASE("A double click in pick mode takes back the pick of its first click, once (design L58) [looks][preview]")
{
    namespace BoneMarks = MuEditor::Looks::BoneMarks;
    constexpr double DoubleClick = 0.3;
    BoneMarks::PickTakeBack pick;
    Data::Looks::BonePlaces places;
    places.bones = {4};
    std::optional<int> from;
    // Nothing picked yet: nothing to take back.
    CHECK_FALSE(pick.TakeBack(places, from, 1.0, DoubleClick));
    CHECK(pick.Pick(places, false, from, 22, 1.0));
    CHECK(places.bones == std::vector<int>{4, 22});
    CHECK(pick.TakeBack(places, from, 1.2, DoubleClick));
    CHECK(places.bones == std::vector<int>{4});
    // Only once.
    CHECK_FALSE(pick.TakeBack(places, from, 1.25, DoubleClick));
    CHECK(places.bones == std::vector<int>{4});

    // A double click long after the pick is not its.
    pick.Pick(places, false, from, 22, 2.0);
    CHECK_FALSE(pick.TakeBack(places, from, 3.0, DoubleClick));
    CHECK(places.bones == std::vector<int>{4, 22});

    // Between bones the start of a pair comes back too.
    Data::Looks::BonePlaces pairs;
    pick.Pick(pairs, true, from, 30, 4.0);
    pick.Pick(pairs, true, from, 22, 5.0);
    CHECK(pairs.between.size() == 1);
    CHECK(pick.TakeBack(pairs, from, 5.1, DoubleClick));
    CHECK(pairs.between.empty());
    CHECK(from == 30);
}

TEST_CASE("A click in pick mode adds a bone or takes it out, and picks lightning pairs start first (design L58) "
          "[looks][preview]")
{
    namespace BoneMarks = MuEditor::Looks::BoneMarks;
    Data::Looks::BonePlaces places;
    std::optional<int> from;
    CHECK(BoneMarks::PickBone(places, false, from, 22));
    CHECK(BoneMarks::PickBone(places, false, from, 25));
    CHECK(places.bones == std::vector<int>{22, 25});
    CHECK(BoneMarks::PickBone(places, false, from, 22));
    CHECK(places.bones == std::vector<int>{25});

    Data::Looks::BonePlaces pairs;
    // The start waits for the end.
    CHECK_FALSE(BoneMarks::PickBone(pairs, true, from, 30));
    CHECK(from == 30);
    CHECK(BoneMarks::PickBone(pairs, true, from, 22));
    CHECK_FALSE(from.has_value());
    CHECK(pairs.between == std::vector<std::pair<int, int>>{{30, 22}});
    // The same pair again takes it out; the other way round is another pair.
    BoneMarks::PickBone(pairs, true, from, 22);
    CHECK(BoneMarks::PickBone(pairs, true, from, 30));
    CHECK(pairs.between == std::vector<std::pair<int, int>>{{30, 22}, {22, 30}});
    BoneMarks::PickBone(pairs, true, from, 30);
    CHECK(BoneMarks::PickBone(pairs, true, from, 22));
    CHECK(pairs.between == std::vector<std::pair<int, int>>{{22, 30}});
    // A second click on the start lets it go.
    BoneMarks::PickBone(pairs, true, from, 5);
    CHECK_FALSE(BoneMarks::PickBone(pairs, true, from, 5));
    CHECK_FALSE(from.has_value());
    CHECK(pairs.between.size() == 1);
}

TEST_CASE("The bone marks project as the preview's camera sees and find the dot under the mouse (design L58) "
          "[looks][preview]")
{
    namespace BoneMarks = MuEditor::Looks::BoneMarks;
    MuEditor::Effects::EffectPreviewCamera camera;
    camera.Frame({0.0f, 0.0f, 0.0f}, 100.0f);
    const ImVec2 min{100.0f, 50.0f};
    const ImVec2 max{500.0f, 350.0f};
    const std::optional<ImVec2> center = BoneMarks::ToScreen(camera, min, max, camera.Center());
    REQUIRE(center.has_value());
    CHECK(center->x == doctest::Approx(300.0f));
    CHECK(center->y == doctest::Approx(200.0f));
    // Right and up on the screen are right and up of the view.
    const MuEditor::Effects::EffectPreviewCamera::Basis basis = camera.GetBasis();
    MuEditor::Effects::PreviewVector rightUp{};
    for (std::size_t axis = 0; axis < 3; ++axis)
    {
        rightUp[axis] = camera.Center()[axis] + 20.0f * basis.right[axis] + 20.0f * basis.up[axis];
    }
    const std::optional<ImVec2> point = BoneMarks::ToScreen(camera, min, max, rightUp);
    REQUIRE(point.has_value());
    CHECK(point->x > center->x);
    CHECK(point->y < center->y);
    // As a click's ray goes the other way.
    const float width = max.x - min.x;
    const float height = max.y - min.y;
    const MuEditor::Items::MeshPicking::Ray ray = MuEditor::Items::MeshPicking::RayThroughView(
        camera, width / height, (point->x - min.x) / width * 2.0f - 1.0f, 1.0f - (point->y - min.y) / height * 2.0f);
    float along = 0.0f;
    float off = 0.0f;
    for (std::size_t axis = 0; axis < 3; ++axis)
    {
        along += (rightUp[axis] - ray.origin[axis]) * ray.direction[axis];
    }
    for (std::size_t axis = 0; axis < 3; ++axis)
    {
        const float d = rightUp[axis] - ray.origin[axis] - along * ray.direction[axis];
        off += d * d;
    }
    CHECK(std::sqrt(off) < 0.01f * along);
    // Behind the eye: nowhere.
    MuEditor::Effects::PreviewVector behind{};
    const MuEditor::Effects::PreviewVector eye = camera.Eye();
    for (std::size_t axis = 0; axis < 3; ++axis)
    {
        behind[axis] = eye[axis] - 10.0f * basis.forward[axis];
    }
    CHECK_FALSE(BoneMarks::ToScreen(camera, min, max, behind).has_value());

    const std::vector<ImVec2> dots{{10.0f, 10.0f}, {20.0f, 10.0f}, {40.0f, 40.0f}};
    CHECK(BoneMarks::NearestWithin(dots, {18.0f, 11.0f}, 7.0f) == 1u);
    CHECK_FALSE(BoneMarks::NearestWithin(dots, {30.0f, 25.0f}, 7.0f).has_value());
}

TEST_CASE("The bone marks find the bones of a shown item as its view poses them (design L58) [looks][preview]")
{
    const ShippedLooksScope looks;
    REQUIRE(looks.IsComplete());
    const std::vector<int> items = LookRecorder::ItemsUsing("wingsOfDarkness", Data::Looks::LookKind::Effect);
    REQUIRE_FALSE(items.empty());
    LookRecorder::ShippedItems shipped;
    shipped.Open(items);
    const MuEditor::Looks::BoneThingsShown shown = Shown(items.front(), MuEditor::Effects::PreviewItemPose::Inventory);
    std::vector<MuEditor::Looks::BoneMarks::ShownBone> bones;
    MuEditor::Looks::BoneMarks::BonesOf(shown.request, 0.0f, bones);
    const MuEditor::Effects::PreviewItemModels models = MuEditor::Effects::PreviewItemModelsOf(shown.request);
    REQUIRE(models.CanDraw());
    CHECK(static_cast<int>(bones.size()) == Models[models.model].NumBones);
    // Each with the bone it hangs on, for the skeleton's lines.
    for (const auto& bone : bones)
    {
        CHECK(bone.parent == Models[models.model].Bones[bone.number].Parent);
    }
    CHECK(bones.front().parent == -1);
    // The wings spread: their bones are not all in one place.
    const auto apart = std::ranges::any_of(bones, [&](const auto& bone) { return bone.place != bones.front().place; });
    CHECK(apart);
    // Turned, they move.
    std::vector<MuEditor::Looks::BoneMarks::ShownBone> turned;
    MuEditor::Looks::BoneMarks::BonesOf(shown.request, 90.0f, turned);
    REQUIRE(turned.size() == bones.size());
    CHECK(turned.back().place != bones.back().place);
}

TEST_CASE("The bone marks sit where a view creates its things on those bones (design L58) [looks][preview]")
{
    const ShippedLooksScope looks;
    REQUIRE(looks.IsComplete());
    const std::vector<int> items = LookRecorder::ItemsUsing("wingsOfDarkness", Data::Looks::LookKind::Effect);
    REQUIRE_FALSE(items.empty());
    LookRecorder::ShippedItems shipped;
    shipped.Open(items);
    const PreviewWorld world;
    // The Wings of Darkness' sprites, alone: one at each of their bones.
    const Data::Looks::LookDefinition* shippedEffect = Render::Items::ItemEffects::EffectOf(
        MuEditor::Effects::PreviewItemModelsOf(
            Shown(items.front(), MuEditor::Effects::PreviewItemPose::Inventory).request)
            .model);
    REQUIRE(shippedEffect != nullptr);
    Data::Looks::LookDefinition sprites = *shippedEffect;
    std::erase_if(sprites.onBones, [](const Data::Looks::OnBones& entry) { return entry.places.bones.empty(); });
    REQUIRE(sprites.onBones.size() == 1);
    const std::vector<int> bones = sprites.onBones.front().places.bones;

    for (const auto pose : {MuEditor::Effects::PreviewItemPose::Inventory, MuEditor::Effects::PreviewItemPose::Ground})
    {
        CAPTURE(static_cast<int>(pose));
        for (const float turn : {0.0f, 70.0f})
        {
            CAPTURE(turn);
            ClearSprites();
            auto preview = std::make_unique<MuEditor::Looks::BoneThingsPreview>();
            MuEditor::Effects::EffectPreviewCamera camera;
            camera.Frame({0.0f, 0.0f, 0.0f}, 100.0f);
            MuEditor::Looks::BoneThingsShown shown = Shown(items.front(), pose);
            shown.effect = &sprites;
            shown.turnDegrees = turn;
            preview->Draw(0, shown, camera);
            Frame(*preview);
            std::vector<MuEditor::Looks::BoneMarks::ShownBone> marks;
            MuEditor::Looks::BoneMarks::BonesOf(shown.request, turn, marks);
            std::vector<std::array<float, 3>> created;
            for (int i = 0; i < MAX_SPRITES; ++i)
            {
                if (Sprites[i].Live)
                {
                    created.push_back({Sprites[i].Position[0] - Hero->Object.Position[0],
                                       Sprites[i].Position[1] - Hero->Object.Position[1],
                                       Sprites[i].Position[2] - Hero->Object.Position[2]});
                }
            }
            REQUIRE(created.size() == bones.size());
            for (std::size_t i = 0; i < bones.size(); ++i)
            {
                CAPTURE(bones[i]);
                const auto& mark = marks[static_cast<std::size_t>(bones[i])].place;
                for (std::size_t axis = 0; axis < 3; ++axis)
                {
                    CHECK(created[i][axis] == doctest::Approx(mark[axis]).epsilon(0.001));
                }
            }
            preview->RemoveAll();
        }
    }
}

#endif // _EDITOR
