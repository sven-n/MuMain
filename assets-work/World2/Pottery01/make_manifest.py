"""Hash the stable candidate artifacts after all export and rendering stages."""
from pathlib import Path
import hashlib
import json
ROOT = Path(__file__).resolve().parent


def main():
    files = []
    counts = {}
    for name in ('Object28', 'Object29'):
        folder = ROOT / name
        counts[name] = json.loads((folder / 'validation/source.json').read_text())['triangles']
        files.extend([folder / 'source.blend', folder / 'exports' / (name + '.bmd')])
        files.extend(folder.joinpath('review').glob('*.png'))
        files.extend(folder.joinpath('validation').glob('*.json'))
    files.extend(ROOT.joinpath('review-assemblies').glob('*/*.png'))
    files.extend([ROOT / 'exported-variant-proof.json', ROOT / 'shared-whole-pot-proof.json',
                  ROOT / 'shared-broken-pot-proof.json', ROOT / 'Object30/baseline/Object30.bmd'])
    hashes = {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(files)}
    (ROOT / 'candidate-manifest.json').write_text(json.dumps(dict(status='PENDING_INDEPENDENT_FINAL_REVIEW',
        triangles=counts, retained='Object30 baseline, subject to explicit independent final retention', sha256=hashes), indent=2))


if __name__ == '__main__':
    main()
