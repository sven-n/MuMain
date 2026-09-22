"""Owned tree models, original material order and complete texture dependencies."""
from pathlib import Path

ROOT = Path(__file__).resolve().parent
BASELINE = '2d827ed5225979087e9b68fbcf306879b48acd29'
REPOSITORY = ROOT.parents[2]
CONVERTER = '/Users/webproduktion3/Documents/claude-test-mumain/MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv'
BLENDER = '/Applications/Blender.app/Contents/MacOS/Blender'
ASSETS = ('Tree03', 'Tree04', 'Tree05', 'Tree07', 'Tree11')
PRODUCTION_ASSETS = ('Tree03', 'Tree04', 'Tree05', 'Tree07')
MATERIALS = {'Tree03': ('tree_03.jpg',), 'Tree04': ('tree_03.jpg',),
             'Tree05': ('tree_03.jpg',), 'Tree07': ('tree_03.jpg', 'tree_04.jpg'),
             'Tree11': ('tree_06.tga', 'tree_03.jpg')}
KEYS = {'Tree03':1, 'Tree04':1, 'Tree05':1, 'Tree07':1, 'Tree11':31}
FROZEN = ('tree_04.OZJ', 'tree_06.OZT')
OWNED_TEXTURES = ('tree_03.OZJ',)
SIZES = {'tree_03':(512,256)}
IDENTITIES = {'Tree03':'broad upright dead tree', 'Tree04':'crooked dead tree',
              'Tree05':'narrow forked snag', 'Tree07':'root-flared seating stump',
              'Tree11':'accepted conifer compatibility consumer'}
