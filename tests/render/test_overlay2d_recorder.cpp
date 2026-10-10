#include <doctest.h>

#include "Render/Renderer/Overlay2DRecorder.h"

#include <stdexcept>

namespace
{
struct CountingRecorder final : Render::Renderer::IOverlay2DRecorder
{
    int texts = 0;
    void RecordText(const Render::Renderer::RecordedText&) override
    {
        ++texts;
    }
    void RecordQuad(const Render::Renderer::RecordedQuad&) override {}
    void RecordBitmap(const Render::Renderer::RecordedBitmap&) override {}
};

// A native label routine that returns early while its caller's scope is active.
bool RecordUntil(CountingRecorder& recorder, bool stopEarly)
{
    Render::Renderer::Overlay2DRecordScope scope(&recorder);
    if (stopEarly)
        return false;
    Render::Renderer::ActiveOverlay2DRecorder()->RecordText({});
    return true;
}
} // namespace

TEST_CASE("no recorder is active outside a scope [render][overlay2d]")
{
    CHECK(Render::Renderer::ActiveOverlay2DRecorder() == nullptr);
}

TEST_CASE("nested record scopes restore the outer recorder [render][overlay2d]")
{
    CountingRecorder outer;
    CountingRecorder inner;
    {
        Render::Renderer::Overlay2DRecordScope outerScope(&outer);
        CHECK(Render::Renderer::ActiveOverlay2DRecorder() == &outer);
        {
            Render::Renderer::Overlay2DRecordScope innerScope(&inner);
            CHECK(Render::Renderer::ActiveOverlay2DRecorder() == &inner);
            // A scope with no recorder draws natively inside a recording one.
            Render::Renderer::Overlay2DRecordScope nativeScope(nullptr);
            CHECK(Render::Renderer::ActiveOverlay2DRecorder() == nullptr);
        }
        CHECK(Render::Renderer::ActiveOverlay2DRecorder() == &outer);
    }
    CHECK(Render::Renderer::ActiveOverlay2DRecorder() == nullptr);
}

TEST_CASE("an early return or an exception leaves no recorder behind [render][overlay2d]")
{
    CountingRecorder recorder;
    CHECK_FALSE(RecordUntil(recorder, true));
    CHECK(Render::Renderer::ActiveOverlay2DRecorder() == nullptr);
    CHECK(RecordUntil(recorder, false));
    CHECK(recorder.texts == 1);
    CHECK(Render::Renderer::ActiveOverlay2DRecorder() == nullptr);

    CHECK_THROWS_AS((
                        [&]
                        {
                            Render::Renderer::Overlay2DRecordScope scope(&recorder);
                            throw std::runtime_error("label code failed");
                        }()),
                    std::runtime_error);
    CHECK(Render::Renderer::ActiveOverlay2DRecorder() == nullptr);
}

TEST_CASE("a recorded text borrows its UTF-8 for the call [render][overlay2d]")
{
    struct CopyingRecorder final : Render::Renderer::IOverlay2DRecorder
    {
        std::string copy;
        void RecordText(const Render::Renderer::RecordedText& text) override
        {
            copy.assign(text.utf8);
        }
        void RecordQuad(const Render::Renderer::RecordedQuad&) override {}
        void RecordBitmap(const Render::Renderer::RecordedBitmap&) override {}
    } recorder;
    std::string scratch = "Cheap potions here";
    Render::Renderer::RecordedText text;
    text.utf8 = scratch;
    recorder.RecordText(text);
    scratch = "overwritten after the call";
    CHECK(recorder.copy == "Cheap potions here");
}
