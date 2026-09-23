// Scripted input frame/ownership tests plus a real RmlUi cancellation check.
// Event delivery uses a synchronous fake sink; no SDL window is required.
//
// Run: ctest --test-dir <build directory> --build-config Release -R "synthetic"

#include "doctest.h"

#include "App/Control/ControlCommands.h"
#include "Core/Input/KeyState.h"
#include "Core/Input/SyntheticInput.h"
#include "Core/Input/UiInputRouter.h"
#include "Core/Platform/WinCompat.h"

#include <cstdint>
#include <SDL3/SDL.h>
#include <RmlUi/Core.h>
#include <RmlUi/Core/EventListener.h>
#include <vector>
#include <string>

extern int MouseX;
extern int MouseY;
extern float g_fWindowMouseX;
extern float g_fWindowMouseY;
extern bool MouseLButton;
extern bool MouseLButtonPush;
extern bool MouseLButtonPop;
extern unsigned int WindowWidth;
extern unsigned int WindowHeight;

using namespace Core::Input::Synthetic;

namespace
{
std::vector<SDL_EventType> delivered;
bool consume = false;
std::string deliveredText;
std::vector<SDL_KeyboardEvent> deliveredKeys;
bool FakeDelivery(SDL_Event& event, bool& propagates)
{
    delivered.push_back(static_cast<SDL_EventType>(event.type));
    if (event.type == SDL_EVENT_TEXT_INPUT)
        deliveredText = event.text.text;
    if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP)
        deliveredKeys.push_back(event.key);
    propagates = !consume;
    return true;
}

struct FakeConsumer : Core::Input::IUiInputConsumer
{
    bool pressed = false;
    bool physicalPrimaryPressed = false;
    int cancellations = 0;
    bool ProcessSdlEvent(SDL_Event& event, SDL_Window*) override
    {
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
            pressed = true;
        if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
            pressed = false;
        return false;
    }
    void CancelSyntheticMousePress(unsigned char button, SDL_Window*) override
    {
        if (button != SDL_BUTTON_LEFT)
            return;
        pressed = false;
        physicalPrimaryPressed = false;
        ++cancellations;
    }
    bool IsMouseOverUI() const override
    {
        return pressed;
    }
};

bool UiDelivery(SDL_Event& event, bool& propagates)
{
    delivered.push_back(static_cast<SDL_EventType>(event.type));
    propagates = Core::Input::RouteToUi(event, nullptr);
    return true;
}

struct NullRenderer : Rml::RenderInterface
{
    Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex>, Rml::Span<const int>) override
    {
        return 0;
    }
    void RenderGeometry(Rml::CompiledGeometryHandle, Rml::Vector2f, Rml::TextureHandle) override {}
    void ReleaseGeometry(Rml::CompiledGeometryHandle) override {}
    Rml::TextureHandle LoadTexture(Rml::Vector2i&, const Rml::String&) override
    {
        return 0;
    }
    Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte>, Rml::Vector2i) override
    {
        return 0;
    }
    void ReleaseTexture(Rml::TextureHandle) override {}
    void EnableScissorRegion(bool) override {}
    void SetScissorRegion(Rml::Rectanglei) override {}
};

struct ClickCounter : Rml::EventListener
{
    int clicks = 0;
    void ProcessEvent(Rml::Event&) override
    {
        ++clicks;
    }
};

struct ResetInjector
{
    ResetInjector()
    {
        Reset();
        SetEventDelivery(&FakeDelivery, nullptr);
        delivered.clear();
        consume = false;
        deliveredText.clear();
        deliveredKeys.clear();
    }
    ~ResetInjector()
    {
        Reset();
        SetEventDelivery(nullptr, nullptr);
    }
};
} // namespace

TEST_CASE("Synthetic input names the keys the shim translates [core][synthetic-input]")
{
    CHECK(VirtualKeyFromName("esc") == VK_ESCAPE);
    CHECK(VirtualKeyFromName("Escape") == VK_ESCAPE);
    CHECK(VirtualKeyFromName("HOME") == VK_HOME);
    CHECK(VirtualKeyFromName("enter") == VK_RETURN);
    CHECK(VirtualKeyFromName("f1") == VK_F1);
    CHECK(VirtualKeyFromName("F12") == VK_F12);
    CHECK(VirtualKeyFromName("printscreen") == VK_SNAPSHOT);
    CHECK(VirtualKeyFromName("i") == 'I');
    CHECK(VirtualKeyFromName("I") == 'I');
    CHECK(VirtualKeyFromName("7") == '7');

    CHECK_FALSE(VirtualKeyFromName("bogus").has_value());
    CHECK_FALSE(VirtualKeyFromName("").has_value());
    CHECK_FALSE(VirtualKeyFromName("?").has_value());
    CHECK_FALSE(VirtualKeyFromName("f13").has_value());

    CHECK(MouseButtonFromName("left") == MouseButton::Left);
    CHECK(MouseButtonFromName("Right") == MouseButton::Right);
    CHECK_FALSE(MouseButtonFromName("middle").has_value());
}

TEST_CASE("A hotkey is down for exactly one frame [core][synthetic-input]")
{
    ResetInjector guard;
    CHECK(IsIdle());
    CHECK(PressKey(VK_HOME));
    CHECK_FALSE(IsIdle());

    // Scheduled, not yet applied: the frame has not begun.
    CHECK_FALSE(IsKeyHeld(VK_HOME));
    CHECK_FALSE(Core::Input::IsKeyDown(VK_HOME));

    BeginFrame();
    CHECK(IsKeyHeld(VK_HOME));
    CHECK(Core::Input::IsKeyDown(VK_HOME));
    CHECK_FALSE(IsKeyHeld(VK_END));

    BeginFrame();
    CHECK_FALSE(IsKeyHeld(VK_HOME));
    CHECK_FALSE(IsIdle());

    BeginFrame();
    CHECK(IsIdle());
}

TEST_CASE("Synthetic navigation keys carry SDL keycodes for RmlSDL [core][synthetic-input]")
{
    ResetInjector guard;
    const struct { int virtualKey; SDL_Scancode scancode; SDL_Keycode keycode; } cases[] = {
        {VK_TAB, SDL_SCANCODE_TAB, SDLK_TAB},
        {VK_DOWN, SDL_SCANCODE_DOWN, SDLK_DOWN},
        {VK_SPACE, SDL_SCANCODE_SPACE, SDLK_SPACE},
    };
    for (const auto& key : cases)
    {
        REQUIRE(PressKey(key.virtualKey));
        BeginFrame();
        BeginFrame();
        BeginFrame();
        REQUIRE(deliveredKeys.size() >= 2);
        CHECK(deliveredKeys[deliveredKeys.size() - 2].type == SDL_EVENT_KEY_DOWN);
        CHECK(deliveredKeys[deliveredKeys.size() - 1].type == SDL_EVENT_KEY_UP);
        CHECK(deliveredKeys.back().scancode == key.scancode);
        CHECK(deliveredKeys.back().key == key.keycode);
        CHECK(deliveredKeys[deliveredKeys.size() - 2].key == key.keycode);
    }
}

TEST_CASE("A second injection is refused while one is in flight [core][synthetic-input]")
{
    ResetInjector guard;
    CHECK(PressKey(VK_ESCAPE));
    CHECK_FALSE(PressKey('I'));
    CHECK_FALSE(Click(1.0f, 1.0f, MouseButton::Left));

    BeginFrame();
    BeginFrame();
    BeginFrame();
    CHECK(IsIdle());
    CHECK(Click(1.0f, 1.0f, MouseButton::Left));
}

TEST_CASE("A click walks press, hold, release through the mouse globals [core][synthetic-input]")
{
    ResetInjector guard;
    WindowWidth = 1280;
    WindowHeight = 960;
    MouseLButton = false;
    MouseLButtonPush = false;
    MouseLButtonPop = false;

    CHECK(Click(1000.0f, 725.0f, MouseButton::Left));

    BeginFrame();
    CHECK(g_fWindowMouseX == doctest::Approx(1000.0f));
    CHECK(g_fWindowMouseY == doctest::Approx(725.0f));
    // The overlay space is 640x480 stretched over the window.
    CHECK(MouseX == 500);
    CHECK(MouseY == 362);
    CHECK(MouseLButton);
    CHECK(MouseLButtonPush);
    CHECK_FALSE(MouseLButtonPop);
    CHECK(IsKeyHeld(VK_LBUTTON));
    CHECK(Core::Input::IsKeyDown(VK_LBUTTON));

    // The scene clears the one-shot push at the end of the frame.
    MouseLButtonPush = false;
    BeginFrame();
    CHECK(MouseLButton);
    CHECK(IsKeyHeld(VK_LBUTTON));

    BeginFrame();
    CHECK_FALSE(MouseLButton);
    CHECK(MouseLButtonPop);
    CHECK_FALSE(IsKeyHeld(VK_LBUTTON));
    CHECK_FALSE(IsIdle());

    BeginFrame();
    CHECK(IsIdle());
}

TEST_CASE("Abandoning an injection releases what it pressed [core][synthetic-input]")
{
    ResetInjector guard;
    WindowWidth = 1280;
    WindowHeight = 960;

    // A key that is dropped while down: the state simply goes away.
    CHECK(PressKey(VK_HOME));
    BeginFrame();
    CHECK(IsKeyHeld(VK_HOME));
    Reset();
    CHECK(IsIdle());
    CHECK_FALSE(IsKeyHeld(VK_HOME));
    CHECK_FALSE(Core::Input::IsKeyDown(VK_HOME));

    // A click that is dropped while the button is down has to take the press
    // back, or the game keeps seeing one no frame will ever end. No release
    // edge is raised: the command it belonged to was already answered.
    MouseLButton = false;
    MouseLButtonPush = false;
    MouseLButtonPop = false;

    CHECK(Click(1000.0f, 725.0f, MouseButton::Left));
    BeginFrame();
    CHECK(MouseLButton);

    Reset();
    CHECK(IsIdle());
    CHECK_FALSE(MouseLButton);
    CHECK_FALSE(MouseLButtonPush);
    CHECK_FALSE(MouseLButtonPop);
    CHECK_FALSE(IsKeyHeld(VK_LBUTTON));
    CHECK_FALSE(Core::Input::IsKeyDown(VK_LBUTTON));

    // And a fresh injection is accepted right away.
    CHECK(Click(10.0f, 10.0f, MouseButton::Right));
}

TEST_CASE("Every accepted injection is numbered [core][synthetic-input]")
{
    ResetInjector guard;

    const std::uint64_t idle = CurrentGeneration();
    CHECK(CurrentGeneration() == idle);

    CHECK(PressKey(VK_HOME));
    const std::uint64_t first = CurrentGeneration();
    CHECK(first != idle);

    // A refused injection is not one: the number belongs to the press in
    // flight, so its owner still recognises it.
    CHECK_FALSE(PressKey('I'));
    CHECK(CurrentGeneration() == first);

    BeginFrame();
    BeginFrame();
    BeginFrame();
    CHECK(IsIdle());
    // Finishing does not hand the injector to anyone else either.
    CHECK(CurrentGeneration() == first);

    CHECK(Click(1.0f, 1.0f, MouseButton::Left));
    CHECK(CurrentGeneration() != first);
}

TEST_CASE("RmlUi clears a cancelled press without click or stuck active state [core][synthetic-input]")
{
    NullRenderer renderer;
    Rml::SetRenderInterface(&renderer);
    REQUIRE(Rml::Initialise());
    auto* context = Rml::CreateContext("synthetic-cancel", {320, 240});
    REQUIRE(context != nullptr);
    auto* document = context->LoadDocumentFromMemory(
        "<rml><head><style>body {margin:0;} #button { width:100px; height:100px; }</style></head>"
        "<body><div id='button' "
        "style='display:block;width:100px;height:100px;background-color:red'></div></body></rml>");
    REQUIRE(document != nullptr);
    ClickCounter listener;
    auto* button = document->GetElementById("button");
    REQUIRE(button != nullptr);
    button->AddEventListener("click", &listener);
    document->Show();
    context->Update();
    REQUIRE(context->GetElementAtPoint({20, 20}) == button);
    context->ProcessMouseMove(20, 20, 0);
    context->ProcessMouseButtonDown(0, 0);
    context->ProcessMouseLeave();
    context->ProcessMouseButtonUp(0, 0);
    context->ProcessMouseMove(20, 20, 0);
    CHECK(listener.clicks == 0);
    context->ProcessMouseButtonDown(0, 0);
    context->ProcessMouseButtonUp(0, 0);
    CHECK(listener.clicks == 1);
    button->RemoveEventListener("click", &listener);
    Rml::RemoveContext("synthetic-cancel");
    Rml::Shutdown();
    Rml::SetRenderInterface(nullptr);
}

TEST_CASE("Synthetic delivery arbitrates UI and keeps text before Return [core][synthetic-input]")
{
    ResetInjector guard;
    WindowWidth = 1280;
    WindowHeight = 960;
    consume = true;
    MouseLButton = false;
    MouseX = 51;
    MouseY = 52;
    g_fWindowMouseX = 53.0f;
    g_fWindowMouseY = 54.0f;
    CHECK(Click(100.0f, 200.0f, MouseButton::Left));
    BeginFrame();
    CHECK(delivered == std::vector<SDL_EventType>{SDL_EVENT_MOUSE_MOTION, SDL_EVENT_MOUSE_BUTTON_DOWN});
    CHECK_FALSE(MouseLButton);
    CHECK_FALSE(IsKeyHeld(VK_LBUTTON));
    CHECK(MouseX == 51);
    CHECK(MouseY == 52);
    CHECK(g_fWindowMouseX == 53.0f);
    CHECK(g_fWindowMouseY == 54.0f);
    Reset();
    CHECK(IsIdle());

    consume = false;
    delivered.clear();
    CHECK(TypeText("hello", true));
    const auto generation = CurrentGeneration();
    BeginFrame();
    CHECK(deliveredText == "hello");
    CHECK(delivered == std::vector<SDL_EventType>{SDL_EVENT_TEXT_INPUT});
    CHECK_FALSE(IsKeyHeld(VK_RETURN));
    BeginFrame();
    CHECK(IsKeyHeld(VK_RETURN));
    BeginFrame();
    CHECK_FALSE(IsKeyHeld(VK_RETURN));
    BeginFrame();
    CHECK(IsIdle());
    CHECK(CurrentGeneration() == generation);
    CHECK(delivered == std::vector<SDL_EventType>{SDL_EVENT_TEXT_INPUT, SDL_EVENT_KEY_DOWN, SDL_EVENT_KEY_UP});
}

TEST_CASE("Synthetic text is bounded printable UTF-8 [core][synthetic-input]")
{
    ResetInjector guard;
    CHECK_FALSE(ValidText(""));
    CHECK(ValidText(std::string(256, 'x')));
    CHECK_FALSE(ValidText(std::string(257, 'x')));
    CHECK(ValidText("é"));
    CHECK_FALSE(ValidText(std::string("a\0b", 3)));
    CHECK_FALSE(ValidText("a\n"));
    CHECK_FALSE(ValidText("a\x7f"));
    CHECK_FALSE(ValidText("\xc0\xaf"));
    CHECK_FALSE(ValidText("\xed\xa0\x80"));
    CHECK_FALSE(ValidText("\xf4\x90\x80\x80"));
    CHECK_FALSE(TypeText("", false));
    CHECK(TypeText("a", true));
    CHECK_FALSE(TypeText("b", false));
    BeginFrame();
    Reset();
    delivered.clear();
    BeginFrame();
    CHECK(delivered.empty());
    CHECK(IsIdle());
}

TEST_CASE("Lost delivery target fails instead of reporting release [core][synthetic-input]")
{
    ResetInjector guard;
    CHECK(PressKey(VK_HOME));
    SetEventDelivery(nullptr, nullptr);
    BeginFrame();
    CHECK(IsIdle());
    CHECK(FailureFor(CurrentGeneration()) == DeliveryFailure::TargetLost);
    CHECK(delivered.empty());
    CHECK_FALSE(IsKeyHeld(VK_HOME));
}

TEST_CASE("Physical button press cancels only its synthetic click [core][synthetic-input]")
{
    ResetInjector guard;
    WindowWidth = 1280;
    WindowHeight = 960;
    CHECK(Click(20.0f, 20.0f, MouseButton::Left));
    BeginFrame();
    REQUIRE(IsKeyHeld(VK_LBUTTON));
    CancelForPhysicalButton(SDL_BUTTON_RIGHT);
    CHECK_FALSE(IsIdle());
    CancelForPhysicalButton(SDL_BUTTON_LEFT);
    CHECK(IsIdle());
    CHECK(FailureFor(CurrentGeneration()) == DeliveryFailure::PhysicalOverlap);
    CHECK_FALSE(IsKeyHeld(VK_LBUTTON));
    delivered.clear();
    BeginFrame();
    CHECK(delivered.empty());
    CHECK(PressKey(VK_HOME));
    CHECK(FailureFor(CurrentGeneration()) == DeliveryFailure::None);
}

TEST_CASE("Abandoned text never submits its pending Enter [core][synthetic-input]")
{
    ResetInjector guard;
    CHECK(TypeText("secret", true));
    const auto first = CurrentGeneration();
    BeginFrame();
    CHECK(deliveredText == "secret");
    Reset();
    BeginFrame();
    CHECK(delivered == std::vector<SDL_EventType>{SDL_EVENT_TEXT_INPUT});
    CHECK(PressKey(VK_HOME));
    const auto second = CurrentGeneration();
    CHECK(first != second);
    // An old owner must compare its generation before resetting a new one.
    if (CurrentGeneration() == first)
        Reset();
    BeginFrame();
    CHECK(IsKeyHeld(VK_HOME));
}

TEST_CASE("Consumer teardown cancels a held click without releasing it [core][synthetic-input]")
{
    ResetInjector guard;
    FakeConsumer consumer;
    Core::Input::SetUiInputConsumer(&consumer);
    SetEventDelivery(&UiDelivery, nullptr);
    WindowWidth = 1280;
    WindowHeight = 960;
    CHECK(Click(50.0f, 60.0f, MouseButton::Left));
    const auto first = CurrentGeneration();
    BeginFrame();
    REQUIRE(consumer.pressed);
    CHECK_FALSE(MouseLButton);
    CancelDelivery();
    CHECK_FALSE(consumer.pressed);
    CHECK(consumer.cancellations == 1);
    CHECK(FailureFor(first) == DeliveryFailure::TargetLost);
    CHECK(IsIdle());
    delivered.clear();
    BeginFrame();
    CHECK(delivered.empty());
    Core::Input::SetUiInputConsumer(nullptr);
    SetEventDelivery(&FakeDelivery, nullptr);
    CHECK(PressKey(VK_HOME));
    CHECK(CurrentGeneration() != first);
    CHECK(FailureFor(CurrentGeneration()) == DeliveryFailure::None);
}

TEST_CASE("A consumed synthetic key does not enter legacy polling [core][synthetic-input]")
{
    ResetInjector guard;
    consume = true;
    CHECK(PressKey(VK_HOME));
    BeginFrame();
    CHECK(delivered == std::vector<SDL_EventType>{SDL_EVENT_KEY_DOWN});
    CHECK_FALSE(IsKeyHeld(VK_HOME));
    BeginFrame();
    CHECK(delivered == std::vector<SDL_EventType>{SDL_EVENT_KEY_DOWN, SDL_EVENT_KEY_UP});
    CHECK_FALSE(IsKeyHeld(VK_HOME));
}

TEST_CASE("Cancelling a synthetic right press preserves physical primary ownership [core][synthetic-input]")
{
    ResetInjector guard;
    FakeConsumer consumer;
    Core::Input::SetUiInputConsumer(&consumer);
    SetEventDelivery(&UiDelivery, nullptr);
    WindowWidth = 1280;
    WindowHeight = 960;
    consumer.physicalPrimaryPressed = true;
    CHECK(Click(50.0f, 60.0f, MouseButton::Right));
    BeginFrame();
    CHECK(consumer.pressed);
    CancelDelivery();
    CHECK(consumer.physicalPrimaryPressed);
    CHECK(consumer.cancellations == 0);
    CHECK(FailureFor(CurrentGeneration()) == DeliveryFailure::TargetLost);
    Core::Input::SetUiInputConsumer(nullptr);
}

TEST_CASE("Failed owners retain independent outcomes across newer schedules [core][synthetic-input]")
{
    ResetInjector guard;
    using App::Control::Act;
    using App::Control::Request;
    std::unique_ptr<Act> click;
    std::unique_ptr<Act> hotkey;
    std::unique_ptr<Act> type;
    const auto clickRequest = Request::Parse(R"({"cmd":"click-ui","id":1,"x":25,"y":30})");
    const auto hotkeyRequest = Request::Parse(R"({"cmd":"hotkey","id":2,"key":"home"})");
    const auto typeRequest = Request::Parse(R"({"cmd":"type","id":3,"text":"x"})");
    REQUIRE(App::Control::Commands::ClickUi(clickRequest, click).empty());
    REQUIRE(click != nullptr);
    click->SetEncodedId(clickRequest.EncodedId());
    const auto clickGeneration = CurrentGeneration();
    BeginFrame();
    CancelForPhysicalButton(SDL_BUTTON_LEFT);
    CHECK(FailureFor(clickGeneration) == DeliveryFailure::PhysicalOverlap);

    REQUIRE(App::Control::Commands::Hotkey(hotkeyRequest, hotkey).empty());
    REQUIRE(hotkey != nullptr);
    hotkey->SetEncodedId(hotkeyRequest.EncodedId());
    const auto hotkeyGeneration = CurrentGeneration();
    CancelDelivery();
    REQUIRE(App::Control::Commands::Type(typeRequest, type).empty());
    REQUIRE(type != nullptr);
    type->SetEncodedId(typeRequest.EncodedId());
    CHECK(FailureFor(hotkeyGeneration) == DeliveryFailure::TargetLost);

    std::string response;
    CHECK(click->Tick(response) == Act::Status::Finished);
    CHECK(response.find(R"("id":1)") != std::string::npos);
    CHECK(response.find(R"("ok":false)") != std::string::npos);
    CHECK(response.find("physical mouse press") != std::string::npos);
    click.reset();
    CHECK(FailureFor(clickGeneration) == DeliveryFailure::None);
    CHECK(hotkey->Tick(response) == Act::Status::Finished);
    CHECK(response.find(R"("id":2)") != std::string::npos);
    CHECK(response.find("target disappeared") != std::string::npos);
    hotkey.reset();
    CHECK(type->Tick(response) == Act::Status::Running);
    BeginFrame();
    BeginFrame();
    CHECK(type->Tick(response) == Act::Status::Finished);
    CHECK(response.find(R"("id":3)") != std::string::npos);
    CHECK(response.find(R"("ok":true)") != std::string::npos);
}

TEST_CASE("Type handler rejects invalid arguments before scheduling and refuses overlap [core][synthetic-input]")
{
    ResetInjector guard;
    std::unique_ptr<App::Control::Act> act;
    const auto generation = CurrentGeneration();
    for (const auto* raw : {R"({"cmd":"type"})", R"({"cmd":"type","text":""})",
                            R"({"cmd":"type","text":"x","enter":null})",
                            R"({"cmd":"type","text":"x","enter":1})"})
    {
        const auto response = App::Control::Commands::Type(App::Control::Request::Parse(raw), act);
        CHECK(response.find(R"("error":"bad_request")") != std::string::npos);
        CHECK(act == nullptr);
        CHECK(CurrentGeneration() == generation);
    }
    CHECK(PressKey(VK_HOME));
    const auto response = App::Control::Commands::Type(
        App::Control::Request::Parse(R"({"cmd":"type","text":"x"})"), act);
    CHECK(response.find(R"("error":"busy")") != std::string::npos);
    CHECK(act == nullptr);
}
