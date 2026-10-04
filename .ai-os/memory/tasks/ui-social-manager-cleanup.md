# Social manager cleanup follow-up

Status: backlog; optional maintenance work, not part of the organization-move batch.
Sources: [shallow review](ui-shallow-cleanup-review.md),
[organization review](ui-organization-review.md).
Rechecked at `9a56ff680` on 2026-10-04.

## Confirmed remaining findings

`src/source/UI/Social/SocialWindowManager.h` still declares CFriendList, CLetterList,
CUIWindowMgr and CUIFriendMenu. Its .cpp's AddWindow still interleaves allocation,
shell registration, cascade placement, initialization and insertion into manager lists.
The formerly unused UIWNDTYPE_EMPTY branch/class is already removed; do not repeat that work.

## Suggested bounded batches

1. Extract CUIFriendMenu into FriendMenu.h/.cpp beside its users. Update references
   and build RelWithDebInfo. Consider list-type extraction when those types are edited.
   Preserve names, namespaces, IDs, queue semantics and ownership.
2. Before changing AddWindow, record side effects for each window type, rejected
   duplicate/compose requests, force-position requests and failed cascade placement.
   In particular, shell registration precedes a placement failure that deletes the
   candidate. Establish intended behavior before extracting or altering that path.
3. Extract placement logic with explicit success/failure output once those contracts
   are understood. Preserve existing registration order and iterator semantics unless
   a separately verified correctness fix is included and documented.

## Completion criteria

- Targeted declarations/definitions are discoverable under Social.
- Creation, cancellation and placement behavior are covered by meaningful checks.
- RelWithDebInfo compiles and links; record any interactive checks separately.
- Update both review dispositions and this task with the actual result.

Do not redesign the manager or retire its base as part of these maintenance batches.
