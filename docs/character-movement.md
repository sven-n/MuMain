# Character movement and animation timing

How the client decides *where* a character is drawn and *how fast* its equipment
animates. Both are easy to get subtly wrong, and both show up as the same
symptom: a character that stutters, slides or teleports while it attacks.

## Tile, destination and rendered position

Every character carries three related but distinct notions of "where it is":

| State | Meaning |
| --- | --- |
| `PositionX` / `PositionY` | The **logical map tile** the character occupies. Whole-tile integers. |
| `TargetX` / `TargetY` | The **destination tile** of the last move packet. Can be a whole path away. |
| `Object.Position[0/1]` | The **rendered world position**, smoothly interpolated between tile centres. |

`MovePath` walks a character along its path one segment at a time. At the start
of each segment it advances `PositionX/PositionY` to the *next* path tile, while
`Object.Position` is still travelling toward it. So during a walk the logical
tile is up to one tile ahead of the model, and the two only coincide for the
single frame in which a segment completes.

`TargetX/TargetY` is not a position at all — it is where the character has been
told to go. Outside of an active walk it happens to equal `PositionX/PositionY`,
because every packet that places a character sets both together.

### Rule: never place a model from `TargetX/TargetY`

When something has to put a model on a tile centre — an action packet, a stop, a
teleport — it uses `PositionX/PositionY`:

```cpp
c->Object.Position[0] = ((c->PositionX) + 0.5f) * TERRAIN_SCALE;
c->Object.Position[1] = ((c->PositionY) + 0.5f) * TERRAIN_SCALE;
```

The original S6 client's action handler used `TargetX/TargetY` here. When an
action arrived mid-walk that flung the model to the far end of the path, and the
next movement tick — which re-paths from `PositionX/PositionY` — dragged it back
again. The result is the classic jump-then-slide: worst on ranged attacks, which
fire between tiles far more often than melee swings do, and worst of all on a
crossbow, whose longer draw makes the correction easy to see.

Placing from `PositionX/PositionY` bounds the correction to at most one tile and
keeps the rendered model, the logical tile and the path in agreement. It does not
change any logical state, so the character still walks on to its destination if
the server told it to.

## Weapon animation follows the character's action

A bow or crossbow is a separate model with its own animation, and its draw has to
stay in step with the character's shot. `RenderCharacter` therefore copies the
playback speed of the **action the character is currently playing** onto the
weapon part:

```cpp
w->PlaySpeed = Models[MODEL_PLAYER].Actions[o->CurrentAction].PlaySpeed;
```

Elves fire from four different poses — on foot, on wings, on a Uniria/Dinorant
mount and on a Fenrir — and each pose has a separate animation track per weapon
type, plus a raised-shot ("_UP") variant of all of them. `IsBowAttackAction` and
`IsRaisedBowAttackAction` in `Engine/Object/PlayerActionState.h` enumerate the
full set; anything that special-cases bow fire should use them rather than
spelling out action constants.

The original S6 client hardcoded `PLAYER_ATTACK_BOW` as the playback-speed source
for every bow action, and only recognised the on-foot and winged poses at all.
That works only for as long as every bow action shares a play speed, and it left
mounted, Fenrir and raised shots falling through to the catch-all branch, which
pins the weapon at `PlaySpeed = 0` — a bow frozen mid-draw for the whole shot.
