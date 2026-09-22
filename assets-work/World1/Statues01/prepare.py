"""Archive/import the owned monuments and graves plus frozen fountain compatibility consumer."""
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
sys.dont_write_bytecode = True
ROOT = Path(__file__).resolve().parent
NAMES = ('StoneStatue01', 'StoneStatue02', 'StoneStatue03', 'SteelStatue01', 'Tomb01', 'Tomb02', 'Waterspout01')
spec = importlib.util.spec_from_file_location('archive_tools', ROOT.parent / 'StoneWalls01/prepare_originals.py')
archive = importlib.util.module_from_spec(spec)
spec.loader.exec_module(archive)
archive.ROOT = ROOT

if __name__ == '__main__':
    inventory = json.loads((ROOT.parent / 'coordination/dependency-map.json').read_text())['models']
    (ROOT / 'baseline-commit.txt').write_text(subprocess.check_output(['git', 'rev-parse', 'HEAD'], text=True))
    for name in NAMES:
        record = inventory[name]
        record['info'] = subprocess.check_output([archive.CONVERTER, 'info', str(archive.REPOSITORY / record['path'])], text=True)
        archive.preserve(name, record)
        (ROOT / name / 'dependencies.json').write_text(json.dumps(record['textures'], indent=2) + '\n')
