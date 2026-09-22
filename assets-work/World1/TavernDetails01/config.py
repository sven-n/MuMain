"""Exact owned paths and original mesh order for the complete tavern-detail group."""
from pathlib import Path
ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
BLENDER = '/Applications/Blender.app/Contents/MacOS/Blender'
CONVERTER = '/Users/webproduktion3/Documents/claude-test-mumain/MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv'
BASELINE = '8b85343485ad231b4e1d6e96a6e196d7f0c08bd6'
ASSETS = ('Beer02', 'Beer03', 'Furniture01', 'Furniture02', 'Furniture06', 'Furniture07')
PRODUCTION_ASSETS = ASSETS
MATERIALS = {'Beer02': ('pot3.tga', 'apple.jpg', 'bottle.tga', 'plate.jpg', 'winecup.jpg'), 'Beer03': ('bottle.tga', 'winecup.jpg'), 'Furniture01': ('pot.jpg', 'pot2.jpg', 'bookshelf.jpg', 'bottle.tga'), 'Furniture02': ('bottle.tga', 'bookshelf.jpg'), 'Furniture06': ('bookshelf.jpg', 'chair2.tga'), 'Furniture07': ('bookshelf.jpg',)}
CONTAINERS = {'pot3.tga': 'pot3.ozt', 'apple.jpg': 'apple.OZJ', 'bottle.tga': 'bottle.ozt', 'plate.jpg': 'plate.OZJ', 'winecup.jpg': 'winecup.OZJ', 'pot.jpg': 'pot.OZJ', 'pot2.jpg': 'pot2.OZJ', 'bookshelf.jpg': 'bookshelf.OZJ', 'chair2.tga': 'chair2.OZT'}
KEYS = {n:1 for n in ASSETS}
FROZEN = ()
OWNED_TEXTURES = tuple(CONTAINERS.values())
IDENTITIES = {'Beer02': 'apples, bottle, tankard and two bowls', 'Beer03': 'two bottles and tankard', 'Furniture01': 'pottery and bottle shelf', 'Furniture02': 'bottle shelf', 'Furniture06': 'carved high-back chair', 'Furniture07': 'round three-legged stool'}
