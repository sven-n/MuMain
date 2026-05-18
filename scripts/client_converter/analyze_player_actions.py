#!/usr/bin/env python3
"""Analyze Season2/Season20 player action indices and emit a mapping report.

Inputs:
- Season2 player.bmd (legacy reference)
- Season20 player.bmd (target)
- muonline-cross commented PlayerAction seed mapping

Output:
- JSON report with coverage and heuristic suggestions for unresolved legacy names.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Dict, List, Tuple


SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.append(str(SCRIPT_DIR))
import bmd_converter as bc  # noqa: E402


def parse_seed_mapping(player_action_cs: Path) -> Dict[str, int]:
    lines = player_action_cs.read_text(encoding="utf-8", errors="replace").splitlines()
    in_block = False
    current = -1
    out: Dict[str, int] = {}

    for line in lines:
        raw = line.strip()
        if raw.startswith("// public enum PlayerAction"):
            in_block = True
            continue
        if not in_block:
            continue
        if raw.startswith("// }"):
            break
        if not raw.startswith("//"):
            continue

        body = raw[2:].strip()
        if not body or body in {"{", "}"}:
            continue
        body = body.split("//", 1)[0].strip()
        if not body or not body.endswith(","):
            continue
        body = body[:-1].strip()

        if "=" in body:
            name, value = [part.strip() for part in body.split("=", 1)]
            current = int(value)
        else:
            name = body
            current += 1

        if name == "MaxPlayerAction":
            continue
        out[name] = current

    return out


def parse_legacy_rust_enum(legacy_action_catalog: Path) -> Dict[str, int]:
    lines = legacy_action_catalog.read_text(encoding="utf-8", errors="replace").splitlines()
    in_enum = False
    current: int | None = None
    out: Dict[str, int] = {}

    for line in lines:
        raw = line.strip()
        if raw.startswith("pub enum PlayerAction"):
            in_enum = True
            continue
        if not in_enum:
            continue
        if raw.startswith("}"):
            break
        if not raw or raw.startswith("//") or raw.startswith("#") or "," not in raw:
            continue

        body = raw.split("//", 1)[0].rstrip(",").strip()
        if not body:
            continue

        if "=" in body:
            name, value = [part.strip() for part in body.split("=", 1)]
            try:
                current = int(value)
            except ValueError:
                continue
        else:
            if current is None:
                continue
            name = body
            current += 1

        out[name] = current

    return out


def root_bone_index(model: bc.BmdModel) -> int:
    for i, bone in enumerate(model.bones):
        if bone.name.strip().lower() == "bip01":
            return i
    return 0


def action_fingerprint(model: bc.BmdModel, root_index: int, action_index: int) -> Tuple[float, ...]:
    action = model.actions[action_index]
    positions, _ = model.bones[root_index].matrices[action_index]

    if not positions:
        return (
            float(action.num_animation_keys),
            float(1 if action.lock_positions else 0),
            0.0,
            0.0,
            0.0,
            0.0,
        )

    sx, sy, sz = positions[0]
    ex, ey, ez = positions[-1]
    arc = 0.0
    for i in range(1, len(positions)):
        x1, y1, z1 = positions[i - 1]
        x2, y2, z2 = positions[i]
        arc += ((x2 - x1) ** 2 + (y2 - y1) ** 2 + (z2 - z1) ** 2) ** 0.5

    return (
        float(action.num_animation_keys),
        float(1 if action.lock_positions else 0),
        ex - sx,
        ey - sy,
        ez - sz,
        arc,
    )


def fingerprint_distance(a: Tuple[float, ...], b: Tuple[float, ...]) -> float:
    return (
        abs(a[0] - b[0]) * 2.0
        + (0.0 if a[1] == b[1] else 5.0)
        + abs(a[2] - b[2]) * 0.2
        + abs(a[3] - b[3]) * 0.2
        + abs(a[4] - b[4]) * 0.2
        + abs(a[5] - b[5]) * 0.1
    )


def build_report(
    season2_player_bmd: Path,
    season20_player_bmd: Path,
    player_action_cs: Path,
    legacy_action_catalog: Path,
) -> Dict[str, object]:
    s2_model = bc.parse_bmd(season2_player_bmd)
    s20_model = bc.parse_bmd(season20_player_bmd)

    seed = parse_seed_mapping(player_action_cs)
    legacy = parse_legacy_rust_enum(legacy_action_catalog)

    s2_root = root_bone_index(s2_model)
    s20_root = root_bone_index(s20_model)

    s2_fp = [action_fingerprint(s2_model, s2_root, i) for i in range(s2_model.num_actions)]
    s20_fp = [action_fingerprint(s20_model, s20_root, i) for i in range(s20_model.num_actions)]

    unresolved: List[Dict[str, object]] = []
    for legacy_name, legacy_index in legacy.items():
        seed_name = f"Player{legacy_name}"
        if seed_name in seed:
            continue
        if not (0 <= legacy_index < len(s2_fp)):
            continue

        base = s2_fp[legacy_index]
        best_idx = min(range(len(s20_fp)), key=lambda i: fingerprint_distance(base, s20_fp[i]))
        unresolved.append(
            {
                "legacy_name": legacy_name,
                "legacy_index": legacy_index,
                "suggested_index": best_idx,
                "distance": round(fingerprint_distance(base, s20_fp[best_idx]), 4),
            }
        )

    unresolved.sort(key=lambda item: (item["distance"], item["legacy_index"]))

    return {
        "season2": {
            "path": str(season2_player_bmd),
            "version": s2_model.version,
            "actions": s2_model.num_actions,
            "bones": s2_model.num_bones,
        },
        "season20": {
            "path": str(season20_player_bmd),
            "version": s20_model.version,
            "actions": s20_model.num_actions,
            "bones": s20_model.num_bones,
        },
        "seed_mapping": {
            "path": str(player_action_cs),
            "entries": len(seed),
        },
        "legacy_catalog": {
            "path": str(legacy_action_catalog),
            "entries": len(legacy),
            "seed_name_matches": sum(1 for name in legacy if f"Player{name}" in seed),
        },
        "unresolved_suggestions": unresolved,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description="Analyze Season2/Season20 player action mapping")
    parser.add_argument(
        "--season2-player-bmd",
        type=Path,
        default=Path("/home/allanbatista/Workspaces/MuData/Season2/DataNorm/player/player.bmd"),
    )
    parser.add_argument(
        "--season20-player-bmd",
        type=Path,
        default=Path("/home/allanbatista/Workspaces/MuData/Season20/DataNorm/player/player.bmd"),
    )
    parser.add_argument(
        "--player-action-cs",
        type=Path,
        default=Path("cpp/muonline-cross/Client.Main/Models/PlayerAction.cs"),
    )
    parser.add_argument(
        "--legacy-action-catalog",
        type=Path,
        default=Path("rust/client/src/game/character/action_catalog.rs"),
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("rust/assets/reports/player_action_mapping_season20.json"),
    )
    args = parser.parse_args()

    report = build_report(
        season2_player_bmd=args.season2_player_bmd,
        season20_player_bmd=args.season20_player_bmd,
        player_action_cs=args.player_action_cs,
        legacy_action_catalog=args.legacy_action_catalog,
    )

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(f"report written: {args.output}")
    print(
        "seed matches:"
        f" {report['legacy_catalog']['seed_name_matches']}/{report['legacy_catalog']['entries']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
