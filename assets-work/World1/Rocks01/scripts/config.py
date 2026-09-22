"""Ownership and output paths for the connected Lorencia rock/fountain material group."""
from pathlib import Path
ROOT=Path(__file__).resolve().parent.parent
REPOSITORY=ROOT.parents[2]
CONVERTER=REPOSITORY.parent/'MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv'
BLENDER=Path('/Applications/Blender.app/Contents/MacOS/Blender')
PROPS=('Stone01','Stone02','Stone03','Stone04','Stone05','Waterspout01')
ROCKS=PROPS[:-1]
TEXTURES=('ston01','ston02')
DIMENSION=512
FROZEN=('stone_statue02.jpg','ston02.jpg','reagon_waterspout.jpg')
