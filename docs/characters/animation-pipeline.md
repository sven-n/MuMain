# How characters, monsters, NPCs and mounts are moved, animated and drawn

A map of the client's character pipeline, written while building the character realism steps
(`docs/superpowers/specs/2026-10-26-character-realism-plan.md`). It names functions rather than line numbers, which
drift. Paths are under `src/source/`.

## Coordinates and angles

- **World axes:** +Z is up. A model with no rotation faces and walks along **-Y**: forward for a facing `f` (degrees)
  is `(sin f, -cos f, 0)`. Ambient boids are the exception (they fly along their local +X).
- **Facing:** `OBJECT::Angle[2]`, degrees, kept 0 to 360 by the angle helpers (direct writes aren't always
  normalized). 0 faces -Y, 90 faces +X. `Angle[0]` and `Angle[1]` are pitch and roll; the flight and mount lean use
  `Angle[1]`.
- **`AngleMatrix(Angle)`** (`Core/Math/ZzzMathLib.cpp`) takes degrees and builds Rz(`Angle[2]`) · Ry(`Angle[1]`) ·
  Rx(`Angle[0]`). `AngleQuaternion` uses the same order.
- **Angle helpers** (`Engine/AI/AngleMath.cpp`, declared in `ZzzAI.h`): `CreateAngle`/`CreateAngle2D` (the facing from
  one point to another, `atan2(dx, -dy)`), `FarAngle(a, b, absolute)` (the shortest gap, signed `b - a` when
  `absolute` is false), `TurnAngle2`, `StepEasedTurn` (an eased turn), `StepSpring` (a damped spring).
- **Head angles:** `OBJECT::HeadAngle[0]` is the head's yaw relative to the body, degrees kept 0 to 360 (300 means -60:
  `Render/Models/HeadTurn.h`'s `SignedYaw`). It's the facing minus the direction to what it looks at, so a positive
  yaw looks clockwise seen from above. `HeadAngle[1]` is the pitch, about `(target z - (own z + 50)) * 0.2`.
  `HeadTargetAngle` is where the head is going; `SmoothHeadAngles` (`ZzzAI.cpp`) eases toward it each frame.

## Who's who

- **`CHARACTER`** (`Engine/Object/w_CharacterInfo.h`) embeds its **`OBJECT Object`** (`w_ObjectInfo.h`) and adds the
  game state: tile position (`PositionX/Y`), `Path`, `Movement`, `TargetCharacter` (an index into `CharactersClient`),
  `SafeZone`, the equipment parts (`BodyPart[6]`, `Weapon[2]`, `Wing`, `Helper`, each a `PART_t` with its own model,
  `LinkBone` and animation frame), smooth turning's state (`MoveHeading`, `TurnPending`, `TurnSpeed`, ...) and, in
  **`Motion`** (`CharacterMotion`, `Engine/Object/Movement/CharacterMotion.h`), the other movement features' state
  (`FlightBank`, `TurnLag`, `LookTarget`, `SnapTurnOffset`, the nods, the pose fade, ...), reset with the character.
- **`CharactersClient`**: 400 usable slots (plus a sentinel), at a random offset into a bigger allocation
  (`Winmain.cpp`). **`Hero`** is an ordinary slot, picked at random on map join. `CharacterOfObject(o)` maps an
  `OBJECT*` back to its slot.
- **`OBJECT::Kind`**: `KIND_PLAYER` 1, `KIND_MONSTER` 2, `KIND_NPC` 4, `KIND_TRAP` 8. `Type` is the model index
  (`MODEL_PLAYER` for every player; the class and equipment change the parts, not the skeleton).
- **Mounts** are separate objects in `Mounts[10]` (`ZzzObject.cpp`), shared by everyone in view. `c->Helper.Type` is
  the item (`MODEL_HORN_OF_UNIRIA`, ...); the mount drawn is `MODEL_UNICON`, `MODEL_PEGASUS` (Dinorant),
  `MODEL_DARK_HORSE` or a `MODEL_FENRIR_*`, with `Owner` pointing at the rider's object.
- **Pets** (`GameLogic/Pets/`) and the Dark Lord's spirit (`CSPetSystem.cpp`) steer toward their owner on their own.

## What the server drives (`Network/Server/WSclient.cpp`)

- **Appearing:** `CreateCharacter` / `CreateMonster` place the character at the centre of its tile, facing the
  packet's direction (`(dir - 1) * 45` degrees), and start a path if it's on the way somewhere.
- **Moving:** `ReceiveMoveCharacter` sets the destination tile and finds a path (`PathFinding2`); no facing. The facing
  comes from walking the path (`MovePath`).
- **Actions:** `ReceiveAction` sets the facing at once, except a stand on another character, which eases it with steps
  (`StartStandingTurn`). Attacks set `TargetCharacter`, the action and `AttackTime`.
- **Skills:** `ReceiveMagic` and friends turn the caster to its target at once (`CreateAngle2D`).
- **Attacks set the facing directly** in about 30 places (`GameLogic/Combat/ClassAttack.cpp`, `SkillCast.cpp`,
  `MoveCharacter`), and effects (arrows, spells) are launched by that facing right after. Don't ease the facing
  itself; ease the drawn body (`UpdateSnapTurn`, below).

## One frame

Every frame runs one move phase then one render phase (`SceneManager.cpp`'s `RenderScene`, `MainScene.cpp`).
`CalcFPS` sets `WorldTime` (milliseconds) and **`FPS_ANIMATION_FACTOR`** = 25 / FPS, at most 1 (`REFERENCE_FPS` is 25);
the seconds of a frame are `FPS_ANIMATION_FACTOR / REFERENCE_FPS`. Below 25 FPS animation slows down rather than skip.

**Move phase** (`MoveMainScene` → `UpdateGameEntities`):

1. Objects, items, boids.
2. **`MoveHero`** (`Engine/Object/ZzzInterface.cpp`): the hero's head (`UpdateHeroLook`: who nodded at it, what it
   looks at, idle glances; else the cursor), walking (`MovePath`, `StepMomentum`, `MoveCharacterPosition`) or turning
   on the spot (toward the cursor), then its flight feel, turn lag and slope lean.
3. **`MoveCharactersClient`** (`ZzzCharacter.cpp`): for every slot `MoveCharacterClient`:
   `MoveMonsterClient` (paths and walking; returns at once for the hero), `FinishPendingTurn`, **`MoveCharacter`**
   (copies the action to the model and **advances the animation**: `CharacterAnimation` → `BMD::PlayAnimation`; also
   the weapon blur; `UpdatePoseFade` runs just before `PlayAnimation`), the flight feel, turn lag and slope lean (not
   the hero's), `UpdateSnapTurn`, `MoveCharacterVisual` (who the head looks at; `RemoveNod`, `SmoothHeadAngles`,
   `UpdateNod`). Then the **bone prepass** (`AnimateCharacterBones`, below).
4. **`MoveMounts`** (`Engine/AI/GOBoid.cpp`): each mount copies its rider's position and angles, picks its action from
   the rider's, then `UpdateMountFeel` (lean, head, shake) and `PlayAnimation`.
5. Pets, effects, joints, particles. These read the characters' prepass bones.

**Render phase** (`RenderMainScene`):

1. **Camera** (`DefaultCamera::Update`): it **resets the hero's height** from the terrain every frame (keeping the
   flight drift, `FlightHoverApplied`), after the move phase.
2. Terrain and objects, then **`RenderCharactersClient`** → `RenderCharacter` per visible character:
   - **Players** (`MODEL_PLAYER`): **`Calc_ObjectAnimation`** rebuilds the skeleton; the body parts (helm, armour,
     pants, gloves, boots) are drawn on it with `RenderPartObject`; weapons, wings and back items with
     `RenderLinkObject`; then the shadow.
   - **Monsters and NPCs**: `RenderObject` → **`Calc_RenderObject`** rebuilds the skeleton and draws the model.
3. Items, **mounts** (`RenderMount` → `RenderObject`, bones into the shared buffer), pets, effects, UI.

## Skeletons: who builds them, and which one is drawn

`BMD::Animation` builds a skeleton (`Render/Models/ZzzBMD.cpp`). A character's is built **more than once a frame**:

| Build | When | Into | What it's for |
| --- | --- | --- | --- |
| `AnimateCharacterBones` | move phase, after `MoveCharacterClient`; on worker threads when enabled and 20+ characters | `o->BoneTransform` | current bones for pets, effects, joints and particles in the move phase |
| `Calc_ObjectAnimation` | render, players | `o->BoneTransform` | **what's drawn** for players |
| `Calc_RenderObject` | render, monsters, NPCs, mounts | `o->BoneTransform` (mounts: the shared `BoneTransform`) | **what's drawn** for everyone else |
| weapon blur, after-images, map and pet hooks | various | mostly the shared `BoneTransform` | positions for effects, extra ghost copies |

**The render builds are what's drawn**; they recompute rather than reuse the prepass. So anything that changes how a
character looks (a turn offset, the ponytail lag, a mount's head) has to reach all three main builds. So all three
call **one builder**, `Engine::Characters::AnimateBones` (`Engine/Object/Movement/MovementBones.cpp`): the facing plus
`SnapTurnOffset`, `TurnLag` for the ponytail, the pose fade (`CharacterPoseFade`), `MountHeadTurn` / `MountHeadRoll`
for a mount's head, then `BMD::Animation`, then, for bones built in model space, `PlaceFeet`. The callers differ only
in where the bones go, whether they're translated, and the prepass passing its own `CurrentAction`. A new input goes
there and reaches every build. The `bones` control command (`App/Control/ControlCommandsQuery.cpp`) compares builds
and reports mismatches.

**Threading:** only `AnimateCharacterBones` runs on workers (`AnimationTaskPool.cpp`), off by default
(`EnableAnimationTaskPool` in config.ini, or `--enable-taskpool`), from 20 characters up. Each worker writes only its
own character's `o->BoneTransform`. Per-object values set on a shared model (`BodyOrigin`, `BodyScale`, `BoneHead`,
`CurrentAction`) and the shared `BoneTransform` are main-thread only: pass anything per character as an argument, not
through the model.

## Inside a model (`Render/Models/ZzzBMD.h/.cpp`)

- **Data:** `Bone_t` has a name, a `Parent` and, per action, keys of `Position`, `Rotation` (Euler, radians) and
  `Quaternion`. `Action_t` has `NumAnimationKeys`, `PlaySpeed` (set by the game, not the file), `Loop` (which means
  "play once and hold", for deaths) and `LockPositions`. Dummy bones have no data; `Animation` gives them identity.
- **Loading** (`BMD::Open2`) also finds `HeadTurnBones` (Bip01 Head and the Neck, Spine1, Spine above it) and
  `PonytailBone` (Bip01 Ponytail1). `BoneHead` is set by the game afterwards (the player's is 20; monsters' in
  `ZzzOpenData.cpp`; mounts look up Bip01 Head by name).
- **`PlayAnimation`** advances `AnimationFrame` by `PlaySpeed * FPS_ANIMATION_FACTOR` keys, records the prior action
  and frame when the whole frame changes, and wraps (or holds, for `Loop`). `PlaySpeed` is per action
  (`ZzzOpenData.cpp`, `CharacterAnimation`'s adjustments: walking speed, freezes, standing turns, the hover breathing).
- **`Animation`**:
  1. The root's matrix from `Angle` (`AngleMatrix`), scaled and placed at `BodyOrigin` when `Translate` (characters
     are built in model space, `Translate` false, and placed when drawn).
  2. The head turn: `HeadAngle[0]`'s yaw split between the head and the Neck/Spine1/Spine bones (`HeadTurn.h`: half to
     the "shoulders", split 0.4 / 0.35 / 0.25), each as an **Euler X delta on the bone's own axes**; `HeadAngle[1]`
     as a Z delta on the head.
  3. Each bone: the prior and current keys blended by the fraction of `AnimationFrame` (quaternion NLERP, linear
     positions), then `parent · local`.
  4. Extras per call: `PonytailLag` swings the ponytail about the head's upright; `HeadTurnUp` and `HeadRoll` turn a
     head in the **world** (`TurnHeadInWorld`): about the upright, and about whichever of its own axes points along
     where the model faces. The rotation math (about an axis, between two directions, a bone turned in place) is
     `Render/Models/BoneMath.h`, shared with the feet (`PlaceFeet`) and the wings.
- **The Euler pitfall:** bones' local axes differ from model to model. On the player's Biped, X runs along the spine,
  so the head turn works. On the four-legged mounts the head bone lies tilted and differently on each (X, Y or a mix
  along the snout), so an Euler delta turned some, rolled others. Turn a head in the world (`TurnHeadInWorld`)
  rather than on a bone axis unless the skeleton is the player's.

## Attachments

- **Body parts** (helm, armour, pants, gloves, boots) are separate models skinned on the **player's skeleton**:
  `RenderPartObject` draws them with the character's `o->BoneTransform`, without an `Animation` of their own.
- **Weapons, wings, capes, back items** (`RenderLinkObject`, `ZzzCharacter.cpp`): placed at a bone (`LinkBone`) and
  given that bone's rotation as their parent (`ParentMatrix`), then animated with their own actions (wings flap). Hand
  weapons hang from bones 33 and 42, wings from **47 (Bone05, on Bip01 Spine)**, Cape of Emperor / Overrule from 19.
  Bone05 gets only the Spine bone's share of the head turn, so on top the wings turn with the back's share
  (`HeadTurn::BackDegrees`, Spine1 and Spine) plus the turn lag, about the upright through Spine1 (tuned by eye, so
  the Spine share counts twice); on a Uniria or Dinorant their bone's rotation comes from the standing action (those
  riding actions tilt it). That's `PlaceWingsOnBack` (`Engine/Object/Movement/Wings.cpp`).
- **Mounts** don't attach: they copy the rider's position and angles each frame, and the rider plays its own riding
  actions. A lean or turn meant for the mount alone goes on after the copy (`UpdateMountFeel`).

## Movement features (`Engine/Object/Movement/`)

The character realism steps live in one folder, one file per feature, in `namespace Engine::Characters`. The game
loop calls them from `MoveHero`, `MoveCharacterClient`, `CharacterAnimation` and `MoveCharacterVisual`; their state
is `CHARACTER::Motion`.

| File | What |
| --- | --- |
| `FlightFeel` | winged players flying or hovering bank into turns and drift up and down; riders and mounts lean (`MountLeanFromRider`); `IsFlying`, `IsRiding` |
| `TurnLag` | the wings and the ponytail trail a turn on a spring |
| `Wings` | the wings turn with the back and the lag (`PlaceWingsOnBack`, from `RenderLinkObject`) |
| `SnapTurn` | a sudden turn of the facing drawn as a quick eased turn (`SnapTurnOffset`) |
| `LookAt` | who a head looks at (`UpdateLookAt`, `LookTowards`); the view and head limits (`LookViewDegrees`, `HeroLookDegrees`) |
| `HeroLook` | the hero's own head before the cursor (`UpdateHeroLook`) and its idle glances (`HeroIsGlancing`) |
| `Greetings` | nods and greetings, from the hero's point of view (`RemoveNod`, `UpdateNod`, `HeroNodAttention`) |
| `Momentum` | walking slows to a stop on a turn back and speeds up from rest (`StepMomentum`) |
| `SlopeLean` | a walker leans into a climb, a mount follows the slope |
| `FootPlacement` | a player's feet on the ground under them, in the bones (`PlaceFeet`) |
| `Stride` | how far each action steps per key (`StepPerKey`), so walking plays as fast as the character moves |
| `PoseFade` | fades between actions (`UpdatePoseFade`, `CharacterPoseFade`), by a table of where the action goes |
| `MovementBones` | the one bone builder (`AnimateBones`, above) |
| `FrameTime` | `FrameSeconds`, `EaseStep`: everything is timed in seconds, the same at any frame rate |

Smooth turning itself (`StepEasedTurn`, `TurnTowardsMoveHeading`, `StartStandingTurn`, `FinishPendingTurn`) stays in
`Engine/AI` with the paths, and the mounts' own feel (`UpdateMountFeel`) in `GOBoid.cpp`; the cursor turn of the
standing hero stays in `ZzzInterface.cpp`.

- **The Enhanced Movement gate** (`Engine/Object/EnhancedMovement.h`, the `[Render] EnhancedMovement` option): every
  feature asks `EnhancedMovementEnabled()` each frame. Off, characters move and are drawn as before them; whatever a
  feature had applied (a bank, a lean, a bow, a lag, a fade) fades out or is taken off, rather than snapping.
- **Pose fades:** a player's switch of action fades the old pose out over 0.25 s when it goes to moving, riding,
  emotes, sitting or poses, 0.08 s into anything else (an attack, a skill, a hit: it still starts at once), and not at
  all from or into death, getting up and the special transformations. Between two steppings (stand, walk, run) the new action starts where
  the old one was in the stride. `BMD::Animation` blends the pose it leaves (`PoseBlend`); a locked root stays locked.
- **Greetings:** a player or NPC within 2 tiles that has looked at the hero for 1.5 s greets it once: a player (or
  an NPC on the player skeleton) standing plays its Hello, another NPC its second animation, anyone else nods. Again
  only after the hero has gone (off screen or 8 tiles away), and at most every 10 s. The hero, if it can turn its head
  to them (60 degrees), looks at them and nods back after 1.6 to 2.8 s. Only on this screen: the server isn't told.

## The player's skeleton (`Data/Player/player.bmd`, 60 bones)

| Index | Bone | Parent |
| --- | --- | --- |
| 0 | Bip01 | root |
| 2 | Pelvis | 0 |
| 3-8, 10-15 | left and right legs (Thigh, Calf, Foot, Toes); `BoneFoot` = 6, 13 | 2 |
| 17 | Spine | 2 |
| 18 | Spine1 | 17 |
| 19 | Neck | 18 |
| 20 | Head (`BoneHead`) | 19 |
| 22, 23 | Ponytail1, Ponytail11 | 20, 22 |
| 25-31 | right arm, R Hand 28 | clavicle on 19 |
| 33 | `knife_gdf` (right weapon) | 28 |
| 34-40 | left arm, L Hand 37 | clavicle on 19 |
| 42 | `hand_bofdgne01` (left weapon) | 37 |
| 43-46 | Bone01-04 | Bone01 on 17 |
| 47-50 | Bone05-08 (wings on 47) | Bone05 on 17 |
| 51-55 | Mesh01-04, gfh | 2 |

Dummies: 9, 16, 21, 24, 32, 41, 56-59.

## Monsters and NPCs each frame

- **Walking:** `MoveMonsterClient` paths toward the server's target tile and walks it like a player (`MovePath`:
  `MoveHeading` along a spline through the path, the facing eased toward it).
- **Head:** `MoveCharacterVisual`: someone to look at (`UpdateLookAt`: a monster its target, an NPC the hero passing,
  a player an enemy, an NPC or another player in sight) comes first; otherwise the old idle glances (`MoveHead` for
  non-players, plus a random glance for everyone but the hero). Skipped while stunned, asleep or hardened.
- **Animation speed:** `CharacterAnimation` adjusts `PlaySpeed` per action and model (transformed players' attacks,
  some bosses' deaths, standing turns, the hover breathing); stun and sleep freeze it.
