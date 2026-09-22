"""Labeled original/generated/final material comparison for offline review."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
HERE=Path(__file__).resolve().parent
NAMES=('well','jar_01','tub')
BACKGROUND=(25,31,30)
FONT=ImageFont.truetype('/System/Library/Fonts/Supplemental/Arial.ttf',25)
sheet=Image.new('RGB',(1500,1510),BACKGROUND)
draw=ImageDraw.Draw(sheet)
draw.text((25,20),'WELL / POTTERY / CASK — ORIGINAL AND PAINTED ATLAS REVIEW',font=FONT,fill='#e0dcc9')
for row,name in enumerate(NAMES):
 for col,(label,path) in enumerate([('ORIGINAL',HERE/'textures/original'/f'{name}.jpg'),('GENERATED MASTER',HERE/'textures/raw'/f'{name}.png'),('FINAL',HERE/'textures/final'/f'{name}.jpg')]):
  image=Image.open(path).convert('RGB');image.thumbnail((460,430))
  x,y=25+500*col,65+475*row
  draw.text((x,y),name+' / '+label,font=FONT,fill='#cbbb85')
  sheet.paste(image,(x,y+35))
sheet.save(HERE/'textures/paint-comparison.jpg',quality=94)
