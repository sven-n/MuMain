"""Coordinator-owned rendering of the asset register; workers must not edit this file."""

import json
from pathlib import Path

HERE = Path(__file__).resolve().parent
WORKTREE_ROOT = '/Users/webproduktion3/Documents/claude-test-mumain/'
PREVIOUS = {
    'Beer01': ('tavern still life: bottle, mug, bowls, grapes, vine', '8d22a912', 'Beer01/notes.md'),
    'Candle01': ('three-candle bronze stand with animated flames', 'c7a9765a', 'Candle01/notes.md'),
    'TreasureChest01': ('arched timber chest', '993304be', 'TreasureChest01/notes.md'),
    'Tomb03': ('upright carved grave marker', '100071aa', 'Tomb03/notes.md'),
}
TAVERN = {'Furniture03': 'four-legged rectangular tavern table',
          'Furniture04': 'half-round pedestal table', 'Furniture05': 'modular tavern counter'}
ASSIGNMENTS = json.loads((HERE / 'production-batches.json').read_text())
LEDGER = json.loads((HERE / 'integration-ledger.json').read_text())


def row(name, model):
    status, owner, branch, worktree, evidence = 'unclaimed', '—', '—', '—', 'Visually identified in baseline inspection; awaiting production'
    identity = json.loads((HERE / 'identities.json').read_text()).get(name, 'Scope marker / animated fauna')
    if name in PREVIOUS:
        identity, commit, notes = PREVIOUS[name]
        status, owner, branch = 'accepted', 'ASTRA previous pilot', 'art/world1-pilot'
        worktree = '/Users/webproduktion3/.codex/worktrees/world1-static-batch/MuMain'
        evidence = f'`{commit}`; [offline validation](../{notes}); client pending'
    if name in TAVERN:
        identity, status, owner, branch = TAVERN[name], 'accepted', 'ASTRA tavern artist / reviewer', 'art/lorencia-tavern-props'
        worktree = WORKTREE_ROOT + 'MuMain-tavern-props'
        evidence = '`9a10643c`, `527e82d8`; independent [review](../reviews/tavern-integration/notes.md) accepted offline; client pending'
    for agent, claim in ASSIGNMENTS.items():
        if name in claim['models']:
            status, owner, branch = 'in progress', agent, claim['branch']
            worktree, evidence = WORKTREE_ROOT + claim['worktree'], 'Baseline geometry visually identified; production and engine checks in progress'
    accepted = [batch for batch in LEDGER if name in batch['models']]
    if accepted:
        status = 'accepted'
        if owner == '—':
            owner = accepted[0]['owner']
        evidence = '<br>'.join(
            '[{0}](../{1}): {2}'.format(
                batch['name'], batch.get('notes_path', batch['name'] + '/notes.md'),
                ', '.join('`' + commit + '`' for commit in batch['integration_commits']))
            for batch in accepted)
        evidence += '; offline accepted; client pending'
    if model['scope_exclusion']:
        status, identity, evidence = 'blocked', model['scope_exclusion'], 'Excluded from this static-art scope; preserve unchanged'
    placements = model['placements']
    first = ', '.join(f'{v:.2f}' for v in placements[0]['tile']) if placements else 'none'
    dependencies = '<br>'.join(f'`{path}` ({texture})' for texture, paths in model['textures'].items() for path in paths)
    return f'| {name} — {identity} | {model["type"]}; {len(placements)}; first ({first}) | `{model["path"]}` | {dependencies} | {owner}; `{branch}`; `{worktree}` | {status} | {evidence} |'


def main():
    data = json.loads((HERE / 'dependency-map.json').read_text())
    text = '''# Lorencia rebuild asset board — ASTRA coordinator — 2026-09-22

Integration: `art/lorencia-rebuild` at `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-rebuild`, based on `ac0f6dd8` (contains reviewed pilot commits `2e2ed427`, `b232470c`, `8d22a912` and completed static batch). Only coordinator edits this board/shared handoff. No push or main merge. All acceptance here is **offline only**; no assets verified in client by this task.

## Ownership and dependency policy

[Dependency map](dependency-map.json) records all 115 actual BMD info reports, all 105 resolved texture dependencies, all exact World1 placements (position/rotation/scale), and source loader references. No missing textures. There are 33 connected texture groups; a large 56-model component connects architecture, rocks, carts and some trees. Shared textures in this component stay unchanged unless the entire consumer set is assigned to one texture owner. Geometry ownership may be split only with those textures frozen. No renaming to escape sharing.

The table records primary BMD ownership and every subsequent shared-material compatibility review. Frozen textures in an earlier batch may be changed later only by their assigned texture owner after reviewing all consumers. Completed terrain, TerrainLight, all terrain alpha strips, Beer01/plate2, Candle01, chest and Tomb03 remain protected. UI and runtime are excluded. Grass02, Tree12 and Tree13 retain their original BMD bytes after the normal-binding review; their accepted paintings remain installed.

All source Data paths are relative to the integration worktree. Full placement arrays and mesh-slot texture order are in the dependency map. All 106 in-scope identities were visually inspected from imported BMDs; see `identities.json` and `inventory-sheet-01.jpg` through `inventory-sheet-06.jpg`. Sheets show the integration baseline (including completed pilot assets), not client evidence. Workers inspect precise geometry/materials and source controls before production.

## Candidates

| Candidate and actual identity | World1 type; count; first tile | BMD path | Complete texture dependencies | Agent; branch; worktree | Status | Commits, validation, blockers |
|---|---|---|---|---|---|---|
'''
    text += '\n'.join(row(name, model) for name, model in sorted(data['models'].items(), key=lambda x: (-len(x[1]['placements']), x[0])))
    text += '\n## Batch texture ownership\n\nEvery listed BMD is exclusively owned by the named batch. Only its owned texture paths may change; all other dependencies are frozen.\n\n| Owner / branch | Owned BMDs | Owned textures | Frozen textures |\n|---|---|---|---|\n'
    for owner, batch in ASSIGNMENTS.items():
        names = ', '.join(batch['models'])
        if batch.get('compatibility_models'):
            names += '; read-only BMD compatibility: ' + ', '.join(batch['compatibility_models'])
        owned = '<br>'.join('`' + value + '`' for value in batch.get('owned_textures', [])) or 'None'
        frozen = '<br>'.join('`' + value + '`' for value in batch.get('frozen_textures', [])) or 'None'
        text += f"| {owner} / `{batch['branch']}` | {names} | {owned} | {frozen} |\n"
    (HERE / 'asset-board.md').write_text(text + '\n')


if __name__ == '__main__':
    main()
