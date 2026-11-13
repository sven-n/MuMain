#include "doctest.h"

#include "Render/Textures/TextureFailure.h"

namespace
{
bool chooseYes = true;
int dialogs = 0;
int quits = 0;

const Render::Textures::TextureErrorActions actions{
    [](const std::string&, const std::string&, bool)
    {
        ++dialogs;
        return chooseYes;
    },
    []() { ++quits; },
};

void Dispatch(bool forceDestroy, bool yes)
{
    chooseYes = yes;
    dialogs = 0;
    quits = 0;
    Render::Textures::DispatchTextureError("Error", "bitmap failure", forceDestroy, actions);
}
} // namespace

TEST_CASE("A texture error goes on only when the player chooses to and may [render][texture]")
{
    CHECK(Render::Textures::ContinueAfterTextureError(false, true));
    CHECK_FALSE(Render::Textures::ContinueAfterTextureError(false, false));
    CHECK_FALSE(Render::Textures::ContinueAfterTextureError(true, true));
}

TEST_CASE("A texture error shows the dialog once and asks to quit unless the player continues [render][texture]")
{
    Dispatch(false, true);
    CHECK(dialogs == 1);
    CHECK(quits == 0);

    Dispatch(false, false);
    CHECK(dialogs == 1);
    CHECK(quits == 1);

    Dispatch(true, true);
    CHECK(dialogs == 1);
    CHECK(quits == 1);
}
