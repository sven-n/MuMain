# Enhanced look: map environments

Which environment each map has in `Data/Render/Look.json` and why. The environments themselves (what they set) are in [enhanced-look.md](enhanced-look.md#environments). Classic uses none of this.

**Physical** describes the scene: outdoor, indoor, underground, underwater, aerial or mixed. **Policy** is the lighting it calls for:

- **day-night:** open air, meant for a day-night cycle (not built yet: these maps keep their fixed sun).
- **hybrid:** mostly open, but dark, dense or special.
- **fixed:** authored lighting, never a day-night cycle.
- **custom:** its own kind of light (sky, underwater).

The classification is a rendering proposal, not Webzen's own. A map is lit as one region: a mixed map can't light its interior and its open ground differently.

| World | Map | Physical | Theme | Policy | Environment |
|---:|---|---|---|---|---|
| 0 | Lorencia | outdoor | grassland, town | day-night | `outdoor` |
| 1 | Dungeon | underground | dungeon, catacombs | fixed | `enclosed` |
| 2 | Devias | outdoor | snow, tundra | day-night | `outdoor` |
| 3 | Noria | outdoor | forest | day-night | `outdoor` |
| 4 | Lost Tower | indoor | dark tower | fixed | `enclosed` |
| 6 | Arena | mixed | gladiatorial arena | fixed | `fixed` |
| 7 | Atlans | underwater | submerged ruins | custom | `underwater` |
| 8 | Tarkan | outdoor | desert | day-night | `outdoor` |
| 9, 32 | Devil Square | indoor | demonic arena | fixed | `enclosed` |
| 10 | Icarus | aerial | sky, floating ruins | custom | `sky` |
| 11–17, 52 | Blood Castle 1–8 | aerial, mixed | floating fortress | fixed | `fixed` |
| 18–23, 53 | Chaos Castle 1–7 | indoor | enclosed battleground | fixed | `enclosed` |
| 24–29, 36 | Kalima 1–6, Lost Kalima | mixed | dark realm | fixed | `fixed` |
| 30 | Valley of Loren | outdoor | castle battlefield | day-night | `outdoor` |
| 31 | Land of Trials | outdoor | rocky wilderness | day-night | `outdoor` |
| 33 | Aida | outdoor | dark forest | hybrid | `outdoorDense` |
| 34, 35 | Crywolf | outdoor | mountain fortress | day-night | `outdoor` |
| 37 | Kanturu Ruins | outdoor, mixed | ancient ruins | hybrid | `outdoorDense` |
| 38 | Kanturu Relics | indoor | mechanical ruins | fixed | `enclosed` |
| 39 | Refinery Tower | indoor | boss chamber | fixed | `enclosed` |
| 41 | Barracks of Balgass | outdoor, mixed | military canyon | fixed | `fixed` |
| 42 | Balgass Refuge | indoor | boss refuge | fixed | `enclosed` |
| 45–50 | Illusion Temple 1–6 | indoor | ritual arena | fixed | `enclosed` |
| 51 | Elbeland | outdoor | enchanted forest | day-night | `outdoor` |
| 56 | Swamp of Calmness | outdoor | swamp | hybrid | `outdoorDense` |
| 57 | Raklion | outdoor | glacier | hybrid | `outdoorDense` |
| 58 | Raklion boss cave | underground | ice cavern | fixed | `enclosed` |
| 62 | Santa Village | outdoor | snowy village | fixed | `fixed` |
| 63 | Vulcanus | outdoor | volcanic | hybrid | `outdoorDense` |
| 64 | Duel Arena | arena | combat area | fixed | `fixed` |
| 65–68 | Doppelganger 1–4 | indoor, mixed | dungeon corridors | fixed | `enclosed` |
| 69–72 | Imperial Guardian 1–4 | indoor, mixed | fortress corridors | fixed | `enclosed` |
| 79 | Loren Market | outdoor | trade town | day-night | `outdoor` |
| 80, 81 | Karutan 1–2 | outdoor | rocky desert | day-night | `outdoor` |

Not listed, so they get only the default values:

- world 5;
- the GM area (40);
- the login and character scenes (54, 55, 73, 74, 77, 78).

The later-season maps (Acheron, Debenter, Deep Dungeon, and so on) aren't in this client.
