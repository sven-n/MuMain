# Port Rust incremental em `port_rust`

Status: READY_FOR_EXEC

## Premissas Confirmadas

- `spec.md` esta em `READY_FOR_PLAN` e nao tem pergunta de produto bloqueante.
- A pilha atual do repositorio continua sendo CMake 3.25+, C++20, MinGW/MSVC, doctest/CTest e .NET `net10.0` para `ClientLibrary`; `ConstantsReplacer` usa `net8.0-windows`.
- Nao ha workspace Rust existente no repositorio.
- Esta entrega inicial cria somente a area paralela `port_rust/`; nao altera `ClientLibrary/`, `ConstantsReplacer/`, `src/` ou `tests/`.
- Nao sera adicionada dependencia externa de runtime nem CI novo nesta fase; a validacao Rust sera local via Cargo.
- A regra duravel exigida e: antes de comecar cada fase, passar por uma etapa de planejamento individual.

## Approach

Criar um workspace Rust minimo e isolado em `port_rust/`, com uma crate `port_rust` sem dependencias externas, contendo biblioteca pequena e CLI de status. A CLI existe apenas como sinal de saude observavel e deve comunicar que o port foi inicializado, e que ainda nao e jogavel.

Documentar o uso inicial em `port_rust/README.md` e criar `port_rust/docs/inventory.md` com todas as entradas obrigatorias do spec, status inicial, evidencia exigida e regra de que nada pode ser marcado como portado sem evidencia.

## Affected Files

Arquivos alterados por esta etapa de planejamento:

- `.features/20260518-1516-port-rust/plan.md`

Arquivos planejados para implementacao:

- `port_rust/Cargo.toml`
- `port_rust/Cargo.lock`
- `port_rust/README.md`
- `port_rust/docs/inventory.md`
- `port_rust/crates/port_rust/Cargo.toml`
- `port_rust/crates/port_rust/src/lib.rs`
- `port_rust/crates/port_rust/src/main.rs`

## Interfaces / Contracts

- Workspace local: `port_rust/Cargo.toml` define um workspace Cargo com `resolver = "2"` e membro `crates/port_rust`.
- Crate: pacote `port_rust`, edition `2021`, sem dependencias externas.
- CLI local: `cargo run -p port_rust -- --status` deve sair com codigo `0` e imprimir uma mensagem estavel contendo `initialized`, `incremental` e `not playable`.
- Biblioteca: `port_rust::status_message() -> &'static str` retorna a mesma mensagem base usada pela CLI.
- Documentacao: `port_rust/README.md` deve registrar objetivo, nao-jogabilidade, comandos de validacao, isolamento em `port_rust/` e gate de planejamento por fase.
- Inventario: `port_rust/docs/inventory.md` deve conter tabela Markdown com colunas `Entrada`, `Status inicial`, `Evidencia exigida`, `Observacao`.
- Status permitidos no inventario: `a portar`, `referencia`, `suporte de validacao`, `fora do runtime`, `pendente de decisao`.
- Status `portada` nao e permitido nesta entrega inicial.
- Sem mudanca de API publica, protocolo de rede, schema, auth/autorizacao, billing, banco, assets runtime ou contrato externo.

## Inventario Tecnico

| Slug | Componentes | Tipo de saida | Consultas | Serializacao/filtros | Dataset/permissao | Renderer/test target | Retailer/industry compatibility |
|---|---|---|---|---|---|---|---|
| `port-rust-workspace` | `port_rust/Cargo.toml`, `crates/port_rust/Cargo.toml` | Workspace Cargo local | Nenhuma consulta externa | Manifest Cargo TOML | Leitura/escrita em `port_rust/` | `cargo metadata`, `cargo test` | Coexiste com CMake/.NET legado |
| `port-rust-status-cli` | `src/lib.rs`, `src/main.rs` | Texto CLI | Nenhuma | Flag local `--status` | Sem dados sensiveis | `cargo run -p port_rust -- --status` | Apenas sinal de saude, nao jogavel |
| `legacy-port-inventory` | `port_rust/docs/inventory.md` | Tabela Markdown | Leitura visual dos caminhos obrigatorios | Status controlado por lista fechada | Caminhos do repo como referencia | Checklist + `rg` | Cobre entradas do spec sem marcar portado |
| `phase-planning-gate` | `port_rust/README.md`, plano/progresso da feature | Regra operacional | Nenhuma | Fase, objetivo, escopo, validacao, evidencias | Revisao humana/agentica antes de S2+ | Checklist + `e2e-validator` final | Bloqueia fases futuras sem plano |

Entradas obrigatorias do inventario:

| Entrada | Status inicial esperado | Evidencia exigida antes de virar portado |
|---|---|---|
| `.github/` | `suporte de validacao` | CI Rust especifico ou decisao documentada |
| `.gemini/` | `fora do runtime` | Evidencia de uso ou decisao de descarte |
| `ClientLibrary/` | `referencia` | Testes/provas de equivalencia de rede |
| `ConstantsReplacer/` | `referencia` | Decisao sobre substituicao ou descarte |
| `cmake/` | `suporte de validacao` | Decisao de coexistencia/remocao futura |
| `docs/` | `referencia` | Docs Rust atualizados quando houver superficie |
| `src/` | `referencia` | Inventario granular e validacao por area |
| `tests/` | `suporte de validacao` | Equivalente Rust ou fixture comparativa |
| `.editorconfig` | `suporte de validacao` | Politica Rust equivalente ou reutilizacao |
| `.gitattributes` | `suporte de validacao` | Decisao de atributos para artefatos Rust |
| `.gitignore` | `suporte de validacao` | Regras para `target/` se necessario |
| `.gitmodules` | `referencia` | Decisao sobre submodulos/assets futuros |
| `AGENTS.md` | `referencia` | Regras aplicadas ao port |
| `CLAUDE.md` | `referencia` | Confirmacao de apontamento para `AGENTS.md` |
| `CMakeLists.txt` | `referencia` | Decisao de coexistencia com Cargo |
| `CMakePresets.json` | `referencia` | Decisao de equivalencia de presets |
| `README.md` | `referencia` | README Rust alinhado ao status real |
| `TRANSLATION_SYSTEM_INTEGRATION.md` | `referencia` | Plano futuro de i18n Rust |
| `stylecop.json` | `fora do runtime` | Decisao de nao aplicabilidade ou equivalente |
| `toolchain-x64.cmake` | `referencia` | Decisao de toolchain Rust Windows x64 futura |
| `toolchain-x86.cmake` | `referencia` | Decisao sobre suporte x86 futuro |

## Gate Obrigatorio De Fase

Antes de qualquer tarefa de implementacao em cada fase, executar a tarefa `S1.T1` da propria fase. Ela deve registrar objetivo, escopo, tarefas, arquivos planejados, validacao e evidencias esperadas daquela fase. Nenhuma tarefa `S2+` pode comecar sem esse registro aprovado.

## Phases / Task Breakdown

### F1. Workspace Rust minimo

| ID | Owner/subagent | Planned files | Dependencies | Parallelizable | Done when | Required evidence |
|---|---|---|---|---|---|---|
| F1.S1.T1 Planejamento individual da F1 | `feature-progress` | Documento/progresso operacional da feature | Nenhuma | Nao | Escopo da F1, arquivos e comandos aceitos antes de editar `port_rust/` | Registro do gate F1 aprovado |
| F1.S2.T1 Criar workspace Cargo | `rust-port` | `port_rust/Cargo.toml` | F1.S1.T1 | Nao | Workspace tem `resolver = "2"` e membro `crates/port_rust` | `rtk cargo metadata --manifest-path port_rust/Cargo.toml --no-deps --format-version 1` |
| F1.S2.T2 Criar crate Rust std-only | `rust-port` | `port_rust/crates/port_rust/Cargo.toml`, `src/lib.rs`, `src/main.rs` | F1.S2.T1 | Nao | `status_message()` e CLI `--status` existem, sem crates externas | `rtk cargo test --manifest-path port_rust/Cargo.toml --workspace` |
| F1.S2.T3 Registrar lockfile do app | `rust-port` | `port_rust/Cargo.lock` | F1.S2.T2 | Nao | `Cargo.lock` gerado se Cargo o produzir para o workspace | `rtk test -f port_rust/Cargo.lock` ou nota se Cargo nao gerar |

Validation Gate F1:

- `rtk cargo fmt --manifest-path port_rust/Cargo.toml --check`
- `rtk cargo test --manifest-path port_rust/Cargo.toml --workspace`
- `rtk cargo run --manifest-path port_rust/Cargo.toml -p port_rust -- --status`
- Evidencia: saida da CLI contem `initialized`, `incremental`, `not playable`.

### F2. Documentacao e inventario inicial

| ID | Owner/subagent | Planned files | Dependencies | Parallelizable | Done when | Required evidence |
|---|---|---|---|---|---|---|
| F2.S1.T1 Planejamento individual da F2 | `feature-progress` | Documento/progresso operacional da feature | F1 gate aprovado | Nao | Escopo da F2, tabelas e comandos aceitos antes de editar docs | Registro do gate F2 aprovado |
| F2.S2.T1 Escrever README do port | `docs-port` | `port_rust/README.md` | F2.S1.T1 | Sim, com F2.S2.T2 | README explica status incremental, nao jogavel, comandos e isolamento | `rtk rg -n "incremental|not playable|nao jogavel|não jogável|planejamento individual" port_rust/README.md` |
| F2.S2.T2 Escrever inventario obrigatorio | `docs-port` | `port_rust/docs/inventory.md` | F2.S1.T1 | Sim, com F2.S2.T1 | Todas as 21 entradas obrigatorias existem com status permitido e evidencia exigida | Checklist de entradas + `rtk rg` |
| F2.S2.T3 Revisar regra anti-portado | `docs-port` | `port_rust/docs/inventory.md` | F2.S2.T2 | Nao | Nenhuma entrada usa `portada` nesta entrega | `rtk proxy bash -lc '! rg -n "\\bportada\\b" port_rust/docs/inventory.md'` |

Validation Gate F2:

- `rtk proxy bash -lc 'for p in ".github/" ".gemini/" "ClientLibrary/" "ConstantsReplacer/" "cmake/" "docs/" "src/" "tests/" ".editorconfig" ".gitattributes" ".gitignore" ".gitmodules" "AGENTS.md" "CLAUDE.md" "CMakeLists.txt" "CMakePresets.json" "README.md" "TRANSLATION_SYSTEM_INTEGRATION.md" "stylecop.json" "toolchain-x64.cmake" "toolchain-x86.cmake"; do rg -q "$p" port_rust/docs/inventory.md || exit 1; done'`
- `rtk rg -n "a portar|referencia|suporte de validacao|fora do runtime|pendente de decisao" port_rust/docs/inventory.md`
- Evidencia: inventario lista todas as entradas obrigatorias e nao marca nada como portado.

### F3. Validacao final e isolamento

| ID | Owner/subagent | Planned files | Dependencies | Parallelizable | Done when | Required evidence |
|---|---|---|---|---|---|---|
| F3.S1.T1 Planejamento individual da F3 | `feature-progress` | Documento/progresso operacional da feature | F1-F2 completos | Nao | Escopo da validacao final e evidencias aceitos | Registro do gate F3 aprovado |
| F3.S2.T1 Rodar validacao Cargo completa | `rust-port` | Nenhum arquivo novo | F3.S1.T1 | Sim, com F3.S2.T2 | Cargo fmt/test/run passam localmente | Logs dos comandos F1 |
| F3.S2.T2 Validar isolamento legado | `reviewer` | Nenhum arquivo novo | F3.S1.T1 | Sim, com F3.S2.T1 | Nao ha diff em `ClientLibrary/`, `ConstantsReplacer/`, `src/`, `tests/` | `rtk git diff --name-only -- ClientLibrary ConstantsReplacer src tests` sem saida |
| F3.S3.T1 Gate `e2e-validator` | `e2e-validator` | Nenhum arquivo novo | F3.S2.T1, F3.S2.T2 | Nao | Validador confirma ACs via filesystem, CLI e docs | Relatorio do `e2e-validator` anexado ao handoff |

Validation Gate F3:

- `rtk cargo fmt --manifest-path port_rust/Cargo.toml --check`
- `rtk cargo test --manifest-path port_rust/Cargo.toml --workspace`
- `rtk cargo run --manifest-path port_rust/Cargo.toml -p port_rust -- --status`
- `rtk git diff --name-only -- ClientLibrary ConstantsReplacer src tests`
- Handoff obrigatorio para `e2e-validator` com `spec.md`, `plan.md`, `port_rust/README.md`, `port_rust/docs/inventory.md` e logs Cargo.

## AC Traceability / Matriz AC

| AC | Tasks | Evidencia concreta |
|---|---|---|
| AC-01 | F1.S2.T1, F3.S2.T2 | Evidencia: `port_rust/` existe; diff legado vazio |
| AC-02 | F1.S2.T1, F1.S2.T2, F3.S2.T1 | Evidencia: `cargo test` e CLI `--status` passam |
| AC-03 | F1.S2.T2, F2.S2.T1 | Evidencia: CLI/README declaram `not playable`/nao jogavel |
| AC-04 | F2.S2.T2 | Evidencia: `port_rust/docs/inventory.md` lista todas as entradas obrigatorias |
| AC-05 | F2.S2.T2 | Evidencia: cada entrada tem status permitido |
| AC-06 | F2.S2.T3 | Evidencia: `rg` confirma ausencia de `portada` |
| AC-07 | F3.S2.T2 | Evidencia: `git diff --name-only -- ClientLibrary ConstantsReplacer src tests` sem saida |
| AC-08 | F2.S2.T1 | Evidencia: README registra regra duravel de planejamento individual por fase |
| AC-09 | F1.S1.T1, F2.S1.T1, F3.S1.T1 | Evidencia: cada fase tem gate S1.T1 antes de S2+ |
| AC-10 | F1.S1.T1, F2.S1.T1, F3.S1.T1 | Evidencia: decisoes tecnicas ficam no plano; spec permanece WHAT/WHY |
| AC-11 | F3.S3.T1 | Evidencia: handoff final passa por `e2e-validator`; README bloqueia fases executaveis futuras sem evidencia |

## Test Strategy

- Cargo local minimo: fmt, test, metadata e CLI status em `port_rust/`.
- Validacao documental: `rg` para termos de nao-jogabilidade, gate de fase, status permitidos e entradas obrigatorias.
- Validacao de isolamento: diff vazio nos diretorios legados proibidos.
- Gate final: `e2e-validator` valida ACs por evidencia de filesystem, CLI e documentos.
- Gaters legados CMake/CTest nao precisam rodar nesta entrega porque nenhum arquivo legado ou de build raiz deve mudar.

## Parallelizacao / Subagents

- F1 e sequencial porque a crate depende do workspace.
- Em F2, `port_rust/README.md` e `port_rust/docs/inventory.md` podem ser preparados em paralelo depois de F2.S1.T1; donos nao devem editar o mesmo arquivo.
- Em F3, Cargo validation e isolation diff podem rodar em paralelo depois de F3.S1.T1.
- `e2e-validator` roda somente apos todos os gates locais.

## Risks & Rollback

- Rust/Cargo ausente no ambiente: bloquear F1 e registrar dependencia; rollback nao necessario.
- Inventario incompleto: corrigir somente `port_rust/docs/inventory.md` e repetir Gate F2.
- Status otimista indevido: remover `portada`/status invalido e repetir Gate F2.
- Mudanca acidental em legado: reverter apenas os arquivos tocados fora de `port_rust/`, apos confirmar que nao sao alteracoes de outro agente.
- Skeleton maior que o necessario: rollback por commit/tarefa removendo arquivos adicionados em `port_rust/`.

## Out of Scope

- Portar runtime, UI, rede, assets, audio, editor/admin ou gameplay.
- Escolher engine, Bevy, Tokio, protocolo Rust final, organizacao de crates definitiva ou CI Rust.
- Alterar raiz CMake, workflows existentes, `ClientLibrary/`, `ConstantsReplacer/`, `src/` ou `tests/`.
- Criar comandos de rede, endpoints, schemas, auth, billing ou migracoes.
- Marcar qualquer area como portada.

## Gate Final

Comandos/evidencias obrigatorios antes de concluir a feature:

- `rtk cargo fmt --manifest-path port_rust/Cargo.toml --check`
- `rtk cargo test --manifest-path port_rust/Cargo.toml --workspace`
- `rtk cargo run --manifest-path port_rust/Cargo.toml -p port_rust -- --status`
- `rtk proxy bash -lc 'for p in ".github/" ".gemini/" "ClientLibrary/" "ConstantsReplacer/" "cmake/" "docs/" "src/" "tests/" ".editorconfig" ".gitattributes" ".gitignore" ".gitmodules" "AGENTS.md" "CLAUDE.md" "CMakeLists.txt" "CMakePresets.json" "README.md" "TRANSLATION_SYSTEM_INTEGRATION.md" "stylecop.json" "toolchain-x64.cmake" "toolchain-x86.cmake"; do rg -q "$p" port_rust/docs/inventory.md || exit 1; done'`
- `rtk git diff --name-only -- ClientLibrary ConstantsReplacer src tests` deve ficar vazio.
- `e2e-validator` deve aprovar AC-01 a AC-11 com evidencia anexada.

## Definition of Done

- `port_rust/` existe com workspace Cargo minimo e crate `port_rust`.
- CLI de status e testes Rust passam localmente.
- README declara port incremental, nao jogavel, comandos e gate de planejamento por fase.
- Inventario cobre todas as entradas obrigatorias com status inicial permitido.
- Nenhuma entrada esta marcada como portada.
- Nenhum diretorio legado proibido foi alterado.
- Gate final, incluindo `e2e-validator`, esta aprovado.
