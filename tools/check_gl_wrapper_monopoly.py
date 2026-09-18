#!/usr/bin/env python3
"""DXP-10 step 4 -- state-wrapper monopoly guard.

Fails if a raw `gl<Name>(...)` call appears outside Render/ (relative to
--source-root) that isn't on the allowlist below. The allowlist mirrors, 1:1,
the "left raw, explicitly out of scope" section of
.ai-os/memory/tasks/dxp/DXP-10-state-wrapper-monopoly.md -- every entry here
is a documented, deliberate deferral to a later task (DXP-11 F10, DXP-12,
DXP-14), not an oversight. A new hit means either a regression (someone
called GL directly instead of the wrapper family) or a genuinely new
deferred case that needs its own allowlist entry AND a doc update.

Allowlist keys are paths relative to --source-root (i.e. relative to
.../src/source), not to the repo root -- this script also runs unmodified
against checkouts that don't wrap the client under a "CLIENT/" folder (e.g.
the upstream repo, whose root IS src/source's grandparent).

Usage: python3 check_gl_wrapper_monopoly.py [--source-root DIR]
Exit code 0 = clean, 1 = violation(s) found (printed to stderr).
"""
import argparse
import pathlib
import re
import sys

# gl<Uppercase> not preceded by an identifier character -- excludes wgl*, fn_gl*,
# and identifiers like `openglMatrix` that merely contain "gl".
RAW_GL_CALL = re.compile(r"(?<![A-Za-z0-9_])gl[A-Z][A-Za-z0-9_]*\s*\(")

# Entire files: confirmed debug/editor-only (DXP-10/DXP-11 census; MainScene.cpp's
# hits are inside #ifdef _EDITOR, FrustumRenderer.cpp is a wireframe debug tool).
# Paths are relative to --source-root.
FULL_FILE_ALLOWLIST = {
    "Camera/FrustumRenderer.cpp",
    "Scenes/MainScene.cpp",
}

# Functions allowed anywhere: already macro-intercepted (glColor* -> g_CurrentColor
# via GLColorIntercept.h, DXP-08a) or is itself a wrapper name, not a raw GL call.
GLOBAL_FUNCTION_ALLOWLIST = {
    "glColor3f", "glColor4f", "glColor3fv", "glColor4fv", "glColor3ub", "glColor4ub",
    "glViewport2",
}

# Per-file function allowlist: each entry is a documented, deliberate deferral.
# Paths are relative to --source-root.
PER_FILE_FUNCTION_ALLOWLIST = {
    # Screenshot readback + _DEBUG-only GL_DEBUG_OUTPUT context setup (DXP-12 / one-time init).
    "App/Platform/Windows/Winmain.cpp": {
        "glReadPixels", "glPixelStorei", "glEnable",
    },
    # SetViewport() canonical impl + TestDepthBuffer readback (DXP-12) + GetOpenGLMatrix
    # (DXP-11 design doc finding F10 -- single live caller, recommended for deletion
    # once that caller moves to GlobalUBO's CPU mirror).
    "Camera/CameraProjection.cpp": {
        "glViewport", "glReadPixels", "glGetFloatv",
    },
    # One-time startup system-info dump.
    "Core/Utilities/Log/ErrorReport.cpp": {
        "glGetString", "glGetIntegerv",
    },
    # Icon/guild/castle-mark texture creation -- DXP-12 territory.
    "Engine/Object/ZzzInventory.cpp": {
        "glGenTextures", "glBindTexture", "glTexParameteri", "glTexImage2D", "glDeleteTextures",
    },
    # Screenshot readback -- DXP-12 territory.
    "Scenes/SceneManager.cpp": {
        "glReadPixels",
    },
    # Font/glyph atlas sub-upload -- DXP-12 territory.
    "UI/Widgets/UIControls.cpp": {
        "glBindTexture", "glTexSubImage2D",
    },
    # Frozen-frame capture texture for the reconnect overlay -- DXP-12 territory.
    "UI/Dialogs/ReconnectDialog.cpp": {
        "glGenTextures", "glBindTexture", "glTexParameteri", "glReadBuffer", "glCopyTexImage2D",
    },
    # Event fog parameters -- D3D has no fixed-function fog; belongs to DXP-14's shader-uniform design.
    "World/GameMaps/GMBattleCastle.cpp": {
        "glFogfv", "glFogf",
    },
}


def scan(source_root: pathlib.Path) -> list[str]:
    violations = []
    for ext in ("*.cpp", "*.h"):
        for path in sorted(source_root.rglob(ext)):
            rel = path.relative_to(source_root).as_posix()
            if rel.startswith("Render/") or "/ThirdParty/" in rel:
                continue
            if rel in FULL_FILE_ALLOWLIST:
                continue
            file_allow = PER_FILE_FUNCTION_ALLOWLIST.get(rel, frozenset())
            try:
                text = path.read_text(encoding="utf-8", errors="ignore")
            except OSError:
                continue
            for lineno, line in enumerate(text.splitlines(), 1):
                code = line.split("//", 1)[0]
                for m in RAW_GL_CALL.finditer(code):
                    name = m.group(0)[: m.group(0).index("(")].strip()
                    if name in GLOBAL_FUNCTION_ALLOWLIST or name in file_allow:
                        continue
                    violations.append(f"{rel}:{lineno}: raw {name}() outside Render/ -- {line.strip()}")
    return violations


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-root", default=None,
                         help="Path to src/source (default: resolved from this script's location)")
    args = parser.parse_args()

    if args.source_root:
        source_root = pathlib.Path(args.source_root)
    else:
        source_root = pathlib.Path(__file__).resolve().parent.parent / "src" / "source"

    if not source_root.is_dir():
        print(f"error: source root not found: {source_root}", file=sys.stderr)
        return 1

    violations = scan(source_root)
    if violations:
        print("DXP-10 state-wrapper monopoly guard: FAILED", file=sys.stderr)
        print(f"{len(violations)} raw GL call(s) outside Render/ not on the allowlist:", file=sys.stderr)
        for v in violations:
            print(f"  {v}", file=sys.stderr)
        print(
            "\nRoute new call sites through the Render/ wrapper family "
            "(ZzzOpenglUtil.h EnableXxx/DisableXxx/SetXxx). If this is a genuine, "
            "deliberate deferral (matches DXP-12/DXP-14 territory), add it to the "
            f"allowlist in {pathlib.Path(__file__).name} (keys are relative to "
            "--source-root) AND to .ai-os/memory/tasks/dxp/"
            "DXP-10-state-wrapper-monopoly.md's deferral list.",
            file=sys.stderr,
        )
        return 1

    print("DXP-10 state-wrapper monopoly guard: OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
