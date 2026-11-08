#include "stdafx.h"
#include <doctest.h>
#include "ClientFolderScope.h"
#include "ExecutableConfigFile.h"
#include "TestModelSlots.h"
#include "PlayerTextureScope.h"
#include "Data/GameData/ItemData/ItemTextureFiles.h"
#include "Core/Text/Utf8.h"
#include "Data/GameConfig/GameConfig.h"
#include "Engine/Object/PlayerBodyModels.h"
#include "Data/DataHandler/LoadData.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/Textures/ZzzTexture.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <system_error>

namespace
{
const std::filesystem::path ClientFolder = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

using Engine::Object::PlayerBodyPart;
using Engine::Object::PlayerBodyParts;

std::string BodyFile(const PlayerBodyPart& part, int stage, int baseClass)
{
    return Core::Text::ToUtf8(Engine::Object::PlayerBodyFileName(stage, part, baseClass).c_str()) + ".bmd";
}

int BodySlot(const PlayerBodyPart& part, int stage, int baseClass)
{
    return part.firstSlot + stage * MAX_CLASS + baseClass;
}

std::string OpenedFile(int slot)
{
    return Core::Text::ToUtf8(gLoadData.GetModelFile(slot).c_str());
}

std::string FirstTextureName(const BMD& model)
{
    const char* name = model.Textures[0].FileName;
    return std::string(name, strnlen(name, sizeof(model.Textures[0].FileName)));
}

// A client folder for one test, with copies of one form's original body parts and the shared textures.
struct BodyClientFolder
{
    explicit BodyClientFolder(int baseClass = CLASS_KNIGHT, int stage = 0)
        : path(std::filesystem::temp_directory_path() /
               ("mu_test_player_bodies_" + std::to_string(std::random_device{}())))
    {
        std::filesystem::create_directories(path / "Data" / "Player");
        for (const PlayerBodyPart& part : PlayerBodyParts)
        {
            const std::string file = BodyFile(part, stage, baseClass);
            std::filesystem::copy_file(std::filesystem::path(MU_TEST_DATA_DIR) / "Player" / file,
                                       path / "Data" / "Player" / file);
        }
        for (const auto& entry :
             std::filesystem::directory_iterator(std::filesystem::path(MU_TEST_DATA_DIR) / "Player"))
        {
            const std::string extension = entry.path().extension().string();
            if (Data::Items::EqualsIgnoringCase(extension, ".ozj") ||
                Data::Items::EqualsIgnoringCase(extension, ".ozt"))
                std::filesystem::copy_file(entry.path(), path / "Data" / "Player" / entry.path().filename());
        }
    }
    ~BodyClientFolder()
    {
        std::error_code ignored;
        std::filesystem::remove_all(path, ignored);
    }
    BodyClientFolder(const BodyClientFolder&) = delete;
    BodyClientFolder& operator=(const BodyClientFolder&) = delete;

    void AddImproved(const std::string& file) const
    {
        const auto updated = path / "Data" / "Player" / "Updated";
        std::filesystem::create_directories(updated);
        std::filesystem::copy_file(std::filesystem::path(MU_TEST_DATA_DIR) / "Player" / "Updated" / file,
                                   updated / file);
    }

    std::filesystem::path path;
};
} // namespace

TEST_CASE("S6 selector defaults and explicit choices survive save and restart [player-selector]")
{
    const ExecutableConfigFile file;
    GameConfig& config = GameConfig::GetInstance();
    file.Write("[Render]\n");
    config.Load();
    CHECK(config.GetUseS6PlayerModels() == true);
    for (bool useOriginal : {false, true})
    {
        config.SetUseS6PlayerModels(useOriginal);
        config.Save();
        config.SetUseS6PlayerModels(!useOriginal);
        config.Load();
        CHECK(config.GetUseS6PlayerModels() == useOriginal);
    }
}

TEST_CASE("Without the S6 models each body part comes from the Updated folder when it has the file [player-selector]")
{
    const std::filesystem::path updatedFolder = std::filesystem::path(MU_TEST_DATA_DIR) / "Player" / "Updated";
    ClientFolderScope folder(ClientFolder);
    PlayerTextureScope textures;
    for (bool useS6 : {true, false})
    {
        // Isolate each setting run; release its models and texture references before the next choice.
        TestModelSlots slots;
        for (int baseClass = 0; baseClass < MAX_CLASS; ++baseClass)
        {
            Engine::Object::OpenPlayerClassBodyParts(baseClass, useS6);
            for (int stage = 0; stage < MAX_CLASS_STAGES; ++stage)
            {
                if (!Engine::Object::HasPlayerBodyForm(stage, baseClass))
                    continue;
                Engine::Object::OpenPlayerBodyTextures(stage, baseClass);
                for (const PlayerBodyPart& part : PlayerBodyParts)
                {
                    const std::string file = BodyFile(part, stage, baseClass);
                    INFO(file, " useS6=", useS6);
                    const int slot = BodySlot(part, stage, baseClass);
                    const bool fromUpdated = !useS6 && std::filesystem::exists(updatedFolder / file);
                    CHECK(OpenedFile(slot) == (fromUpdated ? "Data\\Player\\Updated\\" : "Data\\Player\\") + file);
                    CHECK(Models[slot].NumMeshs > 0);
                }
            }
        }
    }
}

TEST_CASE("Both packaged player model choices open through the native loader [player-selector]")
{
    ClientFolderScope folder(ClientFolder);
    const int slot = MODEL_BODY_HELM + static_cast<int>(CLASS_KNIGHT);
    for (bool useS6 : {true, false})
    {
        TestModelSlots slots;
        Engine::Object::OpenPlayerBodyParts(0, CLASS_KNIGHT, useS6);
        REQUIRE(Models[slot].NumMeshs > 0);
        CHECK(FirstTextureName(Models[slot]) == (useS6 ? "skin_wizard_01.jpg" : "skin_hq_class02.jpg"));
    }
}

TEST_CASE("A missing Updated folder leaves the original body parts [player-selector]")
{
    const BodyClientFolder client;
    TestModelSlots slots;
    ClientFolderScope folder(client.path);
    Engine::Object::OpenPlayerBodyParts(0, CLASS_KNIGHT, false);
    for (const PlayerBodyPart& part : PlayerBodyParts)
    {
        const std::string file = BodyFile(part, 0, CLASS_KNIGHT);
        INFO(file);
        CHECK(OpenedFile(BodySlot(part, 0, CLASS_KNIGHT)) == "Data\\Player\\" + file);
        CHECK(Models[BodySlot(part, 0, CLASS_KNIGHT)].NumMeshs > 0);
    }
}

TEST_CASE("An Updated file with an invalid BMD header leaves the original part [player-selector]")
{
    const BodyClientFolder client;
    const std::string helm = BodyFile(PlayerBodyParts[0], 0, CLASS_KNIGHT);
    std::filesystem::create_directories(client.path / "Data" / "Player" / "Updated");
    std::ofstream(client.path / "Data" / "Player" / "Updated" / helm, std::ios::binary) << "BM";
    TestModelSlots slots;
    ClientFolderScope folder(client.path);
    Engine::Object::OpenPlayerBodyParts(0, CLASS_KNIGHT, false);
    const int slot = BodySlot(PlayerBodyParts[0], 0, CLASS_KNIGHT);
    CHECK(OpenedFile(slot) == "Data\\Player\\" + helm);
    CHECK(Models[slot].NumMeshs > 0);
}

TEST_CASE("Mixed hair asset generations fall back to original Dark Lord heads [player-selector][texture-fallback]")
{
    for (const int stage : {0, 2})
    {
        for (const bool oldImprovedName : {false, true})
        {
            INFO("stage=", stage, " old improved name=", oldImprovedName);
            const BodyClientFolder client(CLASS_DARK_LORD, stage);
            const std::string helm = BodyFile(PlayerBodyParts[0], stage, CLASS_DARK_LORD);
            client.AddImproved(helm);
            // New Updated with older Player: the new alias is absent. Old Updated with new Player:
            // its old hair name is absent. The mesh data is otherwise identical across those releases.
            if (!oldImprovedName)
                std::filesystem::remove(client.path / "Data" / "Player" / "hairhq_R.OZJ");
            ClientFolderScope folder(client.path);
            PlayerTextureScope textures;
            TestModelSlots slots;
            REQUIRE(LoadBitmap(L"Player\\hair_r.jpg", BITMAP_HAIR));
            Engine::Object::OpenPlayerBodyParts(stage, CLASS_DARK_LORD, false);
            const int slot = BodySlot(PlayerBodyParts[0], stage, CLASS_DARK_LORD);
            REQUIRE(OpenedFile(slot) == "Data\\Player\\Updated\\" + helm);
            if (oldImprovedName)
                std::strcpy(Models[slot].Textures[1].FileName, "hair_hq_player.jpg");
            Engine::Object::OpenPlayerBodyTextures(stage, CLASS_DARK_LORD);
            CHECK(OpenedFile(slot) == "Data\\Player\\" + helm);
            REQUIRE(Models[slot].NumMeshs == 2);
            CHECK(Models[slot].IndexTexture[1] == BITMAP_HAIR);
            const BITMAP_t* hair = Bitmaps.FindTexture(BITMAP_HAIR);
            REQUIRE(hair != nullptr);
            CHECK(hair->Ref == 2); // preload plus the original head, no reference from the rejected head
            CHECK(hair->IsHair);
            CHECK_FALSE(hair->IsSkin);
            // The improved head's skin loaded before its hair failed, and must not leak a reference.
            CHECK(Bitmaps.FindTextureByName(
                      Core::Text::FromUtf8(stage == 0 ? "skin_hq_class05.jpg" : "body_hq_class305.jpg")) == nullptr);
        }
    }
}

TEST_CASE("Missing or undecodable improved body textures select originals, S6 choice stays unchanged "
          "[player-selector][texture-fallback]")
{
    for (const bool corrupt : {false, true})
    {
        const BodyClientFolder client;
        const std::string helm = BodyFile(PlayerBodyParts[0], 0, CLASS_KNIGHT);
        client.AddImproved(helm);
        const auto texture = client.path / "Data" / "Player" / "skin_hq_class02.OZJ";
        if (corrupt)
            std::ofstream(texture, std::ios::binary | std::ios::trunc) << "not a JPEG";
        else
            std::filesystem::remove(texture);
        ClientFolderScope folder(client.path);
        PlayerTextureScope textures;
        for (const bool useS6 : {false, true})
        {
            INFO("corrupt=", corrupt, " useS6=", useS6);
            TestModelSlots slots;
            Engine::Object::OpenPlayerBodyParts(0, CLASS_KNIGHT, useS6);
            Engine::Object::OpenPlayerBodyTextures(0, CLASS_KNIGHT);
            const int slot = BodySlot(PlayerBodyParts[0], 0, CLASS_KNIGHT);
            CHECK(OpenedFile(slot) == "Data\\Player\\" + helm);
            REQUIRE(Models[slot].NumMeshs > 0);
            const BITMAP_t* skin = Bitmaps.FindTexture(Models[slot].IndexTexture[0]);
            REQUIRE(skin != nullptr);
            CHECK(skin->IsSkin);
        }
    }
}

TEST_CASE("Improved bodies keep legitimate cached texture fallback [player-selector][texture-fallback]")
{
    const BodyClientFolder client;
    const std::string helm = BodyFile(PlayerBodyParts[0], 0, CLASS_KNIGHT);
    client.AddImproved(helm);
    const auto texture = client.path / "Data" / "Player" / "skin_hq_class02.OZJ";
    std::filesystem::create_directories(client.path / "Data" / "Other");
    std::filesystem::rename(texture, client.path / "Data" / "Other" / texture.filename());
    ClientFolderScope folder(client.path);
    PlayerTextureScope textures;
    TestModelSlots slots;
    const GLuint cached = Bitmaps.LoadImage(L"Data\\Other\\skin_hq_class02.jpg", GL_NEAREST, GL_REPEAT);
    REQUIRE(cached != BITMAP_UNKNOWN);
    Engine::Object::OpenPlayerBodyParts(0, CLASS_KNIGHT, false);
    Engine::Object::OpenPlayerBodyTextures(0, CLASS_KNIGHT);
    const int slot = BodySlot(PlayerBodyParts[0], 0, CLASS_KNIGHT);
    CHECK(OpenedFile(slot) == "Data\\Player\\Updated\\" + helm);
    CHECK(Models[slot].IndexTexture[0] == cached);
    REQUIRE(Bitmaps.FindTexture(cached) != nullptr);
    CHECK(Bitmaps.FindTexture(cached)->Ref == 2);
    CHECK(Bitmaps.FindTexture(cached)->IsSkin);
}

TEST_CASE(
    "Hidden improved textures stay hidden and unsupported entries own no bitmap [player-selector][texture-fallback]")
{
    const BodyClientFolder client;
    const std::string helm = BodyFile(PlayerBodyParts[0], 0, CLASS_KNIGHT);
    client.AddImproved(helm);
    ClientFolderScope folder(client.path);
    PlayerTextureScope textures;
    for (const char* name : {"hide_mesh.jpg", "unused.dds"})
    {
        TestModelSlots slots;
        Engine::Object::OpenPlayerBodyParts(0, CLASS_KNIGHT, false);
        const int slot = BodySlot(PlayerBodyParts[0], 0, CLASS_KNIGHT);
        std::strcpy(Models[slot].Textures[0].FileName, name);
        Engine::Object::OpenPlayerBodyTextures(0, CLASS_KNIGHT);
        CHECK(OpenedFile(slot) == "Data\\Player\\Updated\\" + helm);
        // Unsupported entries own no bitmap; hidden entries retain the native hidden sentinel.
        CHECK(Models[slot].IndexTexture[0] == (Data::Items::IsHiddenTexture(name) ? BITMAP_HIDE : BITMAP_UNKNOWN));
    }
}

TEST_CASE("Improved bodies use textures supplied only by the active pack [player-selector][texture-fallback]")
{
    const BodyClientFolder client;
    const std::string helm = BodyFile(PlayerBodyParts[0], 0, CLASS_KNIGHT);
    client.AddImproved(helm);
    const auto texture = client.path / "Data" / "Player" / "skin_hq_class02.OZJ";
    const auto pack = client.path / "Data" / "TexturePacks" / "body-test" / "Player";
    std::filesystem::create_directories(pack);
    std::filesystem::rename(texture, pack / texture.filename());
    ClientFolderScope folder(client.path);
    PlayerTextureScope textures;
    TestModelSlots slots;
    Bitmaps.SetTexturePack(L"body-test");
    Engine::Object::OpenPlayerBodyParts(0, CLASS_KNIGHT, false);
    Engine::Object::OpenPlayerBodyTextures(0, CLASS_KNIGHT);
    const int slot = BodySlot(PlayerBodyParts[0], 0, CLASS_KNIGHT);
    CHECK(OpenedFile(slot) == "Data\\Player\\Updated\\" + helm);
    const BITMAP_t* skin = Bitmaps.FindTexture(Models[slot].IndexTexture[0]);
    REQUIRE(skin != nullptr);
    CHECK(skin->fromTexturePack);
    CHECK(skin->IsSkin);
}

TEST_CASE("Rejected bodies with shared texture slots release only their acquired references "
          "[player-selector][texture-fallback]")
{
    for (const int sharedTextureSlot : {0, 1})
    {
        for (const bool preloadedSkin : {false, true})
        {
            INFO("shared texture slot=", sharedTextureSlot, " preloaded skin=", preloadedSkin);
            const BodyClientFolder client(CLASS_DARK_LORD);
            const std::string helm = BodyFile(PlayerBodyParts[0], 0, CLASS_DARK_LORD);
            client.AddImproved(helm);
            const auto skin = client.path / "Data" / "Player" / "skin_hq_class05.OZJ";
            std::filesystem::create_directories(client.path / "Data" / "Other");
            std::filesystem::rename(skin, client.path / "Data" / "Other" / skin.filename());
            std::filesystem::remove(client.path / "Data" / "Player" / "hairhq_R.OZJ");
            ClientFolderScope folder(client.path);
            PlayerTextureScope textures;
            TestModelSlots slots;
            GLuint cached = BITMAP_SKIN_BEGIN;
            if (preloadedSkin)
            {
                REQUIRE(LoadBitmap(L"Other\\skin_hq_class05.jpg", cached));
            }
            else
            {
                cached = Bitmaps.LoadImage(L"Data\\Other\\skin_hq_class05.jpg", GL_NEAREST, GL_REPEAT);
            }
            REQUIRE(cached != BITMAP_UNKNOWN);
            REQUIRE(Bitmaps.FindTexture(cached) != nullptr);
            REQUIRE(Bitmaps.FindTexture(cached)->Ref == 1);
            Engine::Object::OpenPlayerBodyParts(0, CLASS_DARK_LORD, false);
            const int slot = BodySlot(PlayerBodyParts[0], 0, CLASS_DARK_LORD);
            BMD& head = Models[slot];
            REQUIRE(head.NumMeshs == 2);
            REQUIRE(head.GetDataUserCount() == 1);
            // Both native-loaded meshes use one material; the loader still acquires by texture index.
            head.Meshs[0].Texture = static_cast<short>(sharedTextureSlot);
            head.Meshs[1].Texture = static_cast<short>(sharedTextureSlot);
            Engine::Object::OpenPlayerBodyTextures(0, CLASS_DARK_LORD);
            CHECK(OpenedFile(slot) == "Data\\Player\\" + helm);
            const BITMAP_t* surviving = Bitmaps.FindTexture(cached);
            CHECK(surviving != nullptr);
            if (surviving != nullptr)
                CHECK(surviving->Ref == 1); // external owner only, not leaked or released by the rejected head
            DeleteBitmap(cached);
            CHECK(Bitmaps.FindTextureByName(L"skin_hq_class05.jpg") == nullptr);
        }
    }
}

TEST_CASE("Rejected bodies distinguish acquired bitmap zero from unsupported texture slots "
          "[player-selector][texture-fallback]")
{
    constexpr GLuint ExternalBitmap = 0;
    for (const bool supported : {true, false})
    {
        INFO("first texture supported=", supported);
        const BodyClientFolder client(CLASS_DARK_LORD);
        const std::string helm = BodyFile(PlayerBodyParts[0], 0, CLASS_DARK_LORD);
        client.AddImproved(helm);
        std::filesystem::remove(client.path / "Data" / "Player" / "hairhq_R.OZJ");
        ClientFolderScope folder(client.path);
        PlayerTextureScope textures;
        TestModelSlots slots;
        // Zero is a valid bitmap-map key, distinct from BITMAP_UNKNOWN. This reference is external.
        REQUIRE(Bitmaps.LoadImage(ExternalBitmap, L"Data\\Player\\skin_hq_class02.jpg", GL_NEAREST, GL_REPEAT));
        REQUIRE(Bitmaps.FindTexture(ExternalBitmap) != nullptr);
        REQUIRE(Bitmaps.FindTexture(ExternalBitmap)->Ref == 1);
        Engine::Object::OpenPlayerBodyParts(0, CLASS_DARK_LORD, false);
        const int slot = BodySlot(PlayerBodyParts[0], 0, CLASS_DARK_LORD);
        BMD& head = Models[slot];
        REQUIRE(head.NumMeshs == 2);
        REQUIRE(head.IndexTexture[0] == 0);
        std::strcpy(head.Textures[0].FileName, supported ? "skin_hq_class02.jpg" : "unsupported.bmp");
        // The other texture is missing, so either an acquired zero ID or an untouched zero is rejected.
        Engine::Object::OpenPlayerBodyTextures(0, CLASS_DARK_LORD);
        CHECK(OpenedFile(slot) == "Data\\Player\\" + helm);
        const BITMAP_t* surviving = Bitmaps.FindTexture(ExternalBitmap);
        CHECK(surviving != nullptr);
        if (surviving != nullptr)
            CHECK(surviving->Ref == 1);
        DeleteBitmap(ExternalBitmap);
        CHECK(Bitmaps.FindTextureByName(L"skin_hq_class02.jpg") == nullptr);
    }
}

TEST_CASE("Accepted improved bodies release only acquired textures through normal model cleanup "
          "[player-selector][texture-fallback]")
{
    constexpr GLuint ExternalBitmap = 0;
    for (const bool supported : {true, false})
    {
        INFO("first texture supported=", supported);
        const BodyClientFolder client(CLASS_DARK_LORD);
        const std::string helm = BodyFile(PlayerBodyParts[0], 0, CLASS_DARK_LORD);
        client.AddImproved(helm);
        ClientFolderScope folder(client.path);
        PlayerTextureScope textures;
        TestModelSlots slots;
        REQUIRE(Bitmaps.LoadImage(ExternalBitmap, L"Data\\Player\\skin_hq_class02.jpg", GL_NEAREST, GL_REPEAT));
        Engine::Object::OpenPlayerBodyParts(0, CLASS_DARK_LORD, false);
        const int slot = BodySlot(PlayerBodyParts[0], 0, CLASS_DARK_LORD);
        BMD& head = Models[slot];
        REQUIRE(head.NumMeshs == 2);
        REQUIRE(head.Meshs[0].Texture == 0);
        REQUIRE(head.Meshs[1].Texture == 1);
        std::strcpy(head.Textures[0].FileName, supported ? "skin_hq_class02.jpg" : "unsupported.bmp");
        // Hair remains loadable: this part is accepted, so later cleanup is ordinary BMD::Release.
        Engine::Object::OpenPlayerBodyTextures(0, CLASS_DARK_LORD);
        CHECK(OpenedFile(slot) == "Data\\Player\\Updated\\" + helm);
        CHECK(head.IndexTexture[0] == (supported ? ExternalBitmap : BITMAP_UNKNOWN));
        REQUIRE(Bitmaps.FindTexture(head.IndexTexture[1]) != nullptr);
        REQUIRE(Bitmaps.FindTexture(ExternalBitmap) != nullptr);
        CHECK(Bitmaps.FindTexture(ExternalBitmap)->Ref == (supported ? 2 : 1));
        head.Release();
        CHECK(head.NumMeshs == 0);
        const BITMAP_t* surviving = Bitmaps.FindTexture(ExternalBitmap);
        CHECK(surviving != nullptr);
        if (surviving != nullptr)
            CHECK(surviving->Ref == 1);
        DeleteBitmap(ExternalBitmap);
        CHECK(Bitmaps.FindTextureByName(L"skin_hq_class02.jpg") == nullptr);
    }
}
