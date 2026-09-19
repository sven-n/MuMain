# ctl Server Configuration Design

## Goal

Let `./ctl run` select a server without editing runtime `config.ini`.

## Configuration

Optional workspace-root `.env` file:

```bash
MUMAIN_SERVER_IP=192.168.1.217
MUMAIN_SERVER_PORT=44406
```

The existing workspace `.gitignore` excludes `.env`.

## Behavior

- `ctl` reads `.env` without exporting unrelated values.
- `./ctl run` translates configured values into Main's existing `/u<ip>` and `/p<port>` arguments.
- `ctl` prints the effective CLI server target before launching `Main`.
- Explicit `/u...` or `/p...` arguments override their corresponding `.env` values.
- Missing `.env` preserves current behavior.
- Empty IP, non-numeric port, or port outside `1..65535` stops launch with a clear error.
- `Main` and `config.ini` remain unchanged.

## Verification

- Shell syntax check passes.
- A temporary fake `Main` confirms generated arguments and explicit overrides.
- `./ctl build` continues staging all CMake-managed runtime artifacts.
