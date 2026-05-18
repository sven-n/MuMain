# Port completo Rust + Bevy do cliente MU

Status: READY_FOR_EXEC

## Premissas Confirmadas

- `spec.md` está em `READY_FOR_PLAN` e define o produto final: cliente MU completo em Rust + Bevy, Windows x64 release, Linux dev/CI, rede nativa Rust, assets convertidos antes do runtime, UI, áudio, MU Helper, GameShop e editor/admin equivalente.
- Esta etapa conclui somente documentação de planejamento. Nenhum código, workflow, crate, fixture runtime ou documentação de uso fora de `.features/` deve ser criado nesta etapa.
- O cliente legado continua como fonte de comparação até o gate final. Mudanças no legado só são permitidas para fixtures, probes ou validação de equivalência.
- O port não mantém x86 como alvo obrigatório; Windows x64 release é obrigatório.
- O port não reescreve servidores OpenMU, banco, billing ou APIs públicas. O cliente consome o protocolo existente/estendido.
- Toda diferença funcional, visual, econômica, de permissão, protocolo ou fórmula exige registro em `docs/rust-client.md` e aprovação de produto antes de release.

## Approach

Criar um workspace Rust paralelo ao cliente atual. O binário `mu_client` usa Bevy no runtime e orquestra crates separadas para app state, protocolo, rede, assets, renderização, UI, áudio, input, gameplay, editor/admin e suporte de testes. O runtime nunca chama o pipeline de conversão de assets.

O trabalho é faseado por inventário verificável. Cada fase produz evidência local/CI e, quando libera superfície visível ou sensível, handoff para `e2e-validator`. Nenhuma superfície do `spec.md` pode ser marcada migrada sem teste automatizado, snapshot, log verificável ou relatório e2e.

## Affected Files

Arquivos alterados por esta etapa documental:

- `.features/20260518-0000-rust-bevy-port/plan.md`
- `.features/20260518-0000-rust-bevy-port/progress.md`
- `.memory/TODO.md`

Arquivos planejados para implementação futura:

- `rust-toolchain.toml`
- `Cargo.toml`
- `Cargo.lock`
- `crates/mu_client/**`
- `crates/mu_app/**`
- `crates/mu_core/**`
- `crates/mu_protocol/**`
- `crates/mu_network/**`
- `crates/mu_assets/**`
- `crates/mu_asset_pipeline/**`
- `crates/mu_render/**`
- `crates/mu_ui/**`
- `crates/mu_audio/**`
- `crates/mu_input/**`
- `crates/mu_gameplay/**`
- `crates/mu_editor_admin/**`
- `crates/mu_test_support/**`
- `tests/rust/**`
- `.github/workflows/rust-client.yml`
- `.github/workflows/rust-client-windows.yml`
- `docs/rust-client.md`
- `docs/build-guide.md`
- `docs/player-rust-client.md`
- `docs/admin-editor-rust.md`

## Interfaces / Contracts

- CLI: `mu_client --asset-root <path> --server <host:port> [--config <path>] [--editor-admin] [--offline-fixture <name>] [--headless] [--evidence-dir <path>]`.
- Asset pipeline CLI: `mu_asset_pipeline --source <legacy-data-root> --out <converted-root> --manifest <path> [--fixture <name>] [--strict]`.
- Env: `MU_ASSET_ROOT`, `MU_SERVER`, `MU_CONFIG`, `MU_LOG`, `MU_OFFLINE_FIXTURE`, `MU_E2E_ARTIFACT_DIR`.
- Persisted config: `config/client.toml` with `[video] width height fullscreen vsync fps_limit reduce_effects`, `[audio] master music effects mute`, `[controls] bindings`, `[performance] overlays`, `[network] last_server remember_username`, `[locale] language`. Passwords, tokens and raw session IDs are never persisted.
- Asset manifest: `manifest.muasset.json` with `schema_version`, `source_client_version`, `content_hash`, `generated_at`, `entries[{source_path, converted_path, kind, source_hash, converted_hash, dependencies}]`.
- Runtime asset contract: manifest missing, schema mismatch, checksum mismatch or unsupported format blocks entry into playable state, shows safe user error and writes technical log.
- Protocol contract: Rust DTOs in `mu_protocol` cover connect server, chat server and client-to-server entrypoints listed in `Protocol Inventory`. Packet divergences require fixture, diff note and product approval.
- Network contract: async TCP client in `mu_network`; fake server fixtures must cover success, timeout, disconnect, malformed packet and authorization denial.
- UI contract: Bevy UI routes represent boot, login, server select, character select/create/delete, world, HUD, inventory, NPC/shop, chat, options, GameShop, MU Helper, quests/events/social, editor/admin, loading and error states.
- Translation contract: Rust loader consumes converted `Translations/{locale}/{game,editor,metadata}.json`; fallback is deterministic and missing keys are logged without crashing.
- Log contract: structured `tracing`; no password, token, raw session ID, payment secret or credential in logs.
- CI contract: Rust PR gates run `cargo fmt --check`, `cargo clippy --workspace --all-targets -- -D warnings`, `cargo test --workspace`, Linux build, Windows x64 release build when applicable, and existing CMake/doctest gates while the legacy client coexists.
- External API/DB/billing: no new public HTTP API, database migration or billing contract change.

## Legacy Source Inventory

| Area | Legacy sources | Migration target |
|---|---|---|
| Build/current client | `CMakeLists.txt`, `CMakePresets.json`, `src/CMakeLists.txt`, `.github/workflows/mingw-build*.yml` | Rust workspace and CI parity |
| C# network bridge | `ClientLibrary/*.cs`, `src/source/Dotnet/*` | `mu_protocol`, `mu_network`, golden packet fixtures |
| Core runtime services | `src/source/Core/**` | `mu_core`, `mu_app`, `mu_input`; every utility/service is ported, replaced by Rust/Bevy equivalent, or marked fixture-only |
| Audio | `src/source/Audio/**`, `src/ThirdParty/SDL_mixer/**` | `mu_audio`; legacy mixer code is reference/fixture only unless explicitly approved as build dependency |
| Native network support | `src/source/Network/**` | `mu_network`; no C++/C# runtime bridge remains in Rust release |
| Platform/native wrappers | `src/source/Platform/**`, `src/dependencies/**`, `src/ThirdParty/SDL/**`, `src/ThirdParty/imgui/**` | Rust/Bevy ecosystem replacement, release packaging audit, or fixture-only classification |
| Scenes | `src/source/Scenes/*` | Bevy app states/routes |
| Login/windows/options | `src/source/UI/Windows/*`, `docs/options-window.md` | `mu_ui` login/server/options routes |
| Character | `src/source/Character/*` | `mu_gameplay::characters`, `mu_ui::character_select` |
| World/maps/engine | `src/source/World/**`, `src/source/Engine/**`, `src/source/Camera/**` | `mu_gameplay::world`, `mu_render`, `mu_input`; AI, object, pathing and physics behavior covered by gameplay/render fixtures |
| External objects | `src/source/ExternalObject/**` | `mu_gameplay` entity data and `mu_render` visual fixtures |
| Render/models/terrain/effects | `src/source/Render/**` | asset conversion plus Bevy render systems |
| Data/config/game data | `src/source/Data/**`, `src/bin/config.ini.template`, `src/bin/Data/**` | config, data loaders, asset manifest |
| Translation | `src/source/Data/Translation/**`, `src/bin/Translations/**` | `mu_ui::i18n`, converted translation bundle |
| Gameplay | `src/source/GameLogic/**` | `mu_gameplay` rules and tests |
| Guild/social | `src/source/Guild/**`, `src/source/GameLogic/Social/**` | `mu_gameplay::guild`, `mu_ui::social` |
| GameShop | `src/source/GameShop/**`, `src/bin/Data/InGameShop*` | `mu_gameplay::game_shop`, `mu_ui::game_shop` |
| MU Helper | `src/source/MUHelper/**`, `src/source/UI/NewUI/NewUIMuHelper.*` | `mu_gameplay::mu_helper`, `mu_ui::mu_helper` |
| Editor/admin | `src/MuEditor/**`, `docs/dev-editor.md` | `mu_editor_admin`, `mu_ui::admin` |
| Tests | `tests/**`, `tests/third_party/doctest` | Rust unit/integration/property/visual fixtures |

Source classification rule: before any release gate can pass, every top-level legacy path under `ClientLibrary`, `src/source`, `src/MuEditor`, `src/bin`, `src/ThirdParty`, `src/dependencies` and `tests` must be classified as `ported`, `replaced`, `fixture-only`, `reference-only`, or `rejected`. Unclassified paths block AC-16 and F11.

## Asset Inventory

Legacy source root is `src/bin/Data`. The pipeline must inventory and convert at least these observed groups before release:

| Group | Source paths | Observed formats/counts | Runtime output |
|---|---|---|---|
| World terrain | `World*`, `world*` | `.map` 90, `.att` 123, `.ozb` 48, `.obj` 56 | terrain mesh, height, attributes, lights, collision |
| Object/model | `Object*`, `object*`, `Monster`, `NPC`, `Player`, `Item`, `Skill` | `.bmd` 4985, `.smd` 3, `.obj` 56 | Bevy mesh/skeleton/material handles |
| Textures/sprites/UI | `Interface`, `Effect`, `Logo`, all object/world folders | `.ozj` 5438, `.ozt` 1517, `.bmp` 1, `.tga` 5 | image assets with material metadata |
| Game data | `Local`, `InGameShopScript`, config-like files | `.dat` 2, `.db` 4, `.csr` 1, `.cws` 2 | parsed data tables or copied validated resources |
| GameShop media | `InGameShopBanner`, `InGameShopScript` | images/scripts in legacy formats | catalog resources and UI images |
| Translation | `src/bin/Translations/{de,en,es,id,pl,pt,ru,tl,uk,zh-TW}` | `game.json`, `editor.json`, `metadata.json` | locale bundle with fallback metadata |
| Runtime binaries | `src/bin/*.dll`, `src/bin/config.ini.template` | legacy support files | not runtime dependencies for Rust client except reference fixtures |

Unsupported, corrupt or duplicate extension variants such as `.att1`, `.ozj2`, `.rar` and anomalous `world1/xxx` must be classified in the manifest as rejected or copied only with explicit approval.

## Protocol Inventory

All C# NativeAOT entrypoints in `ClientLibrary` are comparison inputs. The Rust port must cover these groups:

| Group | Required entrypoints |
|---|---|
| Connection manager | `ConnectionManager_Connect`, `ConnectionManager_Send`, `ConnectionManager_BeginReceive`, `ConnectionManager_Disconnect` |
| Connect server | `SendConnectionInfoRequest075`, `SendConnectionInfoRequest`, `SendConnectionInfo`, `SendServerListRequest`, `SendServerListRequestOld`, `SendHello`, `SendPatchCheckRequest`, `SendPatchVersionOkay`, `SendClientNeedsPatch` |
| Chat server | `SendAuthenticate`, `ConnectionManager_SendAuthenticateExt`, `SendChatRoomClientJoined`, `SendLeaveChatRoom`, `SendChatRoomClientLeft`, `SendChatMessage`, `ConnectionManager_SendChatMessageExt`, `SendKeepAlive` |
| Session/login/chat | `SendPing`, `SendChecksumResponse`, `SendPublicChatMessage`, `SendWhisperMessage`, `SendLoginLongPassword`, `SendLoginShortPassword`, `SendLogin075`, `ConnectionManager_SendLogin`, `SendLogOut`, `SendLogOutByCheatDetection`, `SendPingResponse` |
| Player shop/items/vault | `SendPlayerShopSetItemPrice`, `SendPlayerShopOpen`, `SendPlayerShopClose`, `SendPlayerShopItemListRequest`, `SendPlayerShopItemBuyRequest`, `SendPlayerShopCloseOther`, `SendPickupItemRequest`, `SendPickupItemRequest075`, `SendDropItemRequest`, `SendItemMoveRequest`, `SendItemMoveRequestExtended`, `SendConsumeItemRequest`, `SendConsumeItemRequest075`, `SendUnlockVault`, `SendSetVaultPin`, `SendRemoveVaultPin`, `SendVaultClosed`, `SendVaultMoveMoneyRequest`, `SendItemRepair`, `SendChaosMachineMixRequest`, `SendCraftingDialogCloseRequest` |
| NPC/warp/world | `SendTalkToNpcRequest`, `SendCloseNpcRequest`, `SendBuyItemFromNpcRequest`, `SendSellItemToNpcRequest`, `SendRepairItemRequest`, `SendWarpCommandRequest`, `SendEnterGateRequest`, `SendEnterGateRequest075`, `SendTeleportTarget`, `SendServerChangeAuthentication`, `SendWalkRequest`, `SendWalkRequest075`, `SendInstantMoveRequest`, `SendAnimationRequest`, `SendClientReadyAfterMapChange` |
| Character/progression | `SendRequestCharacterList`, `SendCreateCharacter`, `SendDeleteCharacter`, `SendSelectCharacter`, `SendFocusCharacter`, `SendIncreaseCharacterStatPoint`, `SendInventoryRequest`, `SendSaveKeyConfiguration`, `SendAddMasterSkillPoint`, `SendResetCharacterPointRequest` |
| Combat/skills/pets | `SendHitRequest`, `SendTargetedSkill`, `SendTargetedSkill075`, `SendTargetedSkill095`, `SendMagicEffectCancelRequest`, `SendAreaSkill`, `SendAreaSkill075`, `SendAreaSkill095`, `SendRageAttackRequest`, `SendRageAttackRangeRequest`, `SendPetCommandRequest`, `SendPetInfoRequest`, `SendNpcBuffRequest` |
| Trade/mail/friend/social | `SendTradeCancel`, `SendTradeButtonStateChange`, `SendTradeRequest`, `SendTradeRequestResponse`, `SendSetTradeMoney`, `SendLetterDeleteRequest`, `SendLetterListRequest`, `SendLetterSendRequest`, `SendLetterReadRequest`, `SendFriendListRequest`, `SendFriendAddRequest`, `SendFriendDelete`, `SendChatRoomCreateRequest`, `SendFriendAddResponse`, `SendSetFriendOnlineState`, `SendChatRoomInvitationRequest` |
| Guild/alliance/castle | `SendGuildKickPlayerRequest`, `SendGuildJoinRequest`, `SendGuildJoinResponse`, `SendGuildListRequest`, `SendGuildCreateRequest`, `SendGuildCreateRequest075`, `SendGuildMasterAnswer`, `SendCancelGuildCreation`, `SendGuildWarResponse`, `SendGuildInfoRequest`, `SendGuildRoleAssignRequest`, `SendGuildTypeChangeRequest`, `SendGuildRelationshipChangeRequest`, `SendGuildRelationshipChangeResponse`, `SendRequestAllianceList`, `SendRemoveAllianceGuildRequest`, `SendCastleSiegeStatusRequest`, `SendCastleSiegeRegistrationRequest`, `SendCastleSiegeUnregisterRequest`, `SendCastleSiegeRegistrationStateRequest`, `SendCastleSiegeMarkRegistration`, `SendCastleSiegeDefenseBuyRequest`, `SendCastleSiegeDefenseRepairRequest`, `SendCastleSiegeDefenseUpgradeRequest`, `SendCastleSiegeTaxInfoRequest`, `SendCastleSiegeTaxChangeRequest`, `SendCastleSiegeTaxMoneyWithdraw`, `SendToggleCastleGateRequest`, `SendCastleGuildCommand`, `SendCastleSiegeHuntingZoneEntranceSetting`, `SendCastleSiegeGateListRequest`, `SendCastleSiegeStatueListRequest`, `SendCastleSiegeRegisteredGuildsListRequest`, `SendCastleOwnerListRequest`, `SendFireCatapultRequest`, `SendWeaponExplosionRequest`, `SendGuildLogoOfCastleOwnerRequest`, `SendCastleSiegeHuntingZoneEnterRequest` |
| GameShop/cash shop | `SendCashShopPointInfoRequest`, `SendCashShopOpenState`, `SendCashShopItemBuyRequest`, `SendCashShopItemGiftRequest`, `SendCashShopStorageListRequest`, `SendCashShopDeleteStorageItemRequest`, `SendCashShopStorageItemConsumeRequest`, `SendCashShopEventItemListRequest` |
| MU Helper | `SendMuHelperStatusChangeRequest`, `SendMuHelperSaveDataRequest` |
| Quests/events/minigames | `SendLegacyQuestStateRequest`, `SendLegacyQuestStateSetRequest`, `SendQuestSelectRequest`, `SendQuestProceedRequest`, `SendQuestCompletionRequest`, `SendQuestCancelRequest`, `SendQuestClientActionRequest`, `SendActiveQuestListRequest`, `SendQuestStateRequest`, `SendEventQuestStateListRequest`, `SendAvailableQuestsRequest`, `SendWhiteAngelItemRequest`, `SendEnterOnWerewolfRequest`, `SendEnterOnGatekeeperRequest`, `SendLeoHelperItemRequest`, `SendMoveToDeviasBySnowmanRequest`, `SendSantaClausItemRequest`, `SendIllusionTempleEnterRequest`, `SendIllusionTempleSkillRequest`, `SendIllusionTempleRewardRequest`, `SendLuckyCoinCountRequest`, `SendLuckyCoinRegistrationRequest`, `SendLuckyCoinExchangeRequest`, `SendDoppelgangerEnterRequest`, `SendEnterMarketPlaceRequest`, `SendEnterEmpireGuardianEvent`, `SendDevilSquareEnterRequest`, `SendMiniGameOpeningStateRequest`, `SendEventChipRegistrationRequest`, `SendMutoNumberRequest`, `SendEventChipExitDialog`, `SendEventChipExchangeRequest`, `SendLuckyNumberRequest`, `SendBloodCastleEnterRequest`, `SendMiniGameEventCountRequest`, `SendChaosCastleEnterRequest`, `SendChaosCastlePositionSet` |
| Region/faction/duel | `SendCrywolfInfoRequest`, `SendCrywolfContractRequest`, `SendCrywolfChaosRateBenefitRequest`, `SendKanturuInfoRequest`, `SendKanturuEnterRequest`, `SendRaklionStateInfoRequest`, `SendGensJoinRequest`, `SendGensLeaveRequest`, `SendGensRewardRequest`, `SendGensRankingRequest`, `SendServerImmigrationRequest`, `SendDuelStartRequest`, `SendDuelStartResponse`, `SendDuelStopRequest`, `SendDuelChannelJoinRequest`, `SendDuelChannelQuitRequest` |

## UI / Editor Inventory

| Surface | Legacy inventory | Rust target |
|---|---|---|
| Login/server/options/messages | `src/source/UI/Windows/{LoginMainWin,LoginWin,ServerSelWin,OptionWin,SysMenuWin,MsgWin,ServerMsgWin,CreditWin,CBTMessageBox}.*` | `mu_ui::login`, `server_select`, `options`, `messages` |
| UI core/widgets | `src/source/UI/NewUI/NewUI*`, `src/source/UI/NewUI/Widgets/*`, `src/source/UI/Widgets/*`, `src/source/UI/Legacy/*` | Bevy UI primitives and compatibility layout fixtures |
| HUD/chat/minimap/status | `src/source/UI/NewUI/HUD/*` | `mu_ui::hud`, `chat`, `minimap`, `hotkeys` |
| Inventory/trade/storage/shop inventory | `src/source/UI/NewUI/Inventory/*` | `mu_ui::inventory`, `trade`, `vault`, `mix`, `marketplace` |
| NPC/dialog/shop/gates | `src/source/UI/NewUI/NPCs/*`, `src/source/UI/Legacy/UIGateKeeper.*`, `UIGuardsMan.*` | `mu_ui::npc`, `shop`, `gate` |
| Events/combat/duel/siege | `src/source/UI/NewUI/Events/*`, `src/source/UI/NewUI/Combat/*` | `mu_ui::events`, `combat`, `duel`, `siege` |
| Party/friend/social/guild | `src/source/UI/NewUI/Party/*`, `src/source/Guild/*` | `mu_ui::party`, `friend`, `guild` |
| Quests | `src/source/UI/NewUI/Quests/*`, `src/source/GameLogic/Quests/*` | `mu_ui::quests`, `mu_gameplay::quests` |
| MU Helper | `src/source/UI/NewUI/NewUIMuHelper.*`, `src/source/MUHelper/*` | `mu_ui::mu_helper`, `mu_gameplay::mu_helper` |
| GameShop | `src/source/GameShop/*`, `src/bin/Data/InGameShop*` | `mu_ui::game_shop`, `mu_gameplay::game_shop` |
| Editor/admin | `src/MuEditor/Config/*`, `Core/*`, `UI/Common/*`, `Console/*`, `DevEditor/*`, `ItemEditor/*`, `SkillEditor/*` | `mu_editor_admin::{auth,core,item_editor,skill_editor,console}`, `mu_ui::admin` |

## Technical Inventory

| Slug | Components | Output type | Queries/consultas | Serializacao/filtros/URL state | Dataset/permission gating | Renderer/test target | Retailer/industry compatibility |
|---|---|---|---|---|---|---|---|
| `client-launch` | `mu_client`, `mu_app`, `mu_assets`, `mu_render` | Bevy window/headless boot | no remote query; reads config/manifest | CLI/env/config TOML | converted manifest and local config | smoke, screenshot, log | OpenMU-compatible, no retailer/industry split |
| `login` | `mu_ui`, `mu_network`, `mu_protocol` | auth form/states | connect/login TCP packets | server, username, password in memory | server auth/session permission | fake server, UI screenshot | OpenMU extended protocol, no retailer/industry split |
| `character-select` | `mu_ui`, `mu_gameplay`, `mu_protocol` | character list/create/delete | character list/create/delete packets | account session DTO | account characters/classes | fake server, screenshots | Season 6 classes, no retailer/industry split |
| `world-play` | `mu_gameplay`, `mu_render`, `mu_assets`, `mu_input` | playable scene | map enter/sync/world packets | map id, entity ids, ECS state | converted maps/entities/server session | movement replay, screenshot | current map set, no retailer/industry split |
| `combat-skills` | `mu_gameplay`, `mu_protocol`, `mu_audio`, `mu_render` | combat actions/effects | hit/skill/buff packets | skill id, target, area, buff DTOs | character stats/skills/server authority | golden formula/replay | Season 6 formulas, no retailer/industry split |
| `inventory-vault-equipment` | `mu_gameplay`, `mu_ui`, `mu_protocol` | item grids/actions | item/vault/trade/player-shop packets | item DTO, slot, serial, extended serialization | inventory/vault/equipment permissions | property tests, fake server | extended item serialization, no retailer/industry split |
| `npc-dialog-trade` | `mu_gameplay`, `mu_ui`, `mu_protocol` | dialogs/shop/actions | NPC talk/close/buy/sell/repair packets | npc id, item slot, money | NPC/shop/inventory permissions | integration tests | current NPC/shop behavior, no retailer/industry split |
| `chat-commands` | `mu_ui`, `mu_network`, `mu_protocol` | chat log/input | public/whisper/chat server packets | channel, target, text, command | chat command permissions | parser tests, fake server | current commands, no retailer/industry split |
| `hud-status` | `mu_ui`, `mu_gameplay` | bars/minimap/buffs/party | consumes world/session state, no own query | ECS state | session/player/party state | visual screenshots | supported resolutions, no retailer/industry split |
| `options` | `mu_ui`, `mu_app`, `mu_input`, `mu_audio` | persisted settings | no remote query | `client.toml` video/audio/input/performance filters | local config write permission | roundtrip + restart tests | Windows/Linux, no retailer/industry split |
| `audio` | `mu_audio`, `mu_assets` | music/SFX | no remote query | manifest refs, volume/mute config | converted audio refs/device availability | diagnostics/log tests | Bevy audio, no retailer/industry split |
| `mu-helper` | `mu_gameplay`, `mu_ui`, `mu_protocol` | helper rules panel | MU Helper status/save packets | helper rule DTO/config | server limits, character/inventory state | fake server/rules tests | server-limited, no retailer/industry split |
| `game-shop` | `mu_gameplay`, `mu_ui`, `mu_protocol` | catalog/buy/storage | cash shop point/open/buy/gift/storage packets | catalog id, item id, quantity, transaction id | server currency/purchase permission | transaction tests | current economy, no retailer/industry split |
| `editor-admin` | `mu_editor_admin`, `mu_ui`, `mu_protocol` | admin/editor UI | admin inspect/save commands or fake server fixtures | admin command DTO, edited row/state | admin permission, release player block | auth/save/audit tests | admin-only, no retailer/industry split |
| `asset-conversion-output` | `mu_asset_pipeline`, `mu_assets` | manifest/package | filesystem queries over legacy data root | manifest JSON, checksums, strict filters | source asset read permission | converter tests | package per client version, no retailer/industry split |
| `diagnostics` | all crates via `tracing` | log artifacts | no remote query | structured log fields with redaction filters | filesystem log write permission | redaction tests | CI/player-safe logs, no retailer/industry split |
| `translation-localization` | `mu_ui::i18n`, `mu_assets` | locale bundle | no remote query | locale code, fallback URL/state none | converted translation files | missing-key/fallback tests | locale split only, no retailer/industry split |
| `guild-social-events` | `mu_gameplay`, `mu_ui`, `mu_protocol` | social/event flows | guild/social/quest/event/duel/gens packets | guild ids, party ids, quest ids, event params | server permissions/event state | fake server/e2e tests | Season 6 server behavior, no retailer/industry split |

## Phases / Task Breakdown

### F1. Rust foundation and gates

| ID | Owner | Planned files | Dependencies | Done when | Required evidence |
|---|---|---|---|---|---|
| F1.S1.T1 Workspace skeleton | `feature-rust-foundation` | `rust-toolchain.toml`, `Cargo.toml`, `Cargo.lock`, `crates/*/Cargo.toml`, `crates/*/src/lib.rs` | none | `cargo metadata` lists all planned crates | `cargo metadata --no-deps --format-version 1` |
| F1.S1.T2 Dependency policy | `feature-rust-foundation` | `Cargo.toml`, crate manifests | F1.S1.T1 | Bevy `0.18`, `serde`, `thiserror`, `tracing`, `tokio`, `clap`, `toml`, `camino`, `insta`, `proptest` centralized | `cargo tree --workspace --depth 1` |
| F1.S1.T3 App state shell | `feature-rust-foundation` | `crates/mu_client/src/main.rs`, `crates/mu_app/src/{lib.rs,state.rs,cli.rs}` | F1.S1.T2 | states `Boot`, `AssetCheckFailed`, `ReadyForLogin`, `Exit` are testable in headless mode | unit tests + headless smoke |
| F1.S2.T1 Linux Rust CI | `feature-ci` | `.github/workflows/rust-client.yml` | F1.S1.T1 | workflow runs fmt, clippy, test, build | local equivalent or Actions log |
| F1.S2.T2 Windows x64 CI | `feature-ci` | `.github/workflows/rust-client-windows.yml` | F1.S1.T1 | release artifact `mu_client.exe` is produced | artifact/checksum + release build log |
| F1.S2.T3 Legacy CI coexistence | `feature-ci` | `.github/workflows/*.yml`, docs notes if needed | F1.S2.T1 | Rust gates do not disable current CMake/doctest gates | CI matrix evidence |

Validation Gate F1: `cargo fmt --check`; `cargo clippy --workspace --all-targets -- -D warnings`; `cargo test --workspace`; `cargo build -p mu_client`; existing CMake/doctest gates still present.

### F2. Inventory, fixtures and comparison harness

| ID | Owner | Planned files | Dependencies | Done when | Required evidence |
|---|---|---|---|---|---|
| F2.S1.T1 Asset inventory scanner | `feature-assets` | `crates/mu_asset_pipeline/src/{inventory.rs,main.rs}` | F1 | scanner emits counts by dir/extension and rejects anomalies | fixture report matching `Asset Inventory` |
| F2.S1.T2 Protocol inventory fixtures | `feature-network` | `crates/mu_protocol/tests/fixtures/**`, `tests/rust/protocol_inventory.rs` | F1 | all protocol groups have fixture placeholders or captured golden packets | fixture manifest checked into tests |
| F2.S1.T3 Legacy comparison harness | `feature-test-support` | `crates/mu_test_support/src/{legacy.rs,fixtures.rs,evidence.rs}` | F1 | tests can record output paths, screenshots/logs and legacy-vs-Rust diffs | harness unit tests |
| F2.S1.T4 UI visual fixture list | `feature-ui` | `tests/rust/ui_fixtures/**`, `crates/mu_ui/src/fixture_routes.rs` | F1 | every UI/editor surface has named offline fixture | snapshot manifest |
| F2.S1.T5 Source/dependency classification audit | `feature-test-support` | `crates/mu_test_support/src/source_inventory.rs`, `tests/rust/source_inventory.rs`, `docs/rust-client.md` | F1 | every legacy source/dependency path is classified and mapped to target, replacement, fixture-only use or rejection | source inventory report + test |
| F2.S2.T1 Evidence artifact conventions | `feature-test-support` | `docs/rust-client.md`, `crates/mu_test_support/src/evidence.rs` | F2.S1.T3 | logs/screenshots/reports follow stable path naming | sample evidence output |

Validation Gate F2: `cargo test -p mu_test_support -p mu_protocol -p mu_asset_pipeline`; source/dependency classification report has no unclassified paths; generated fixture inventories under test artifact directory; `e2e-validator` not required yet unless UI smoke is enabled.

### F3. Assets and localization

| ID | Owner | Planned files | Dependencies | Done when | Required evidence |
|---|---|---|---|---|---|
| F3.S1.T1 Manifest schema/runtime validator | `feature-assets` | `crates/mu_assets/src/{manifest.rs,validation.rs,runtime.rs}`, tests | F2 | valid manifest passes; missing/schema/hash mismatch blocks boot | unit/integration tests |
| F3.S1.T2 Texture conversion | `feature-assets` | `crates/mu_asset_pipeline/src/{textures.rs,manifest_writer.rs}` | F3.S1.T1 | `.ozj`, `.ozt`, `.bmp`, `.tga` convert or reject with reason | converter tests with sample fixtures |
| F3.S1.T3 Model conversion | `feature-assets` | `crates/mu_asset_pipeline/src/{models.rs,bmd.rs,smd.rs,obj.rs}` | F3.S1.T1 | `.bmd`, `.smd`, `.obj` produce loadable mesh/skeleton metadata | golden model fixture |
| F3.S1.T4 Terrain conversion | `feature-assets` | `crates/mu_asset_pipeline/src/{terrain.rs,map.rs,att.rs,ozb.rs}` | F3.S1.T1 | `.map`, `.att`, `.ozb` produce terrain/collision/light data | terrain fixture + checksum |
| F3.S1.T5 Game data conversion | `feature-assets` | `crates/mu_asset_pipeline/src/{game_data.rs,shop_data.rs}` | F3.S1.T1 | `Local`, GameShop scripts and data files are parsed or classified | data parser tests |
| F3.S1.T6 Runtime no-conversion architecture test | `feature-assets` | `crates/mu_assets/tests/runtime_dependencies.rs` | F3.S1.T2-F3.S1.T5 | runtime crates do not depend on pipeline crate | dependency test |
| F3.S2.T1 Translation loader | `feature-ui` | `crates/mu_ui/src/i18n.rs`, `crates/mu_assets/src/translations.rs` | F3.S1.T1 | locales `de,en,es,id,pl,pt,ru,tl,uk,zh-TW` load with fallback | missing-key/fallback tests |
| F3.S2.T2 Audio asset classification | `feature-audio` | `crates/mu_assets/src/audio.rs`, `crates/mu_audio/src/assets.rs` | F3.S1.T1 | music/SFX references are in manifest and missing assets diagnose safely | audio asset tests |

Validation Gate F3: `cargo test -p mu_assets -p mu_asset_pipeline -p mu_ui -p mu_audio`; converter runs on minimal fixture; `mu_client --asset-root <invalid>` shows safe error; e2e-validator validates asset error and valid boot smoke.

### F4. Config, logging and app services

| ID | Owner | Planned files | Dependencies | Done when | Required evidence |
|---|---|---|---|---|---|
| F4.S1.T1 Config persistence | `feature-config` | `crates/mu_app/src/config.rs`, `crates/mu_input/src/bindings.rs`, `crates/mu_audio/src/settings.rs` | F1 | TOML roundtrip preserves video/audio/controls/network/locale and omits secrets | roundtrip tests |
| F4.S1.T2 CLI/env merge rules | `feature-config` | `crates/mu_app/src/cli.rs`, `crates/mu_app/src/config.rs` | F4.S1.T1 | CLI/env/config precedence is deterministic | precedence tests |
| F4.S2.T1 Structured logging | `feature-observability` | `crates/mu_app/src/logging.rs`, `crates/mu_network/src/redaction.rs` | F1 | logs include component/error IDs and redact secrets | redaction tests |
| F4.S2.T2 Error catalog | `feature-observability` | `crates/mu_core/src/error.rs`, `crates/mu_ui/src/error.rs` | F4.S2.T1 | user-safe messages map to technical diagnostics | catalog tests |

Validation Gate F4: `cargo test -p mu_app -p mu_input -p mu_audio -p mu_network -p mu_core`; generated config artifact; redaction report.

### F5. Protocol and network

| ID | Owner | Planned files | Dependencies | Done when | Required evidence |
|---|---|---|---|---|---|
| F5.S1.T1 Packet framing/codec | `feature-network` | `crates/mu_protocol/src/{codec.rs,frame.rs,error.rs}` | F2.S1.T2 | encode/decode supports legacy framing and error cases | golden tests |
| F5.S1.T2 Connect/chat/session packets | `feature-network` | `crates/mu_protocol/src/{connect.rs,chat.rs,session.rs,login.rs}` | F5.S1.T1 | entrypoints in connect/chat/session groups covered | golden tests |
| F5.S1.T3 Character/world/movement packets | `feature-network` | `crates/mu_protocol/src/{character.rs,world.rs,movement.rs}` | F5.S1.T1 | character/world groups covered | golden/property tests |
| F5.S1.T4 Item/vault/trade/shop packets | `feature-network` | `crates/mu_protocol/src/{items.rs,vault.rs,trade.rs,player_shop.rs}` | F5.S1.T1 | item/vault/trade/player-shop groups covered | property + golden tests |
| F5.S1.T5 Combat/skill/pet packets | `feature-network` | `crates/mu_protocol/src/{combat.rs,skills.rs,pets.rs}` | F5.S1.T1 | combat/skill/pet groups covered | golden tests |
| F5.S1.T6 Guild/social/quest/event packets | `feature-network` | `crates/mu_protocol/src/{guild.rs,social.rs,quests.rs,events.rs}` | F5.S1.T1 | guild/social/quest/event groups covered | golden tests |
| F5.S1.T7 GameShop/MU Helper/admin packets | `feature-network` | `crates/mu_protocol/src/{cash_shop.rs,mu_helper.rs,admin.rs}` | F5.S1.T1 | cash shop/helper/admin commands covered | golden tests |
| F5.S2.T1 Async transport/fake server | `feature-network` | `crates/mu_network/src/{transport.rs,client.rs,fake_server.rs}` | F5.S1.T2 | connect, timeout, disconnect, malformed packet and reconnect behavior covered | integration tests |
| F5.S2.T2 Session state integration | `feature-network` | `crates/mu_app/src/session_state.rs`, `crates/mu_network/src/session.rs` | F5.S2.T1 | login/logout/disconnect transitions are observable | fake server tests + logs |

Validation Gate F5: `cargo test -p mu_protocol -p mu_network -p mu_app`; offline fixtures `login-success`, `login-failure`, `disconnect`; e2e-validator required for login/network.

### F6. UI shell, options, HUD and audio

| ID | Owner | Planned files | Dependencies | Done when | Required evidence |
|---|---|---|---|---|---|
| F6.S1.T1 Bevy UI shell/routes | `feature-ui` | `crates/mu_ui/src/{lib.rs,routes.rs,widgets.rs,layout.rs}` | F4, F5 | boot/login/server/character/world/options/error routes exist | route tests |
| F6.S1.T2 Login/server/options UI | `feature-ui` | `crates/mu_ui/src/{login.rs,server_select.rs,options.rs,messages.rs}` | F6.S1.T1 | legacy states from UI Windows inventory visible | snapshots |
| F6.S1.T3 Character select/create/delete UI | `feature-ui` | `crates/mu_ui/src/character_select.rs` | F6.S1.T1, F5 | list/empty/loading/error/action denied states visible | fake server + screenshots |
| F6.S2.T1 HUD/chat/minimap/hotkeys | `feature-ui` | `crates/mu_ui/src/{hud.rs,chat.rs,minimap.rs,hotkeys.rs}` | F6.S1.T1 | health/mana/XP/buffs/party/chat/minimap states visible | visual snapshots |
| F6.S2.T2 Audio runtime | `feature-audio` | `crates/mu_audio/src/{lib.rs,diagnostics.rs,events.rs}` | F3, F4 | music/SFX/volume/mute work or fail diagnostically | audio tests/logs |

Validation Gate F6: `cargo test -p mu_ui -p mu_audio -p mu_app`; offline fixtures `ui-login`, `ui-options`, `hud-audio`; e2e-validator validates visible UI/audio diagnostics.

### F7. World, render, input and entities

| ID | Owner | Planned files | Dependencies | Done when | Required evidence |
|---|---|---|---|---|---|
| F7.S1.T1 Render asset loader | `feature-render` | `crates/mu_render/src/{assets.rs,materials.rs,models.rs,textures.rs}` | F3 | converted textures/models load in Bevy without legacy files | render smoke |
| F7.S1.T2 Terrain/world scene | `feature-world` | `crates/mu_render/src/terrain.rs`, `crates/mu_gameplay/src/world.rs`, `crates/mu_assets/src/map.rs` | F7.S1.T1 | fixture map renders terrain, collision and lights | screenshot + terrain tests |
| F7.S1.T3 Camera/input/movement | `feature-input-world` | `crates/mu_input/src/{movement.rs,camera.rs}`, `crates/mu_gameplay/src/movement.rs` | F7.S1.T2, F5 | local/remote movement and camera are observable | replay/integration tests |
| F7.S2.T1 Entities/objects/NPCs/monsters | `feature-world` | `crates/mu_gameplay/src/{entities.rs,npcs.rs,monsters.rs}`, `crates/mu_render/src/entities.rs` | F7.S1.T2 | local player, remote players, objects, NPCs and monsters spawn from fixture | e2e screenshot |
| F7.S2.T2 Effects/particles | `feature-render` | `crates/mu_render/src/{effects.rs,particles.rs}` | F7.S2.T1 | representative effects render without frame errors | visual fixture |

Validation Gate F7: `cargo test -p mu_gameplay -p mu_render -p mu_input -p mu_assets`; offline fixture `world-minimal`; e2e-validator required for playable surface.

### F8. Gameplay systems

| ID | Owner | Planned files | Dependencies | Done when | Required evidence |
|---|---|---|---|---|---|
| F8.S1.T1 Character stats/classes/master level | `feature-gameplay` | `crates/mu_gameplay/src/{characters.rs,stats.rs,classes.rs,master_level.rs}` | F5, F6 | Season 6 class/stat behavior matches fixtures | golden tests |
| F8.S1.T2 Combat formulas/buffs/XP | `feature-combat` | `crates/mu_gameplay/src/{combat.rs,buffs.rs,experience.rs}` | F7 | damage, XP, buffs/debuffs match vectors or approved diffs | formula/golden tests |
| F8.S1.T3 Skill execution/feedback | `feature-combat` | `crates/mu_gameplay/src/skills.rs`, `crates/mu_render/src/effects.rs`, `crates/mu_audio/src/events.rs` | F8.S1.T2, F6 | targeted/area/rage skills have feedback and server sync | replay + visual/audio evidence |
| F8.S2.T1 Items/inventory/equipment/vault | `feature-items` | `crates/mu_gameplay/src/{items.rs,inventory.rs,equipment.rs,vault.rs}`, `crates/mu_ui/src/inventory.rs` | F5, F6 | move/use/equip/desequip/serialize/sync errors work | property + fake server tests |
| F8.S2.T2 NPC/shop/dialog/repair | `feature-npc` | `crates/mu_gameplay/src/npc.rs`, `crates/mu_ui/src/{npc.rs,shop.rs}` | F8.S2.T1 | buy/sell/repair/confirm/cancel/fail visible | integration tests |
| F8.S2.T3 Trade/player shop/mail | `feature-social` | `crates/mu_gameplay/src/{trade.rs,player_shop.rs,mail.rs}`, UI files | F8.S2.T1 | trade/player shop/mail flows match protocol fixtures | fake server tests |
| F8.S3.T1 Party/friend/guild/alliance | `feature-social` | `crates/mu_gameplay/src/{party.rs,friend.rs,guild.rs}`, UI files | F5, F6 | party/friend/guild/alliance actions and denials visible | fake server tests |
| F8.S3.T2 Quests/events/duel/gens | `feature-events` | `crates/mu_gameplay/src/{quests.rs,events.rs,duel.rs,gens.rs}`, UI files | F7, F8.S1 | listed quest/event/duel/gens packet groups have UI/gameplay coverage | integration + e2e fixtures |
| F8.S3.T3 Pets/summons/mounts | `feature-gameplay` | `crates/mu_gameplay/src/{pets.rs,summons.rs,mounts.rs}` | F8.S1 | pet commands/info and visual states match fixtures | fake server + visual tests |

Validation Gate F8: `cargo test -p mu_gameplay -p mu_ui -p mu_protocol -p mu_render -p mu_audio`; fixtures `combat-items-npc-chat`, `social-events-pets`; e2e-validator validates gameplay surface.

### F9. MU Helper and GameShop

| ID | Owner | Planned files | Dependencies | Done when | Required evidence |
|---|---|---|---|---|---|
| F9.S1.T1 MU Helper data model | `feature-helper` | `crates/mu_gameplay/src/mu_helper.rs`, `crates/mu_protocol/src/mu_helper.rs` | F8 | helper data serializes and validates limits | rules/property tests |
| F9.S1.T2 MU Helper UI/execution | `feature-helper` | `crates/mu_ui/src/mu_helper.rs`, `crates/mu_gameplay/src/mu_helper_runtime.rs` | F9.S1.T1 | active/inactive/invalid/resource/server-limit states visible | fake server + screenshots |
| F9.S2.T1 GameShop catalog/storage | `feature-gameshop` | `crates/mu_gameplay/src/game_shop.rs`, `crates/mu_ui/src/game_shop.rs` | F5, F6 | catalog/details/storage/empty/error states visible | fake server tests |
| F9.S2.T2 GameShop transaction safety | `feature-gameshop` | `crates/mu_gameplay/src/game_shop_transaction.rs`, `crates/mu_network/src/shop.rs` | F9.S2.T1 | purchase success/failure/insufficient funds do not duplicate purchase | idempotency tests + redacted logs |

Validation Gate F9: `cargo test -p mu_gameplay -p mu_ui -p mu_protocol -p mu_network`; fixture `helper-shop`; e2e-validator required for MU Helper/GameShop.

### F10. Editor/admin equivalent

| ID | Owner | Planned files | Dependencies | Done when | Required evidence |
|---|---|---|---|---|---|
| F10.S1.T1 Admin auth and release blocking | `feature-editor-admin` | `crates/mu_editor_admin/src/auth.rs`, `crates/mu_ui/src/admin.rs` | F5, F6 | unauthorized users see blocked state; release player mode hides admin entry | auth tests + screenshots |
| F10.S1.T2 Editor core shell/console/dev editor | `feature-editor-admin` | `crates/mu_editor_admin/src/{core.rs,console.rs,dev_editor.rs}`, UI files | F10.S1.T1 | console/dev panels match supported behavior | UI tests |
| F10.S2.T1 Item editor parity | `feature-editor-admin` | `crates/mu_editor_admin/src/item_editor.rs`, `crates/mu_ui/src/admin_item_editor.rs` | F10.S1.T2, F8.S2.T1 | inspect/edit/save/cancel/fail item data | fake server + audit log |
| F10.S2.T2 Skill editor parity | `feature-editor-admin` | `crates/mu_editor_admin/src/skill_editor.rs`, `crates/mu_ui/src/admin_skill_editor.rs` | F10.S1.T2, F8.S1.T3 | inspect/edit/save/cancel/fail skill data | fake server + audit log |
| F10.S2.T3 Admin documentation evidence | `feature-docs` | `docs/admin-editor-rust.md`, `docs/dev-editor.md` if needed | F10.S2.T1-F10.S2.T2 | admin usage documented without internals | docs review + screenshot |

Validation Gate F10: `cargo test -p mu_editor_admin -p mu_ui -p mu_gameplay`; fixture `admin-editor`; redacted audit logs; e2e-validator required.

### F11. Packaging, docs and release parity

| ID | Owner | Planned files | Dependencies | Done when | Required evidence |
|---|---|---|---|---|---|
| F11.S1.T1 Windows x64 release package | `feature-release` | `.github/workflows/rust-client-windows.yml`, packaging scripts if needed | F3-F10 | clean artifact contains `mu_client.exe`, converted assets and manifest | artifact + checksum |
| F11.S1.T2 Linux dev/CI package smoke | `feature-release` | `.github/workflows/rust-client.yml`, docs | F3-F10 | Linux CI runs all required automated gates | CI status |
| F11.S2.T1 Difference register | `feature-product-qa` | `docs/rust-client.md` | all functional phases | every intentional difference has reason and approval evidence | difference table |
| F11.S2.T2 Player and QA docs | `feature-docs` | `docs/player-rust-client.md`, `docs/rust-client.md`, `docs/build-guide.md` | F11.S1 | player/QA/dev usage documented | smoke following docs |
| F11.S3.T1 Final e2e validation | `e2e-validator` | evidence artifacts under `MU_E2E_ARTIFACT_DIR` | F11.S1-F11.S2 | login to playable world, assets invalid path, GameShop, MU Helper, editor/admin and logs pass | e2e-validator report |

Validation Gate F11: `cargo fmt --check`; `cargo clippy --workspace --all-targets -- -D warnings`; `cargo test --workspace`; `cargo build --release -p mu_client`; Linux/Windows workflows pass; current CMake/doctest gates pass while legacy coexists; final `e2e-validator` approved.

## Test Strategy

- Unit tests per crate for parsing, state machines, config, redaction, data rules and error catalogs.
- Property tests for packet encode/decode, inventory/item moves, helper rules and transaction idempotency.
- Source/dependency classification tests prove every legacy path is ported, replaced, fixture-only, reference-only or rejected before release.
- Golden tests from legacy packet/function/data fixtures for protocol, formulas, assets and translations.
- Integration tests with fake server for login, character, world, inventory, combat, NPC, chat, social, GameShop, MU Helper and editor/admin.
- Visual/e2e snapshots for boot, login, character select, world, HUD, inventory, NPC/shop, options, audio diagnostics, GameShop, MU Helper and admin/editor.
- CI Linux runs fast gates every PR; Windows x64 release workflow produces artifact evidence.
- `e2e-validator` gates F3, F5, F6, F7, F8, F9, F10 and F11 when those phases expose user-visible or sensitive surfaces.

## Matriz AC

| AC | Tasks | Evidence |
|---|---|---|
| AC-01 | F1.S2.T2, F3.S1.T1-F3.S1.T6, F11.S1.T1 | Evidencia: Windows x64 artifact boots with converted assets and no legacy runtime dependency |
| AC-02 | F1.S2.T1, F11.S1.T2 | Evidencia: Linux CI test/build report for documented Rust gates |
| AC-03 | F3.S1.T1, F3.S1.T6 | Evidencia: invalid manifest UI/log evidence and runtime dependency test |
| AC-04 | F5.S2.T1-F5.S2.T2, F6.S1.T2 | Evidencia: fake server login/failure/disconnect tests, screenshots and logs |
| AC-05 | F5.S1.T3, F6.S1.T3 | Evidencia: character fake server tests and screenshots |
| AC-06 | F7.S1.T1-F7.S2.T1 | Evidencia: world fixture screenshot and movement replay |
| AC-07 | F8.S1.T2-F8.S1.T3 | Evidencia: formula/golden/replay/visual/audio evidence |
| AC-08 | F5.S1.T4, F8.S2.T1 | Evidencia: property and fake server inventory/vault/equipment tests |
| AC-09 | F8.S2.T2 | Evidencia: NPC/shop/dialog integration tests |
| AC-10 | F5.S1.T2, F6.S2.T1 | Evidencia: chat parser/fake server/screenshot evidence |
| AC-11 | F4.S1.T1-F4.S1.T2, F6.S1.T2 | Evidencia: config roundtrip test and restart restoration screenshot/log |
| AC-12 | F3.S2.T2, F6.S2.T2, F8.S1.T3 | Evidencia: audio unit/runtime tests, diagnostics and visual/audio fixture logs |
| AC-13 | F9.S1.T1-F9.S1.T2 | Evidencia: helper rules tests, fake server and screenshots |
| AC-14 | F5.S1.T7, F9.S2.T1-F9.S2.T2 | Evidencia: GameShop transaction/idempotency tests |
| AC-15 | F10.S1.T1-F10.S2.T2 | Evidencia: auth tests, blocked/authorized screenshots, save/fail audit logs |
| AC-16 | F2.S1.T1-F2.S1.T5, F3-F10 | Evidencia: every inventory row and legacy path maps to test/evidence or explicit non-runtime classification |
| AC-17 | F11.S2.T1 | Evidencia: approved difference register report |
| AC-18 | F4.S2.T1-F4.S2.T2, F5.S2.T2, F9.S2.T2, F10.S2.T1-F10.S2.T2 | Evidencia: redaction tests and safe message screenshots/logs |
| AC-19 | F3, F5, F6, F7, F8, F9, F10, F11.S3.T1 | Evidencia: intermediate and final e2e-validator reports |
| AC-20 | F10.S2.T3, F11.S2.T2 | Evidencia: final player/QA/dev/admin docs and smoke test following docs |

## Parallelization / Ownership

- F1.S2 can run in parallel after F1.S1.T1, owned by `feature-ci`.
- F2 inventory tasks can run in parallel by asset/protocol/UI/test-support ownership.
- F3 texture/model/terrain/game-data/translation/audio tasks can run in parallel after manifest schema is stable.
- F5 packet groups can run in parallel by protocol module; transport depends on connect/session packets.
- F6 UI/audio can run in parallel after routes and config contracts exist.
- F7 render/world/input can overlap after converted asset fixtures exist, but world scene owns `mu_gameplay/src/world.rs`.
- F8 social/events/pets can run in parallel with items/combat after world and protocol gates pass.
- F9 Helper and GameShop can run in parallel after F8 foundations.
- F10 admin item and skill editor tasks can run in parallel after admin auth/core shell.
- F11 docs, difference register and packaging can run in parallel, but final e2e waits for all gates.

## Risks & Rollback

- Bevy API instability. Mitigation: pin toolchain/dependencies in `Cargo.lock`; rollback by reverting dependency upgrade commit.
- Protocol gaps in OpenMU extensions. Mitigation: golden fixtures per protocol group; rollback by reverting affected protocol task and blocking dependent UI/gameplay surface.
- Asset converter incompleteness. Mitigation: strict manifest classification and per-format fixtures; rollback by reverting converter for affected format and marking dependent surfaces blocked.
- Runtime accidentally depending on converter. Mitigation: dependency architecture test; rollback by removing dependency edge before release.
- GameShop transaction duplication. Mitigation: idempotency tests and fake server retries; rollback by disabling GameShop route with safe blocked state.
- Admin/editor authorization leak. Mitigation: release blocking/auth tests and redacted audit logs; rollback by hiding admin entry and rejecting admin CLI flag.
- Performance divergence on Windows x64. Mitigation: release workflow, profiling evidence for playable phases; rollback by reverting specific render/effect/task commit.
- Legacy/Rust coexistence increasing CI time. Mitigation: workflow split and caching; rollback by disabling Rust release workflow only, leaving legacy gates intact.

## Out of Scope

- Mandatory x86 Rust target.
- Runtime asset conversion.
- Server rewrite, database migration, billing changes or public API redesign.
- New gameplay modes, classes, maps, economies, GameShop behavior or MU Helper behavior beyond equivalence.
- Broad cleanup of C++/C# legacy code unrelated to fixtures/comparison.

## Gate Final

- All F1-F11 tasks done with required evidence.
- `cargo fmt --check`
- `cargo clippy --workspace --all-targets -- -D warnings`
- `cargo test --workspace`
- `cargo build --release -p mu_client`
- Linux Rust workflow approved.
- Windows x64 release workflow approved with artifact/checksum.
- Current CMake/doctest gates approved while legacy coexists.
- `e2e-validator` approved for boot, login, character, world, combat, inventory, NPC/shop, chat, options, audio, translations, social/events, MU Helper, GameShop, editor/admin, invalid assets and log redaction.
- Final docs approved for player, QA/dev and admin/editor.

## Definition of Done

- All AC-01 to AC-20 validated by mapped evidence.
- Every inventory row has implementation, test/evidence and no unresolved blocker.
- Windows x64 release artifact is playable without C++/C# client runtime dependency.
- Linux dev/CI executes the required automated validation.
- No asset conversion occurs during runtime.
- Logs do not expose credentials, tokens, raw session IDs or secrets.
- Intentional differences are approved and documented.
- Final `e2e-validator` passes.
