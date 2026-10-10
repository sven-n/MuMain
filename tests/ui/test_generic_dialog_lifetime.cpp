#include <doctest.h>

#include "stdafx.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "UI/Social/FriendDialogs.h"

extern int g_iChatInputType;

using mu::ui::window::CGenericConfirmDialog;
using mu::ui::window::GenericDialogConfig;

TEST_CASE("Cancelling a queued dialog preserves unrelated requests [ui][dialog-lifetime]")
{
    CGenericConfirmDialog dialog;
    int callbacks = 0;
    GenericDialogConfig config;
    config.onPrimary = [&] { ++callbacks; };
    config.onCancel = [&] { ++callbacks; };
    const auto first = dialog.Show(config);
    const auto cancelled = dialog.Show(config);
    const auto third = dialog.Show(config);

    dialog.Cancel(cancelled);
    CHECK_FALSE(dialog.IsPending(cancelled));
    CHECK(dialog.IsActive(first));
    CHECK(dialog.IsPending(third));
    dialog.Cancel(first);
    CHECK(dialog.IsActive(third));
    dialog.Cancel(third);
    CHECK_FALSE(dialog.IsVisible());
    CHECK(callbacks == 0);
}

TEST_CASE("Expired queued and active dialogs never run actions [ui][dialog-lifetime]")
{
    CGenericConfirmDialog dialog;
    bool valid = true;
    int callbacks = 0;
    GenericDialogConfig config;
    config.isValid = [&] { return valid; };
    config.onPrimary = [&] { ++callbacks; };
    config.onCancel = [&] { ++callbacks; };
    const auto first = dialog.Show(config);
    const auto second = dialog.Show(config);
    const auto unrelated = dialog.Show({});

    valid = false;
    dialog.Update();
    CHECK_FALSE(dialog.IsPending(first));
    CHECK_FALSE(dialog.IsPending(second));
    CHECK(dialog.IsActive(unrelated));
    CHECK(dialog.Show(config) == 0);
    CHECK(callbacks == 0);
    dialog.Cancel(unrelated);
}

TEST_CASE("Friend session reset dismisses only its own dialogs [ui][dialog-lifetime]")
{
    CGenericConfirmDialog dialog;
    auto* previousDialog = mu::ui::window::g_pGenericConfirmDialog;
    const int previousChatType = g_iChatInputType;
    mu::ui::window::g_pGenericConfirmDialog = &dialog;
    g_iChatInputType = 1;
    {
        UI::Social::FriendDialogs friends;
        friends.Notice(L"Friend notice");
        friends.FriendRequest(L"Incoming request", L"Friend");
        const auto unrelated = dialog.Show({});
        friends.Reset();
        CHECK(dialog.IsActive(unrelated));
        dialog.Cancel(unrelated);
        CHECK_FALSE(dialog.IsVisible());
    }
    mu::ui::window::g_pGenericConfirmDialog = previousDialog;
    g_iChatInputType = previousChatType;
}
