"""Assemble review sheets and install only the explicitly owned source-game files."""
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import sys
sys.dont_write_bytecode = True
from PIL import Image, ImageDraw
from config import ASSETS, BASELINE, CONTAINERS, IDENTITIES, MATERIALS, OWNED_TEXTURES, REPOSITORY, ROOT
BACKGROUND = (29,33,34)
def panel(path,size):
    image=Image.open(path).convert('RGBA')
    image.thumbnail(size,Image.Resampling.LANCZOS)
    canvas=Image.new('RGB',size,BACKGROUND)
    canvas.paste(image,((size[0]-image.width)//2,(size[1]-image.height)//2),image)
    return canvas


def review_sheet(name):
    folder=ROOT/name/'review'
    sheet=Image.new('RGB',(1440,1120),BACKGROUND)
    draw=ImageDraw.Draw(sheet)
    draw.text((20,12),f'{name} | {IDENTITIES[name]} | OFFLINE - client review pending',fill='white')
    for index,stage in enumerate(('before','after')):
        x=index*720
        draw.text((x+20,38),stage.upper()+' - matching camera',fill='white')
        sheet.paste(panel(folder/f'{stage}.png',(720,460)),(x,58))
        sheet.paste(panel(folder/f'terrain-{stage}.png',(720,420)),(x,525))
        thumb=panel(folder/f'terrain-{stage}.png',(240,160))
        thumb.save(folder/f'readability-{stage}.png')
        sheet.paste(thumb,(x+30,950))
    draw.text((1000,1000),'Staging proxy: figure 190 / terrain repeat 100',fill='white')
    sheet.save(folder/'comparison.jpg',quality=94)



def original_hashes():
    paths = [ROOT/n/'original'/f'{n}.bmd' for n in ASSETS]
    paths += [ROOT/'textures/original'/n for n in CONTAINERS.values()]
    record = {}
    for path in paths:
        original = subprocess.check_output(['git','show',BASELINE+':src/bin/Data/Object1/'+path.name],cwd=REPOSITORY)
        assert path.read_bytes() == original, path.name
        record[str(path.relative_to(ROOT))] = hashlib.sha256(original).hexdigest()
    (ROOT/'original-hashes.json').write_text(json.dumps(record,indent=2)+'\n')


def protected_files(owned):
    manifest = ROOT/'protected-baseline.json'
    if not manifest.exists():
        names = subprocess.check_output(['git','ls-files','src/bin/Data/Object1','src/bin/Data/World1'],cwd=REPOSITORY,text=True).splitlines()
        record = {n:hashlib.sha256((REPOSITORY/n).read_bytes()).hexdigest() for n in names if n not in owned}
        manifest.write_text(json.dumps(record,indent=2)+'\n')
    record = json.loads(manifest.read_text())
    for name,digest in record.items():
        assert hashlib.sha256((REPOSITORY/name).read_bytes()).hexdigest() == digest, name
    return len(record)


def install():
    files = {f'src/bin/Data/Object1/{name}.bmd':ROOT/name/f'exports/{name}.bmd' for name in ASSETS}
    files.update({f'src/bin/Data/Object1/{n}':ROOT/'textures/final'/n for n in OWNED_TEXTURES})
    protected = protected_files(files)
    previous = ROOT/'installed-source-files.json'
    old = json.loads(previous.read_text()) if previous.exists() else {}
    for name,source in files.items():
        baseline = subprocess.check_output(['git','show',BASELINE+':'+name],cwd=REPOSITORY)
        digest = hashlib.sha256((REPOSITORY/name).read_bytes()).hexdigest()
        assert digest in {hashlib.sha256(baseline).hexdigest(),old.get(name)}, name
        shutil.copy2(source,REPOSITORY/name)
    protected_files(files)
    manifest = {name:hashlib.sha256((REPOSITORY/name).read_bytes()).hexdigest() for name in files}
    previous.write_text(json.dumps(manifest,indent=2)+'\n')
    (ROOT/'ownership-check.json').write_text(json.dumps(dict(status='PASS',owned_game_files=list(files),protected_file_count=protected),indent=2)+'\n')


def placement_sheets():
    folder=ROOT/'review/placements'
    for record in sorted(folder.glob('*.json')):
        group=record.stem
        sheet=Image.new('RGB',(1440,600),BACKGROUND)
        draw=ImageDraw.Draw(sheet)
        draw.text((20,15),group+' | Actual World1 transforms | OFFLINE Blender; surroundings omitted; not client',fill='white')
        for index,stage in enumerate(('before','after')):
            draw.text((index*720+20,40),stage.upper(),fill='white')
            sheet.paste(panel(folder/(group+'-'+stage+'.png'),(720,530)),(index*720,65))
        sheet.save(folder/(group+'-comparison.jpg'),quality=94)


def main():
    original_hashes()
    for name in ASSETS:
        review_sheet(name)
        for filename in ('summary.json','source-audit.json','geometry-UV-contacts.json','local-motion.json'):
            assert (ROOT/name/'validation'/filename).exists(), (name,filename)
    placement_sheets()
    install()


if __name__ == '__main__':
    main()
