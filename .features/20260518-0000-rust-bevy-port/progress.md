# Port completo Rust + Bevy do cliente MU

Status atual: F2.S1.T1 concluído localmente; F1.S2.T2 segue aguardando evidência de Actions

Próximo passo concreto: iniciar `F2.S1.T2 Protocol inventory fixtures` localmente; `F1.S2.T2` permanece bloqueado até existir artefato/checksum/log de Actions.

Sincronização com git: `rtk git status --short --untracked-files=all` mostra as atualizações em `.features/20260518-0000-rust-bevy-port/{plan.md,progress.md}`, `.memory/{RULES_AND_DEFINITION.md,TODO.md}`, os workflows Rust e o workspace Rust em `port_rust/Cargo.toml`, `port_rust/Cargo.lock`, `port_rust/rust-toolchain.toml` e `port_rust/crates/mu_*/...`; o relatório local `tmp/asset-inventory/data-report.txt` é artefato ignorado e também está representado em "Arquivos tocados".

## Controle documental

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| DOC.SPEC | done | `feature-spec` | `.features/20260518-0000-rust-bevy-port/spec.md` | `.features/20260518-0000-rust-bevy-port/spec.md` | `spec.md` em `READY_FOR_PLAN` | Spec existente validado como contrato de produto | nenhum |
| DOC.PLAN | done | `feature-plan` | `.features/20260518-0000-rust-bevy-port/plan.md` | `.features/20260518-0000-rust-bevy-port/plan.md` | Plano completo com inventários, contratos, fases, tarefas, gates e AC traceability | Plano atualizado com classificação explícita de fontes/dependências legadas; sem código escrito | nenhum |
| DOC.PROGRESS | done | `feature-progress` | `.features/20260518-0000-rust-bevy-port/progress.md` | `.features/20260518-0000-rust-bevy-port/progress.md` | Progresso sincronizado com task tree do plano | Este arquivo atualizado | nenhum |
| DOC.TODO | done | local | `.memory/TODO.md` | `.memory/TODO.md` | Pendência de implementação registrada por regra do repo | Entrada adicionada | nenhum |

## Árvore executável

O skeleton do workspace Rust, a política central de dependências, o app state shell, o Linux Rust CI, a coexistência com CI legado e o scanner local de inventário de assets foram criados; F1.S2.T2 tem workflow Windows escrito, mas segue bloqueado até existir artefato/checksum/log de Actions.

### F1. Rust foundation and gates

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| F1.S1.T1 | done | `feature-rust-foundation` | `port_rust/rust-toolchain.toml`, `port_rust/Cargo.toml`, `port_rust/Cargo.lock`, `port_rust/crates/*/Cargo.toml`, `port_rust/crates/*/src/lib.rs` | `port_rust/rust-toolchain.toml`, `port_rust/Cargo.toml`, `port_rust/Cargo.lock`, `port_rust/crates/mu_client/{Cargo.toml,src/lib.rs}`, `port_rust/crates/mu_app/{Cargo.toml,src/lib.rs}`, `port_rust/crates/mu_core/{Cargo.toml,src/lib.rs}`, `port_rust/crates/mu_protocol/{Cargo.toml,src/lib.rs}`, `port_rust/crates/mu_network/{Cargo.toml,src/lib.rs}`, `port_rust/crates/mu_assets/{Cargo.toml,src/lib.rs}`, `port_rust/crates/mu_asset_pipeline/{Cargo.toml,src/lib.rs}`, `port_rust/crates/mu_render/{Cargo.toml,src/lib.rs}`, `port_rust/crates/mu_ui/{Cargo.toml,src/lib.rs}`, `port_rust/crates/mu_audio/{Cargo.toml,src/lib.rs}`, `port_rust/crates/mu_input/{Cargo.toml,src/lib.rs}`, `port_rust/crates/mu_gameplay/{Cargo.toml,src/lib.rs}`, `port_rust/crates/mu_editor_admin/{Cargo.toml,src/lib.rs}`, `port_rust/crates/mu_test_support/{Cargo.toml,src/lib.rs}` | `cargo metadata --manifest-path port_rust/Cargo.toml --no-deps --format-version 1` | `cargo metadata` listou 14 crates do workspace; `cargo test --manifest-path port_rust/Cargo.toml --workspace` passou; `cargo fmt --manifest-path port_rust/Cargo.toml --all --check` passou; `port_rust/Cargo.lock` foi gerado | nenhum |
| F1.S1.T2 | done | `feature-rust-foundation` | `port_rust/Cargo.toml`, crate manifests | `port_rust/Cargo.toml`, `port_rust/crates/mu_client/Cargo.toml`, `port_rust/crates/mu_app/Cargo.toml`, `port_rust/crates/mu_core/Cargo.toml`, `port_rust/crates/mu_protocol/Cargo.toml`, `port_rust/crates/mu_network/Cargo.toml`, `port_rust/crates/mu_assets/Cargo.toml`, `port_rust/crates/mu_asset_pipeline/Cargo.toml`, `port_rust/crates/mu_render/Cargo.toml`, `port_rust/crates/mu_ui/Cargo.toml`, `port_rust/crates/mu_audio/Cargo.toml`, `port_rust/crates/mu_input/Cargo.toml`, `port_rust/crates/mu_gameplay/Cargo.toml`, `port_rust/crates/mu_editor_admin/Cargo.toml`, `port_rust/crates/mu_test_support/Cargo.toml` | `cargo tree --manifest-path port_rust/Cargo.toml --workspace --depth 1` | `port_rust/Cargo.toml` centraliza Bevy `0.18`, `serde`, `thiserror`, `tracing`, `tokio`, `clap`, `toml`, `camino`, `insta` e `proptest`; crate manifests herdam `version`, `edition` e `publish`; `cargo tree --manifest-path port_rust/Cargo.toml --workspace --depth 1` passou | nenhum |
| F1.S1.T3 | done | `feature-rust-foundation` | `port_rust/crates/mu_client/src/main.rs`, `port_rust/crates/mu_app/src/{lib.rs,state.rs,cli.rs}` | `port_rust/crates/mu_client/src/main.rs`, `port_rust/crates/mu_client/Cargo.toml`, `port_rust/crates/mu_app/src/lib.rs`, `port_rust/crates/mu_app/src/state.rs`, `port_rust/crates/mu_app/src/cli.rs`, `port_rust/crates/mu_app/Cargo.toml`, `port_rust/Cargo.lock` | unit tests + headless smoke | `mu_app` expõe `Boot`, `AssetCheckFailed`, `ReadyForLogin` e `Exit`; testes unitários passaram; `mu_client --headless` imprime `ready-for-login`; asset root ausente imprime `asset-check-failed` e retorna código 3 | nenhum |
| F1.S2.T1 | done | `feature-ci` | `.github/workflows/rust-client.yml` | `.github/workflows/rust-client.yml` | local equivalent or Actions log | workflow executa `cargo fmt --manifest-path port_rust/Cargo.toml --all --check`, `cargo clippy --manifest-path port_rust/Cargo.toml --workspace --all-targets -- -D warnings`, `cargo test --manifest-path port_rust/Cargo.toml --workspace` e `cargo build --manifest-path port_rust/Cargo.toml -p mu_client`; equivalentes locais passaram | nenhum |
| F1.S2.T2 | blocked | `feature-ci` | `.github/workflows/rust-client-windows.yml` | `.github/workflows/rust-client-windows.yml` | artifact/checksum + release build log | workflow Windows x64 criado com release build, SHA256 e upload de `mu_client.exe`; YAML parse local passou | aguarda execução do GitHub Actions para produzir artefato/checksum/log |
| F1.S2.T3 | done | `feature-ci` | `.github/workflows/*.yml`, docs notes if needed | `.github/workflows/rust-client.yml`, `.github/workflows/rust-client-windows.yml` | CI matrix evidence | workflows Rust adicionados separadamente; `git diff --name-only -- .github/workflows/mingw-build*.yml` sem saída; workflows legados MinGW seguem presentes | nenhum |

Validation Gate F1: todo.

### F2. Inventory, fixtures and comparison harness

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| F2.S1.T1 | done | `feature-assets` | `port_rust/crates/mu_asset_pipeline/src/{inventory.rs,main.rs}` | `port_rust/crates/mu_asset_pipeline/src/inventory.rs`, `port_rust/crates/mu_asset_pipeline/src/main.rs`, `port_rust/crates/mu_asset_pipeline/src/lib.rs`, `port_rust/crates/mu_asset_pipeline/Cargo.toml`, `tmp/asset-inventory/data-report.txt` | fixture report matching Asset Inventory | scanner conta arquivos por extensão e diretório, rejeita `.att1`, `.ozj2`, `.rar` e `World1/xxx`; relatório real encontrou 13.168 arquivos e 4 rejeições esperadas | nenhum |
| F2.S1.T2 | todo | `feature-network` | `port_rust/crates/mu_protocol/tests/fixtures/**`, `port_rust/tests/rust/protocol_inventory.rs` | nenhum | fixture manifest checked into tests | pendente | aguarda autorização de implementação |
| F2.S1.T3 | todo | `feature-test-support` | `port_rust/crates/mu_test_support/src/{legacy.rs,fixtures.rs,evidence.rs}` | nenhum | harness unit tests | pendente | aguarda autorização de implementação |
| F2.S1.T4 | todo | `feature-ui` | `port_rust/tests/rust/ui_fixtures/**`, `port_rust/crates/mu_ui/src/fixture_routes.rs` | nenhum | snapshot manifest | pendente | aguarda autorização de implementação |
| F2.S1.T5 | todo | `feature-test-support` | `port_rust/crates/mu_test_support/src/source_inventory.rs`, `port_rust/tests/rust/source_inventory.rs`, `docs/rust-client.md` | nenhum | source inventory report + test | pendente | aguarda autorização de implementação |
| F2.S2.T1 | todo | `feature-test-support` | `docs/rust-client.md`, `port_rust/crates/mu_test_support/src/evidence.rs` | nenhum | sample evidence output | pendente | aguarda autorização de implementação |

Validation Gate F2: todo.

### F3. Assets and localization

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| F3.S1.T1 | todo | `feature-assets` | `port_rust/crates/mu_assets/src/{manifest.rs,validation.rs,runtime.rs}`, tests | nenhum | unit/integration tests | pendente | aguarda autorização de implementação |
| F3.S1.T2 | todo | `feature-assets` | `port_rust/crates/mu_asset_pipeline/src/{textures.rs,manifest_writer.rs}` | nenhum | converter tests with sample fixtures | pendente | aguarda autorização de implementação |
| F3.S1.T3 | todo | `feature-assets` | `port_rust/crates/mu_asset_pipeline/src/{models.rs,bmd.rs,smd.rs,obj.rs}` | nenhum | golden model fixture | pendente | aguarda autorização de implementação |
| F3.S1.T4 | todo | `feature-assets` | `port_rust/crates/mu_asset_pipeline/src/{terrain.rs,map.rs,att.rs,ozb.rs}` | nenhum | terrain fixture + checksum | pendente | aguarda autorização de implementação |
| F3.S1.T5 | todo | `feature-assets` | `port_rust/crates/mu_asset_pipeline/src/{game_data.rs,shop_data.rs}` | nenhum | data parser tests | pendente | aguarda autorização de implementação |
| F3.S1.T6 | todo | `feature-assets` | `port_rust/crates/mu_assets/tests/runtime_dependencies.rs` | nenhum | dependency test | pendente | aguarda autorização de implementação |
| F3.S2.T1 | todo | `feature-ui` | `port_rust/crates/mu_ui/src/i18n.rs`, `port_rust/crates/mu_assets/src/translations.rs` | nenhum | missing-key/fallback tests | pendente | aguarda autorização de implementação |
| F3.S2.T2 | todo | `feature-audio` | `port_rust/crates/mu_assets/src/audio.rs`, `port_rust/crates/mu_audio/src/assets.rs` | nenhum | audio asset tests | pendente | aguarda autorização de implementação |

Validation Gate F3: todo.

### F4. Config, logging and app services

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| F4.S1.T1 | todo | `feature-config` | `port_rust/crates/mu_app/src/config.rs`, `port_rust/crates/mu_input/src/bindings.rs`, `port_rust/crates/mu_audio/src/settings.rs` | nenhum | roundtrip tests | pendente | aguarda autorização de implementação |
| F4.S1.T2 | todo | `feature-config` | `port_rust/crates/mu_app/src/cli.rs`, `port_rust/crates/mu_app/src/config.rs` | nenhum | precedence tests | pendente | aguarda autorização de implementação |
| F4.S2.T1 | todo | `feature-observability` | `port_rust/crates/mu_app/src/logging.rs`, `port_rust/crates/mu_network/src/redaction.rs` | nenhum | redaction tests | pendente | aguarda autorização de implementação |
| F4.S2.T2 | todo | `feature-observability` | `port_rust/crates/mu_core/src/error.rs`, `port_rust/crates/mu_ui/src/error.rs` | nenhum | catalog tests | pendente | aguarda autorização de implementação |

Validation Gate F4: todo.

### F5. Protocol and network

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| F5.S1.T1 | todo | `feature-network` | `port_rust/crates/mu_protocol/src/{codec.rs,frame.rs,error.rs}` | nenhum | golden tests | pendente | aguarda autorização de implementação |
| F5.S1.T2 | todo | `feature-network` | `port_rust/crates/mu_protocol/src/{connect.rs,chat.rs,session.rs,login.rs}` | nenhum | golden tests | pendente | aguarda autorização de implementação |
| F5.S1.T3 | todo | `feature-network` | `port_rust/crates/mu_protocol/src/{character.rs,world.rs,movement.rs}` | nenhum | golden/property tests | pendente | aguarda autorização de implementação |
| F5.S1.T4 | todo | `feature-network` | `port_rust/crates/mu_protocol/src/{items.rs,vault.rs,trade.rs,player_shop.rs}` | nenhum | property + golden tests | pendente | aguarda autorização de implementação |
| F5.S1.T5 | todo | `feature-network` | `port_rust/crates/mu_protocol/src/{combat.rs,skills.rs,pets.rs}` | nenhum | golden tests | pendente | aguarda autorização de implementação |
| F5.S1.T6 | todo | `feature-network` | `port_rust/crates/mu_protocol/src/{guild.rs,social.rs,quests.rs,events.rs}` | nenhum | golden tests | pendente | aguarda autorização de implementação |
| F5.S1.T7 | todo | `feature-network` | `port_rust/crates/mu_protocol/src/{cash_shop.rs,mu_helper.rs,admin.rs}` | nenhum | golden tests | pendente | aguarda autorização de implementação |
| F5.S2.T1 | todo | `feature-network` | `port_rust/crates/mu_network/src/{transport.rs,client.rs,fake_server.rs}` | nenhum | integration tests | pendente | aguarda autorização de implementação |
| F5.S2.T2 | todo | `feature-network` | `port_rust/crates/mu_app/src/session_state.rs`, `port_rust/crates/mu_network/src/session.rs` | nenhum | fake server tests + logs | pendente | aguarda autorização de implementação |

Validation Gate F5: todo.

### F6. UI shell, options, HUD and audio

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| F6.S1.T1 | todo | `feature-ui` | `port_rust/crates/mu_ui/src/{lib.rs,routes.rs,widgets.rs,layout.rs}` | nenhum | route tests | pendente | aguarda autorização de implementação |
| F6.S1.T2 | todo | `feature-ui` | `port_rust/crates/mu_ui/src/{login.rs,server_select.rs,options.rs,messages.rs}` | nenhum | snapshots | pendente | aguarda autorização de implementação |
| F6.S1.T3 | todo | `feature-ui` | `port_rust/crates/mu_ui/src/character_select.rs` | nenhum | fake server + screenshots | pendente | aguarda autorização de implementação |
| F6.S2.T1 | todo | `feature-ui` | `port_rust/crates/mu_ui/src/{hud.rs,chat.rs,minimap.rs,hotkeys.rs}` | nenhum | visual snapshots | pendente | aguarda autorização de implementação |
| F6.S2.T2 | todo | `feature-audio` | `port_rust/crates/mu_audio/src/{lib.rs,diagnostics.rs,events.rs}` | nenhum | audio tests/logs | pendente | aguarda autorização de implementação |

Validation Gate F6: todo.

### F7. World, render, input and entities

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| F7.S1.T1 | todo | `feature-render` | `port_rust/crates/mu_render/src/{assets.rs,materials.rs,models.rs,textures.rs}` | nenhum | render smoke | pendente | aguarda autorização de implementação |
| F7.S1.T2 | todo | `feature-world` | `port_rust/crates/mu_render/src/terrain.rs`, `port_rust/crates/mu_gameplay/src/world.rs`, `port_rust/crates/mu_assets/src/map.rs` | nenhum | screenshot + terrain tests | pendente | aguarda autorização de implementação |
| F7.S1.T3 | todo | `feature-input-world` | `port_rust/crates/mu_input/src/{movement.rs,camera.rs}`, `port_rust/crates/mu_gameplay/src/movement.rs` | nenhum | replay/integration tests | pendente | aguarda autorização de implementação |
| F7.S2.T1 | todo | `feature-world` | `port_rust/crates/mu_gameplay/src/{entities.rs,npcs.rs,monsters.rs}`, `port_rust/crates/mu_render/src/entities.rs` | nenhum | e2e screenshot | pendente | aguarda autorização de implementação |
| F7.S2.T2 | todo | `feature-render` | `port_rust/crates/mu_render/src/{effects.rs,particles.rs}` | nenhum | visual fixture | pendente | aguarda autorização de implementação |

Validation Gate F7: todo.

### F8. Gameplay systems

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| F8.S1.T1 | todo | `feature-gameplay` | `port_rust/crates/mu_gameplay/src/{characters.rs,stats.rs,classes.rs,master_level.rs}` | nenhum | golden tests | pendente | aguarda autorização de implementação |
| F8.S1.T2 | todo | `feature-combat` | `port_rust/crates/mu_gameplay/src/{combat.rs,buffs.rs,experience.rs}` | nenhum | formula/golden tests | pendente | aguarda autorização de implementação |
| F8.S1.T3 | todo | `feature-combat` | `port_rust/crates/mu_gameplay/src/skills.rs`, `port_rust/crates/mu_render/src/effects.rs`, `port_rust/crates/mu_audio/src/events.rs` | nenhum | replay + visual/audio evidence | pendente | aguarda autorização de implementação |
| F8.S2.T1 | todo | `feature-items` | `port_rust/crates/mu_gameplay/src/{items.rs,inventory.rs,equipment.rs,vault.rs}`, `port_rust/crates/mu_ui/src/inventory.rs` | nenhum | property + fake server tests | pendente | aguarda autorização de implementação |
| F8.S2.T2 | todo | `feature-npc` | `port_rust/crates/mu_gameplay/src/npc.rs`, `port_rust/crates/mu_ui/src/{npc.rs,shop.rs}` | nenhum | integration tests | pendente | aguarda autorização de implementação |
| F8.S2.T3 | todo | `feature-social` | `port_rust/crates/mu_gameplay/src/{trade.rs,player_shop.rs,mail.rs}`, UI files | nenhum | fake server tests | pendente | aguarda autorização de implementação |
| F8.S3.T1 | todo | `feature-social` | `port_rust/crates/mu_gameplay/src/{party.rs,friend.rs,guild.rs}`, UI files | nenhum | fake server tests | pendente | aguarda autorização de implementação |
| F8.S3.T2 | todo | `feature-events` | `port_rust/crates/mu_gameplay/src/{quests.rs,events.rs,duel.rs,gens.rs}`, UI files | nenhum | integration + e2e fixtures | pendente | aguarda autorização de implementação |
| F8.S3.T3 | todo | `feature-gameplay` | `port_rust/crates/mu_gameplay/src/{pets.rs,summons.rs,mounts.rs}` | nenhum | fake server + visual tests | pendente | aguarda autorização de implementação |

Validation Gate F8: todo.

### F9. MU Helper and GameShop

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| F9.S1.T1 | todo | `feature-helper` | `port_rust/crates/mu_gameplay/src/mu_helper.rs`, `port_rust/crates/mu_protocol/src/mu_helper.rs` | nenhum | rules/property tests | pendente | aguarda autorização de implementação |
| F9.S1.T2 | todo | `feature-helper` | `port_rust/crates/mu_ui/src/mu_helper.rs`, `port_rust/crates/mu_gameplay/src/mu_helper_runtime.rs` | nenhum | fake server + screenshots | pendente | aguarda autorização de implementação |
| F9.S2.T1 | todo | `feature-gameshop` | `port_rust/crates/mu_gameplay/src/game_shop.rs`, `port_rust/crates/mu_ui/src/game_shop.rs` | nenhum | fake server tests | pendente | aguarda autorização de implementação |
| F9.S2.T2 | todo | `feature-gameshop` | `port_rust/crates/mu_gameplay/src/game_shop_transaction.rs`, `port_rust/crates/mu_network/src/shop.rs` | nenhum | idempotency tests + redacted logs | pendente | aguarda autorização de implementação |

Validation Gate F9: todo.

### F10. Editor/admin equivalent

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| F10.S1.T1 | todo | `feature-editor-admin` | `port_rust/crates/mu_editor_admin/src/auth.rs`, `port_rust/crates/mu_ui/src/admin.rs` | nenhum | auth tests + screenshots | pendente | aguarda autorização de implementação |
| F10.S1.T2 | todo | `feature-editor-admin` | `port_rust/crates/mu_editor_admin/src/{core.rs,console.rs,dev_editor.rs}`, UI files | nenhum | UI tests | pendente | aguarda autorização de implementação |
| F10.S2.T1 | todo | `feature-editor-admin` | `port_rust/crates/mu_editor_admin/src/item_editor.rs`, `port_rust/crates/mu_ui/src/admin_item_editor.rs` | nenhum | fake server + audit log | pendente | aguarda autorização de implementação |
| F10.S2.T2 | todo | `feature-editor-admin` | `port_rust/crates/mu_editor_admin/src/skill_editor.rs`, `port_rust/crates/mu_ui/src/admin_skill_editor.rs` | nenhum | fake server + audit log | pendente | aguarda autorização de implementação |
| F10.S2.T3 | todo | `feature-docs` | `docs/admin-editor-rust.md`, `docs/dev-editor.md` if needed | nenhum | docs review + screenshot | pendente | aguarda autorização de implementação |

Validation Gate F10: todo.

### F11. Packaging, docs and release parity

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| F11.S1.T1 | todo | `feature-release` | `.github/workflows/rust-client-windows.yml`, packaging scripts if needed | nenhum | artifact + checksum | pendente | aguarda autorização de implementação |
| F11.S1.T2 | todo | `feature-release` | `.github/workflows/rust-client.yml`, docs | nenhum | CI status | pendente | aguarda autorização de implementação |
| F11.S2.T1 | todo | `feature-product-qa` | `docs/rust-client.md` | nenhum | difference table | pendente | aguarda autorização de implementação |
| F11.S2.T2 | todo | `feature-docs` | `docs/player-rust-client.md`, `docs/rust-client.md`, `docs/build-guide.md` | nenhum | smoke following docs | pendente | aguarda autorização de implementação |
| F11.S3.T1 | todo | `e2e-validator` | evidence artifacts under `MU_E2E_ARTIFACT_DIR` | nenhum | e2e-validator report | pendente | aguarda autorização de implementação |

Validation Gate F11: todo.

## Arquivos tocados

Novos:

- `.features/20260518-0000-rust-bevy-port/spec.md` — representado por `DOC.SPEC`.
- `.features/20260518-0000-rust-bevy-port/plan.md` — representado por `DOC.PLAN`.
- `port_rust/Cargo.toml` — representado por `F1.S1.T1` e `F1.S1.T2`.
- `port_rust/Cargo.lock` — gerado por `F1.S1.T1` e atualizado por `F1.S1.T3`.
- `port_rust/rust-toolchain.toml` — representado por `F1.S1.T1`.
- `.github/workflows/rust-client.yml` — representado por `F1.S2.T1`.
- `.github/workflows/rust-client-windows.yml` — representado por `F1.S2.T2` e `F1.S2.T3`.
- `port_rust/crates/mu_client/{Cargo.toml,src/lib.rs}` — representado por `F1.S1.T1` e `F1.S1.T2`.
- `port_rust/crates/mu_client/src/main.rs` — representado por `F1.S1.T3`.
- `port_rust/crates/mu_app/{Cargo.toml,src/lib.rs}` — representado por `F1.S1.T1` e `F1.S1.T2`.
- `port_rust/crates/mu_app/src/cli.rs` — representado por `F1.S1.T3`.
- `port_rust/crates/mu_app/src/state.rs` — representado por `F1.S1.T3`.
- `port_rust/crates/mu_core/{Cargo.toml,src/lib.rs}` — representado por `F1.S1.T1` e `F1.S1.T2`.
- `port_rust/crates/mu_protocol/{Cargo.toml,src/lib.rs}` — representado por `F1.S1.T1` e `F1.S1.T2`.
- `port_rust/crates/mu_network/{Cargo.toml,src/lib.rs}` — representado por `F1.S1.T1` e `F1.S1.T2`.
- `port_rust/crates/mu_assets/{Cargo.toml,src/lib.rs}` — representado por `F1.S1.T1` e `F1.S1.T2`.
- `port_rust/crates/mu_asset_pipeline/{Cargo.toml,src/lib.rs}` — representado por `F1.S1.T1` e `F1.S1.T2`.
- `port_rust/crates/mu_asset_pipeline/src/inventory.rs` — representado por `F2.S1.T1`.
- `port_rust/crates/mu_asset_pipeline/src/main.rs` — representado por `F2.S1.T1`.
- `tmp/asset-inventory/data-report.txt` — artefato local ignorado, representado por `F2.S1.T1`.
- `port_rust/crates/mu_render/{Cargo.toml,src/lib.rs}` — representado por `F1.S1.T1` e `F1.S1.T2`.
- `port_rust/crates/mu_ui/{Cargo.toml,src/lib.rs}` — representado por `F1.S1.T1` e `F1.S1.T2`.
- `port_rust/crates/mu_audio/{Cargo.toml,src/lib.rs}` — representado por `F1.S1.T1` e `F1.S1.T2`.
- `port_rust/crates/mu_input/{Cargo.toml,src/lib.rs}` — representado por `F1.S1.T1` e `F1.S1.T2`.
- `port_rust/crates/mu_gameplay/{Cargo.toml,src/lib.rs}` — representado por `F1.S1.T1` e `F1.S1.T2`.
- `port_rust/crates/mu_editor_admin/{Cargo.toml,src/lib.rs}` — representado por `F1.S1.T1` e `F1.S1.T2`.
- `port_rust/crates/mu_test_support/{Cargo.toml,src/lib.rs}` — representado por `F1.S1.T1` e `F1.S1.T2`.

Modificados:

- `.features/20260518-0000-rust-bevy-port/progress.md` — representado por `DOC.PROGRESS`, `F1.S1.T1` e atualização de rastreabilidade.
- `.memory/RULES_AND_DEFINITION.md` — regra registrada: todo código do port Rust deve ficar em `port_rust/`.
- `.memory/TODO.md` — representado por `DOC.TODO` e nova entrada de pendência.

Removidos:

- nenhum

## Evidência de validação

| Comando/artefato | Resultado |
|---|---|
| `rtk sed -n '1,220p' AGENTS.md` | instruções do projeto lidas |
| `rtk sed -n '1,220p' docs/CODING_RULES.md` | regras do projeto lidas |
| `rtk sed -n '1,220p' /home/allanbatista/.codex/skills/feature-workflow/references/plan-checklist.md` | checklist do plano lido |
| `rtk sed -n '1,220p' /home/allanbatista/.codex/skills/feature-workflow/references/progress-checklist.md` | checklist de progresso lido |
| `rtk node /home/allanbatista/.codex/skills/feature-workflow/scripts/audit-feature-docs.mjs .features/20260518-0000-rust-bevy-port` | sem saída; auditoria estrutural passou |
| `rtk cargo metadata --manifest-path port_rust/Cargo.toml --no-deps --format-version 1` | passou; workspace com 14 crates listadas |
| `rtk cargo tree --manifest-path port_rust/Cargo.toml --workspace --depth 1` | passou; workspace lista 14 crates |
| `rtk rg -n "bevy\|serde\|thiserror\|tracing\|tokio\|clap\|toml\|camino\|insta\|proptest" port_rust/Cargo.toml port_rust/crates/*/Cargo.toml` | passou; política central registrada no `port_rust/Cargo.toml` raiz |
| `rtk cargo test --manifest-path port_rust/Cargo.toml --workspace` | passou; 5 testes, 33 suites de unidade/doc |
| `rtk cargo build --manifest-path port_rust/Cargo.toml -p mu_client` | passou |
| `rtk cargo clippy --manifest-path port_rust/Cargo.toml --workspace --all-targets -- -D warnings` | passou sem issues |
| `rtk cargo run --manifest-path port_rust/Cargo.toml -p mu_client -- --headless` | passou; saída `ready-for-login` |
| `rtk proxy bash -lc 'rtk cargo run --manifest-path port_rust/Cargo.toml -p mu_client -- --headless --asset-root __missing_mu_asset_root__; test $? -eq 3'` | passou; saída `asset-check-failed` e código 3 |
| `rtk cargo fmt --manifest-path port_rust/Cargo.toml --all --check` | passou |
| `rtk rg -n "cargo fmt --manifest-path port_rust/Cargo.toml --all --check\|cargo clippy --manifest-path port_rust/Cargo.toml --workspace --all-targets -- -D warnings\|cargo test --manifest-path port_rust/Cargo.toml --workspace\|cargo build --manifest-path port_rust/Cargo.toml -p mu_client" .github/workflows/rust-client.yml` | passou; workflow Linux contém os gates exigidos |
| `rtk python3 -c "import yaml; yaml.safe_load(open('.github/workflows/rust-client-windows.yml'))"` | passou; YAML Windows parseia localmente |
| `rtk rg -n "cargo build --manifest-path port_rust/Cargo.toml --release -p mu_client\|mu_client\\.exe\|sha256\|upload-artifact" .github/workflows/rust-client-windows.yml` | passou; workflow Windows contém release build, checksum e upload |
| `rtk git diff --name-only -- .github/workflows/mingw-build.yml .github/workflows/mingw-build-dev.yml .github/workflows/mingw-build-pr.yml` | passou sem saída; CI legado não foi alterado |
| `rtk proxy find .github/workflows -maxdepth 1 -type f` | passou; workflows MinGW e Rust coexistem |
| `rtk cargo test --manifest-path port_rust/Cargo.toml -p mu_asset_pipeline` | passou; 1 teste, 3 suites |
| `rtk proxy bash -lc 'rtk cargo run --manifest-path port_rust/Cargo.toml -p mu_asset_pipeline -- --source src/bin/Data --report tmp/asset-inventory/data-report.txt; test $? -eq 4'` | passou; relatório gerado e código 4 validado por rejeições |
| `tmp/asset-inventory/data-report.txt` | relatório local: 13.168 arquivos, `.map` 90, `.att` 123, `.ozb` 48, `.obj` 56, `.bmd` 5308, `.smd` 3, `.ozj` 5730, `.ozt` 1751; 4 rejeições |
| `rtk rg -n "anomalous" tmp/asset-inventory/data-report.txt` | passou; rejeitou `World7/TerrainLight.OZJ2`, `World1/xxx`, `World1/EncTerrain1.att1`, `InGameShopScript/512.2011.006.rar` |
| `rtk rg ... stale root port paths ...` | passou sem saída; docs não apontam mais código do port fora de `port_rust/` |
| `rtk git status --short --untracked-files=all` | mostra `.features/20260518-0000-rust-bevy-port/{plan.md,progress.md}`, `.memory/{RULES_AND_DEFINITION.md,TODO.md}`, os workflows Rust, `port_rust/Cargo.toml`, `port_rust/Cargo.lock`, `port_rust/rust-toolchain.toml` e os crates novos do workspace |

## Contagem de tarefas

- done: 4 documentais + 6 implementação
- doing: 0
- todo: 59 implementação
- blocked: 1
- failed: 0

## Pendências e evidências ausentes

- F1.S1.T1 concluída com skeleton do workspace.
- F1.S1.T2 concluída com política central de dependências.
- F1.S1.T3 concluída com app state shell headless.
- F1.S2.T1 concluída com workflow Linux Rust CI.
- F1.S2.T2 tem workflow Windows x64 criado, mas falta evidência de GitHub Actions.
- F1.S2.T3 concluída; workflows Rust coexistem com os workflows MinGW existentes.
- F2.S1.T1 concluída com scanner e relatório local de inventário de assets.
- Nenhuma fase jogável foi iniciada.
- Próxima tarefa executável local é `F2.S1.T2`.
- Evidências já disponíveis: `cargo metadata`, `cargo tree --manifest-path port_rust/Cargo.toml --workspace --depth 1`, `cargo test --manifest-path port_rust/Cargo.toml --workspace`, `cargo build --manifest-path port_rust/Cargo.toml -p mu_client`, `cargo clippy --manifest-path port_rust/Cargo.toml --workspace --all-targets -- -D warnings`, `cargo fmt --manifest-path port_rust/Cargo.toml --all --check`, smokes headless, `rg` da política central, `port_rust/Cargo.lock` e relatório local de inventário.
