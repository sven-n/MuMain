#include "stdafx.h"

#include "UI/Dialogs/ConfirmRequest.h"

#include "UI/Core/WindowSystem.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "UI/Dialogs/MessageBox.h"

namespace UI::Dialogs
{
void ShowConfirm(ConfirmRequest request)
{
    mu::ui::window::GenericDialogConfig cfg;
    if (!request.acceptLabel.empty())
        cfg.primaryLabel = std::move(request.acceptLabel);
    cfg.showCancel = request.cancellable;
    cfg.lines.reserve(request.lines.size());
    for (ConfirmRequest::Line& line : request.lines)
        cfg.lines.push_back({ std::move(line.text), line.bold, line.color });
    if (request.duelCaption)
    {
        cfg.portrait2D = mu::ui::window::GenericDialogConfig::Portrait2D{ std::move(*request.duelCaption) };
        // The duel art and its lines don't fit the default panel.
        cfg.tallPanel = true;
    }
    cfg.onPrimary = std::move(request.onAccept);
    cfg.onCancel = std::move(request.onCancel);
    mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
}

bool IsMessageBoxOpen()
{
    return !g_MessageBox->IsEmpty();
}

bool IsDuelRequestBlocked()
{
    return g_pNewUISystem->IsImpossibleDuelInterface();
}
}
