"""Package six painted stone atlases and retain every frozen fountain container."""
import importlib.util
import json
from pathlib import Path
import shutil
import sys
sys.dont_write_bytecode=True
ROOT=Path(__file__).resolve().parent
spec=importlib.util.spec_from_file_location('paint_package',ROOT.parent/'SignsBanners01/package_textures.py')
package=importlib.util.module_from_spec(spec); spec.loader.exec_module(package)
package.ROOT=ROOT
SPECS={'stone_statue02':('StoneStatue01','.jpg',(256,512)),
       'stone_statue01':('StoneStatue02','.jpg',(512,512)),
       'angel_stone_statue':('StoneStatue03','.jpg',(512,512)),
       'tombstone_big':('SteelStatue01','.jpg',(512,512)),
       'grave_02':('Tomb01','.jpg',(512,512)), 'grave_01':('Tomb02','.jpg',(512,512))}
if __name__=='__main__':
    records={name:package.package(name,*record) for name,record in SPECS.items()}
    for original in ROOT.glob('*/original'):
        for path in original.iterdir():
            if path.suffix not in ('.OZJ','.OZT') or path.stem in SPECS: continue
            shutil.copy2(path,original.parent/'exports')
            suffix='.jpg' if path.suffix=='.OZJ' else '.tga'
            shutil.copy2(original/(path.stem+suffix),original.parent/'textures')
    (ROOT/'paintings/packaging.json').write_text(json.dumps(records,indent=2)+'\n')
