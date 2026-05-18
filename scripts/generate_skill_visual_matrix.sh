#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
INPUT_FILE="${1:-$ROOT_DIR/client/src/bin/character_viewer/skills.rs}"
OUTPUT_FILE="${2:-$ROOT_DIR/../docs/skill_visual_migration_matrix.md}"

if [[ ! -f "$INPUT_FILE" ]]; then
  echo "Input file not found: $INPUT_FILE" >&2
  exit 1
fi

tmp_entries="$(mktemp)"
trap 'rm -f "$tmp_entries"' EXIT

awk '
function map_class(token) {
    if (token == "DK") return "DarkKnight";
    if (token == "DW") return "DarkWizard";
    if (token == "ELF") return "FairyElf";
    if (token == "MG") return "MagicGladiator";
    if (token == "DL") return "DarkLord";
    if (token == "SUMMONER") return "Summoner";
    if (token == "RF") return "RageFighter";
    return token;
}

{
    line = $0;

    if (match(line, /^const ([A-Z_]+)_SKILLS:[[:space:]]*&\[/, m)) {
        current_class = map_class(m[1]);
        next;
    }

    if (index(line, "skill(") > 0) {
        in_skill = 1;
        id = "";
        name = "";
        vfx = "";
        next;
    }

    if (!in_skill) {
        next;
    }

    trimmed = line;
    gsub(/^[[:space:]]+|[[:space:]]+$/, "", trimmed);

    if (id == "" && match(trimmed, /^([0-9]+),/, m)) {
        id = m[1];
    }
    if (name == "" && match(trimmed, /^"([^"]+)",/, m)) {
        name = m[1];
    }
    if (vfx == "" && match(trimmed, /SkillVfxProfile::([A-Za-z0-9_]+)/, m)) {
        vfx = m[1];
    }

    if (trimmed ~ /^\),$/) {
        if (id != "" && current_class != "") {
            print id "|" current_class "|" name "|" vfx;
        }
        in_skill = 0;
    }
}
' "$INPUT_FILE" | sort -t'|' -k1,1n -k2,2 > "$tmp_entries"

mkdir -p "$(dirname "$OUTPUT_FILE")"

{
  echo "# Skill Visual Migration Matrix"
  echo
  echo "_Generated from \`rust/client/src/bin/character_viewer/skills.rs\` by \`rust/scripts/generate_skill_visual_matrix.sh\`._"
  echo
  echo "| Skill ID | Name(s) | Class(es) | VFX Profile(s) | C++ Source | Legacy Life (frames) | Viewer | Runtime | Temporal Parity | Notes |"
  echo "|---|---|---|---|---|---|---|---|---|---|"

  awk -F'|' '
  function append_unique(current, value,    n, items, i) {
      if (value == "") return current;
      if (current == "") return value;
      n = split(current, items, ", ");
      for (i = 1; i <= n; i++) {
          if (items[i] == value) return current;
      }
      return current ", " value;
  }

  function emit_row(    viewer, runtime, temporal, notes, cxx_source, legacy_frames) {
      if (skill_id == "") return;
      viewer = "pending";
      runtime = "pending";
      temporal = "pending";
      notes = "";
      cxx_source = "TBD";
      legacy_frames = "TBD";

      if (skill_id == "43") {
          viewer = "done";
          runtime = "done";
          temporal = "validated";
          notes = "PoC death stab";
      } else if (skill_id == "9") {
          viewer = "done";
          runtime = "pending";
          temporal = "in_progress";
          notes = "PoC evil spirit viewer";
      }

      printf "| %s | %s | %s | %s | %s | %s | %s | %s | %s | %s |\n",
          skill_id,
          names,
          classes,
          vfx_profiles,
          cxx_source,
          legacy_frames,
          viewer,
          runtime,
          temporal,
          notes;
  }

  {
      if ($1 != skill_id) {
          emit_row();
          skill_id = $1;
          classes = "";
          names = "";
          vfx_profiles = "";
      }

      classes = append_unique(classes, $2);
      names = append_unique(names, $3);
      vfx_profiles = append_unique(vfx_profiles, $4);
  }

  END {
      emit_row();
  }
  ' "$tmp_entries"

  echo
  echo "Total unique skill IDs: $(cut -d'|' -f1 "$tmp_entries" | uniq | wc -l | tr -d '[:space:]')"
} > "$OUTPUT_FILE"

echo "Wrote: $OUTPUT_FILE"
