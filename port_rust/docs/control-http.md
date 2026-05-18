# Controle remoto do cliente por HTTP

`mu_client` pode expor um servidor HTTP local de controle quando iniciado com
`--control-http`.

Ele serve para testar e inspecionar o estado do cliente sem mexer na rede do
jogo.

## Como iniciar

```bash
rtk cargo run --manifest-path port_rust/Cargo.toml -p mu_client -- \
  --headless \
  --control-http 127.0.0.1:0
```

O binario imprime o estado inicial e a URL efetiva do servidor.

## Consultar estado

`GET /state` e `GET /` retornam JSON com o estado atual:

- `state`
- `last_command`
- `command_count`

Exemplo:

```json
{"state":"boot","last_command":null,"command_count":0}
```

## Enviar comandos

`POST /command?name=...` aceita estes comandos:

- `boot`
- `asset-check-failed`
- `ready-for-login`
- `exit`
- `ping`

`exit` atualiza o estado para `exit` e encerra o servidor. Os demais apenas
atualizam o snapshot.

O comando tambem pode vir no corpo da requisicao como `name=...` ou como texto
puro.

Exemplos:

```bash
curl http://127.0.0.1:12345/state
curl -X POST 'http://127.0.0.1:12345/command?name=ready-for-login'
curl -X POST 'http://127.0.0.1:12345/command' -d 'name=ping'
curl -X POST 'http://127.0.0.1:12345/command' -d 'exit'
```

## Respostas

- `200` para estado consultado e comandos aceitos.
- `400` para comando ausente ou desconhecido.
- `404` para rota inexistente.

`GET /__shutdown` existe apenas para encerramento interno do servidor.
