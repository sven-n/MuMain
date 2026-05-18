#!/usr/bin/env python3
"""Generate monster catalog YAML from legacy MuServerCpp/MuClient sources."""

from __future__ import annotations

import argparse
import re
from dataclasses import dataclass
from pathlib import Path

@dataclass
class SpotRow:
    monster_id: int
    map_number: int
    begin_x: int
    begin_y: int
    end_x: int
    end_y: int
    quantity: int
    comment: str


@dataclass
class MonsterRow:
    monster_id: int
    name: str
    level: int
    max_hp: int
    scale: float | None = None
    model_index: int | None = None


def normalize_name(name: str) -> str:
    return "".join(ch for ch in name.strip().lower() if ch.isalnum())


def map_name_to_legacy_number(name: str) -> int | None:
    mapping = {
        "lorencia": 0,
        "dungeon": 1,
        "devias": 2,
        "noria": 3,
        "losttower": 4,
        "atlans": 7,
        "tarkan": 8,
        "icarus": 10,
        "elbeland": 51,
        "raklion": 57,
        "karutan1": 80,
        "karutan2": 81,
        "acheron": 91,
    }
    return mapping.get(normalize_name(name))


def parse_monster_set_base(path: Path) -> list[SpotRow]:
    rows: list[SpotRow] = []
    in_spots = False

    for raw in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        line = raw.strip()

        if "SPOTS" in line.upper():
            in_spots = True
            continue

        if not in_spots:
            continue

        if not line or line.startswith("//"):
            continue

        comment = ""
        if "//" in line:
            line, comment = line.split("//", 1)
            line = line.strip()
            comment = comment.strip()

        if not line:
            continue

        if line.lower() == "end":
            continue

        tokens = line.split()
        if len(tokens) < 9:
            continue

        try:
            monster_id = int(tokens[0], 10)
            map_number = int(tokens[1], 10)
            begin_x = int(tokens[3], 10)
            begin_y = int(tokens[4], 10)
            end_x = int(tokens[5], 10)
            end_y = int(tokens[6], 10)
            quantity = int(tokens[8], 10)
        except ValueError:
            continue

        rows.append(
            SpotRow(
                monster_id=monster_id,
                map_number=map_number,
                begin_x=begin_x,
                begin_y=begin_y,
                end_x=end_x,
                end_y=end_y,
                quantity=max(quantity, 0),
                comment=comment,
            )
        )

    return rows


def slugify(raw: str) -> str:
    normalized = re.sub(r"[^a-zA-Z0-9]+", "_", raw.strip().lower()).strip("_")
    return normalized or "spot"


def dump_world_yaml(map_id: int, map_name: str, rows: list[SpotRow], out_path: Path) -> None:
    lines: list[str] = []
    lines.append("schema_version: 1")
    lines.append(f"map_id: {map_id}")
    lines.append(f"map_name: \"{map_name}\"")
    lines.append("spots:")

    if not rows:
        lines.append("  []")
    else:
        for idx, row in enumerate(rows, start=1):
            base_name = slugify(row.comment) if row.comment else "spot"
            spot_name = f"{base_name}_{(idx):03d}"
            lines.append(f"  - spot_name: \"{spot_name}\"")
            lines.append(
                f"    xyxy: [{row.begin_x}, {row.begin_y}, {row.end_x}, {row.end_y}]"
            )
            lines.append("    monsters:")
            lines.append(f"      - monster_id: {row.monster_id}")
            lines.append(f"        quantity: {row.quantity}")

    out_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def parse_monster_txt(path: Path) -> list[MonsterRow]:
    rows: list[MonsterRow] = []
    for raw in path.read_text(encoding="utf-8", errors="ignore").splitlines():
        line = raw.strip()
        if not line or line.startswith("//"):
            continue

        head, _, _comment = line.partition("//")
        line = head.strip()
        if not line:
            continue

        tokens = line.split()
        if not tokens:
            continue

        try:
            monster_id = int(tokens[0], 10)
        except ValueError:
            continue

        quote_start = line.find('"')
        quote_end = line.find('"', quote_start + 1) if quote_start != -1 else -1
        if quote_start == -1 or quote_end == -1:
            continue

        name = line[quote_start + 1 : quote_end].strip()
        if not name:
            continue

        trailing = line[quote_end + 1 :].split()
        if len(trailing) < 2:
            continue
        try:
            level = int(trailing[0], 10)
            max_hp = int(trailing[1], 10)
        except ValueError:
            continue

        rows.append(
            MonsterRow(
                monster_id=monster_id,
                name=name,
                level=max(level, 0),
                max_hp=max(max_hp, 0),
                scale=None,
                model_index=None,
            )
        )

    dedup: dict[int, MonsterRow] = {}
    for row in rows:
        dedup[row.monster_id] = row
    return [dedup[mid] for mid in sorted(dedup.keys())]


def _extract_braced_block(source: str, start_offset: int) -> str | None:
    open_idx = source.find("{", start_offset)
    if open_idx == -1:
        return None

    depth = 0
    for idx in range(open_idx, len(source)):
        ch = source[idx]
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return source[open_idx + 1 : idx]
    return None


def _extract_function_block(source: str, signature: str) -> str | None:
    start = source.find(signature)
    if start == -1:
        return None
    return _extract_braced_block(source, start)


def _extract_switch_type_block(source: str) -> str | None:
    switch_pos = source.find("switch(Type)")
    if switch_pos == -1:
        return None
    return _extract_braced_block(source, switch_pos)


def parse_monster_visuals_from_muclient(path: Path) -> dict[int, tuple[float | None, int | None]]:
    source = path.read_text(encoding="utf-8", errors="ignore")
    create_monster_block = _extract_function_block(source, "CHARACTER *CreateMonster(")
    if create_monster_block is None:
        return {}

    switch_block = _extract_switch_type_block(create_monster_block)
    if switch_block is None:
        return {}

    case_re = re.compile(r"^\s*case\s+(\d+)\s*:")
    scale_re = re.compile(r"c->Object\.Scale\s*=\s*([0-9]+(?:\.[0-9]+)?)f")
    model_re = re.compile(r"CreateCharacter\(\s*Key\s*,\s*MODEL_MONSTER01\+(\d+)\s*,")

    resolved: dict[int, tuple[float | None, int | None]] = {}
    current_cases: list[int] = []
    current_lines: list[str] = []
    depth = 0

    def flush_case_group() -> None:
        if not current_cases:
            return
        block = "\n".join(current_lines)
        scales = {float(match.group(1)) for match in scale_re.finditer(block)}
        model_indices = {int(match.group(1)) for match in model_re.finditer(block)}

        scale_value: float | None = None
        if len(scales) == 1:
            candidate = next(iter(scales))
            if candidate > 0.0:
                scale_value = candidate

        model_index_value: int | None = None
        if len(model_indices) == 1:
            candidate = next(iter(model_indices))
            if candidate >= 0:
                model_index_value = candidate

        if scale_value is None and model_index_value is None:
            return
        for case_id in current_cases:
            resolved[case_id] = (scale_value, model_index_value)

    for raw_line in switch_block.splitlines():
        line = raw_line.rstrip("\n")
        stripped = line.strip()

        if depth == 0:
            case_match = case_re.match(stripped)
            if case_match:
                if current_cases and current_lines:
                    flush_case_group()
                    current_cases = []
                    current_lines = []
                current_cases.append(int(case_match.group(1)))
                continue

            if stripped.startswith("default:"):
                if current_cases:
                    flush_case_group()
                current_cases = []
                current_lines = []
                continue

        if current_cases:
            current_lines.append(line)

        depth += line.count("{")
        depth -= line.count("}")
        if depth < 0:
            depth = 0

    flush_case_group()
    return resolved


def dump_monster_catalog_yaml(rows: list[MonsterRow], out_path: Path) -> None:
    lines: list[str] = []
    lines.append("schema_version: 1")
    lines.append("monsters:")
    if not rows:
        lines.append("  []")
    else:
        for row in rows:
            escaped_name = row.name.replace('"', '\\"')
            lines.append(f"  - id: {row.monster_id}")
            lines.append(f"    name: \"{escaped_name}\"")
            lines.append(f"    level: {row.level}")
            lines.append(f"    max_hp: {row.max_hp}")
            include_scale = (
                row.scale is not None and row.scale > 0.0 and abs(row.scale - 1.0) > 0.0001
            )
            include_model = row.model_index is not None and row.model_index >= 0
            if include_scale or include_model:
                lines.append("    visual:")
                if include_scale:
                    lines.append(f"      scale: {row.scale:g}")
                if include_model:
                    lines.append(f"      model_index: {row.model_index}")
    out_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--runtime-config",
        default="rust/server/config/runtime.toml",
        help="Deprecated: ignored (spots are no longer generated)",
    )
    parser.add_argument(
        "--monster-set-base",
        default="cpp/MuServerCpp/Data/Monster/MonsterSetBase.txt",
        help="Deprecated: ignored (spots are no longer generated)",
    )
    parser.add_argument(
        "--monster-stats",
        default="cpp/MuServerCpp/Data/Monster/Monster.txt",
        help="Path to Monster.txt",
    )
    parser.add_argument(
        "--output-dir",
        default="rust/server/config/monsters",
        help="Deprecated: ignored (spots are no longer generated)",
    )
    parser.add_argument(
        "--monster-catalog-output",
        default="rust/config/monsters.yaml",
        help="Output YAML path for common monster catalog",
    )
    parser.add_argument(
        "--muclient-character-source",
        default="cpp/MuClient5.2/source/ZzzCharacter.cpp",
        help="Path to MuClient character source for scale extraction",
    )
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parents[3]
    monster_stats_path = (repo_root / args.monster_stats).resolve()
    muclient_character_path = (repo_root / args.muclient_character_source).resolve()
    monster_catalog_out = (repo_root / args.monster_catalog_output).resolve()
    monster_catalog_out.parent.mkdir(parents=True, exist_ok=True)

    monster_rows = parse_monster_txt(monster_stats_path)
    monster_visuals = parse_monster_visuals_from_muclient(muclient_character_path)

    enriched_rows: list[MonsterRow] = []
    for row in monster_rows:
        visual = monster_visuals.get(row.monster_id) or (None, None)
        enriched_rows.append(
            MonsterRow(
                monster_id=row.monster_id,
                name=row.name,
                level=row.level,
                max_hp=row.max_hp,
                scale=visual[0],
                model_index=visual[1],
            )
        )
    monster_rows = enriched_rows

    dump_monster_catalog_yaml(monster_rows, monster_catalog_out)
    print(
        f"[ok] {monster_catalog_out.relative_to(repo_root)} monsters={len(monster_rows)} visuals={len(monster_visuals)}"
    )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
