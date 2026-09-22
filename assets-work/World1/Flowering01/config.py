"""Owned hollow log and accent trees, material order and exact dependency group."""
from pathlib import Path

ROOT = Path(__file__).resolve().parent
BASELINE = 'ac16ffebbf209ca7fa9968fe514f244e33b3903b'
REPOSITORY = ROOT.parents[2]
CONVERTER = '/Users/webproduktion3/Documents/claude-test-mumain/MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv'
BLENDER = '/Applications/Blender.app/Contents/MacOS/Blender'
ASSETS = ('Tree08', 'Tree12', 'Tree13')
MATERIALS = {'Tree08':('tree_01.jpg',), 'Tree12':('tree_01.jpg','tree_04.tga'),
             'Tree13':('tree_05.tga','tree_01.jpg')}
KEYS = {'Tree08':1, 'Tree12':31, 'Tree13':31}
FROZEN = None
OWNED_TEXTURES = ('tree_01.OZJ','tree_04.OZT','tree_05.OZT')
SIZES = {'tree_01':(512,256),'tree_04':(512,512),'tree_05':(512,512)}
IDENTITIES = {'Tree08':'hollow fallen trunk', 'Tree12':'pink-lilac flowering tree', 'Tree13':'ochre autumn tree'}
