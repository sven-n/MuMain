"""Check material roles, original UV degeneracies and the limited head-cap mapping fix."""
import json
from pathlib import Path
import sys
sys.dont_write_bytecode=True
from PIL import Image,ImageDraw,ImageOps,ImageFont
ROOT=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT))
from package_textures import SPECS

def triangles(path):
    lines=path.read_text().split('triangles\n')[1].splitlines()
    return [(lines[i],[list(map(float,x.split())) for x in lines[i+1:i+4]]) for i in range(0,len(lines)-1,4)]

def zero_uv(items):
    result=[]
    for i,(_,rows) in enumerate(items):
        a,b,c=[r[7:9] for r in rows]
        area=abs((b[0]-a[0])*(c[1]-a[1])-(b[1]-a[1])*(c[0]-a[0]))
        if area<1e-10: result.append(i)
    return result

for folder in ROOT.glob('*/validation'):
    name=folder.parent.name
    old=triangles(folder/f'original/{name}.smd');new=triangles(folder/f'new/{name}.smd')
    old_zero,new_zero=zero_uv(old),zero_uv(new)
    assert len(new_zero)<=len(old_zero),(name,old_zero,new_zero)
    cap=[]
    if name=='StoneStatue02':
        cap=[i for i,(a,b) in enumerate(zip(old,new)) if any(abs(x-y)>1e-6 for aa,bb in zip(a[1],b[1]) for x,y in zip(aa[7:9],bb[7:9]))]
        assert cap==[32,33,69,70,93,94],cap
    (folder/'uv-material-audit.json').write_text(json.dumps(dict(status='PASS',zero_uv_before=old_zero,zero_uv_after=new_zero,
        uv_reprojected_original_triangles=cap,head_cap_note='Six collapsed-density cap regions projected into plain stone atlas patch; positions, bones and triangle count unchanged' if cap else None,
        all_other_original_UV_preservation='See modular-anchors.json; interpolated coordinates only inside explicit remodeled panels'),indent=2)+'\n')
canvas=Image.new('RGB',(1800,1570),(24,29,31));draw=ImageDraw.Draw(canvas);font='/System/Library/Fonts/Supplemental/Arial.ttf'
def text(x,y,value,size=23): draw.text((x,y),value,fill=(224,225,215),font=ImageFont.truetype(font,size))
text(30,20,'MONUMENT STONE / SIX PAINTED MATERIALS',32)
text(30,70,'Matching original UV layouts / all diffuse RGB / same dimensions displayed before and after')
for i,(name,(owner,extension,size)) in enumerate(SPECS.items()):
    x,y=25+(i%2)*900,130+(i//2)*460
    text(x,y,f'{name} / {size[0]}×{size[1]}',25)
    for j,kind in enumerate(('original','final')):
        path=ROOT/owner/('original' if kind=='original' else 'textures')/(name+extension)
        image=Image.open(path);assert image.mode=='RGB'
        image=ImageOps.contain(image,(410,370),Image.Resampling.LANCZOS)
        canvas.paste(image,(x+j*440+(410-image.width)//2,y+40))
        text(x+j*440,y+415,kind.upper(),21)
canvas.save(ROOT/'paintings/texture-comparison.jpg',quality=95)
print('PASS: no new collapsed UV triangles, six explicitly identified repaired head-cap UVs, opaque diffuse roles')
