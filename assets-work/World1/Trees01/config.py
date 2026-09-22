"""Owned tree models, original material order and complete texture dependencies."""
from pathlib import Path

ROOT = Path(__file__).resolve().parent
BASELINE = '6000afba058d73616e3310f32a752c116015b606'
REPOSITORY = ROOT.parents[2]
CONVERTER = '/Users/webproduktion3/Documents/claude-test-mumain/MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv'
BLENDER = '/Applications/Blender.app/Contents/MacOS/Blender'
ASSETS = ('Tree01', 'Tree02', 'Tree11', 'Tree06')
MATERIALS = {'Tree01': ('tree.jpg', 'Tree_a.tga'), 'Tree02': ('tree.jpg', 'Tree_a.tga'),
             'Tree11': ('tree_06.tga', 'tree_03.jpg'), 'Tree06': ('tree_02.jpg',)}
KEYS = {'Tree01':31, 'Tree02':31, 'Tree11':31, 'Tree06':1}
FROZEN = 'tree_03.OZJ'
OWNED_TEXTURES = ('tree.OZJ', 'Tree_a.OZT', 'tree_06.OZT', 'tree_02.OZJ')
SIZES = {'tree':(512,512), 'Tree_a':(512,512), 'tree_06':(512,512), 'tree_02':(512,256)}
IDENTITIES = {'Tree01':'spreading oak-like tree', 'Tree02':'large spreading oak-like tree',
              'Tree11':'tiered conifer', 'Tree06':'tall bare pale birch snag'}
