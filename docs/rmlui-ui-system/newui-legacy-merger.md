# Retiring the CUIMng/CNewUIManager split

**Status: complete, 2026-09-05.** This document used to carry the full phase-by-phase execution
record (Phases 0–6: migrating `CCreditWin`/`CServerMsgWin`, deleting dead `COptionWin`, migrating
`CServerSelWin`/`CMsgWin`/`CSysMenuWin`/`CCharSelMainWin`/`CCharMakeWin`/`CLoginMainWin`/`CLoginWin`/
`CCharInfoBalloonMng`, deleting `CUIMng` itself, then the namespace/rename and `UI/` directory
restructure) plus a "Gotchas" section of still-relevant `CObject`/`CManager`/`LayoutMode` findings.

**Outcome, confirmed in `migration-ledger.md`'s "CWin-tier (retired)" table**: zero live `CWin`/
`CWinEx` subclasses remain anywhere in the tree. Every window that used to derive from `CWin` is now
on `mu::ui::window::CObject`/`CManager`, in `UI/` topic folders by domain (`UI/HUD/`, `UI/Windows/`,
etc. — the old `UI/NewUI/...` split no longer exists). Nothing will ever migrate off `CWin` again;
no future session needs this file to do new work.

The still-relevant technical content — general `CObject`/`CManager`/`LayoutMode`/`UI::Scaling`
gotchas this migration surfaced, not specific to the migration itself — moved to
[`engine-findings.md`](engine-findings.md)'s own "`CObject`/`CManager`/`LayoutMode` gotchas"
section. The full phase-by-phase narrative (bugs found and fixed at the time: z-order races, ESC
double-triggers, transform double-scaling) is preserved in git history on this branch, not here —
see `git log` for the individual commits if the detail is ever needed.
