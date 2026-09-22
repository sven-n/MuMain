"""Exclusive asset ownership and paths for Lorencia modular ironwork."""
from pathlib import Path
ROOT=Path(__file__).resolve().parent.parent
REPOSITORY=ROOT.parents[2]
CONVERTER=REPOSITORY.parent/'MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv'
BLENDER=Path('/Applications/Blender.app/Contents/MacOS/Blender')
PROPS=('SteelWall01','SteelWall02','SteelWall03','SteelDoor01','HouseEtc03')
OPAQUE='steel_barred_b'
ALPHA=('steel_barred_a','steel_barred_door')
TEXTURES=(OPAQUE,*ALPHA)
DIMENSION=512
REMESHED=PROPS[:-1]
