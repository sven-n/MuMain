"""Explicit ownership and output paths for the Scrub01 vegetation material pass."""
from pathlib import Path
ROOT = Path(__file__).resolve().parent.parent
REPOSITORY = ROOT.parents[2]
CONVERTER = REPOSITORY.parent / 'MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv'
BLENDER = Path('/Applications/Blender.app/Contents/MacOS/Blender')
PROPS = ('Tree09', 'Tree10', 'Grass03', 'Grass04')
TEXTURES = ('tree_01', 'tree_02', 'tree_07')
DIMENSION = 512
