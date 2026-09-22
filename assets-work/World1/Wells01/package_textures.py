"""Wrap two exclusive well/pottery paintings; leave cart-shared materials frozen."""
from pathlib import Path
import shutil
import subprocess
import sys
from PIL import Image
HERE=Path(__file__).resolve().parent
ROOT=HERE.parents[2]
PAINTS=('well','jar_01','tub')
SIZE=(512,512)
FINAL=HERE/'textures/final'
FINAL.mkdir(parents=True,exist_ok=True)
for path in (HERE/'textures/original').iterdir():
    if path.suffix.lower() in ('.jpg','.tga','.ozj','.ozt'):
        shutil.copy2(path,FINAL/path.name)
for name in PAINTS:
    image=Image.open(HERE/'textures/raw'/f'{name}.png').convert('RGB').resize((512,256) if name == 'tub' else SIZE,Image.Resampling.LANCZOS)
    image.save(FINAL/f'{name}.png')
    image.save(FINAL/f'{name}.jpg',quality=96,subsampling=0)
    subprocess.run([sys.executable,str(ROOT/'tools/mu_texture.py'),'wrap',str(FINAL/f'{name}.jpg'),'--out',str(FINAL/f'{name}.OZJ')],cwd=ROOT,check=True)
containers=sorted(str(p) for p in FINAL.iterdir() if p.suffix.upper() in ('.OZJ','.OZT'))
result=subprocess.check_output([sys.executable,str(ROOT/'tools/mu_texture.py'),'check',*containers],cwd=ROOT,text=True)
(FINAL/'loader-check.txt').write_text(result)
