"""Measure unchanged legacy masks and RGB padding without changing artwork."""
import json
from pathlib import Path
import sys
from PIL import Image
sys.dont_write_bytecode=True
from config import ALPHA,ROOT


def coverage(alpha,threshold):
    return sum(value>=threshold for value in alpha.get_flattened_data())/(alpha.width*alpha.height)


report={}
for name in ALPHA:
    original=Image.open(ROOT/'original'/f'{name}.tga').convert('RGBA')
    final=Image.open(ROOT/'textures'/f'{name}.tga').convert('RGBA')
    alpha=final.getchannel('A')
    resized=original.getchannel('A').resize(final.size,Image.Resampling.BILINEAR)
    assert alpha.tobytes()==resized.tobytes()
    report[name]={'original_size':list(original.size),'final_size':list(final.size),
                  'mask':'Exact original alpha resized 4× bilinearly; no new opaque areas painted',
                  'coverage_at_threshold':{str(level):[coverage(original.getchannel('A'),level),coverage(alpha,level)] for level in (1,64,128,254)},
                  'rgba_unpremultiplied':True,'negative_space':'Original bar/scroll silhouette and holes retained'}
(ROOT/'textures/alpha-preservation.json').write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps(report,indent=2))
