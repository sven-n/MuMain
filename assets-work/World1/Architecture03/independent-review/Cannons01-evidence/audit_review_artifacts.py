"""Tie viewed producer artifacts and installed files to the immutable reviewed commit."""
import hashlib
import json
from pathlib import Path
import subprocess
import sys

sys.dont_write_bytecode = True
REPOSITORY = Path.cwd()
PRODUCER = REPOSITORY.parent / 'MuMain-lorencia-cannons'
OUTPUT = Path(__file__).resolve().parent
REVISION = 'ab832f5de4cea0dcaee144dce3df77870e565815'
BATCH = 'assets-work/World1/Cannons01'


def blob(path):
    return subprocess.check_output(['git', 'show', REVISION + ':' + path], cwd=REPOSITORY)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def main():
    manifest = json.loads(blob(BATCH + '/review/final-artifact-hashes.json'))
    atlas = blob('src/bin/Data/Object1/horse_drawn_01.OZJ')
    images, reviewed_reports = {}, {}
    paths = subprocess.check_output(['git', 'ls-tree', '-r', '--name-only', REVISION, BATCH], cwd=REPOSITORY, text=True).splitlines()
    for path in paths:
        if '/review/' in path and Path(path).suffix.lower() in ('.png', '.jpg'):
            data = blob(path)
            assert data == (PRODUCER / path).read_bytes(), path
            images[path] = digest(data)
    for name, record in manifest.items():
        assert digest(blob(f'{BATCH}/{name}/exports/{name}.bmd')) == record['export_BMD_sha256']
        assert digest(atlas) == record['atlas_sha256']
        for filename, expected in record['review_images'].items():
            assert images[f'{BATCH}/{name}/review/{filename}'] == expected
        reports = {}
        for filename in ('summary.json', 'geometry-UV-contacts.json', 'local-motion.json', 'all-key-posed-bounds.json'):
            data = json.loads(blob(f'{BATCH}/{name}/validation/{filename}'))
            assert data['status'].startswith('PASS')
            reports[filename] = data if filename != 'all-key-posed-bounds.json' else {
                key: value for key, value in data.items() if key not in ('original_bounds', 'new_bounds')}
        reviewed_reports[name] = reports
    protected = json.loads(blob(BATCH + '/protected-baseline.json'))
    for path, expected in protected.items():
        assert digest(blob(path)) == expected, path
    report = dict(status='PASS', pinned_commit=REVISION, protected_game_file_count=len(protected),
                  consumer_count=len(manifest), pinned_review_image_count=len(images), image_sha256=images,
                  producer_reports_cross_checked=reviewed_reports)
    (OUTPUT / 'pinned-artifact-audit.json').write_text(json.dumps(report, indent=2) + '\n')
    print('PASS:', len(manifest), 'consumers;', len(images), 'pinned review images;', len(protected), 'protected game files')


if __name__ == '__main__':
    main()
