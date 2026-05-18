# Reaproveitamento de `scripts/` para port de arquivos

Status: READY_FOR_EXEC

## Interfaces / Contracts

- Fonte legada: `src/bin/Data`.
- Destino piloto: `port_rust/assets/data`.
- Relatorios: `port_rust/assets/reports/*.json`.
- Ambiente Python: `python3` com `Pillow`, `PyYAML` e `httpx`.
- Conversao piloto:
  - `scripts/client_converter/assets_convert.py --legacy-root src/bin/Data --output-root port_rust/assets/data --world 1 --skip-models`
  - `scripts/client_converter/bmd_converter.py --bmd-root src/bin/Data --output-root port_rust/assets/data --world 1 --disable-remaster-variant`
  - `scripts/client_converter/validate_assets.py --assets-root port_rust/assets --report port_rust/assets/reports/validation_report.json`

## Inventario Tecnico

| Slugs | Queries | Components | Filters/url state | Dataset/permissao | Saida | Renderer/test target | Retailer/industry compatibility | Observacao |
|---|---|---|---|---|---|---|---|---|
| `script-deps` | imports Python, `ldd` | `Pillow`, `PyYAML`, `httpx`, `libcrypto++8t64` | N/A | ambiente local | dependencias instaladas | `python3 -c`, `ldd` | N/A | Requerido para scripts. |
| `world1-textures-terrain` | filesystem scan | `scripts/client_converter/assets_convert.py` | `--world 1`, `--skip-models` | `src/bin/Data` | PNG/JSON + report | `validate_assets.py` | N/A | Requer Pillow; piloto controlado. |
| `world1-models` | filesystem scan | `scripts/client_converter/bmd_converter.py` | `--world 1`, `--disable-remaster-variant` | `src/bin/Data` | GLB + report | `validate_assets.py` | N/A | Dry-run confirmou 117 BMDs. |
| `asset-validation` | filesystem scan | `scripts/client_converter/validate_assets.py` | `--assets-root port_rust/assets` | `port_rust/assets` | validation JSON | exit code + report | N/A | Valida PNG/GLB/JSON. |
| `full-orchestration` | filesystem scan | `scripts/client_converter/convert_all_assets.sh` | env `LEGACY_ROOT`, `OUTPUT_ROOT`, filtros futuros | `src/bin/Data` | orquestracao | reports | N/A | Futuro; defaults antigos exigem env/args. |
| `items-yaml` | config parse | `scripts/generate_items/main.py` | paths CLI | fontes item futuras | YAML | `--check` | N/A | Futuro; nao executar no piloto. |
| `monster-yaml` | config parse | `scripts/generate_monster_spots/main.py` | paths CLI | fontes monstro futuras | YAML | command exit | N/A | Futuro; nao executar no piloto. |
| `remaster-assets` | GLB/textures scan | `scripts/remaster*.py` | input paths/API args | GLBs convertidos | GLB/texturas remaster | script validation | N/A | Futuro; requer API key. |

## Gate Obrigatorio Da Fase

Antes de executar conversao real, registrar neste `progress.md`: escopo, comandos, arquivos planejados, fonte, destino e evidencias esperadas.

## Phases / Tasks

### F1. Planejamento e dependencias

| ID | Owner | Planned files | Done when | Required evidence |
|---|---|---|---|---|
| F1.S1.T1 | local | `.features/20260518-1551-port-scripts-assets/progress.md` | Gate da fase registrado | Secao gate no progress |
| F1.S2.T1 | local | ambiente Python/sistema | Dependencias Python instaladas; Crypto++ avaliado | imports Python e `ldd`/apt quando aplicavel |

### F2. Dry-run e conversao piloto

| ID | Owner | Planned files | Done when | Required evidence |
|---|---|---|---|---|
| F2.S1.T1 | local | `port_rust/assets/reports` | Dry-run texturas/modelos executado | logs dos comandos dry-run |
| F2.S2.T1 | local | `port_rust/assets/data`, `port_rust/assets/reports/textures_report.json` | Texturas/JSON World1 convertidos | report JSON |
| F2.S2.T2 | local | `port_rust/assets/data`, `port_rust/assets/reports/models_report.json` | BMD World1 convertido para GLB | report JSON |
| F2.S3.T1 | local | `port_rust/assets/reports/validation_report.json` | Assets piloto validados | report JSON + exit code 0 |

### F3. Fechamento

| ID | Owner | Planned files | Done when | Required evidence |
|---|---|---|---|---|
| F3.S1.T1 | local | nenhum | Legado sem diff | `git diff --name-only -- ClientLibrary ConstantsReplacer src tests` sem saida |
| F3.S2.T1 | local | `.features/20260518-1551-port-scripts-assets/progress.md` | AC-01..AC-07 verificados por comandos CLI/filesystem | validacao final registrada no progress |

## AC Traceability

| AC | Tasks | Validation evidence |
|---|---|---|
| AC-01 | F1.S2.T1 | evidencia: imports Python, `ldd` Crypto++ e logs de instalacao |
| AC-02 | F1.S1.T1 | evidencia: plano/inventario tecnico com scripts mapeados |
| AC-03 | F2.S1.T1 | evidencia: logs de dry-run para texturas/modelos |
| AC-04 | F2.S2.T1, F2.S2.T2 | evidencia: arquivos em `port_rust/assets/data` e relatorios JSON |
| AC-05 | F2.S3.T1 | evidencia: `validation_report.json` com 0 falhas |
| AC-06 | F3.S1.T1 | evidencia: `git diff --name-only -- ClientLibrary ConstantsReplacer src tests` sem saida |
| AC-07 | F1.S1.T1, F3.S2.T1 | evidencia: gate registrado e validacao final no progress |

## Validation Gate

- `rtk python3 -c "import PIL, yaml, httpx"`
- `rtk python3 scripts/client_converter/assets_convert.py ... --dry-run`
- `rtk python3 scripts/client_converter/bmd_converter.py ... --dry-run --disable-remaster-variant`
- conversao piloto sem `--dry-run`
- `rtk python3 scripts/client_converter/validate_assets.py --assets-root port_rust/assets --report port_rust/assets/reports/validation_report.json`
- `rtk git diff --name-only -- ClientLibrary ConstantsReplacer src tests`
- e2e-validator handoff: filesystem/CLI-only feature; final validation evidence is recorded in `progress.md`, with a dedicated validator handoff planned only when a UI/runtime surface exists.

## Riscos

- Conversao completa pode gerar muitos arquivos; por isso esta fase usa piloto `--world 1`.
- Crypto++ pode depender de pacote de sistema; se indisponivel, terrain Season16+ fica para fase futura.
- Remaster via API depende de credenciais e fica fora desta fase.
