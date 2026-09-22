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
IDENTITIES={'Tree09':'FIVE TALL GRASS TUFTS','Tree10':'TEN TALL GRASS TUFTS',
            'Grass03':'FOUR UPRIGHT SCRUB CLUMPS','Grass04':'FOUR UPRIGHT SCRUB CLUMPS / VARIANT'}


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
    for x,title,filename in [(30,'ORIGINAL','before-offline.png'),(660,'REPAINTED / PRESERVED FORMS','after-offline.png'),
                             (1290,'EXPORTED WIREFRAME','wireframe-offline.png')]:
        label(canvas,(x,130),title,22,ACCENT)
        place(canvas,folder/filename,(x,170),(600,500))
    label(canvas,(35,690),'REDUCED-SCALE READABILITY',24,ACCENT)
    for x,state in [(100,'before'),(470,'after')]:
        place(canvas,folder/f'{state}-offline.png',(x,740),(280,233))
        label(canvas,(x+60,985),'Original' if state=='before' else 'Repainted',22)
    summary=json.loads((ROOT/name/'validation/summary.json').read_text())
    lines=[f'{summary["geometry"]["triangles_before_after"][1]} triangles • geometry and UV layout preserved',
           'Original bone names, order, parents and single-frame action',
           '512 × 512 diffuse / original graded alpha retained',
           'Full BMD compare: EQUIVALENT (0.05-unit tool tolerance)']
    for index,text in enumerate(lines):
        label(canvas,(840,770+index*48),text,23)
    label(canvas,(35,1040),'Offline renders are not client evidence. Per-vertex lighting, alpha testing and placed scale remain pending.',21)
    canvas.save(folder/'comparison.png')


def alpha_sheet():
    canvas=Image.new('RGB',(1560,1270),BACKGROUND)
    label(canvas,(25,20),'SCRUB01 / ALPHA AND FRINGE REVIEW',30)
    label(canvas,(25,65),'Original scalar alpha retained with bilinear enlargement • continuous painted color underneath',21)
    for index,name in enumerate(TEXTURES):
        x=20+index*515
        label(canvas,(x,110),name+' / 512² RGBA',24,ACCENT)
        path=ROOT/'textures'/f'{name}.png'
        for y,color in [(160,(230,230,218)),(685,(20,27,21))]:
            canvas.paste(color,(x,y,x+495,y+495))
            place(canvas,path,(x,y),(495,495))
    label(canvas,(25,1220),'Light and dark backdrops expose halos; no alpha thresholding or black/white background matting was introduced.',21)
    canvas.save(ROOT/'alpha-review.png')


def overview():
    canvas=Image.new('RGB',(1600,1260),BACKGROUND)
    label(canvas,(35,25),'LORENCIA / SCRUB AND TALL GRASS',34)
    label(canvas,(35,78),'OFFLINE BLENDER • 530 existing placements • Original mesh forms and gameplay data retained',23)
    for index,name in enumerate(PROPS):
        x,y=35+(index%2)*790,140+(index//2)*535
        label(canvas,(x,y),f'{name} / {IDENTITIES[name]}',24,ACCENT)
        place(canvas,ROOT/name/'review/after-offline.png',(x,y+45),(700,460))
    label(canvas,(35,1220),'No runtime installation or client verification. Original World1 terrain, lighting and alpha strips untouched.',22)
    canvas.save(ROOT/'review-overview.jpg',quality=94)


for prop in PROPS:
    comparison(prop)
alpha_sheet()
overview()
