# Reaproveitamento de `scripts/` para port de arquivos

Status atual: done

Proximo passo concreto: nenhum; piloto `World1` concluido e validado.

## Gate da fase aprovado

- Escopo aceito: instalar dependencias, executar dry-run, converter piloto `--world 1`, validar assets e registrar evidencias.
- Fonte: `src/bin/Data`.
- Destino: `port_rust/assets`.
- Arquivos planejados: `port_rust/assets/data/**`, `port_rust/assets/reports/*.json`, este progress e relatorios e2e.
- Comandos planejados: imports Python, dry-run dos scripts, conversao piloto, `validate_assets.py`, diff legado.

## Task Tree

| ID | Status | Owner | Planned files | Actual files touched | Required evidence | Produced evidence | Blocker |
|---|---|---|---|---|---|---|---|
| F1.S1.T1 | done | local | `progress.md` | `.features/20260518-1551-port-scripts-assets/progress.md` | gate registrado | secao "Gate da fase aprovado" | nenhum |
| F1.S2.T1 | done | local | ambiente Python/sistema | sistema local | imports Python e Crypto++ avaliado | `Pillow`, `PyYAML`, `httpx` instalados via pip user; `libcrypto++8t64` instalado via apt; `ldd` resolve `libcrypto++.so.8` | nenhum |
| F2.S1.T1 | done | local | nenhum | nenhum | dry-run texturas/modelos | dry-run texturas achou 30 fontes; dry-run modelos achou 117 BMDs | nenhum |
| F2.S2.T1 | done | local | `port_rust/assets/data`, `textures_report.json` | `port_rust/assets/data/world_1/**`, `port_rust/assets/reports/textures_report.json` | report texturas | 30 texturas, 2 terrain maps, 8 terrain attrs, 1 config, 1 slots, 1 camera, 1 scene objects; 0 falhas | nenhum |
| F2.S2.T2 | done | local | `port_rust/assets/data`, `models_report.json` | `port_rust/assets/data/object_1/**`, `port_rust/assets/reports/models_report.json` | report modelos | 117 encontrados, 115 convertidos, 2 ignorados, 0 falhas | nenhum |
| F2.S3.T1 | done | local | `validation_report.json` | `port_rust/assets/reports/validation_report.json`, `scripts/client_converter/validate_assets.py` | validacao assets | 163 total, 163 passed, 0 failed; validador ajustado para `terrain_map.json` canonico com `terrain_size` | nenhum |
| F3.S1.T1 | done | local | nenhum | nenhum | diff legado vazio | `rtk git diff --name-only -- ClientLibrary ConstantsReplacer src tests` sem saida | nenhum |
| F3.S2.T1 | done | local | `.features/20260518-1551-port-scripts-assets/progress.md` | `.features/20260518-1551-port-scripts-assets/progress.md` | validacao final registrada | AC-01..AC-07 cobertos por evidencias neste progress | nenhum |

## Arquivos tocados

Novos:
- `.features/20260518-1551-port-scripts-assets/spec.md`
- `.features/20260518-1551-port-scripts-assets/plan.md`
- `.features/20260518-1551-port-scripts-assets/progress.md`
- `port_rust/docs/script-dependencies.md`

Modificados:
- `.gitignore`
- `port_rust/README.md`
- `scripts/client_converter/validate_assets.py`

Entradas nao rastreadas fornecidas pelo usuario e usadas como fonte:
- `scripts/**`

Artefatos locais ignorados:
- `port_rust/assets/**` - gerado pelo piloto World1 e mantido fora do git por enquanto.

Removidos:
- nenhum

## Evidencia

| Comando/artefato | Resultado |
|---|---|
| `rtk sed -n '1,220p' AGENTS.md` | lido |
| `rtk sed -n '1,220p' docs/CODING_RULES.md` | lido |
| `rtk find scripts -maxdepth 3 -type f` | scripts inventariados |
| `rtk python3 -m pip install --user Pillow PyYAML httpx` | instalou dependencias Python |
| `rtk sudo -n apt-get install -y libcrypto++8t64` | instalou runtime Crypto++ |
| `port_rust/docs/script-dependencies.md` | dependencias e comandos de bootstrap documentados |
| `rtk python3 -c "import PIL, yaml, httpx"` | passou |
| `rtk ldd scripts/client_converter/mu_terrain_decrypt` | `libcrypto++.so.8` resolvido |
| `rtk python3 scripts/client_converter/assets_convert.py ... --dry-run` | passou; 30 texturas selecionadas |
| `rtk python3 scripts/client_converter/bmd_converter.py ... --dry-run` | passou; 117 BMDs encontrados |
| `rtk python3 scripts/client_converter/assets_convert.py --legacy-root src/bin/Data --output-root port_rust/assets/data --world 1 --skip-models --workers 4 --force --report port_rust/assets/reports/textures_report.json` | passou; 0 falhas |
| `rtk python3 scripts/client_converter/bmd_converter.py --bmd-root src/bin/Data --output-root port_rust/assets/data --world 1 --workers 4 --force --disable-remaster-variant --report port_rust/assets/reports/models_report.json` | passou; 115 convertidos, 0 falhas |
| `rtk python3 scripts/client_converter/validate_assets.py --assets-root port_rust/assets --report port_rust/assets/reports/validation_report.json` | passou apos ajuste do validador; 163/163 |
| `rtk python3 -m compileall -q scripts/client_converter/validate_assets.py` | passou |
| `rtk git diff --name-only -- ClientLibrary ConstantsReplacer src tests` | passou sem saida |
| `rtk node /home/allanbatista/.codex/skills/feature-workflow/scripts/audit-feature-docs.mjs .features/20260518-1551-port-scripts-assets` | passou |
