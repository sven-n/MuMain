#include "doctest.h"

#include <atomic>
#include <chrono>
#include <future>
#include <thread>

#include "Dotnet/HandleRegistry.h"

namespace
{
struct FakeConnection
{
    int calls = 0;
};

using Registry = DotNetBridge::HandleRegistry<FakeConnection>;

int CallsAfterDispatch(Registry& registry, int32_t handle, FakeConnection& connection)
{
    registry.Dispatch(handle, [](FakeConnection& target) { ++target.calls; });
    return connection.calls;
}
} // namespace

TEST_CASE("HandleRegistry dispatches to the target of the handle [network][dotnet]")
{
    Registry registry;
    FakeConnection first;
    FakeConnection second;
    registry.Add(1, &first);
    registry.Add(2, &second);

    CHECK(CallsAfterDispatch(registry, 2, second) == 1);
    CHECK(first.calls == 0);
    CHECK_FALSE(registry.Dispatch(3, [](FakeConnection&) {}));
}

TEST_CASE("HandleRegistry finds nothing for a removed handle [network][dotnet]")
{
    // A disconnect callback that arrives after the Connection is gone must not reach it.
    Registry registry;
    FakeConnection connection;
    registry.Add(7, &connection);
    registry.Remove(7);

    CHECK_FALSE(registry.Dispatch(7, [](FakeConnection& target) { ++target.calls; }));
    CHECK(connection.calls == 0);
}

TEST_CASE("HandleRegistry Remove waits for a callback in progress [network][dotnet]")
{
    Registry registry;
    FakeConnection connection;
    registry.Add(1, &connection);

    std::promise<void> entered;
    std::promise<void> release;
    std::shared_future<void> released = release.get_future().share();
    std::thread callbackThread([&] {
        registry.Dispatch(1, [&](FakeConnection& target) {
            entered.set_value();
            released.wait();
            ++target.calls;
        });
    });
    entered.get_future().wait();

    std::atomic<bool> removed = false;
    std::thread destroyingThread([&] {
        registry.Remove(1);
        removed = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    const bool removedWhileCallbackRan = removed;
    release.set_value();
    callbackThread.join();
    destroyingThread.join();

    CHECK_FALSE(removedWhileCallbackRan);
    CHECK(removed);
    CHECK(connection.calls == 1);
    CHECK_FALSE(registry.Dispatch(1, [](FakeConnection&) {}));
}

TEST_CASE("HandleRegistry lets a callback remove and add handles [network][dotnet]")
{
    // OnDisconnected removes its own handle from inside the callback.
    Registry registry;
    FakeConnection closing;
    FakeConnection opened;
    registry.Add(1, &closing);

    CHECK(registry.Dispatch(1, [&](FakeConnection&) {
        registry.Remove(1);
        registry.Add(2, &opened);
    }));

    CHECK_FALSE(registry.Dispatch(1, [](FakeConnection&) {}));
    CHECK(CallsAfterDispatch(registry, 2, opened) == 1);
}
