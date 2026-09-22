# Lorencia engine controls to preserve

Read-only audit of `src/source/Engine/Object/ZzzObject.cpp` on integration baseline `ac0f6dd8`.
These are reminders for production, not permission to change engine behavior. Inspect all
references for the model being edited; mesh indices below address the original material order.

| Model | Existing control | Production requirement |
|---|---|---|
| House03 | BlendMesh=4 | Keep light_02.jpg at mesh 4 and original animation |
| House04 | BlendMesh=8; V scroll | Keep tile_space01.jpg at mesh 8 and UV meaning |
| House05 | BlendMesh=2; V scroll | Keep ston02.jpg at mesh 2 and UV meaning |
| HouseWall02 | BlendMesh=4; flicker | Keep light_02.jpg at mesh 4 |
| HouseWall05/06 | Fade when hero terrain tile is 4 | Preserve roof shape, contacts and opening geometry |
| Waterspout01 | BlendMesh=3 | Keep the original four material slots and water UVs |
| Bonfire01 | BlendMesh=1 | Preserve fire_02.jpg slot and additive fire geometry/UV meaning |
| Carriage01 | BlendMesh=2 | Keep horse_drawn_03.jpg mesh index and role |
| Tree01/02 | Velocity=0.4/scale; fixed collision bounds | Preserve all 31 sway keys, original bones/order/parents and root position |
| StreetLight01 | BlendMesh=1; Velocity=0.3 | Keep streetlight_brightness2.jpg slot, all 11 bones and 21 keys |
| Candle01 | BlendMesh=1; Velocity=0.3 | Protected completed pilot; no edits |
| TreasureChest01 | Velocity=0 | Protected completed pilot, still retain its original action |
| Sign01/02 | Velocity=0.3 | Preserve full original sign motion and attachment pivots |
| Tree07, Furniture06/07 | CreateOperate interaction | Preserve stump/chair/stool seating identity, footprint and all bones |
| Light01/02/03 | HiddenMesh=-2, fire creation | Hidden engine markers; excluded and unchanged |
| PoseBox01 | HiddenMesh=-2, CreateOperate | Hidden engine marker; excluded and unchanged |
| Bridge01 | CollisionRange=-50, procedural fire at offsets | Preserve footprint and fire anchor relationships |
| DoungeonGate01 | Procedural fire at offsets ±150,-150,140 | Preserve entrance opening and both flanking plinths |
| FireLight01/02 | Procedural fire at fixed offsets | Preserve basket center and rim height |

The loader at `src/source/World/MapInfra/MapManager.cpp:1026` maps these filenames into the
World1 enum slots and resolves textures in Object1. Full exact placement transforms are in
`dependency-map.json`. No texture filename may be changed; embedded `_R/_H/_S/_N` controls
must not be introduced accidentally. TerrainLight and World1 alpha strips remain byte-identical.
