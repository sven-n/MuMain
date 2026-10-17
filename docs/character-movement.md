# Character movement and animation timing

How the client decides *where* a character is drawn and *how fast* its equipment
animates.

## Tile, destination and rendered position

Every character carries three related but distinct notions of "where it is":

| State | Meaning |
| --- | --- |
| `PositionX` / `PositionY` | The **logical map tile**, as whole-tile integers. |
| `TargetX` / `TargetY` | The **destination tile** of the last move packet. Can be a whole path away. |
| `Object.Position[0/1]` | The **rendered world position**, smoothly interpolated between tile centres. |

`MovePath` walks a character along its path one segment at a time. At the start
of each segment it advances `PositionX/PositionY` to the *next* path tile, and
`Object.Position` then travels toward it. So mid-walk the logical tile runs ahead
of the model — a full tile at the start of a segment, and up to about two right
after a re-path — and the two coincide only on the frame a segment completes.

None of the three is a safe stand-in for the others, and which one to use depends
on what the code is doing:

- `MoveMonsterClient` re-paths from `PositionX/PositionY` to `TargetX/TargetY`
  whenever the two differ, so a character with a stale destination keeps walking
  toward it even after something set `Movement = false`.
- `PushingCharacter` deliberately pulls the model toward `TargetX/TargetY` — that
  is how a server-sent position correction is applied, and the pull is the point.
- `ReceiveAction` places the model on `TargetX/TargetY`, which is the tile the
  server believes the character to be standing on when it acts.

### Known issue: a walking attack still jumps

An Elf that starts a normal attack mid-walk visibly jumps, on its own screen and
on watchers'. It is not an attack-speed or animation problem — all 14 bow and
crossbow actions have 7 keys and the same play speed.

`Action()` stops the hero between two tiles (`LetHeroStop` plus
`Movement = false`) and sends the hit request from there. `LetHeroStop` sends a
zero-step walk, which carries only a rotation, so the server is never told the
hero stopped early and keeps walking it to the old destination. Watchers get
`TargetX/TargetY` still pointing at that destination, so `ReceiveAction` places
the model there and `MoveMonsterClient` walks it on.

The fix has to make the stop reach the server as a real walk — finish the current
step, then shoot from the tile centre — rather than nudge the rendered position.

Two details are worth knowing before touching this:

- The symptom looks crossbow-specific because of the instant move the click
  handler sends for a walking Elf. Its guard calls the `ITEM*` overload of
  `GetEquipedBowType`, which only inspects `Equipment[0]` — the right hand. A
  crossbow sits there; with a bow equipped that slot holds the arrows, so the
  instant move is only ever sent for crossbows.
- An Elf clicking a monster that is out of range walks toward it on the client
  only: that branch of `Action()` re-paths and sets `Movement = true` without
  sending the move. The server and watchers never see the walk at all.

## Weapon animation follows the character's action

A bow or crossbow is a separate model with its own animation, and its draw has to
stay in step with the character's shot. `RenderCharacter` therefore copies the
playback speed of the **action the character is currently playing** onto the
weapon part:

```cpp
w->PlaySpeed = Models[MODEL_PLAYER].Actions[o->CurrentAction].PlaySpeed;
```

Elves fire from four poses — on foot, on wings, on a Uniria/Dinorant mount and on
a Fenrir — each with a separate animation track per weapon type, plus raised-shot
("_UP") variants for three of them (a Fenrir reuses the mounted track).
`IsBowAttackAction` and `IsRaisedBowAttackAction` in
`Engine/Object/PlayerActionState.h` enumerate the full set; anything that
special-cases bow fire should use them rather than spelling out action constants.

The original S6 client hardcoded `PLAYER_ATTACK_BOW` as the playback-speed source
for every bow action, and only recognised the on-foot and winged poses at all.
The hardcoded source works only for as long as every bow action shares a play
speed, which `SetAttackSpeed` currently gives all 14 of them. The narrow
condition was the visible half: mounted, Fenrir and raised shots fell through to
the catch-all branch, which parks the weapon on frame 0 at `PlaySpeed = 0`, so
the bow held its idle pose instead of drawing. (A Stinger Bow has its own branch
and was never affected.)
