"""Label offline evidence and compare alpha filtering against light/dark backgrounds."""
import json
from pathlib import Path
import sys
from PIL import Image, ImageDraw, ImageFont
sys.dont_write_bytecode = True
from config import PROPS, ROOT, TEXTURES
BACKGROUND=(28,34,29)
INK=(225,225,208)
ACCENT=(193,175,125)
FONT='/System/Library/Fonts/Supplemental/Arial.ttf'
IDENTITIES={'Stone01':'LOW BOULDER, SATELLITES AND DRY GRASS','Stone02':'UPRIGHT SLAB, SATELLITES AND DRY GRASS',
            'Stone03':'LARGE ROUNDED BOULDER','Stone04':'THREE SMALL SCATTERED ROCKS',
            'Stone05':'LOW ROCK CLUSTER','Waterspout01':'DRAGON FOUNTAIN / BASE TEXTURE COMPATIBILITY'}


def label(canvas,position,text,size=24,color=INK):
    ImageDraw.Draw(canvas).text(position,text,font=ImageFont.truetype(FONT,size),fill=color)


def place(canvas,path,position,size):
    image=Image.open(path).convert('RGBA').resize(size,Image.Resampling.LANCZOS)
    canvas.paste(image,position,image)


def comparison(name):
    folder=ROOT/name/'review'
    canvas=Image.new('RGB',(1920,1080),BACKGROUND)
    label(canvas,(35,25),f'{name.upper()} / {IDENTITIES[name]}',31)
    label(canvas,(35,75),'OFFLINE BLENDER • Exported BMD/OZT • Matching camera and lighting • Client acceptance pending',24)
    for x,title,filename in [(30,'ORIGINAL','before-offline.png'),(660,'FINAL EXPORTED GAME FILES','after-offline.png'),
                             (1290,'EXPORTED WIREFRAME','wireframe-offline.png')]:
        label(canvas,(x,130),title,22,ACCENT)
        place(canvas,folder/filename,(x,170),(600,500))
    label(canvas,(35,690),'REDUCED-SCALE READABILITY',24,ACCENT)
    for x,state in [(100,'before'),(470,'after')]:
        place(canvas,folder/f'{state}-offline.png',(x,740),(280,233))
        label(canvas,(x+60,985),'Original' if state=='before' else 'Repainted',22)
    summary=json.loads((ROOT/name/'validation/summary.json').read_text())
    counts=summary['geometry']['triangles_before_after']
    lines=[f'{counts[0]} → {counts[1]} triangles • original source extrema retained',
           'Original bones, hierarchy and complete action preserved',
           ('512 × 512 rock diffuse / original grass alpha retained' if name in ('Stone01','Stone02')
            else '512 × 512 shared rock diffuse / original fountain preserved' if name == 'Waterspout01'
            else '512 × 512 opaque rock diffuse / corrected rock UVs'),
           f'Full compare: {summary["full_compare"]} • Rig: EQUIVALENT']
    for index,text in enumerate(lines):
        label(canvas,(840,770+index*48),text,23)
    label(canvas,(35,1040),'Offline renders are not client evidence. Per-vertex lighting, alpha testing and placed scale remain pending.',21)
    canvas.save(folder/'comparison.png')


def alpha_sheet():
    canvas=Image.new('RGB',(1560,760),BACKGROUND)
    label(canvas,(25,20),'ROCKS01 / SHARED SLATE AND GRASS ALPHA',30)
    label(canvas,(25,65),'Opaque natural slate • Original graded grass alpha preserved • Light and dark fringe checks',21)
    for index,(name,color) in enumerate([('ston01',(40,45,40)),('ston02',(230,230,218)),('ston02',(20,27,21))]):
        x=20+index*515
        label(canvas,(x,110),name+(' / 512² RGB' if name == 'ston01' else ' / 512² RGBA'),24,ACCENT)
        path=ROOT/'textures'/f'{name}.png'
        canvas.paste(color,(x,160,x+495,655))
        place(canvas,path,(x,160),(495,495))
    label(canvas,(25,705),'Grass alpha was enlarged from the original 32px scalar mask; no thresholding or new opaque background.',21)
    canvas.save(ROOT/'alpha-review.png')


def overview():
    canvas=Image.new('RGB',(1600,1800),BACKGROUND)
    label(canvas,(35,25),'LORENCIA / ROCKS AND FOUNTAIN COMPATIBILITY',34)
    label(canvas,(35,78),'OFFLINE BLENDER • 193 rock placements + 1 fountain • 512px slate and dry grass',23)
    for index,name in enumerate(PROPS):
        x,y=35+(index%2)*790,140+(index//2)*535
        label(canvas,(x,y),f'{name} / {IDENTITIES[name]}',19,ACCENT)
        place(canvas,ROOT/name/'review/after-offline.png',(x,y+45),(700,460))
    label(canvas,(35,1760),'No runtime/client verification. Fountain BMD, water and shared statue texture remain byte-identical.',22)
    canvas.save(ROOT/'review-overview.jpg',quality=94)


def fountain_actions():
    canvas=Image.new('RGB',(1440,960),BACKGROUND)
    label(canvas,(25,20),'WATERSPOUT01 / ORIGINAL ANIMATED FOUNTAIN PRESERVED',28)
    label(canvas,(25,65),'OFFLINE BLENDER • 21 keys unchanged • New shared rock-base diffuse only • Water UV scrolling not emulated',21)
    for row,state in enumerate(('before','after')):
        for column,frame in enumerate((0,10,20)):
            x,y=column*480,125+row*395
            label(canvas,(x+20,y),f'{state.upper()} / KEY {frame}',23,ACCENT)
            place(canvas,ROOT/'Waterspout01/review'/f'action-{state}-{frame}-offline.png',(x,y+35),(480,350))
    label(canvas,(25,920),'Final fountain BMD and frozen textures are byte-identical. After image uses the repainted ston01.OZJ base.',21)
    canvas.save(ROOT/'Waterspout01/review/action-comparison.png')


for prop in PROPS:
    comparison(prop)
alpha_sheet()
overview()
fountain_actions()
