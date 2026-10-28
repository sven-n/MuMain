#include <doctest.h>

// Exercise the real retirement storage without a GPU or a production test hook.
#include "UI/RmlBridge/RmlRenderTarget.cpp"

namespace UI::RmlBridge
{
namespace
{
// Mirrors a global window with a by-value preview target. Its final retirement must
// precede destruction of the storage, even when rendering first uses it in main().
struct GlobalPreview
{
    RenderTarget target{nullptr};

    ~GlobalPreview()
    {
        Retire(2, "rtt:global-preview-lifetime-regression-final");
    }
};

GlobalPreview preview;
} // namespace

TEST_CASE("global preview retirement storage outlives its owner [ui][render_target]")
{
    Retire(1, "rtt:global-preview-lifetime-regression-first");
    CHECK(RetiredTextures().size() == 1);
}
} // namespace UI::RmlBridge
