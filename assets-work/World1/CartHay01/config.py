"""Owned cart/hay batch, original dependency and mesh-order contract."""
from pathlib import Path
ROOT = Path(__file__).resolve().parent
REPOSITORY = ROOT.parents[2]
BLENDER = '/Applications/Blender.app/Contents/MacOS/Blender'
CONVERTER = '/Users/webproduktion3/Documents/claude-test-mumain/MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv'
BASELINE = '8094616d85e4f54edee4b0ac0bf23774651fa29d'
ASSETS = ('Carriage01', 'Carriage02', 'Carriage03', 'Carriage04', 'Straw01', 'Straw02')
PRODUCTION_ASSETS = ASSETS
MATERIALS = {'Carriage01': ('horse_drawn_01.jpg', 'horse_drawn_02.tga', 'horse_drawn_03.jpg'), 'Carriage02': ('horse_drawn_01.jpg',), 'Carriage03': ('tub.jpg', 'horse_drawn_01.jpg'), 'Carriage04': ('horse_drawn_01.jpg', 'grass_01.tga'), 'Straw01': ('grass_01.tga',), 'Straw02': ('grass_01.tga',)}
CONTAINERS = {'horse_drawn_01.jpg': 'horse_drawn_01.OZJ', 'horse_drawn_02.tga': 'horse_drawn_02.OZT', 'horse_drawn_03.jpg': 'horse_drawn_03.OZJ', 'tub.jpg': 'tub.OZJ', 'grass_01.tga': 'grass_01.OZT'}
OWNED_TEXTURES = ('horse_drawn_02.OZT','horse_drawn_03.OZJ','grass_01.OZT')
FROZEN = ('horse_drawn_01.OZJ','tub.OZJ')
KEYS = {n:21 if n=='Carriage01' else 1 for n in ASSETS}
IDENTITIES = {'Carriage01':'covered-back two-wheel carriage with hanging lantern','Carriage02':'empty two-wheel handcart','Carriage03':'handcart with two adjacent casks','Carriage04':'handcart loaded with three tied straw bundles','Straw01':'three tied straw bundles stacked','Straw02':'eight crossed tied straw bundles'}
