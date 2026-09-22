"""Complete shared atlas ownership; readonly geometry compatibility snapshots."""
from pathlib import Path
ROOT=Path(__file__).resolve().parent
REPOSITORY=ROOT.parents[2]
BLENDER='/Applications/Blender.app/Contents/MacOS/Blender'
CONVERTER='/Users/webproduktion3/Documents/claude-test-mumain/MuMain/out/build/macos-arm64/tools/bmdconv/Release/bmdconv'
BASELINE='0bb845121f92e451a5d61afff38a4abf948a637a'
WELLS_REVISION='46daddf2'
PRODUCTION_ASSETS=('Cannon01', 'Cannon02', 'Cannon03', 'Hanging01')
COMPATIBILITY_ASSETS=('Carriage01', 'Carriage02', 'Carriage03', 'Carriage04', 'HouseEtc02', 'StoneMuWall04', 'StoneWall03', 'Well01')
ASSETS=PRODUCTION_ASSETS+COMPATIBILITY_ASSETS
MATERIALS={'Cannon01': ('horse_drawn_01.jpg',), 'Cannon02': ('horse_drawn_01.jpg',), 'Cannon03': ('horse_drawn_01.jpg',), 'Hanging01': ('horse_drawn_01.jpg',), 'Carriage01': ('horse_drawn_01.jpg', 'horse_drawn_02.tga', 'horse_drawn_03.jpg'), 'Carriage02': ('horse_drawn_01.jpg',), 'Carriage03': ('tub.jpg', 'horse_drawn_01.jpg'), 'Carriage04': ('horse_drawn_01.jpg', 'grass_01.tga'), 'HouseEtc02': ('tile_wood02.jpg', 'tile_ston04.jpg', 'c_wall04.jpg', 'tile_wood03.jpg', 'tile_house01.jpg', 'tile_ston01.jpg', 'horse_drawn_01.jpg', 'tile_ston06.jpg'), 'StoneMuWall04': ('c_wall04.jpg', 'c_wall06.jpg', 'horse_drawn_01.jpg'), 'StoneWall03': ('tile_01.jpg', 'horse_drawn_01.jpg', 'tile_03.jpg'), 'Well01': ('tub.jpg', 'horse_drawn_01.jpg', 'well.jpg', 'jar_01.jpg')}
CONTAINERS={'horse_drawn_01.jpg': 'horse_drawn_01.OZJ', 'horse_drawn_02.tga': 'horse_drawn_02.OZT', 'horse_drawn_03.jpg': 'horse_drawn_03.OZJ', 'tub.jpg': 'tub.OZJ', 'grass_01.tga': 'grass_01.OZT', 'tile_wood02.jpg': 'tile_wood02.OZJ', 'tile_ston04.jpg': 'tile_ston04.OZJ', 'c_wall04.jpg': 'c_wall04.OZJ', 'tile_wood03.jpg': 'tile_wood03.OZJ', 'tile_house01.jpg': 'tile_house01.OZJ', 'tile_ston01.jpg': 'tile_ston01.OZJ', 'tile_ston06.jpg': 'tile_ston06.OZJ', 'c_wall06.jpg': 'c_wall06.OZJ', 'tile_01.jpg': 'tile_01.OZJ', 'tile_03.jpg': 'tile_03.OZJ', 'well.jpg': 'well.OZJ', 'jar_01.jpg': 'jar_01.OZJ'}
OWNED_TEXTURES=('horse_drawn_01.OZJ',)
FROZEN=tuple(n for n in CONTAINERS.values() if n not in OWNED_TEXTURES)
KEYS={n:25 if n=='Hanging01' else 21 if n=='Carriage01' else 1 for n in ASSETS}
IDENTITIES={'Cannon01': 'small swivel cannon with two stacked ammunition boxes', 'Cannon02': 'small swivel cannon with one ammunition box', 'Cannon03': 'small swivel cannon on timber stand', 'Hanging01': 'timber gallows platform with stairs and swaying hanging frame', 'Carriage01': 'Readonly shared-atlas consumer Carriage01', 'Carriage02': 'Readonly shared-atlas consumer Carriage02', 'Carriage03': 'Readonly shared-atlas consumer Carriage03', 'Carriage04': 'Readonly shared-atlas consumer Carriage04', 'HouseEtc02': 'Readonly shared-atlas consumer HouseEtc02', 'StoneMuWall04': 'Readonly shared-atlas consumer StoneMuWall04', 'StoneWall03': 'Readonly shared-atlas consumer StoneWall03', 'Well01': 'Readonly shared-atlas consumer Well01'}
IDENTITIES.update({'Hanging01':'timber gallows platform with stairs and swaying rope/noose',
'Carriage01':'two-wheeled passenger cart with lantern','Carriage02':'open two-wheeled cart',
'Carriage03':'cart carrying wooden tubs','Carriage04':'cart loaded with tied hay',
'HouseEtc02':'small roofed masonry annex','StoneMuWall04':'beast-face wall corner with mortar behind',
'StoneWall03':'rough masonry wall with mortar behind','Well01':'roofed well with casks and pottery'})
