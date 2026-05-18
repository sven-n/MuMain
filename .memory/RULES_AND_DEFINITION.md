# Planejamento por fase

Antes de começar cada fase, passar por uma etapa de planejamento individual.

# Código do port em port_rust

Todo código do port Rust deve ficar dentro do projeto `port_rust/`. Não criar workspace, crates, lockfile ou toolchain do port na raiz do repositório.

# Prefixo rtk

Todos os comandos de shell devem ser prefixados com `rtk`.

# Fake server minimalista

O fake connect server do `port_rust/` e CLI-only, sem persistencia, e deve ser
estendido por handlers de protocolo explicitos e helpers em `mu_protocol`.
