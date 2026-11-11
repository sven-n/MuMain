# Exit Game Shutdown Design

## Problem

The in-game Exit Game button sends `LogOutType::CloseGame`, then waits for the
server. The server closes the connection before a logout response arrives.
Auto-reconnect interprets that deliberate disconnect as a network failure and
rejoins the game.

## Design

After sending the logout packet, request the platform's normal window-close
path with `PostMessage(g_hWnd, WM_CLOSE, 0, 0)`. SDL converts this to
`SDL_EVENT_QUIT`; the main loop sets `Destroy`; existing shutdown code releases
audio, rendering, networking, and the window.

Keep the server packet send for clean server-side logout. Avoid reconnect-state
changes, timeouts, new APIs, or direct process termination.

## Verification

Add a source-contract test requiring the close request to follow the
`LogOutType::CloseGame` send. Build Release. Run the contract plus nearby UI and
platform shutdown tests. Manually confirm Exit Game terminates the process when
the local server is available.
