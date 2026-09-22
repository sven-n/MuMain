"""Package the fixed-layout imagegen painting; retain all frozen texture bytes."""
from pathlib import Path
import json
import shutil
import subprocess
import sys
sys.dont_write_bytecode=True
from PIL import Image
from config import ROOT, REPOSITORY, CONTAINERS
final=ROOT/'textures/final';final.mkdir(exist_ok=True)
for filename,container in CONTAINERS.items():
    if filename=='horse_drawn_01.jpg':continue
    for name in (filename,container):shutil.copy2(ROOT/'textures/original'/name,final/name)
painting=Image.open(ROOT/'textures/raw/horse_drawn_01-gutters.png').convert('RGB')
painting.resize((1024,1024),Image.Resampling.LANCZOS).save(final/'horse_drawn_01.png')
Image.open(final/'horse_drawn_01.png').save(final/'horse_drawn_01.jpg',quality=96,subsampling=0)
result=subprocess.run([sys.executable,str(REPOSITORY/'tools/mu_texture.py'),'wrap',str(final/'horse_drawn_01.jpg'),'--out',str(final/'horse_drawn_01.OZJ')],cwd=REPOSITORY,capture_output=True,text=True,check=True)
(ROOT/'textures/packaging.log').write_text(result.stdout+result.stderr)
