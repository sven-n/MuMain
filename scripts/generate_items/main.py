#!/usr/bin/env python3
"""Generate a shared MU item catalog YAML from legacy item sources."""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import re
import shlex
import struct
import sys
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

try:
    import yaml  # type: ignore
except Exception:  # pragma: no cover
    yaml = None


SECTION_HEADERS: dict[int, list[str]] = {
    0: [
        "Index",
        "Slot",
        "Skill",
        "X",
        "Y",
        "Serial",
        "Opt",
        "Drop",
        "Nombre",
        "Lvl",
        "DmgMin",
        "DmgMax",
        "Speed",
        "Dur",
        "MagiDur",
        "MagiPwr",
        "ReqLvl",
        "Str",
        "Agi",
        "Ene",
        "Vit",
        "Com",
        "Tipo",
        "DwSmGm",
        "DkBkBm",
        "FeMeHe",
        "MgDm",
        "DlLe",
        "SBsDiM",
    ],
    1: [
        "Index",
        "Slot",
        "Skill",
        "X",
        "Y",
        "Serial",
        "Opt",
        "Drop",
        "Nombre",
        "Lvl",
        "DmgMin",
        "DmgMax",
        "Speed",
        "Dur",
        "MagiDur",
        "MagiPwr",
        "ReqLvl",
        "Str",
        "Agi",
        "Ene",
        "Vit",
        "Com",
        "Tipo",
        "DwSmGm",
        "DkBkBm",
        "FeMeHe",
        "MgDm",
        "DlLe",
        "SBsDiM",
    ],
    2: [
        "Index",
        "Slot",
        "Skill",
        "X",
        "Y",
        "Serial",
        "Opt",
        "Drop",
        "Nombre",
        "Lvl",
        "DmgMin",
        "DmgMax",
        "Speed",
        "Dur",
        "MagiDur",
        "MagiPwr",
        "ReqLvl",
        "Str",
        "Agi",
        "Ene",
        "Vit",
        "Com",
        "Tipo",
        "DwSmGm",
        "DkBkBm",
        "FeMeHe",
        "MgDm",
        "DlLe",
        "SBsDiM",
    ],
    3: [
        "Index",
        "Slot",
        "Skill",
        "X",
        "Y",
        "Serial",
        "Opt",
        "Drop",
        "Nombre",
        "Lvl",
        "DmgMin",
        "DmgMax",
        "Speed",
        "Dur",
        "MagiDur",
        "MagiPwr",
        "ReqLvl",
        "Str",
        "Agi",
        "Ene",
        "Vit",
        "Com",
        "Tipo",
        "DwSmGm",
        "DkBkBm",
        "FeMeHe",
        "MgDm",
        "DlLe",
        "SBsDiM",
    ],
    4: [
        "Index",
        "Slot",
        "Skill",
        "X",
        "Y",
        "Serial",
        "Opt",
        "Drop",
        "Nombre",
        "Lvl",
        "DmgMin",
        "DmgMax",
        "Speed",
        "Dur",
        "MagiDur",
        "MagiPwr",
        "ReqLvl",
        "Str",
        "Agi",
        "Ene",
        "Vit",
        "Com",
        "Tipo",
        "DwSmGm",
        "DkBkBm",
        "FeMeHe",
        "MgDm",
        "DlLe",
        "SBsDiM",
    ],
    5: [
        "Index",
        "Slot",
        "Skill",
        "X",
        "Y",
        "Serial",
        "Opt",
        "Drop",
        "Nombre",
        "Lvl",
        "DmgMin",
        "DmgMax",
        "Speed",
        "Dur",
        "MagiDur",
        "MagiPwr",
        "ReqLvl",
        "Str",
        "Agi",
        "Ene",
        "Vit",
        "Com",
        "Tipo",
        "DwSmGm",
        "DkBkBm",
        "FeMeHe",
        "MgDm",
        "DlLe",
        "SBsDiM",
    ],
    6: [
        "Index",
        "Slot",
        "Skill",
        "X",
        "Y",
        "Serial",
        "Opt",
        "Drop",
        "Nombre",
        "Lvl",
        "Def",
        "DefRate",
        "Dur",
        "ReqLvl",
        "Str",
        "Agi",
        "Ene",
        "Vit",
        "Com",
        "Tipo",
        "DwSmGm",
        "DkBkBm",
        "FeMeHe",
        "MgDm",
        "DlLe",
        "SBsDiM",
    ],
    7: [
        "Index",
        "Slot",
        "Skill",
        "X",
        "Y",
        "Serial",
        "Opt",
        "Drop",
        "Nombre",
        "Lvl",
        "Def",
        "DefRate",
        "Dur",
        "ReqLvl",
        "Str",
        "Agi",
        "Ene",
        "Vit",
        "Com",
        "Tipo",
        "DwSmGm",
        "DkBkBm",
        "FeMeHe",
        "MgDm",
        "DlLe",
        "SBsDiM",
    ],
    8: [
        "Index",
        "Slot",
        "Skill",
        "X",
        "Y",
        "Serial",
        "Opt",
        "Drop",
        "Nombre",
        "Lvl",
        "Def",
        "DefRate",
        "Dur",
        "ReqLvl",
        "Str",
        "Agi",
        "Ene",
        "Vit",
        "Com",
        "Tipo",
        "DwSmGm",
        "DkBkBm",
        "FeMeHe",
        "MgDm",
        "DlLe",
        "SBsDiM",
    ],
    9: [
        "Index",
        "Slot",
        "Skill",
        "X",
        "Y",
        "Serial",
        "Opt",
        "Drop",
        "Nombre",
        "Lvl",
        "Def",
        "DefRate",
        "Dur",
        "ReqLvl",
        "Str",
        "Agi",
        "Ene",
        "Vit",
        "Com",
        "Tipo",
        "DwSmGm",
        "DkBkBm",
        "FeMeHe",
        "MgDm",
        "DlLe",
        "SBsDiM",
    ],
    10: [
        "Index",
        "Slot",
        "Skill",
        "X",
        "Y",
        "Serial",
        "Opt",
        "Drop",
        "Nombre",
        "Lvl",
        "Def",
        "DefRate",
        "Dur",
        "ReqLvl",
        "Str",
        "Agi",
        "Ene",
        "Vit",
        "Com",
        "Tipo",
        "DwSmGm",
        "DkBkBm",
        "FeMeHe",
        "MgDm",
        "DlLe",
        "SBsDiM",
    ],
    11: [
        "Index",
        "Slot",
        "Skill",
        "X",
        "Y",
        "Serial",
        "Opt",
        "Drop",
        "Nombre",
        "Lvl",
        "Def",
        "DefRate",
        "Dur",
        "ReqLvl",
        "Str",
        "Agi",
        "Ene",
        "Vit",
        "Com",
        "Tipo",
        "DwSmGm",
        "DkBkBm",
        "FeMeHe",
        "MgDm",
        "DlLe",
        "SBsDiM",
    ],
    12: [
        "Index",
        "Slot",
        "Skill",
        "X",
        "Y",
        "Serial",
        "Opt",
        "Drop",
        "Nombre",
        "Lvl",
        "Def",
        "Dur",
        "ReqLvl",
        "Ene",
        "Str",
        "Agi",
        "Com",
        "Zen",
        "DwSmGm",
        "DkBkBm",
        "FeMeHe",
        "MgDm",
        "DlLe",
        "SBsDiM",
    ],
    13: [
        "Index",
        "Slot",
        "Skill",
        "X",
        "Y",
        "Serial",
        "Opt",
        "Drop",
        "Nombre",
        "Lvl",
        "Dur",
        "Ice",
        "Poison",
        "Light",
        "Fire",
        "Earth",
        "Wind",
        "Water",
        "Tipo",
        "DwSmGm",
        "DkBkBm",
        "FeMeHe",
        "MgDm",
        "DlLe",
        "SBsDiM",
    ],
    14: [
        "Index",
        "Slot",
        "Skill",
        "X",
        "Y",
        "Serial",
        "Opt",
        "Drop",
        "Nombre",
        "Valor",
        "ItemLvl",
    ],
    15: [
        "Index",
        "Slot",
        "Skill",
        "X",
        "Y",
        "Serial",
        "Opt",
        "Drop",
        "Nombre",
        "Lvl",
        "ReqLvl",
        "Ene",
        "Zen",
        "DwSmGm",
        "DkBkBm",
        "FeMeHe",
        "MgDm",
        "DlLe",
        "SBsDiM",
    ],
}


@dataclass(frozen=True)
class Paths:
    item_txt: Path
    item_option: Path
    item_move: Path
    item_stack: Path
    item_csv: Path | None
    item_bmd: Path | None
    output: Path


def default_item_bmd_path(repo_root: Path) -> Path | None:
    candidates = [
        repo_root / "cpp" / "MuServerCpp" / "Tools" / "MuEditor MuEmu" / "Data" / "item.bmd",
        repo_root / "cpp" / "MuClientTools16" / "_bin_" / "Data" / "Item_bmd" / "item.bmd",
    ]
    for candidate in candidates:
        if candidate.exists():
            return candidate
    return None


def parse_args() -> argparse.Namespace:
    repo_root = Path(__file__).resolve().parents[3]
    rust_root = repo_root / "rust"
    detected_item_bmd = default_item_bmd_path(repo_root)

    parser = argparse.ArgumentParser(description="Generate rust/config/items.yaml")
    parser.add_argument(
        "--item-txt",
        default=str(repo_root / "cpp" / "MuServerCpp" / "Data" / "Item" / "Item.txt"),
    )
    parser.add_argument(
        "--item-option",
        default=str(repo_root / "cpp" / "MuServerCpp" / "Data" / "Item" / "ItemOption.txt"),
    )
    parser.add_argument(
        "--item-move",
        default=str(repo_root / "cpp" / "MuServerCpp" / "Data" / "Item" / "ItemMove.txt"),
    )
    parser.add_argument(
        "--item-stack",
        default=str(repo_root / "cpp" / "MuServerCpp" / "Data" / "Item" / "ItemStack.txt"),
    )
    parser.add_argument(
        "--item-csv",
        default=str(Path(__file__).resolve().parent / "item.csv"),
        help="Optional mesh mapping CSV with columns: section,index,dir,model.",
    )
    parser.add_argument(
        "--item-bmd",
        default=str(detected_item_bmd) if detected_item_bmd else None,
        help="Path to legacy item.bmd used as source of model_folder/model_name.",
    )
    parser.add_argument(
        "--strict-model-path",
        action="store_true",
        help="Require model resolution from item.bmd and fail when unresolved.",
    )
    parser.add_argument(
        "--output",
        default=str(rust_root / "config" / "items.yaml"),
    )
    parser.add_argument("--check", action="store_true")
    return parser.parse_args()


def strip_inline_comment(raw: str) -> str:
    in_quotes = False
    i = 0
    while i < len(raw):
        ch = raw[i]
        if ch == '"':
            in_quotes = not in_quotes
            i += 1
            continue
        if not in_quotes and ch == "/" and i + 1 < len(raw) and raw[i + 1] == "/":
            return raw[:i].rstrip()
        i += 1
    return raw.rstrip()


def parse_int(value: str) -> int:
    if value == "*" or value == "":
        return 0
    return int(value)


def parse_opt_int(value: str) -> int | None:
    return None if value == "*" else int(value)


def to_bool_flag(value: int) -> bool:
    return value != 0


def normalize_mesh_stem(name: str) -> str:
    lowered = name.strip().lower()
    lowered = re.sub(r"[^a-z0-9]+", "_", lowered)
    lowered = re.sub(r"_+", "_", lowered).strip("_")
    return lowered


def normalize_match_key(value: str) -> str:
    return re.sub(r"[^a-z0-9]+", "", value.lower())


def normalize_model_folder(folder: str) -> str:
    normalized = folder.replace("\\", "/").strip().strip("/")
    if normalized.lower().startswith("data/"):
        normalized = normalized[5:]
    return normalized


def normalize_snake_component(name: str) -> str:
    normalized = name.replace("!", "")
    normalized = normalized.replace("(", "_")
    normalized = normalized.replace(")", "_")
    normalized = normalized.replace(" ", "_")
    normalized = normalized.replace("-", "_")
    normalized = re.sub(r"([a-z])([A-Z])", r"\1_\2", normalized)
    normalized = re.sub(r"([A-Z]+)([A-Z][a-z])", r"\1_\2", normalized)
    normalized = re.sub(r"([a-zA-Z])(\d)", r"\1_\2", normalized)
    normalized = normalized.lower()
    normalized = re.sub(r"_+", "_", normalized).strip("_")
    return normalized


def normalize_model_name(name: str) -> str:
    base_name = Path(name.replace("\\", "/")).name.strip()
    if not base_name:
        return ""

    stem, ext = base_name, ""
    dot_index = base_name.rfind(".")
    if dot_index > 0:
        stem = base_name[:dot_index]
        ext = base_name[dot_index:].lower()

    return f"{normalize_snake_component(stem)}{ext}"


def normalize_model_folder_for_catalog(folder: str) -> str:
    normalized = normalize_model_folder(folder)
    if not normalized:
        return ""
    parts = [normalize_snake_component(part) for part in normalized.split("/") if part]
    return "/".join(part for part in parts if part)


def collect_available_mesh_paths(repo_root: Path) -> tuple[set[str], set[str]]:
    item_meshes: set[str] = set()
    player_meshes: set[str] = set()

    assets_root = repo_root / "rust" / "assets"
    item_roots = [
        assets_root / "data" / "item",
        assets_root / "remaster" / "data" / "item",
    ]
    player_roots = [
        assets_root / "data" / "player",
        assets_root / "remaster" / "data" / "player",
    ]

    for base in item_roots:
        if not base.exists():
            continue
        for glb in base.rglob("*.glb"):
            rel = glb.relative_to(base).as_posix()
            item_meshes.add(f"item/{rel}")

    for base in player_roots:
        if not base.exists():
            continue
        for glb in base.rglob("*.glb"):
            rel = glb.relative_to(base).as_posix()
            player_meshes.add(f"player/{rel}")

    return item_meshes, player_meshes


def is_set_section(section: int) -> bool:
    return section in (7, 8, 9, 10, 11)


def parse_item_bmd_model_map(path: Path) -> dict[tuple[int, int], tuple[str, str]]:
    raw = path.read_bytes()
    if len(raw) < 8:
        raise ValueError(f"{path}: invalid item.bmd size ({len(raw)} bytes)")

    item_count = struct.unpack_from("<i", raw, 0)[0]
    if item_count <= 0:
        raise ValueError(f"{path}: invalid item count ({item_count})")

    payload = raw[4:-4]
    if len(payload) % item_count != 0:
        raise ValueError(
            f"{path}: invalid item.bmd layout (payload={len(payload)}, count={item_count})"
        )

    bytes_per_item = len(payload) // item_count
    if bytes_per_item < 528:
        raise ValueError(f"{path}: unsupported item record size ({bytes_per_item} bytes)")

    out: dict[tuple[int, int], tuple[str, str]] = {}
    xor_key = (0xFC, 0xCF, 0xAB)

    for row_index in range(item_count):
        start = row_index * bytes_per_item
        end = start + bytes_per_item
        chunk = bytearray(payload[start:end])
        for i in range(bytes_per_item):
            chunk[i] ^= xor_key[i % 3]

        section = struct.unpack_from("<H", chunk, 4)[0]
        index = struct.unpack_from("<H", chunk, 6)[0]

        model_folder_raw = bytes(chunk[8:268]).split(b"\x00", 1)[0]
        model_name_raw = bytes(chunk[268:528]).split(b"\x00", 1)[0]

        model_folder = normalize_model_folder(
            model_folder_raw.decode("latin-1", errors="ignore").strip()
        )
        model_name = (
            Path(model_name_raw.decode("latin-1", errors="ignore").strip().replace("\\", "/"))
            .name
            .strip()
        )

        if not model_name:
            continue

        key = (int(section), int(index))
        if key not in out:
            out[key] = (model_folder, model_name)

    return out


def parse_item_mesh_csv(path: Path) -> list[tuple[int, int, str, str]]:
    rows: list[tuple[int, int, str, str]] = []
    with path.open("r", encoding="utf-8", errors="replace", newline="") as handle:
        reader = csv.DictReader(handle)
        if reader.fieldnames is None:
            raise ValueError(f"{path}: invalid CSV header")

        normalized_headers = {name.strip().lower() for name in reader.fieldnames}
        expected = {"section", "index", "dir", "model"}
        if not expected.issubset(normalized_headers):
            raise ValueError(
                f"{path}: invalid CSV header. expected columns: section,index,dir,model"
            )

        for line_number, row in enumerate(reader, start=2):
            try:
                section = int((row.get("section") or "").strip())
                index = int((row.get("index") or "").strip())
            except Exception as exc:
                raise ValueError(
                    f"{path}:{line_number}: invalid section/index in item.csv"
                ) from exc

            dir_name = (row.get("dir") or "").strip().replace("\\", "/").strip("/")
            model_name = (
                Path((row.get("model") or "").strip().replace("\\", "/")).name.strip()
            )
            if not model_name:
                continue
            rows.append((section, index, dir_name, model_name))

    return rows


def build_mesh_lookup(meshes: set[str]) -> dict[str, list[str]]:
    lookup: dict[str, list[str]] = {}
    for mesh_path in sorted(meshes, key=lambda value: (value.count("/"), len(value), value)):
        stem_key = normalize_match_key(Path(mesh_path).stem)
        full_key = normalize_match_key(mesh_path.rsplit(".", 1)[0])

        for key in (stem_key, full_key):
            if not key:
                continue
            lookup.setdefault(key, []).append(mesh_path)
    return lookup


def pick_best_mesh_candidate(candidates: list[str], folder_hint: str) -> str | None:
    if not candidates:
        return None

    hint_key = normalize_match_key(folder_hint)

    def sort_key(path: str) -> tuple[int, int, int, str]:
        path_key = normalize_match_key(path.rsplit(".", 1)[0])
        has_hint = 1 if hint_key and hint_key in path_key else 0
        return (-has_hint, path.count("/"), len(path), path)

    return sorted(candidates, key=sort_key)[0]


def lookup_mesh_candidates(lookup: dict[str, list[str]], keys: list[str]) -> list[str]:
    out: list[str] = []
    seen: set[str] = set()
    for key in keys:
        if not key:
            continue
        for candidate in lookup.get(key, []):
            if candidate in seen:
                continue
            seen.add(candidate)
            out.append(candidate)
    return out


def resolve_mesh_path_from_csv_row(
    dir_name: str,
    model_name: str,
    item_mesh_lookup: dict[str, list[str]],
    player_mesh_lookup: dict[str, list[str]],
) -> str:
    normalized_dir = dir_name.lower()
    prefer_player = normalized_dir.startswith("player")
    stem = Path(model_name).stem
    keys = [normalize_match_key(stem)]
    if normalized_dir:
        keys.insert(0, normalize_match_key(f"{normalized_dir}/{stem}"))

    preferred_lookup = player_mesh_lookup if prefer_player else item_mesh_lookup
    fallback_lookup = item_mesh_lookup if prefer_player else player_mesh_lookup
    preferred_hint = "player" if prefer_player else "item"

    preferred = pick_best_mesh_candidate(
        lookup_mesh_candidates(preferred_lookup, keys), normalized_dir
    )
    if preferred is not None:
        return preferred

    fallback = pick_best_mesh_candidate(
        lookup_mesh_candidates(fallback_lookup, keys), normalized_dir
    )
    if fallback is not None:
        return fallback

    return f"{preferred_hint}/{normalize_mesh_stem(stem)}.glb"


def slot_for_synthetic_item(section: int, model_name: str) -> str:
    lower = model_name.lower()
    if section in (0, 1, 2, 3, 4, 5):
        return "weapon_right"
    if section == 6:
        return "weapon_left"
    if section == 7:
        return "helm"
    if section == 8:
        return "armor"
    if section == 9:
        return "pants"
    if section == 10:
        return "gloves"
    if section == 11:
        return "boots"
    if section == 12:
        if any(token in lower for token in ("wing", "robe", "cape", "cloak")):
            return "wing"
        if any(
            token in lower
            for token in ("helper", "pet", "panda", "angel", "devil", "fenrir", "horse")
        ):
            return "helper"
        if "ring" in lower:
            return "ring"
        if any(token in lower for token in ("necklace", "amulet", "pendant")):
            return "amulet"
        return "unknown"
    if section == 13:
        if "ring" in lower:
            return "ring"
        if any(token in lower for token in ("necklace", "amulet", "pendant")):
            return "amulet"
        return "unknown"
    if section in (14, 15):
        return "inventory_only"
    return "unknown"


def equip_family_for_synthetic_item(section: int, slot: str, model_name: str) -> str | None:
    lower = model_name.lower()
    if section == 0:
        return "sword"
    if section == 1:
        return "axe"
    if section == 2:
        return "mace"
    if section == 3:
        return "spear"
    if section == 4:
        if "cross" in lower:
            return "cross_bow"
        return "bow"
    if section == 5:
        return "staff"
    if section == 6:
        return "shield"
    if section in (7, 8, 9, 10, 11):
        return "armor"
    if section == 12:
        if slot == "wing":
            if any(token in lower for token in ("robe", "cape", "cloak")):
                return "cloak"
            return "wing"
        if slot == "helper":
            return "helper"
        if slot in ("ring", "amulet"):
            return "accessory"
    if section == 13:
        return "accessory"
    return None


def default_kind_for_synthetic_item(section: int) -> dict[str, Any]:
    if section in (0, 1, 2, 3, 4, 5):
        return {
            "type": "weapon",
            "level": 0,
            "min_damage": 0,
            "max_damage": 0,
            "attack_speed": 0,
            "durability": 0,
            "magic_durability": 0,
            "magic_power": 0,
        }
    if section in (6, 7, 8, 9, 10, 11):
        return {
            "type": "defense",
            "level": 0,
            "defense": 0,
            "defense_rate": 0,
            "durability": 0,
        }
    if section == 12:
        return {
            "type": "wing_or_helper",
            "level": 0,
            "defense": 0,
            "durability": 0,
            "required_level": 0,
            "required_energy": 0,
            "zen": 0,
        }
    if section == 13:
        return {
            "type": "accessory",
            "level": 0,
            "durability": 0,
            "resist_ice": 0,
            "resist_poison": 0,
            "resist_lightning": 0,
            "resist_fire": 0,
            "resist_earth": 0,
            "resist_wind": 0,
            "resist_water": 0,
        }
    if section == 15:
        return {
            "type": "quest_or_event",
            "level": 0,
            "required_level": 0,
            "required_energy": 0,
            "zen": 0,
        }
    return {
        "type": "consumable_or_jewel",
        "value": 0,
        "item_level": 0,
    }


def synthesize_name_from_model(model_name: str) -> str:
    stem = Path(model_name).stem
    readable = re.sub(r"[_\\-]+", " ", stem).strip()
    if not readable:
        return "Unknown Item"
    return readable.title()


def make_synthetic_item_from_csv(
    section: int,
    index: int,
    dir_name: str,
    model_name: str,
    item_mesh_lookup: dict[str, list[str]],
    player_mesh_lookup: dict[str, list[str]],
) -> dict[str, Any]:
    item_id = section * 512 + index
    slot = slot_for_synthetic_item(section, model_name)
    return {
        "id": item_id,
        "section": section,
        "index": index,
        "name": synthesize_name_from_model(model_name),
        "slot": slot,
        "mesh_path": resolve_mesh_path_from_csv_row(
            dir_name=dir_name,
            model_name=model_name,
            item_mesh_lookup=item_mesh_lookup,
            player_mesh_lookup=player_mesh_lookup,
        ),
        "width": 1,
        "height": 1,
        "equip_family": equip_family_for_synthetic_item(section, slot, model_name),
        "requirements": {
            "required_level": 0,
            "strength": 0,
            "agility": 0,
            "energy": 0,
            "vitality": 0,
            "command": 0,
        },
        "class_mask": {
            "dark_wizard": True,
            "dark_knight": True,
            "fairy_elf": True,
            "magic_gladiator": True,
            "dark_lord": True,
            "summoner": True,
            "rage_fighter": True,
        },
        "flags": {
            "allow_serial": True,
            "allow_option": True,
            "allow_drop": True,
        },
        "kind": default_kind_for_synthetic_item(section),
        "options": {"rules": []},
        "movement": {
            "allow_drop": True,
            "allow_sell": True,
            "allow_trade": True,
            "allow_vault": True,
        },
        "stack": {
            "max_stack": 1,
            "create_index": None,
        },
    }


def resolve_mesh_path_from_item_bmd(
    section: int,
    index: int,
    item_models_by_key: dict[tuple[int, int], tuple[str, str]],
    item_mesh_lookup: dict[str, list[str]],
    player_mesh_lookup: dict[str, list[str]],
) -> str | None:
    model_info = item_models_by_key.get((section, index))
    if model_info is None:
        return None

    model_folder, model_name = model_info
    normalized_name = model_name.replace("\\", "/")
    stem = Path(normalized_name).stem
    normalized_folder = normalize_model_folder(model_folder)

    keys = [normalize_match_key(stem)]
    if normalized_folder:
        keys.insert(0, normalize_match_key(f"{normalized_folder}/{stem}"))

    set_item = is_set_section(section)
    preferred_lookup = player_mesh_lookup if set_item else item_mesh_lookup
    fallback_lookup = item_mesh_lookup if set_item else player_mesh_lookup

    preferred = pick_best_mesh_candidate(
        lookup_mesh_candidates(preferred_lookup, keys),
        normalized_folder,
    )
    if preferred is not None:
        return preferred

    return pick_best_mesh_candidate(
        lookup_mesh_candidates(fallback_lookup, keys),
        normalized_folder,
    )


def resolve_mesh_path_legacy(
    section: int,
    index: int,
    slot: int,
    name: str,
    available_item_meshes: set[str],
    available_player_meshes: set[str],
) -> str | None:
    set_item = is_set_section(section)
    idx = index + 1
    lower = name.lower()
    candidates: list[str] = []

    normalized = normalize_mesh_stem(name)
    if normalized:
        candidates.append(f"{normalized}.glb")

    if section == 0:
        candidates.append(f"sword_{idx:02}.glb")
    elif section == 1:
        candidates.append(f"axe_{idx:02}.glb")
    elif section == 2:
        candidates.append(f"mace_{idx:02}.glb")
        if "scepter" in lower:
            candidates.append(f"staff_{idx:02}.glb")
    elif section == 3:
        candidates.append(f"spear_{idx:02}.glb")
    elif section == 4:
        if "bolt" in lower:
            candidates.append("arrows_02.glb")
        if "arrow" in lower:
            candidates.append("arrows_01.glb")
        if "crossbow" in lower:
            candidates.append(f"cross_bow_{idx:02}.glb")
        if "bow" in lower and "crossbow" not in lower:
            candidates.append(f"bow_{idx:02}.glb")
        candidates.append(f"bow_{idx:02}.glb")
        candidates.append(f"cross_bow_{idx:02}.glb")
    elif section == 5:
        if "book" in lower:
            candidates.append(f"book_{idx:02}.glb")
        candidates.append(f"staff_{idx:02}.glb")
    elif section == 6:
        candidates.append(f"shield_{idx:02}.glb")
    elif section == 12:
        if slot == 7:
            if any(token in lower for token in ("cloak", "mantle", "cape", "robe")):
                candidates.append("dark_lord_robe.glb")
                candidates.append("dark_lord_robe_02.glb")
            candidates.append(f"wing_{idx:02}.glb")
        elif slot == 8:
            if "panda" in lower:
                candidates.append("panda_pet.glb")
            if "horse" in lower:
                candidates.append("dark_horse_horn.glb")
                candidates.append("dark_horse_soul.glb")
            if "fenrir" in lower:
                candidates.append("devil_02.glb")
            if "dinorant" in lower or "uniria" in lower:
                candidates.append("horseshoe.glb")
            if "angel" in lower:
                candidates.append("devil_00.glb")
            if "imp" in lower:
                candidates.append("devil_01.glb")
        elif slot == 9:
            candidates.append(f"ring_{idx:02}.glb")
        elif slot == 10:
            candidates.append(f"necklace_{idx:02}.glb")
    elif section == 13:
        if "ring" in lower:
            candidates.append("ring_01.glb")
            candidates.append("ring_02.glb")
        if any(token in lower for token in ("pendant", "necklace")):
            candidates.append("necklace_01.glb")
            candidates.append("necklace_02.glb")

    seen: set[str] = set()
    preferred_prefix = "player" if set_item else "item"
    preferred_meshes = (
        available_player_meshes if set_item else available_item_meshes
    )
    fallback_meshes = (
        available_item_meshes if set_item else available_player_meshes
    )
    for candidate in candidates:
        if candidate in seen:
            continue
        seen.add(candidate)

        preferred_path = f"{preferred_prefix}/{candidate}"
        if preferred_path in preferred_meshes:
            return preferred_path

        fallback_prefix = "item" if set_item else "player"
        fallback_path = f"{fallback_prefix}/{candidate}"
        if fallback_path in fallback_meshes:
            return fallback_path

    return None


def resolve_mesh_path(
    section: int,
    index: int,
    slot: int,
    name: str,
    available_item_meshes: set[str],
    available_player_meshes: set[str],
    item_models_by_key: dict[tuple[int, int], tuple[str, str]],
    item_mesh_lookup: dict[str, list[str]],
    player_mesh_lookup: dict[str, list[str]],
    allow_legacy_fallback: bool,
) -> str | None:
    from_bmd = resolve_mesh_path_from_item_bmd(
        section=section,
        index=index,
        item_models_by_key=item_models_by_key,
        item_mesh_lookup=item_mesh_lookup,
        player_mesh_lookup=player_mesh_lookup,
    )
    if from_bmd is not None:
        return from_bmd

    if not allow_legacy_fallback:
        return None

    return resolve_mesh_path_legacy(
        section=section,
        index=index,
        slot=slot,
        name=name,
        available_item_meshes=available_item_meshes,
        available_player_meshes=available_player_meshes,
    )


def slot_kind(section: int, slot: int) -> str:
    if section in (14, 15):
        return "inventory_only"
    return {
        0: "weapon_right",
        1: "weapon_left",
        2: "helm",
        3: "armor",
        4: "pants",
        5: "gloves",
        6: "boots",
        7: "wing",
        8: "helper",
        9: "ring",
        10: "amulet",
    }.get(slot, "unknown")


def equip_family(section: int, slot: int, name: str) -> str | None:
    lower = name.lower()

    if section == 0:
        return "sword"
    if section == 1:
        return "axe"
    if section == 2:
        return "mace"
    if section == 3:
        return "spear"
    if section == 4:
        if "arrow" in lower or "bolt" in lower:
            return None
        if "crossbow" in lower:
            return "cross_bow"
        return "bow"
    if section == 5:
        if "book of" in lower:
            return "summon_book"
        if "book" in lower:
            return "book"
        return "staff"
    if section == 6:
        return "shield"
    if 7 <= section <= 11:
        return "armor"
    if section == 12:
        if slot == 7:
            if any(token in lower for token in ("cloak", "mantle", "cape")):
                return "cloak"
            return "wing"
        if slot == 8:
            return "helper"
        if slot in (9, 10):
            return "accessory"
        return None
    if section == 13:
        return "accessory"
    return None


def parse_class_mask(row: dict[str, str], section: int) -> dict[str, bool]:
    if section == 14:
        return {
            "dark_wizard": False,
            "dark_knight": False,
            "fairy_elf": False,
            "magic_gladiator": False,
            "dark_lord": False,
            "summoner": False,
            "rage_fighter": False,
        }

    return {
        "dark_wizard": to_bool_flag(parse_int(row.get("DwSmGm", "0"))),
        "dark_knight": to_bool_flag(parse_int(row.get("DkBkBm", "0"))),
        "fairy_elf": to_bool_flag(parse_int(row.get("FeMeHe", "0"))),
        "magic_gladiator": to_bool_flag(parse_int(row.get("MgDm", "0"))),
        "dark_lord": to_bool_flag(parse_int(row.get("DlLe", "0"))),
        "summoner": to_bool_flag(parse_int(row.get("SBsDiM", "0"))),
        # Legacy source does not split Rage Fighter; keep false unless extension exists.
        "rage_fighter": False,
    }


def parse_requirements(row: dict[str, str], section: int) -> dict[str, int]:
    req = {
        "required_level": 0,
        "strength": 0,
        "agility": 0,
        "energy": 0,
        "vitality": 0,
        "command": 0,
    }

    if section in tuple(range(0, 12)):
        req["required_level"] = parse_int(row.get("ReqLvl", "0"))
        req["strength"] = parse_int(row.get("Str", "0"))
        req["agility"] = parse_int(row.get("Agi", "0"))
        req["energy"] = parse_int(row.get("Ene", "0"))
        req["vitality"] = parse_int(row.get("Vit", "0"))
        req["command"] = parse_int(row.get("Com", "0"))
    elif section == 12:
        req["required_level"] = parse_int(row.get("ReqLvl", "0"))
        req["strength"] = parse_int(row.get("Str", "0"))
        req["agility"] = parse_int(row.get("Agi", "0"))
        req["energy"] = parse_int(row.get("Ene", "0"))
        req["command"] = parse_int(row.get("Com", "0"))
    elif section == 15:
        req["required_level"] = parse_int(row.get("ReqLvl", "0"))
        req["energy"] = parse_int(row.get("Ene", "0"))

    return req


def parse_kind(section: int, row: dict[str, str]) -> dict[str, Any]:
    if section in (0, 1, 2, 3, 4, 5):
        return {
            "type": "weapon",
            "level": parse_int(row["Lvl"]),
            "min_damage": parse_int(row["DmgMin"]),
            "max_damage": parse_int(row["DmgMax"]),
            "attack_speed": parse_int(row["Speed"]),
            "durability": parse_int(row["Dur"]),
            "magic_durability": parse_int(row["MagiDur"]),
            "magic_power": parse_int(row["MagiPwr"]),
        }

    if section in (6, 7, 8, 9, 10, 11):
        return {
            "type": "defense",
            "level": parse_int(row["Lvl"]),
            "defense": parse_int(row["Def"]),
            "defense_rate": parse_int(row["DefRate"]),
            "durability": parse_int(row["Dur"]),
        }

    if section == 12:
        return {
            "type": "wing_or_helper",
            "level": parse_int(row["Lvl"]),
            "defense": parse_int(row["Def"]),
            "durability": parse_int(row["Dur"]),
            "required_level": parse_int(row["ReqLvl"]),
            "required_energy": parse_int(row["Ene"]),
            "zen": parse_int(row["Zen"]),
        }

    if section == 13:
        return {
            "type": "accessory",
            "level": parse_int(row["Lvl"]),
            "durability": parse_int(row["Dur"]),
            "resist_ice": parse_int(row["Ice"]),
            "resist_poison": parse_int(row["Poison"]),
            "resist_lightning": parse_int(row["Light"]),
            "resist_fire": parse_int(row["Fire"]),
            "resist_earth": parse_int(row["Earth"]),
            "resist_wind": parse_int(row["Wind"]),
            "resist_water": parse_int(row["Water"]),
        }

    if section == 14:
        return {
            "type": "consumable_or_jewel",
            "value": parse_int(row["Valor"]),
            "item_level": parse_int(row["ItemLvl"]),
        }

    if section == 15:
        return {
            "type": "quest_or_event",
            "level": parse_int(row["Lvl"]),
            "required_level": parse_int(row["ReqLvl"]),
            "required_energy": parse_int(row["Ene"]),
            "zen": parse_int(row["Zen"]),
        }

    raise ValueError(f"unsupported item section {section}")


def parse_item_txt(
    path: Path,
    available_item_meshes: set[str],
    available_player_meshes: set[str],
    item_models_by_key: dict[tuple[int, int], tuple[str, str]],
    item_mesh_lookup: dict[str, list[str]],
    player_mesh_lookup: dict[str, list[str]],
    strict_model_path: bool,
) -> list[dict[str, Any]]:
    items: list[dict[str, Any]] = []
    active_section: int | None = None

    with path.open("r", encoding="utf-8", errors="replace") as handle:
        for line_number, raw_line in enumerate(handle, start=1):
            line = raw_line.strip()
            if not line:
                continue

            if line.startswith("//"):
                continue

            if re.fullmatch(r"\d+", line):
                active_section = int(line)
                if active_section not in SECTION_HEADERS:
                    raise ValueError(f"{path}:{line_number}: unknown section {active_section}")
                continue

            if line.lower() == "end":
                active_section = None
                continue

            if active_section is None:
                continue

            payload = strip_inline_comment(raw_line).strip()
            if not payload:
                continue
            if payload.startswith("//"):
                continue

            tokens = shlex.split(payload, posix=True)
            headers = SECTION_HEADERS[active_section]
            if len(tokens) != len(headers):
                raise ValueError(
                    f"{path}:{line_number}: expected {len(headers)} columns for section {active_section}, got {len(tokens)}"
                )

            row = dict(zip(headers, tokens, strict=True))
            section = active_section
            index = parse_int(row["Index"])
            item_id = section * 512 + index
            slot = parse_int(row["Slot"])
            name = row["Nombre"].strip()
            mesh_path = resolve_mesh_path(
                section=section,
                index=index,
                slot=slot,
                name=name,
                available_item_meshes=available_item_meshes,
                available_player_meshes=available_player_meshes,
                item_models_by_key=item_models_by_key,
                item_mesh_lookup=item_mesh_lookup,
                player_mesh_lookup=player_mesh_lookup,
                allow_legacy_fallback=not strict_model_path,
            )

            if strict_model_path and mesh_path is None:
                model_info = item_models_by_key.get((section, index))
                if model_info is None:
                    raise ValueError(
                        f"{path}:{line_number}: strict model path failed for item "
                        f"section={section} index={index} name='{name}' (missing item.bmd entry)"
                    )
                model_folder, model_name = model_info
                raise ValueError(
                    f"{path}:{line_number}: strict model path failed for item "
                    f"section={section} index={index} name='{name}' "
                    f"(item.bmd model='{model_folder}/{model_name}')"
                )

            item: dict[str, Any] = {
                "id": item_id,
                "section": section,
                "index": index,
                "name": name,
                "slot": slot_kind(section, slot),
                "mesh_path": mesh_path,
                "width": max(1, parse_int(row["X"])),
                "height": max(1, parse_int(row["Y"])),
                "equip_family": equip_family(section, slot, name),
                "requirements": parse_requirements(row, section),
                "class_mask": parse_class_mask(row, section),
                "flags": {
                    "allow_serial": to_bool_flag(parse_int(row["Serial"])),
                    "allow_option": to_bool_flag(parse_int(row["Opt"])),
                    "allow_drop": to_bool_flag(parse_int(row["Drop"])),
                },
                "kind": parse_kind(section, row),
                "options": {"rules": []},
                "movement": {
                    "allow_drop": True,
                    "allow_sell": True,
                    "allow_trade": True,
                    "allow_vault": True,
                },
                "stack": {
                    "max_stack": 1,
                    "create_index": None,
                },
            }
            items.append(item)

    return items


def parse_item_option(path: Path) -> list[dict[str, Any]]:
    rules: list[dict[str, Any]] = []

    with path.open("r", encoding="utf-8", errors="replace") as handle:
        for line_number, raw_line in enumerate(handle, start=1):
            line = strip_inline_comment(raw_line).strip()
            if not line or line.startswith("//"):
                continue
            if line.lower() == "end":
                break

            tokens = line.split()
            if len(tokens) < 9:
                raise ValueError(f"{path}:{line_number}: invalid ItemOption row")

            rules.append(
                {
                    "category": parse_int(tokens[0]),
                    "option_index": parse_int(tokens[1]),
                    "option_value": parse_int(tokens[2]),
                    "item_min_id": parse_int(tokens[3]),
                    "item_max_id": parse_int(tokens[4]),
                    "has_skill": parse_opt_int(tokens[5]),
                    "has_luck": parse_opt_int(tokens[6]),
                    "has_additional_option": parse_opt_int(tokens[7]),
                    "has_excellent_option": parse_opt_int(tokens[8]),
                }
            )

    return rules


def parse_item_move(path: Path) -> dict[int, dict[str, bool]]:
    movement: dict[int, dict[str, bool]] = {}

    with path.open("r", encoding="utf-8", errors="replace") as handle:
        for line_number, raw_line in enumerate(handle, start=1):
            line = strip_inline_comment(raw_line).strip()
            if not line or line.startswith("//"):
                continue
            if line.lower() == "end":
                break

            tokens = line.split()
            if len(tokens) < 5:
                raise ValueError(f"{path}:{line_number}: invalid ItemMove row")

            item_id = parse_int(tokens[0])
            movement[item_id] = {
                "allow_drop": to_bool_flag(parse_int(tokens[1])),
                "allow_sell": to_bool_flag(parse_int(tokens[2])),
                "allow_trade": to_bool_flag(parse_int(tokens[3])),
                "allow_vault": to_bool_flag(parse_int(tokens[4])),
            }

    return movement


def parse_item_stack(path: Path) -> dict[int, dict[str, Any]]:
    stacks: dict[int, dict[str, Any]] = {}

    with path.open("r", encoding="utf-8", errors="replace") as handle:
        for line_number, raw_line in enumerate(handle, start=1):
            line = strip_inline_comment(raw_line).strip()
            if not line or line.startswith("//"):
                continue
            if line.lower() == "end":
                break

            tokens = line.split()
            if len(tokens) < 3:
                raise ValueError(f"{path}:{line_number}: invalid ItemStack row")

            item_id = parse_int(tokens[0])
            create_index = None if tokens[2] == "*" else parse_int(tokens[2])
            stacks[item_id] = {
                "max_stack": parse_int(tokens[1]),
                "create_index": create_index,
            }

    return stacks


def build_catalog(paths: Paths, strict_model_path: bool) -> dict[str, Any]:
    repo_root = Path(__file__).resolve().parents[3]
    available_item_meshes, available_player_meshes = collect_available_mesh_paths(repo_root)
    item_models_by_key: dict[tuple[int, int], tuple[str, str]] = {}
    if paths.item_bmd is not None:
        item_models_by_key = parse_item_bmd_model_map(paths.item_bmd)
    elif strict_model_path:
        raise ValueError("strict model path requires --item-bmd")

    item_mesh_lookup = build_mesh_lookup(available_item_meshes)
    player_mesh_lookup = build_mesh_lookup(available_player_meshes)

    items = parse_item_txt(
        path=paths.item_txt,
        available_item_meshes=available_item_meshes,
        available_player_meshes=available_player_meshes,
        item_models_by_key=item_models_by_key,
        item_mesh_lookup=item_mesh_lookup,
        player_mesh_lookup=player_mesh_lookup,
        strict_model_path=strict_model_path,
    )
    if paths.item_csv is not None:
        csv_rows = parse_item_mesh_csv(paths.item_csv)
        existing_keys = {(int(item["section"]), int(item["index"])) for item in items}
        for section, index, dir_name, model_name in csv_rows:
            key = (section, index)
            if key in existing_keys:
                continue
            items.append(
                make_synthetic_item_from_csv(
                    section=section,
                    index=index,
                    dir_name=dir_name,
                    model_name=model_name,
                    item_mesh_lookup=item_mesh_lookup,
                    player_mesh_lookup=player_mesh_lookup,
                )
            )
            existing_keys.add(key)

    option_rules = parse_item_option(paths.item_option)
    movement_by_item = parse_item_move(paths.item_move)
    stack_by_item = parse_item_stack(paths.item_stack)

    for item in items:
        item_id = item["id"]
        item["options"] = {
            "rules": [
                rule
                for rule in option_rules
                if rule["item_min_id"] <= item_id <= rule["item_max_id"]
            ]
        }
        item["movement"] = movement_by_item.get(
            item_id,
            {
                "allow_drop": True,
                "allow_sell": True,
                "allow_trade": True,
                "allow_vault": True,
            },
        )
        item["stack"] = stack_by_item.get(
            item_id,
            {
                "max_stack": 1,
                "create_index": None,
            },
        )

    items.sort(key=lambda entry: int(entry["id"]))

    seen_ids: set[int] = set()
    for item in items:
        item_id = int(item["id"])
        if item_id in seen_ids:
            raise ValueError(f"duplicate item id detected: {item_id}")
        seen_ids.add(item_id)

    source_paths = [paths.item_txt, paths.item_option, paths.item_move, paths.item_stack]
    if paths.item_csv is not None:
        source_paths.append(paths.item_csv)
    if paths.item_bmd is not None:
        source_paths.append(paths.item_bmd)

    source_mtime = max(
        paths.item_txt.stat().st_mtime,
        paths.item_option.stat().st_mtime,
        paths.item_move.stat().st_mtime,
        paths.item_stack.stat().st_mtime,
        paths.item_csv.stat().st_mtime if paths.item_csv is not None else 0,
        paths.item_bmd.stat().st_mtime if paths.item_bmd is not None else 0,
    )
    generated_at = datetime.fromtimestamp(source_mtime, tz=timezone.utc).replace(
        microsecond=0
    ).isoformat()
    files = []
    for source in source_paths:
        digest = hashlib.sha256(source.read_bytes()).hexdigest()
        files.append(
            {
                "path": str(source),
                "sha256": digest,
            }
        )

    return {
        "schema_version": 1,
        "source": {
            "generated_at_utc": generated_at,
            "generator": "rust/scripts/generate_items/main.py",
            "files": files,
        },
        "items": items,
    }


def encode_catalog(catalog: dict[str, Any]) -> str:
    if yaml is None:
        return json.dumps(catalog, ensure_ascii=True, indent=2) + "\n"

    class NoAliasDumper(yaml.SafeDumper):
        def ignore_aliases(self, data: Any) -> bool:
            return True

    encoded = yaml.dump(
        catalog,
        Dumper=NoAliasDumper,
        sort_keys=False,
        allow_unicode=False,
        width=120,
        default_flow_style=False,
    )
    return encoded


def write_output(content: str, output: Path, check_only: bool) -> int:
    if check_only:
        if not output.exists():
            print(f"[FAIL] missing generated file: {output}")
            return 1
        current = output.read_text(encoding="utf-8")
        if current != content:
            print(f"[FAIL] {output} is out of date. Re-run generator.")
            return 1
        print(f"[OK] {output} is up to date")
        return 0

    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(content, encoding="utf-8")
    return 0


def main() -> int:
    args = parse_args()
    paths = Paths(
        item_txt=Path(args.item_txt).resolve(),
        item_option=Path(args.item_option).resolve(),
        item_move=Path(args.item_move).resolve(),
        item_stack=Path(args.item_stack).resolve(),
        item_csv=Path(args.item_csv).resolve() if args.item_csv else None,
        item_bmd=Path(args.item_bmd).resolve() if args.item_bmd else None,
        output=Path(args.output).resolve(),
    )

    for file_path in [paths.item_txt, paths.item_option, paths.item_move, paths.item_stack]:
        if not file_path.exists():
            print(f"[FAIL] missing input file: {file_path}")
            return 1
    if paths.item_bmd is not None and not paths.item_bmd.exists():
        print(f"[FAIL] missing input file: {paths.item_bmd}")
        return 1
    if paths.item_csv is not None and not paths.item_csv.exists():
        print(f"[FAIL] missing input file: {paths.item_csv}")
        return 1
    if args.strict_model_path and paths.item_bmd is None:
        print("[FAIL] --strict-model-path requires --item-bmd")
        return 1

    catalog = build_catalog(paths, strict_model_path=args.strict_model_path)
    content = encode_catalog(catalog)
    rc = write_output(content, paths.output, args.check)
    if rc != 0:
        return rc
    if args.check:
        return 0

    print(
        f"[OK] generated {paths.output} with {len(catalog['items'])} items "
        f"(schema_version={catalog['schema_version']})"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
