"""Label matched beginning/middle/end samples from actual exported BMD animations."""
import json
from pathlib import Path
import sys
sys.dont_write_bytecode = True
from PIL import Image, ImageDraw
ROOT=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT))
from assemble_review import text,place,PAPER,TEXT,MUTED,ACCENT
for name in ('Waterspout01',):
    folder=ROOT/name
    frames=json.loads((folder/'review/action-context.json').read_text())['samples']
    canvas=Image.new('RGB',(1800,1190),PAPER)
    draw=ImageDraw.Draw(canvas)
    text(draw,(30,20),f'{name.upper()} / ORIGINAL ANIMATION CONTRACT',31)
    text(draw,(30,65),'Matching camera and original key indices / actual exported BMD / offline Blender evidence',22,MUTED)
    for row,stage in enumerate(('before','after')):
        for col,frame in enumerate(frames):
            x,y=col*600+20,110+row*515
            text(draw,(x,y),f'{stage.upper()} / KEY {frame}',23,ACCENT)
            place(canvas,folder/f'review/action-{stage}-{frame}.png',(x,y+40),(560,485))
    text(draw,(30,1150),'All 21 keys retained. Static diffuse view; water UV scrolling and runtime particles not emulated. Client review pending.',21,MUTED)
    canvas.save(folder/'review/action-comparison.jpg',quality=95)
