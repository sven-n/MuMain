# Reaproveitamento de `scripts/` para port de arquivos

Status: READY_FOR_PLAN

## Objetivo

Reaproveitar a pasta `./scripts/` para iniciar o port de arquivos legados em `src/bin/Data` para artefatos consumiveis pelo workspace Rust em `port_rust/`, com instalacao das dependencias necessarias e validacao observavel.

Esta fase entrega um piloto controlado de conversao de assets, nao o port completo do runtime.

## Usuarios e Jornadas

- Desenvolvedor do port: executa comandos documentados, converte um recorte inicial de assets para `port_rust/assets`, valida relatorios e usa os resultados como base para proximas fases.
- Revisor tecnico: confirma que os scripts sao reaproveitados sem alterar `src/`, `ClientLibrary/`, `ConstantsReplacer/` ou `tests/`, e que a conversao tem evidencias.
- Usuario em falha: se dependencias, fonte de dados ou validacao faltarem, a fase fica bloqueada com comando e causa registrados.

## Inventario de Produto

| Saida | Rota/pasta | Slug/ID | User-visible label | Tipo | Entradas | Estados | Persona differences |
|---|---|---|---|---|---|---|---|
| Ambiente de conversao | Python/user env + sistema | `script-deps` | Dependencias dos scripts | Dependencias locais | `Pillow`, `PyYAML`, `httpx`, Crypto++ runtime quando disponivel | instalado, ausente, bloqueado | Desenvolvedor instala; revisor confere evidencias |
| Assets convertidos piloto | `port_rust/assets/data` | `world1-assets` | Assets World1 piloto | PNG/GLB/JSON | `src/bin/Data`, filtro `--world 1` | ausente, dry-run, convertido, validado, falhou | Desenvolvedor consome no port; revisor valida contagem e relatorios |
| Relatorios | `port_rust/assets/reports` | `conversion-reports` | Relatorios de conversao | JSON | scripts de conversao/validacao | ausente, gerado, validado | Desenvolvedor usa para depurar; revisor usa para aprovar |

## Criterios de Aceitacao

AC-01. As dependencias Python necessarias para os scripts principais estao instaladas ou bloqueios ficam registrados.

AC-02. O plano inclui explicitamente o reaproveitamento de `scripts/client_converter/*`, `scripts/generate_items/*`, `scripts/generate_monster_spots/*` e scripts de remaster como fases futuras.

AC-03. Um dry-run confirma que `src/bin/Data` pode ser usado como fonte para `port_rust/assets`.

AC-04. O piloto converte pelo menos `World1`/`Object1` para `port_rust/assets/data` quando dependencias estiverem disponiveis.

AC-05. Os assets gerados sao validados por `validate_assets.py` com relatorio em `port_rust/assets/reports`.

AC-06. Nenhum arquivo legado de implementacao em `ClientLibrary/`, `ConstantsReplacer/`, `src/` ou `tests/` e modificado.

AC-07. A fase respeita o gate: planejamento individual antes da execucao.

## Escopo

Inclui instalacao de dependencias, dry-run, conversao piloto de assets e relatorio de validacao.

Fora do escopo: remaster via API externa, conversao completa de todos os assets sem piloto, port do runtime, alteracoes no cliente legado.

## Perguntas em Aberto

Nenhuma pergunta bloqueante. A conversao completa fica para fase futura apos o piloto medir tempo, tamanho e falhas.

## Open Questions

None blocking. Full conversion is deferred until the pilot establishes time, size, and failure profile.
