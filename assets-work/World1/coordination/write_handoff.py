"""Write the coordinator handoff and exact game-file manifest from accepted evidence."""

import json
from pathlib import Path
import subprocess

HERE = Path(__file__).resolve().parent
ROOT = HERE.parents[2]
PILOT = ['Beer01', 'Candle01', 'TreasureChest01', 'Tomb03']


def read(name):
    return json.loads((HERE / name).read_text())


def main():
    ledger = read('integration-ledger.json')
    validation = read('combined-validation.json')
    models = read('dependency-map.json')['models']
    covered = set(PILOT)
    rows = []
    for batch in ledger:
        fresh = [name for name in batch['models'] if name not in covered]
        covered.update(batch['models'])
        names = ', '.join(fresh) if fresh else 'Binding-preservation correction; no additional models'
        notes = '../' + batch.get('notes_path', batch['name'] + '/notes.md')
        sources = ', '.join('`' + c + '`' for c in batch['source_commits'])
        commits = ', '.join('`' + c + '`' for c in batch['integration_commits'])
        rows.append(f'| [{batch["name"]}]({notes}) | {batch["owner"]} | {names} | {sources} | {commits} |')
    pending = [n for n, m in models.items() if not m['scope_exclusion'] and n not in covered]
    excluded = [n for n, m in models.items() if m['scope_exclusion']]
    files = validation['changed_game_files']
    bmd_count = sum(p.lower().endswith('.bmd') for p in files)
    normal = read('normal-binding-audit.json')
    vertices = read('authored-vertex-audit.json')
    head = subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip()
    accepted_count = len(covered)
    text = f'''# Lorencia rebuild consolidated handoff — 2026-09-22

**{accepted_count}/106 in-scope static assets accepted offline**, representing the actual World1 inventory; the complete static inventory has 2,786 placements. Four completed pilot models are preserved, and later batches cover the remaining models. The 17 completed terrain paintings remain unchanged. Client acceptance is pending for this entire pass.

Integration branch: **art/lorencia-rebuild**. Worktree: `{ROOT}`. Reviewed baseline: `ac0f6dd8` (contains `2e2ed427`, `b232470c`, `8d22a912` and completed static pilot). Art/review revision when this handoff was generated: `{head}`. The final documentation commit may follow it. No push or merge to main.

## Accepted batches and owners

Preserved pilot: Beer01 (`8d22a912`, 784 triangles and 512² plate2 atlas), Candle01 (`c7a9765a`), TreasureChest01 (`993304be`) and Tomb03 (`100071aa`). Existing tavern work was inspected, independently reviewed and integrated from its original branch. Production workers used separate worktrees; coordinator integrated accepted commits sequentially.

The model column lists newly covered models once. Batch notes also list all readonly shared-texture consumers and their pinned original revisions. The [asset board](asset-board.md) records exact ownership, worktrees, BMD paths, complete texture dependencies and World1 placements for every candidate.

| Batch | Owner | Newly covered models | Source commits | Integration commits |
|---|---|---|---|---|
'''
    text += '\n'.join(rows)
    text += f'''

## Exact game changes and protected data

**{len(files)} game files differ from the reviewed pilot: {bmd_count} BMDs and {len(files)-bmd_count} texture containers**, all under `src/bin/Data/Object1/`. See the [complete path list](changed-game-files.txt) and [path/export/SHA-256 manifest](changed-game-files.md). No game files were renamed or added outside the authorized directory.

All other **{validation['protected_file_count']}** files in the 324-file World1/Object1 baseline remain byte-identical, including all World1 terrain paintings, placement, height, walk data, baked lighting, TerrainLight and alpha strips. Beer01/plate2 and the other pilot props remain unchanged. The bridge shadow strip is also unchanged. Nine excluded fauna/hidden-marker models remain untouched: {', '.join(excluded)}.

The [dependency map](dependency-map.json) resolves all 115 BMDs and 105 original texture containers, including exact JPG/OZJ versus TGA/OZT distinctions. Shared-material changes were reviewed on every actual consumer. Latest combined checks report no missing textures, no ownership overlap, preserved mesh-material order and bone/action metadata, matching source/export hashes and exact untouched original archives.

## Evidence and practical limits

- [Combined integration validation](combined-validation.json): {validation['result']}; all 115 models resolve.
- [Independent final integration review](../Statues01/independent-review/FinalIntegration.md) records the complete source, scope, converter and combined-gallery acceptance.
- [Authored vertex audit](authored-vertex-audit.json): {vertices['result']} for {len(vertices['models'])} remodeled models. Every authored and final-reimport vertex is matched in both directions only within its intended named bone; measured errors are retained.
- [Raw normal-node audit](normal-binding-audit.json): {normal['result']} for {len(normal['models'])} current changed/retained BMDs, across bind and every animation key. See [readable report](normal-binding-audit.txt).
- [Final combined gallery](final-review.md): six labeled sheets of all 106 actual integrated BMD/material combinations. Each `final-inspection/<model>/provenance.json` records exact game-file hashes; the gallery is an offline diffuse preview, not engine shading or client evidence.
- Per-asset sources, `REF_ORIGINAL`, untouched originals, full converter comparisons, before/after bind bounds, triangle counts, texture dimensions, UV/alpha checks, rig/action proofs, matching-camera renders, wireframes and reduced-scale previews remain in the batch directories linked above. Image-generation prompts, generated masters, editable paintings and reproduction scripts are retained.
- Original inventory validation/imports and all 105 original containers remain under `baseline-validation/`, `inspection/` and `texture-baseline/`. `inventory-sheet-01.jpg` through `inventory-sheet-06.jpg` show the original integration baseline.

Remodeled geometry correctly reports **DIFFERENT** in full-model comparisons; exact skeleton/actions are checked independently. Full-model EQUIVALENT alone does not prove UV or raw normal-node equivalence. The converter merges some shared normals across bones, so raw BMD checks were added. Grass02, Tree12 and Tree13 keep **exact original BMD bytes** with their new paintings; this preserves original behavior, including legacy Tree12/13 normal sharing. No engine fix is claimed. Material-only assets intentionally retain geometry.

Modular connections, openings, footprints and original anchor vertices are preserved. Bone-local animation, hierarchy, mesh-slot effects and original placements are retained. Batch-specific harmless floating-point export differences and original collapsed UV faces are explicitly documented instead of being hidden by an equivalence claim. Source packaging uses the official Blender importer/exporter and supported converter SMD/manifest workflow.

## Remaining work

Production still pending: {', '.join(pending) if pending else '**none within the 106-model static scope**'}. No unresolved validator rejection remains in accepted batches. Excluded models are out of scope, not unfinished static production.

**Actually verified in client: none.** The user authorized offline production after repeated client crashes. This task did not launch the client or install into shared runtime Data. In a stable serial client session, verify loading, lighting/filtering/alpha, animated poses, mesh-index additive/scroll effects, collision/interaction alignment, modular seams and gameplay readability, then capture genuine matching 1920×1080 before/after views. Blender additive/scroll/fade previews remain approximations.

No engine/CMake, UI, character, monster, equipment, other-map, placement/collision or terrain-data edits. Other task checkouts and their uncommitted work were preserved. Shared handoffs and the board were edited only by the coordinator.

## Reproduce

Run from the integration worktree explicitly. Use the bundled Python with Pillow for texture checks and image assembly, and Blender 5.2.2 for Blender scripts. `validate_integration.py`, `audit_normal_bindings.py`, Blender `audit_authored_vertices.py`, Blender `inspect_final.py`, `assemble_final.py`, `update_board.py` and `write_handoff.py` are the coordinator entry points. The cached gallery rerenders only models whose BMD or exact texture-container hashes changed. Per-batch scripts retain the production reproduction commands and pinned original revisions.
'''
    (HERE / 'notes.md').write_text(text)
    (HERE / 'changed-game-files.txt').write_text('\n'.join(sorted(files)) + '\n')
    manifest = ['# Exact game files changed', '',
                'Compared with reviewed pilot ac0f6dd8. All paths are relative to the integration worktree. Original-retained Grass02/Tree12/Tree13 BMDs are intentionally absent; their painted textures are present.', '',
                '| Game file | Batch | Reviewed export | SHA-256 |', '|---|---|---|---|']
    for game, record in sorted(files.items()):
        manifest.append(f'| `{game}` | {record["batch"]} | `{record["export"]}` | `{record["sha256"]}` |')
    (HERE / 'changed-game-files.md').write_text('\n'.join(manifest) + '\n')
    print('Handoff:', accepted_count, 'accepted;', len(files), 'changed game files; pending:', pending)


if __name__ == '__main__':
    main()
