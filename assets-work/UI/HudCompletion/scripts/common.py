"""Paths and fixed engine mappings for the remaining bottom-HUD artwork."""

from __future__ import annotations

import hashlib
import io
import json
import sys
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[4]
WORK = ROOT / "assets-work/UI/HudCompletion"
UI_WORK = WORK.parent
sys.dont_write_bytecode = True
sys.path.insert(0, str(UI_WORK / "scripts"))
sys.path.insert(0, str(ROOT / "tools"))
import assemble as pilot_assembly  # noqa: E402
import preview as pilot_preview  # noqa: E402
import mu_texture  # noqa: E402

BRANCH = "codex/ui-hud-completion"
BASE = "0f589224"
HUD_TOP = 429
HUD_HEIGHT = 41
XP_SOURCE_Y = 41
XP_HEIGHT = 10
STATE_HEIGHT = 41
STATE_NAMES = ("normal", "hover", "pressed-selected", "selected-hover")
CELL_SIZE = (32, 38)
GAUGE_SIZE = (45, 39)
COLUMN_SIZE = (16, 39)
BUTTON_SIZE = (30, 41)
FULL_HD = (1920, 1080)
ITEM_KEYS = "QWER"
SKILL_KEYS = "12345"
ALTERNATE_KEYS = "67890"
PANEL_PATHS = ("Interface/newui_menu01.OZJ", "Interface/newui_menu02.OZJ",
               "Interface/partCharge1/newui_menu03.OZJ")
CASH_PATH = "Interface/partCharge1/newui_menu_Bt05.OZJ"
SKILL_PATHS = ("Interface/newui_skillbox.OZJ", "Interface/newui_skillbox2.OZJ")
EXP_PATHS = ("Interface/newui_Exbar.OZJ", "Interface/Exbar_Master.OZJ")
GAUGES = {
    "red": {"path": "Interface/newui_menu_red.OZJ", "position": (158, 432), "size": GAUGE_SIZE},
    "green": {"path": "Interface/newui_menu_green.OZJ", "position": (158, 432), "size": GAUGE_SIZE},
    "blue": {"path": "Interface/newui_menu_blue.OZJ", "position": (437, 432), "size": GAUGE_SIZE},
    "SD": {"path": "Interface/newui_menu_SD.OZJ", "position": (204, 431), "size": COLUMN_SIZE},
    "AG": {"path": "Interface/newui_menu_AG.OZJ", "position": (420, 431), "size": COLUMN_SIZE},
}
ASSETS = (*PANEL_PATHS, "Interface/newui_menu02-03.OZJ", CASH_PATH,
          *(entry["path"] for entry in GAUGES.values()), *EXP_PATHS, *SKILL_PATHS)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def original(relative):
    return Image.open(WORK / "original" / Path(relative).with_suffix(".jpg")).convert("RGB")


def decoded(relative, after=True):
    path = WORK / "exports" / relative
    if not after or not path.exists():
        path = WORK / "original/containers" / relative
    if not path.exists():
        path = ROOT / "src/bin/Data" / relative
    return Image.open(io.BytesIO(mu_texture.unwrap_bytes(path.read_bytes(), ".ozj"))).convert("RGBA")


def write_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2) + "\n")
