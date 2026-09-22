"""Install original BMD bindings only and record the narrow compatibility correction."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
from prepare import ASSETS,BATCHES,HERE,ROOT


def main():
    assert subprocess.check_output(['git','branch','--show-current'],cwd=ROOT,text=True).strip()=='codex/lorencia-normal-bindings'
    files={'src/bin/Data/Object1/'+n+'.bmd':HERE/n/'exports'/f'{n}.bmd' for n in ASSETS}
    protected={}
    for kind in ('Object1','World1'):
        for p in (ROOT/'src/bin/Data'/kind).iterdir():
            if not p.is_file() or str(p.relative_to(ROOT)) in files:continue
            key=str(p.relative_to(ROOT));expected=subprocess.check_output(['git','show','HEAD:'+key],cwd=ROOT)
            assert p.read_bytes()==expected,key
            protected[key]=hashlib.sha256(expected).hexdigest()
    for game,export in files.items():
        assert export.read_bytes()==subprocess.check_output(['git','show','ac0f6dd8:'+game],cwd=ROOT)
        shutil.copy2(export,ROOT/game)
    report=dict(result='PASS',restored_original_bmds={k:str(v.relative_to(ROOT)) for k,v in files.items()},
                protected_count=len(protected),protected=protected,client_verified=False,runtime_modified=False)
    (HERE/'installed-files.json').write_text(json.dumps(report,indent=2)+'\n')
    notes='''# NormalBindings01 — preserve original raw normal ownership

2026-09-22. ASTRA coordinator, branch `codex/lorencia-normal-bindings`, isolated worktree `/Users/webproduktion3/Documents/claude-test-mumain/MuMain-lorencia-normal-bindings`.

This follow-up retains the exact original BMD bytes for Grass02, Tree12 and Tree13 while keeping their accepted new material paintings. No geometry remodeling was intended for these models. Counts remain120/393/393; bones, hierarchy and one-key/31-key/31-key actions are byte-identical to the original. Full original→final and superseded roundtrip→final geometry comparisons are EQUIVALENT. Complete original/final reference and action SMD bytes match exactly. All six model/action validations pass on both original and final sets.

The extended raw-normal audit found a newly shared Grass02 normal using another bone with a different rotation (direction-component delta0.179355), absent from the original. Tree12/13 have pre-existing cross-bone normal sharing with nonzero rotation differences, so their mathematical sharing test alone cannot establish intended lighting. Retaining original BMD bytes guarantees their exact original runtime normal-node behavior instead of introducing importer/exporter reinterpretation. Those legacy tree bindings are preserved, not claimed repaired. Geometry/UV equivalence alone does not prove normal-node equivalence: the current bmdconv SMD expansion applies vertex-node rotation when expanding normals.

The earlier official import/export evidence, imagegen prompts, editable artwork and original roundtrip logs remain in Groundcover01/Grass02 and Flowering01/Tree12,Tree13. Their packed authored sources are copied here with REF_ORIGINAL. This narrow follow-up intentionally chooses unchanged original game BMDs after inspecting the roundtrip; it does not patch the converter or engine. Complete untouched originals, every dependency container, accepted final textures, previous roundtrip BMD, fresh original/final extracted SMDs, validations/info/comparison logs and reimported blends remain here.

All authored and final reimport vertices match bidirectionally at their named bones; packed source/reference and UV audits pass. Matching original/final camera renders, reverse views, wireframes and reduced previews are retained per asset. These diffuse Blender studies do not emulate the raw-normal-node difference in the runtime; raw original-byte identity is the compatibility evidence. No image is client evidence.

Three game paths only are restored: src/bin/Data/Object1/Grass02.bmd, Tree12.bmd and Tree13.bmd. All321 other World1/Object1 files in this isolated checkout, including every texture, are unchanged. World1 terrain, alpha strips, placement/height/walk data, lighting, runtime, client, engine/CMake and UI are untouched.

Reproduce here with bundled Python prepare.py; Blender audit_sources.py and render_review.py; bundled Python assemble_review.py and install_and_notes.py. Raw painting generation/reproduction remains in the prior accepted production batches. Pending independent review, integration and actual client observation. No client verification occurred.
'''
    (HERE/'notes.md').write_text(notes)
    for name in ASSETS:
        folder=HERE/name;s=json.loads((folder/'validation/blender.json').read_text())
        text=f'# {name} raw binding preservation\n\n'+notes.split('\n',2)[2]+f'\nExact before/after engine bind bounds: `{s["bounds_before"]}`. Count{s["triangles"]}.\n\nDependencies: '+(folder/'dependencies.json').read_text()
        (folder/'notes.md').write_text(text)


if __name__=='__main__':main()
