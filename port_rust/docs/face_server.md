# Fake server de connect-server

`mu_fake_server` e um servidor local de teste para a tela de login do cliente.
Ele responde com uma lista fixa de servidores e guarda tudo em memoria.

Use quando voce quiser testar selecao de servidor sem depender do servidor real.

## Como iniciar

```bash
rtk cargo run --manifest-path port_rust/Cargo.toml -p mu_fake_server -- \
  --bind 127.0.0.1:44405 \
  --server 0:100
```

`--bind` define o endereco local do fake server. Use `127.0.0.1:0` se quiser
que o sistema escolha a porta.

`--server CONNECT_INDEX:PERCENT` adiciona uma entrada na lista. O argumento
pode ser repetido quantas vezes for preciso.

Exemplo com tres servidores:

```bash
rtk cargo run --manifest-path port_rust/Cargo.toml -p mu_fake_server -- \
  --bind 127.0.0.1:0 \
  --server 0:100 \
  --server 1:75 \
  --server 2:10
```

## O que ele faz

- imprime `mu_fake_server listening on ...` com o endereco efetivo;
- envia a lista de servidores assim que o cliente conecta;
- responde novamente quando o cliente pede a lista de servidores de
  connect-server;
- encerra quando o processo e finalizado.

## Dicas

- Use indices e percentuais que batam com o que voce quer validar no cliente.
- O `percent` e um `u8`; na pratica, valores entre `0` e `100` sao os mais
  uteis.
- Se quiser automatizar testes, combine esse binario com o cliente em modo
  headless ou com o harness local que estiver validando o login.
