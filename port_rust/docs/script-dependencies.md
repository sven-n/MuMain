# Dependencias dos scripts de port

## Sistema

- Rust/Cargo para validar o workspace `port_rust`.
- Python 3.12+ para executar os scripts em `scripts/`.
- Crypto++ runtime para `scripts/client_converter/mu_terrain_decrypt`.

Ubuntu:

```bash
rtk sudo -n apt-get install -y libcrypto++8t64
```

## Python

Instalar no ambiente do usuario:

```bash
rtk python3 -m pip install --user Pillow PyYAML httpx
```

Dependencias:

- `Pillow`: conversao/validacao de texturas PNG.
- `PyYAML`: geradores YAML de itens/monstros.
- `httpx`: scripts de remaster via API.

## Verificacao

```bash
rtk python3 -c "import PIL, yaml, httpx"
rtk ldd scripts/client_converter/mu_terrain_decrypt
```

`ldd` deve resolver `libcrypto++.so.8`.

## Piloto World1

Os assets gerados em `port_rust/assets/` sao artefatos locais e ficam ignorados
por git por enquanto.

```bash
rtk python3 scripts/client_converter/assets_convert.py \
  --legacy-root src/bin/Data \
  --output-root port_rust/assets/data \
  --world 1 \
  --skip-models \
  --workers 4 \
  --force \
  --report port_rust/assets/reports/textures_report.json

rtk python3 scripts/client_converter/bmd_converter.py \
  --bmd-root src/bin/Data \
  --output-root port_rust/assets/data \
  --world 1 \
  --workers 4 \
  --force \
  --disable-remaster-variant \
  --report port_rust/assets/reports/models_report.json

rtk python3 scripts/client_converter/validate_assets.py \
  --assets-root port_rust/assets \
  --report port_rust/assets/reports/validation_report.json
```
