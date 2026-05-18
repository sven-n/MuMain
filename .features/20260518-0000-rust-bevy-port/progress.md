# Port completo Rust + Bevy do cliente MU

Status atual: PLANO_CONCLUIDO_SEM_IMPLEMENTACAO

Próximo passo concreto: iniciar `F1.S1.T1 Workspace skeleton` somente após autorização explícita para escrever código.

Sincronização com git: `rtk git status --short --untracked-files=all` mostra somente `.features/20260518-0000-rust-bevy-port/{spec.md,plan.md,progress.md}` e `.memory/TODO.md`. Todos estão representados em "Arquivos tocados".

## Controle documental

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| DOC.SPEC | done | `feature-spec` | `.features/20260518-0000-rust-bevy-port/spec.md` | `.features/20260518-0000-rust-bevy-port/spec.md` | `spec.md` em `READY_FOR_PLAN` | Spec existente validado como contrato de produto | nenhum |
| DOC.PLAN | done | `feature-plan` | `.features/20260518-0000-rust-bevy-port/plan.md` | `.features/20260518-0000-rust-bevy-port/plan.md` | Plano completo com inventários, contratos, fases, tarefas, gates e AC traceability | Plano atualizado com classificação explícita de fontes/dependências legadas; sem código escrito | nenhum |
| DOC.PROGRESS | done | `feature-progress` | `.features/20260518-0000-rust-bevy-port/progress.md` | `.features/20260518-0000-rust-bevy-port/progress.md` | Progresso sincronizado com task tree do plano | Este arquivo atualizado | nenhum |
| DOC.TODO | done | local | `.memory/TODO.md` | `.memory/TODO.md` | Pendência de implementação registrada por regra do repo | Entrada adicionada | nenhum |

## Árvore executável

Todas as tarefas de implementação estão `todo`. Nenhum código, workflow, crate, fixture runtime ou documentação de uso fora de `.features/` foi implementado nesta etapa.

### F1. Rust foundation and gates

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| F1.S1.T1 | todo | `feature-rust-foundation` | `rust-toolchain.toml`, `Cargo.toml`, `Cargo.lock`, `crates/*/Cargo.toml`, `crates/*/src/lib.rs` | nenhum | `cargo metadata --no-deps --format-version 1` | pendente | aguarda autorização de implementação |
| F1.S1.T2 | todo | `feature-rust-foundation` | `Cargo.toml`, crate manifests | nenhum | `cargo tree --workspace --depth 1` | pendente | aguarda autorização de implementação |
| F1.S1.T3 | todo | `feature-rust-foundation` | `crates/mu_client/src/main.rs`, `crates/mu_app/src/{lib.rs,state.rs,cli.rs}` | nenhum | unit tests + headless smoke | pendente | aguarda autorização de implementação |
| F1.S2.T1 | todo | `feature-ci` | `.github/workflows/rust-client.yml` | nenhum | local equivalent or Actions log | pendente | aguarda autorização de implementação |
| F1.S2.T2 | todo | `feature-ci` | `.github/workflows/rust-client-windows.yml` | nenhum | artifact/checksum + release build log | pendente | aguarda autorização de implementação |
| F1.S2.T3 | todo | `feature-ci` | `.github/workflows/*.yml`, docs notes if needed | nenhum | CI matrix evidence | pendente | aguarda autorização de implementação |

Validation Gate F1: todo.

### F2. Inventory, fixtures and comparison harness

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| F2.S1.T1 | todo | `feature-assets` | `crates/mu_asset_pipeline/src/{inventory.rs,main.rs}` | nenhum | fixture report matching Asset Inventory | pendente | aguarda autorização de implementação |
| F2.S1.T2 | todo | `feature-network` | `crates/mu_protocol/tests/fixtures/**`, `tests/rust/protocol_inventory.rs` | nenhum | fixture manifest checked into tests | pendente | aguarda autorização de implementação |
| F2.S1.T3 | todo | `feature-test-support` | `crates/mu_test_support/src/{legacy.rs,fixtures.rs,evidence.rs}` | nenhum | harness unit tests | pendente | aguarda autorização de implementação |
| F2.S1.T4 | todo | `feature-ui` | `tests/rust/ui_fixtures/**`, `crates/mu_ui/src/fixture_routes.rs` | nenhum | snapshot manifest | pendente | aguarda autorização de implementação |
| F2.S1.T5 | todo | `feature-test-support` | `crates/mu_test_support/src/source_inventory.rs`, `tests/rust/source_inventory.rs`, `docs/rust-client.md` | nenhum | source inventory report + test | pendente | aguarda autorização de implementação |
| F2.S2.T1 | todo | `feature-test-support` | `docs/rust-client.md`, `crates/mu_test_support/src/evidence.rs` | nenhum | sample evidence output | pendente | aguarda autorização de implementação |

Validation Gate F2: todo.

### F3. Assets and localization

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| F3.S1.T1 | todo | `feature-assets` | `crates/mu_assets/src/{manifest.rs,validation.rs,runtime.rs}`, tests | nenhum | unit/integration tests | pendente | aguarda autorização de implementação |
| F3.S1.T2 | todo | `feature-assets` | `crates/mu_asset_pipeline/src/{textures.rs,manifest_writer.rs}` | nenhum | converter tests with sample fixtures | pendente | aguarda autorização de implementação |
| F3.S1.T3 | todo | `feature-assets` | `crates/mu_asset_pipeline/src/{models.rs,bmd.rs,smd.rs,obj.rs}` | nenhum | golden model fixture | pendente | aguarda autorização de implementação |
| F3.S1.T4 | todo | `feature-assets` | `crates/mu_asset_pipeline/src/{terrain.rs,map.rs,att.rs,ozb.rs}` | nenhum | terrain fixture + checksum | pendente | aguarda autorização de implementação |
| F3.S1.T5 | todo | `feature-assets` | `crates/mu_asset_pipeline/src/{game_data.rs,shop_data.rs}` | nenhum | data parser tests | pendente | aguarda autorização de implementação |
| F3.S1.T6 | todo | `feature-assets` | `crates/mu_assets/tests/runtime_dependencies.rs` | nenhum | dependency test | pendente | aguarda autorização de implementação |
| F3.S2.T1 | todo | `feature-ui` | `crates/mu_ui/src/i18n.rs`, `crates/mu_assets/src/translations.rs` | nenhum | missing-key/fallback tests | pendente | aguarda autorização de implementação |
| F3.S2.T2 | todo | `feature-audio` | `crates/mu_assets/src/audio.rs`, `crates/mu_audio/src/assets.rs` | nenhum | audio asset tests | pendente | aguarda autorização de implementação |

Validation Gate F3: todo.

### F4. Config, logging and app services

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| F4.S1.T1 | todo | `feature-config` | `crates/mu_app/src/config.rs`, `crates/mu_input/src/bindings.rs`, `crates/mu_audio/src/settings.rs` | nenhum | roundtrip tests | pendente | aguarda autorização de implementação |
| F4.S1.T2 | todo | `feature-config` | `crates/mu_app/src/cli.rs`, `crates/mu_app/src/config.rs` | nenhum | precedence tests | pendente | aguarda autorização de implementação |
| F4.S2.T1 | todo | `feature-observability` | `crates/mu_app/src/logging.rs`, `crates/mu_network/src/redaction.rs` | nenhum | redaction tests | pendente | aguarda autorização de implementação |
| F4.S2.T2 | todo | `feature-observability` | `crates/mu_core/src/error.rs`, `crates/mu_ui/src/error.rs` | nenhum | catalog tests | pendente | aguarda autorização de implementação |

Validation Gate F4: todo.

### F5. Protocol and network

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| F5.S1.T1 | todo | `feature-network` | `crates/mu_protocol/src/{codec.rs,frame.rs,error.rs}` | nenhum | golden tests | pendente | aguarda autorização de implementação |
| F5.S1.T2 | todo | `feature-network` | `crates/mu_protocol/src/{connect.rs,chat.rs,session.rs,login.rs}` | nenhum | golden tests | pendente | aguarda autorização de implementação |
| F5.S1.T3 | todo | `feature-network` | `crates/mu_protocol/src/{character.rs,world.rs,movement.rs}` | nenhum | golden/property tests | pendente | aguarda autorização de implementação |
| F5.S1.T4 | todo | `feature-network` | `crates/mu_protocol/src/{items.rs,vault.rs,trade.rs,player_shop.rs}` | nenhum | property + golden tests | pendente | aguarda autorização de implementação |
| F5.S1.T5 | todo | `feature-network` | `crates/mu_protocol/src/{combat.rs,skills.rs,pets.rs}` | nenhum | golden tests | pendente | aguarda autorização de implementação |
| F5.S1.T6 | todo | `feature-network` | `crates/mu_protocol/src/{guild.rs,social.rs,quests.rs,events.rs}` | nenhum | golden tests | pendente | aguarda autorização de implementação |
| F5.S1.T7 | todo | `feature-network` | `crates/mu_protocol/src/{cash_shop.rs,mu_helper.rs,admin.rs}` | nenhum | golden tests | pendente | aguarda autorização de implementação |
| F5.S2.T1 | todo | `feature-network` | `crates/mu_network/src/{transport.rs,client.rs,fake_server.rs}` | nenhum | integration tests | pendente | aguarda autorização de implementação |
| F5.S2.T2 | todo | `feature-network` | `crates/mu_app/src/session_state.rs`, `crates/mu_network/src/session.rs` | nenhum | fake server tests + logs | pendente | aguarda autorização de implementação |

Validation Gate F5: todo.

### F6. UI shell, options, HUD and audio

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| F6.S1.T1 | todo | `feature-ui` | `crates/mu_ui/src/{lib.rs,routes.rs,widgets.rs,layout.rs}` | nenhum | route tests | pendente | aguarda autorização de implementação |
| F6.S1.T2 | todo | `feature-ui` | `crates/mu_ui/src/{login.rs,server_select.rs,options.rs,messages.rs}` | nenhum | snapshots | pendente | aguarda autorização de implementação |
| F6.S1.T3 | todo | `feature-ui` | `crates/mu_ui/src/character_select.rs` | nenhum | fake server + screenshots | pendente | aguarda autorização de implementação |
| F6.S2.T1 | todo | `feature-ui` | `crates/mu_ui/src/{hud.rs,chat.rs,minimap.rs,hotkeys.rs}` | nenhum | visual snapshots | pendente | aguarda autorização de implementação |
| F6.S2.T2 | todo | `feature-audio` | `crates/mu_audio/src/{lib.rs,diagnostics.rs,events.rs}` | nenhum | audio tests/logs | pendente | aguarda autorização de implementação |

Validation Gate F6: todo.

### F7. World, render, input and entities

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| F7.S1.T1 | todo | `feature-render` | `crates/mu_render/src/{assets.rs,materials.rs,models.rs,textures.rs}` | nenhum | render smoke | pendente | aguarda autorização de implementação |
| F7.S1.T2 | todo | `feature-world` | `crates/mu_render/src/terrain.rs`, `crates/mu_gameplay/src/world.rs`, `crates/mu_assets/src/map.rs` | nenhum | screenshot + terrain tests | pendente | aguarda autorização de implementação |
| F7.S1.T3 | todo | `feature-input-world` | `crates/mu_input/src/{movement.rs,camera.rs}`, `crates/mu_gameplay/src/movement.rs` | nenhum | replay/integration tests | pendente | aguarda autorização de implementação |
| F7.S2.T1 | todo | `feature-world` | `crates/mu_gameplay/src/{entities.rs,npcs.rs,monsters.rs}`, `crates/mu_render/src/entities.rs` | nenhum | e2e screenshot | pendente | aguarda autorização de implementação |
| F7.S2.T2 | todo | `feature-render` | `crates/mu_render/src/{effects.rs,particles.rs}` | nenhum | visual fixture | pendente | aguarda autorização de implementação |

Validation Gate F7: todo.

### F8. Gameplay systems

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| F8.S1.T1 | todo | `feature-gameplay` | `crates/mu_gameplay/src/{characters.rs,stats.rs,classes.rs,master_level.rs}` | nenhum | golden tests | pendente | aguarda autorização de implementação |
| F8.S1.T2 | todo | `feature-combat` | `crates/mu_gameplay/src/{combat.rs,buffs.rs,experience.rs}` | nenhum | formula/golden tests | pendente | aguarda autorização de implementação |
| F8.S1.T3 | todo | `feature-combat` | `crates/mu_gameplay/src/skills.rs`, `crates/mu_render/src/effects.rs`, `crates/mu_audio/src/events.rs` | nenhum | replay + visual/audio evidence | pendente | aguarda autorização de implementação |
| F8.S2.T1 | todo | `feature-items` | `crates/mu_gameplay/src/{items.rs,inventory.rs,equipment.rs,vault.rs}`, `crates/mu_ui/src/inventory.rs` | nenhum | property + fake server tests | pendente | aguarda autorização de implementação |
| F8.S2.T2 | todo | `feature-npc` | `crates/mu_gameplay/src/npc.rs`, `crates/mu_ui/src/{npc.rs,shop.rs}` | nenhum | integration tests | pendente | aguarda autorização de implementação |
| F8.S2.T3 | todo | `feature-social` | `crates/mu_gameplay/src/{trade.rs,player_shop.rs,mail.rs}`, UI files | nenhum | fake server tests | pendente | aguarda autorização de implementação |
| F8.S3.T1 | todo | `feature-social` | `crates/mu_gameplay/src/{party.rs,friend.rs,guild.rs}`, UI files | nenhum | fake server tests | pendente | aguarda autorização de implementação |
| F8.S3.T2 | todo | `feature-events` | `crates/mu_gameplay/src/{quests.rs,events.rs,duel.rs,gens.rs}`, UI files | nenhum | integration + e2e fixtures | pendente | aguarda autorização de implementação |
| F8.S3.T3 | todo | `feature-gameplay` | `crates/mu_gameplay/src/{pets.rs,summons.rs,mounts.rs}` | nenhum | fake server + visual tests | pendente | aguarda autorização de implementação |

Validation Gate F8: todo.

### F9. MU Helper and GameShop

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| F9.S1.T1 | todo | `feature-helper` | `crates/mu_gameplay/src/mu_helper.rs`, `crates/mu_protocol/src/mu_helper.rs` | nenhum | rules/property tests | pendente | aguarda autorização de implementação |
| F9.S1.T2 | todo | `feature-helper` | `crates/mu_ui/src/mu_helper.rs`, `crates/mu_gameplay/src/mu_helper_runtime.rs` | nenhum | fake server + screenshots | pendente | aguarda autorização de implementação |
| F9.S2.T1 | todo | `feature-gameshop` | `crates/mu_gameplay/src/game_shop.rs`, `crates/mu_ui/src/game_shop.rs` | nenhum | fake server tests | pendente | aguarda autorização de implementação |
| F9.S2.T2 | todo | `feature-gameshop` | `crates/mu_gameplay/src/game_shop_transaction.rs`, `crates/mu_network/src/shop.rs` | nenhum | idempotency tests + redacted logs | pendente | aguarda autorização de implementação |

Validation Gate F9: todo.

### F10. Editor/admin equivalent

| ID | Status | Owner/subagent | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker/cause |
|---|---|---|---|---|---|---|---|
| F10.S1.T1 | todo | `feature-editor-admin` | `crates/mu_editor_admin/src/auth.rs`, `crates/mu_ui/src/admin.rs` | nenhum | auth tests + screenshots | pendente | aguarda autorização de implementação |
| F10.S1.T2 | todo | `feature-editor-admin` | `crates/mu_editor_admin/src/{core.rs,console.rs,dev_editor.rs}`, UI files | nenhum | UI tests | pendente | aguarda autorização de implementação |
| F10.S2.T1 | todo | `feature-editor-admin` | `crates/mu_editor_admin/src/item_editor.rs`, `crates/mu_ui/src/admin_item_editor.rs` | nenhum | fake server + audit log | pendente | aguarda autorização de implementação |
| F10.S2.T2 | todo | `feature-editor-admin` | `crates/mu_editor_admin/src/skill_editor.rs`, `crates/mu_ui/src/admin_skill_editor.rs` | nenhum | fake server + audit log | pendente | aguarda autorização de implementação |
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
- `.features/20260518-0000-rust-bevy-port/progress.md` — representado por `DOC.PROGRESS`.
- `.memory/TODO.md` — representado por `DOC.TODO`.

Modificados:

- nenhum

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
| `rtk git status --short --untracked-files=all` | somente `.features/20260518-0000-rust-bevy-port/{spec.md,plan.md,progress.md}` e `.memory/TODO.md` aparecem |

## Contagem de tarefas

- done: 4 documentais
- doing: 0
- todo: 66 implementação
- blocked: 0
- failed: 0

## Pendências e evidências ausentes

- Implementação não iniciada por solicitação explícita.
- Nenhuma evidência de build/test/runtime/e2e existe ainda.
- Próxima tarefa executável é `F1.S1.T1`.
