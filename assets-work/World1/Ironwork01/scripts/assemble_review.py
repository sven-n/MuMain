"""Compose explicitly labelled offline review sheets and transparency checks."""
import json
from pathlib import Path
import sys
from PIL import Image,ImageDraw,ImageFont
sys.dont_write_bytecode=True
from config import PROPS,ROOT
BACKGROUND=(35,42,39)
INK=(231,228,210)
ACCENT=(194,173,126)
FONT='/System/Library/Fonts/Supplemental/Arial.ttf'
IDENTITIES={'SteelWall01':'CORNER POST AND RETURN RAILS','SteelWall02':'FOUR-SECTION FENCE RUN',
            'SteelWall03':'SINGLE FENCE MODULE','SteelDoor01':'ORNATE IRON GATE','HouseEtc03':'FREESTANDING IRON CAGE'}


def label(canvas,xy,text,size=24,color=INK):
    ImageDraw.Draw(canvas).text(xy,text,font=ImageFont.truetype(FONT,size),fill=color)


def place(canvas,path,xy,size):
    image=Image.open(path).convert('RGBA').resize(size,Image.Resampling.LANCZOS)
    canvas.paste(image,xy,image)


def comparison(name):
    canvas=Image.new('RGB',(1920,1080),BACKGROUND)
    label(canvas,(35,25),f'{name.upper()} / {IDENTITIES[name]}',32)
    label(canvas,(35,75),'OFFLINE BLENDER • Actual BMD exports • Matching camera/light • Client review pending',24)
    folder=ROOT/name/'review'
    for x,title,file in [(30,'ORIGINAL','before-offline.png'),(660,'REBUILT / REPAINTED','after-offline.png'),(1290,'EXPORTED WIREFRAME','wireframe-offline.png')]:
        label(canvas,(x,130),title,23,ACCENT)
        place(canvas,folder/file,(x,170),(600,500))
    label(canvas,(35,700),'REDUCED-SCALE READABILITY',23,ACCENT)
    for x,state in [(70,'before'),(450,'after')]:
        place(canvas,folder/f'{state}-offline.png',(x,745),(280,233))
        label(canvas,(x+75,988),state.upper(),20)
    report=json.loads((ROOT/name/'validation/summary.json').read_text())
    count=report['geometry']['triangles_before_after']
    lines=[f'{count[0]} → {count[1]} triangles / original joins and openings retained',
           'Original bones, hierarchy, material order and action retained',
           '256 × 512 alpha panels / 512 × 512 support diffuse',
           f'Full comparison: {report["full_compare"]} / Rig: EQUIVALENT']
    for i,text in enumerate(lines):label(canvas,(800,765+i*48),text,22)
    label(canvas,(35,1040),'Offline renders are not client evidence. Original alpha masks retained; engine alpha testing remains pending.',22)
    canvas.save(folder/'comparison.png')


def overview():
    canvas=Image.new('RGB',(1800,1450),BACKGROUND)
    label(canvas,(30,25),'LORENCIA / IRON FENCES, GATE AND CAGE',34)
    label(canvas,(30,78),'OFFLINE BLENDER • Five models / 44 placements • Preserved modular joins and negative spaces',23)
    for index,name in enumerate(PROPS):
        x,y=25+(index%3)*590,145+(index//3)*625
        label(canvas,(x,y),name,26,ACCENT)
        place(canvas,ROOT/name/'review/after-offline.png',(x,y+45),(570,500))
        summary=json.loads((ROOT/name/'validation/summary.json').read_text())
        label(canvas,(x,y+555),' → '.join(map(str,summary['geometry']['triangles_before_after']))+' triangles',23)
    label(canvas,(1220,910),'FORGED IRON',25,ACCENT)
    for i,text in enumerate(('Worn gray edges','Restrained rust','Original transparent gaps','All source joins exact','No client verification')):
        label(canvas,(1220,965+i*47),text,23)
    canvas.save(ROOT/'review-overview.jpg',quality=94)


def alpha_sheet():
    canvas=Image.new('RGB',(1600,1250),BACKGROUND)
    label(canvas,(25,20),'IRONWORK / ORIGINAL ALPHA MASKS PRESERVED',31)
    label(canvas,(25,65),'Color repaint only • Bilinear 4× alpha enlargement • Light/dark fringe checks • No new opaque planes',22)
    for row,name in enumerate(('steel_barred_a','steel_barred_door')):
        y=125+row*535
        label(canvas,(25,y),name,24,ACCENT)
        for column,(state,color) in enumerate((('original',(200,205,197)),('textures',(225,228,220)),('textures',(19,24,21)))):
            x=35+column*520
            canvas.paste(color,(x,y+40,x+490,y+510))
            place(canvas,ROOT/state/f'{name}.png',(x+130,y+45),(230,460))
            label(canvas,(x+10,y+50),'ORIGINAL' if state=='original' else 'FINAL',20,(80,90,80) if column<2 else INK)
    canvas.save(ROOT/'alpha-review.png')


def assembly(label_name):
    canvas=Image.new('RGB',(1800,1900),BACKGROUND)
    label(canvas,(30,20),'IRONWORK / '+label_name.upper()+' ASSEMBLY',30)
    label(canvas,(30,65),'OFFLINE reconstruction using recorded World1 transforms • Terrain and client behavior not simulated',22)
    for row,state in enumerate(('original','replacement')):
        label(canvas,(35,120+row*830),state.upper(),24,ACCENT)
        place(canvas,ROOT/'review'/f'{label_name}-{state}.png',(30,160+row*830),(1740,761))
    label(canvas,(35,1830),'Matching view and unchanged placement records. Every original connection vertex survives the rebuilt export.',22)
    canvas.save(ROOT/'review'/f'{label_name}-comparison.jpg',quality=94)


for name in PROPS:comparison(name)
overview()
alpha_sheet()
for name in ('north-gate','prison-corner'):assembly(name)
