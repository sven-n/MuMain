# Port Rust incremental em `port_rust`

Status: READY_FOR_PLAN

## Objetivo

Iniciar o port em Rust do projeto atual em uma area paralela `./port_rust`, sem mover nem reescrever o cliente legado nesta etapa.

O resultado de produto desta feature e deixar um ponto de partida claro para migracao incremental: um workspace/skeleton Rust identificavel, um inventario inicial do projeto existente e uma regra operacional de que cada fase futura so comeca depois de um planejamento individual aprovado.

Esta etapa nao entrega um cliente jogavel em Rust. Ela prepara o trabalho para que o port avance por fases verificaveis, sem a proxima pessoa precisar adivinhar escopo, limites ou o estado inicial.

## Usuarios e Jornadas

- Desenvolvedor do port: entra no repositorio, abre `./port_rust`, entende que ali vive o novo port Rust, valida que o skeleton inicial esta saudavel e consulta o inventario para escolher a proxima fase.
- Revisor tecnico: compara o inventario com as areas existentes do projeto, confirma que o cliente legado nao foi movido nem alterado indevidamente e bloqueia a entrega se o skeleton ou inventario estiverem incompletos.
- Responsavel de produto/manutencao: antes de cada fase, revisa o planejamento individual daquela fase, valida o resultado esperado e impede inicio de trabalho sem decisao explicita.
- Usuario em estado de falha: quando `port_rust` esta ausente, invalido ou sem inventario, a validacao da feature falha com sinal observavel em comando, teste, log ou checklist de revisao.

## Inventario de Produto

| Superficie/saida | Rota/pagina | Slug/ID | Rotulo visivel | Tipo de saida | Filtros/controles | Datasets/permissoes | Estados vazio/carregando/erro/bloqueado | Diferencas por persona |
|---|---|---|---|---|---|---|---|---|
| Workspace do port Rust | `./port_rust` | `port-rust-workspace` | Port Rust | Area local de trabalho/skeleton | Nao aplicavel; validacao definida no plano | Permissao de leitura do repositorio e escrita em `port_rust` | Ausente, skeleton incompleto, validacao falhou, bloqueado por falta de planejamento | Desenvolvedor usa para iniciar o port; revisor valida existencia e isolamento |
| Inventario inicial do port | Dentro de `./port_rust` | `legacy-port-inventory` | Inventario do Port | Documento ou artefato humano-legivel | Agrupamento por area do projeto e status de migracao | Deve cobrir `.github/`, `.gemini/`, `ClientLibrary/`, `ConstantsReplacer/`, `cmake/`, `docs/`, `src/`, `tests/`, `.editorconfig`, `.gitattributes`, `.gitignore`, `.gitmodules`, `AGENTS.md`, `CLAUDE.md`, `CMakeLists.txt`, `CMakePresets.json`, `README.md`, `TRANSLATION_SYSTEM_INTEGRATION.md`, `stylecop.json`, `toolchain-x64.cmake`, `toolchain-x86.cmake` | Inventario ausente, area obrigatoria ausente, area sem status, erro de leitura | Desenvolvedor usa para planejar fases; revisor usa para bloquear lacunas; produto usa para entender cobertura |
| Gate de planejamento por fase | `.features/{YYYYMMDD-HHMM}-{feature-name}/` | `phase-planning-gate` | Planejamento da Fase | Registro de decisao/checklist de fase | Fase, objetivo, escopo, validacao e evidencias esperadas | Specs, planos e progresso da fase correspondente | Bloqueado quando a fase nao tem planejamento individual antes do inicio | Produto aprova intencao; desenvolvedor executa; revisor valida que a fase nao comecou sem plano |

## Requisitos Nao Funcionais

- Isolamento: o port inicial deve coexistir com o cliente atual; `ClientLibrary/`, `ConstantsReplacer/`, `src/` e `tests/` nao podem ser movidos, apagados ou reescritos por esta feature.
- Observabilidade: skeleton, inventario e gate de fase precisam ter sinal de sucesso/falha verificavel por teste, comando, checklist de revisao, log ou status de arquivo.
- Manutenibilidade: o inventario deve registrar status e lacunas de forma legivel para orientar fases futuras.
- Seguranca: nenhum segredo, credencial, token ou dado sensivel deve ser introduzido no skeleton, inventario ou logs de validacao.
- Compatibilidade operacional: a build/documentacao atual do projeto CMake e .NET permanece como referencia enquanto o port Rust ainda nao substitui o cliente.
- Acessibilidade documental: documentos desta feature devem ser legiveis em PT-BR e deixar claro quando o port ainda nao e jogavel.
- Governanca: antes de iniciar qualquer fase futura, deve existir planejamento individual daquela fase.

## Criterios de Aceitacao

AC-01. `./port_rust` existe como a area unica e paralela do port Rust, sem mover ou substituir as areas legadas existentes.

AC-02. `./port_rust` contem um skeleton inicial identificavel como port Rust do projeto, com um sinal de validacao de saude definido no plano e observavel durante a execucao da feature.

AC-03. O skeleton comunica claramente que esta etapa inicia o port incremental e nao entrega cliente jogavel em Rust.

AC-04. O inventario inicial do port existe dentro de `./port_rust` e lista, no minimo, todas as entradas obrigatorias declaradas no Inventario de Produto.

AC-05. Cada entrada obrigatoria do inventario possui um status inicial observavel, como `a portar`, `referencia`, `suporte de validacao`, `fora do runtime` ou `pendente de decisao`.

AC-06. Nenhuma entrada do inventario pode ser marcada como portada sem evidencia de validacao associada.

AC-07. A entrega inicial nao altera arquivos de implementacao do cliente legado em `ClientLibrary/`, `ConstantsReplacer/`, `src/` ou `tests/`.

AC-08. A regra "antes de comecar cada fase, passar por uma etapa de planejamento individual" esta registrada como regra duravel do projeto.

AC-09. Qualquer fase futura relacionada ao port Rust fica bloqueada se nao houver planejamento individual antes do inicio da execucao.

AC-10. O planejamento futuro deve manter decisoes de arquitetura, organizacao interna, ferramentas, comandos, endpoints, schemas e ordem de implementacao fora deste spec e dentro do `plan.md`.

AC-11. Fases futuras que liberarem superficie executavel, jogavel, fluxo de rede, assets, UI ou editor/admin devem definir evidencia de validacao; o handoff final do port completo deve ser bloqueado por `e2e-validator`.

## Escopo

Inclui:

- Criar o ponto de partida do port em `./port_rust`.
- Registrar o skeleton inicial do port Rust como nao jogavel.
- Criar inventario inicial do projeto existente para orientar migracao incremental.
- Registrar e aplicar a regra de planejamento individual antes de cada fase.
- Definir sinais observaveis de validacao para skeleton, inventario e gate de fase.

Fora do escopo:

- Portar o cliente completo nesta etapa.
- Entregar runtime, UI, rede, assets, audio, editor/admin ou gameplay em Rust.
- Definir engine, crates, arquitetura interna, comandos, endpoints, schemas ou ordem de implementacao.
- Reescrever, mover ou limpar retroativamente codigo legado.
- Alterar comportamento jogavel, protocolo, economia, permissoes ou assets do cliente atual.

## Limites

- Esta especificacao segue `AGENTS.md`, `docs/CODING_RULES.md` e o checklist de `feature-workflow`.
- O trabalho desta etapa documental altera somente documentos de feature e memoria de regra quando necessario.
- O port deve viver em `./port_rust`; outras areas do repositorio continuam como fonte de referencia e validacao.
- O inventario nao pode usar termos vagos como "todo o projeto" sem listar entradas obrigatorias.
- Decisoes tecnicas pertencem ao plano, nao a este spec.
- A regra de planejamento por fase e obrigatoria para todo trabalho futuro deste port.

## Premissas Nao Bloqueantes

- `./port_rust` e uma area paralela, nao uma mudanca de raiz do repositorio.
- O primeiro trabalho de implementacao cria apenas workspace/skeleton Rust e inventario para permitir port incremental.
- O cliente legado continua sendo a referencia funcional ate fases futuras definirem equivalencia e validacao.
- O spec anterior de port completo Rust + Bevy nao altera esta solicitacao; este spec trata do inicio incremental em `./port_rust`.

## Open Questions / Perguntas em Aberto

Nenhuma pergunta de produto bloqueia o planejamento.

Incertezas tecnicas a decidir no `plan.md`:

- Formato exato do workspace/skeleton Rust.
- Comando ou teste usado como validacao de saude do skeleton.
- Formato exato e manutencao do inventario inicial.
- Se a etapa inicial precisara ou nao de CI alem de validacao local.

## Definicao de Pronto

- `./port_rust` existe e passa no sinal de validacao definido no plano.
- O inventario inicial cobre todas as entradas obrigatorias deste spec.
- A regra de planejamento individual por fase esta registrada.
- Nao ha alteracao de implementacao legado fora do escopo aprovado.
- O status do port inicial e claro: iniciado, incremental e ainda nao jogavel.
