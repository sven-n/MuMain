"""Conservative triangle/pixel overlap with periodic texture-address filtering guards."""
from pathlib import Path
import hashlib
import json
import math
import sys
sys.dont_write_bytecode=True
import numpy as np
from PIL import Image,JpegImagePlugin
ROOT=Path(__file__).resolve().parent
from audit_exports import triangles
SIZE=(64,128)
GUARD=2


def intersects(points,x,y):
    rectangle=[(x,y),(x+1,y),(x+1,y+1),(x,y+1)]
    axes=[(1,0),(0,1)]+[(b[1]-a[1],a[0]-b[0]) for a,b in zip(points,points[1:]+points[:1])]
    for axis in axes:
        a=[p[0]*axis[0]+p[1]*axis[1] for p in points]
        b=[p[0]*axis[0]+p[1]*axis[1] for p in rectangle]
        if max(a)<min(b)-1e-9 or max(b)<min(a)-1e-9:return False
    return True


def raster(faces):
    width,height=SIZE;mask=np.zeros((height,width),dtype=bool)
    for _,rows in faces:
        points=[(row[7]*width,(1-row[8])*height) for row in rows]
        for y in range(math.floor(min(p[1] for p in points)),math.floor(max(p[1] for p in points))+1):
            for x in range(math.floor(min(p[0] for p in points)),math.floor(max(p[0] for p in points))+1):
                if intersects(points,x,y):mask[y%height,x%width]=True
    return mask


def dilate(mask):
    return np.logical_or.reduce([np.roll(mask,(y,x),(0,1)) for y in range(-GUARD,GUARD+1) for x in range(-GUARD,GUARD+1)])


def main():
    output=ROOT/'artwork/input-references';output.mkdir(parents=True,exist_ok=True)
    masks={}
    records={}
    for name in ('Object16','Object17'):
        path=ROOT/'context'/name/'baseline'/(name+'.smd')
        faces=[face for face in triangles(path) if face[0]=='deep_wall03.jpg']
        assert len(faces)==4
        masks[name]=raster(faces)
        records[name]=dict(triangles=4,uv_bounds=[[min(row[k] for _,rs in faces for row in rs) for k in (7,8)],
            [max(row[k] for _,rs in faces for row in rs) for k in (7,8)]])
    original=triangles(ROOT/'Object04/validation/baseline/Object04.smd')
    masks['Object04-cap']=raster(original[82:90])
    masks['Object04-backing']=raster(original[6:10])
    masks['Object04-relief']=raster(original[10:82])
    protected=masks['Object16']|masks['Object17']|masks['Object04-cap']
    guarded=dilate(protected)
    allowed=np.zeros((16,8),dtype=np.uint8)
    for y in range(16):
        for x in range(8):allowed[y,x]=not guarded[y*8:y*8+8,x*8:x*8+8].any()
    masks['protected-consumers-and-cap']=protected
    masks['protected-plus2px-periodic']=guarded
    masks['editable-jpeg-blocks']=np.repeat(np.repeat(allowed.astype(bool),8,0),8,1)
    for name,mask in masks.items():Image.fromarray(mask.astype('uint8')*255).save(output/(name+'-mask.png'))
    (output/'allowed-blocks.bin').write_bytes(allowed.tobytes())
    baseline=Image.open(ROOT/'Object04/textures/deep_wall03.jpg');baseline.load()
    assert baseline.size==SIZE and JpegImagePlugin.get_sampling(baseline)==0
    report=dict(status='PROVEN_CROSSCONSUMER_SEPARATION_NOT_PAINT_APPROVAL',dimensions=SIZE,
        target_sha256=hashlib.sha256((ROOT/'Object04/baseline/deep_wall03.OZJ').read_bytes()).hexdigest(),
        consumers=records,protected_pixels=int(protected.sum()),protected_guard_pixels=int(guarded.sum()),
        editable_dct_blocks=int(allowed.sum()),preserved_dct_blocks=int((allowed==0).sum()),
        editable_image_rows=[int(i) for i in np.flatnonzero(masks['editable-jpeg-blocks'].any(axis=1))],
        sampling='4:4:4',quantization=baseline.quantization,
        engine_sources=['src/source/Data/DataHandler/LoadData.h:13 (GL_REPEAT/GL_NEAREST defaults)', 'src/source/Render/Sprites/GlobalBitmap.cpp:144 (num_levels=1)', 'src/source/Render/Sprites/GlobalBitmap.cpp:107-112 (filter/wrap translation)'],
        method='Closed triangle/pixel-cell SAT intersection; all repeated U/V addresses modulo64/128;2px Chebyshev dilation with periodic wrap; preserve any intersecting8x8DCT block',
        owned_structural_overlap='Object04 faces6–9 cover entire atlas; any portrait repaint also changes the owned flat backing/back portrait. Geometry and UVs remain frozen; explicit all-side review required.',
        cap='Object04 faces82–89 protected decoded-exact with same filter guard',
        filter='Engine uses one mip level; periodic2px guard conservatively covers nearest/bilinear footprint, including wrap to top image rows')
    (ROOT/'texture-mask-proof.json').write_text(json.dumps(report,indent=2))
    print({k:report[k] for k in ('protected_pixels','protected_guard_pixels','editable_dct_blocks','preserved_dct_blocks')})


if __name__=='__main__':main()
