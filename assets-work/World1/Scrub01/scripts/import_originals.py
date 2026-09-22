"""Official BMD imports into this batch only; preserved files are never overwritten."""
from pathlib import Path
import sys
sys.dont_write_bytecode = True
sys.path.insert(0, str(Path(__file__).resolve().parent))
from config import CONVERTER, PROPS, REPOSITORY, ROOT
sys.path.insert(0, str(REPOSITORY / 'tools/blender'))
import mu_bmd_import
for name in PROPS:
    folder = ROOT / name / 'original'
    if (folder / 'source.blend').exists():
        continue
    sys.argv = ['blender', '--', '--bmd', str(folder / f'{name}.bmd'),
                '--textures', str(ROOT / 'original'), '--bmdconv', str(CONVERTER),
                '--out', str(folder / 'source.blend')]
    mu_bmd_import.main()
