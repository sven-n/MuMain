"""Encode painted owned materials; copy frozen shared dependencies byte for byte."""
import json
from pathlib import Path
import shutil
import subprocess
import sys
sys.dont_write_bytecode = True
import numpy as np
from PIL import Image
from config import CONTAINERS, FROZEN, ROOT, REPOSITORY
sys.path.insert(0,str(ROOT.parent/'TavernDetails01'))
from package_textures import alpha_statistics, bleed_transparent_rgb

SIZES = {'grass_01':(512,512),'horse_drawn_02':(512,512),'horse_drawn_03':(512,512)}
RAW_FILES = {'grass_01':'grass_01-final.png','horse_drawn_02':'horse_drawn_02-v2.png','horse_drawn_03':'horse_drawn_03-v2.png'}


def package(material,container):
    folder=ROOT/'textures/final'
    if container in FROZEN:
        for filename in (material,container):
            shutil.copy2(ROOT/'textures/original'/filename,folder/filename)
        return
    name=Path(material).stem
    image=Image.open(ROOT/'textures/raw'/RAW_FILES[name]).convert('RGBA').resize(SIZES[name],Image.Resampling.LANCZOS)
    original=Image.open(ROOT/'textures/original'/material).convert('RGBA')
    if material.endswith('.tga'):
        assert image.getchannel('A').getextrema()==(0,255)
        image=bleed_transparent_rgb(image)
        image.save(folder/material,compression=None,origin=0)
    else:
        image=image.convert('RGB')
        image.save(folder/material,quality=96,subsampling=0)
    image.save(folder/(name+'.png'))
    subprocess.run([sys.executable,str(REPOSITORY/'tools/mu_texture.py'),'wrap',str(folder/material),'--out',str(folder/container)],cwd=REPOSITORY,check=True)
    report=dict(original=alpha_statistics(original),final=alpha_statistics(image.convert('RGBA')),
                rgb_mean_original=np.asarray(original)[:,:,:3].mean(axis=(0,1)).tolist(),
                rgb_mean_final=np.asarray(image.convert('RGBA'))[:,:,:3].mean(axis=(0,1)).tolist(),
                processing='Imagegen artwork resized and encoded; generated alpha retained. RGB padded under cutoff64.')
    (folder/(name+'-audit.json')).write_text(json.dumps(report,indent=2)+'\n')


def main():
    (ROOT/'textures/final').mkdir(parents=True,exist_ok=True)
    for material,container in CONTAINERS.items():
        package(material,container)
    command=[sys.executable,str(REPOSITORY/'tools/mu_texture.py'),'check',*[str(ROOT/'textures/final'/n) for n in CONTAINERS.values()]]
    result=subprocess.run(command,cwd=REPOSITORY,capture_output=True,text=True,check=True)
    (ROOT/'textures/final/texture-check.txt').write_text(result.stdout)


if __name__=='__main__':
    main()
