# Port Rust

`port_rust` e a area paralela para iniciar o port incremental em Rust do projeto.
Esta etapa cria apenas o scaffold inicial: ainda e nao jogavel / not playable.

## Status

- Inicializado como workspace Cargo isolado.
- Incremental: o cliente legado continua sendo a referencia funcional.
- Sem runtime, UI, rede, assets, audio, editor ou gameplay em Rust nesta fase.
- Nenhum arquivo de implementacao legado deve ser movido para ca sem uma fase planejada.

## Validacao local prevista

```bash
rtk cargo metadata --manifest-path port_rust/Cargo.toml --no-deps --format-version 1
rtk cargo fmt --manifest-path port_rust/Cargo.toml --check
rtk cargo test --manifest-path port_rust/Cargo.toml --workspace
rtk cargo run --manifest-path port_rust/Cargo.toml -p port_rust -- --status
```

O comando `--status` deve imprimir uma mensagem contendo `initialized`,
`incremental` e `not playable`.

## Dependencias dos scripts de port

As dependencias para reaproveitar `scripts/` estao documentadas em
`docs/script-dependencies.md`.

## Gate por fase

Antes de comecar cada fase do port, deve existir uma etapa de planejamento
individual com objetivo, escopo, arquivos planejados, comandos de validacao e
evidencias esperadas. Nenhuma fase de execucao deve comecar sem esse registro.

## Inventario

O inventario inicial esta em `docs/inventory.md`. Ele lista as areas atuais do
projeto, o status inicial e a evidencia exigida antes de qualquer area ser
considerada equivalente no port Rust.
