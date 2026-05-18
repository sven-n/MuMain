# Port completo Rust + Bevy do cliente MU

Status: READY_FOR_PLAN

## Objetivo

Entregar um port completo do cliente atual de MU Online para Rust + Bevy, mantendo o produto jogável e equivalente para jogadores, operadores e administradores, com runtime Bevy, rede nativa em Rust, assets convertidos antes da execução, UI, áudio, MU Helper, GameShop e ferramenta de editor/admin equivalente.

O resultado esperado é um cliente Windows x64 de release que substitui o cliente atual para uso por jogadores e uma experiência Linux adequada para desenvolvimento e CI. Cada migração funcional deve ter testes que comprovem equivalência ou comportamento intencionalmente novo.

## Usuários e Jornadas

- Jogador: abre o cliente Windows x64, configura vídeo/áudio/conta/servidor quando aplicável, autentica, seleciona personagem, entra no mundo, joga com combate, inventário, NPCs, chat, mapas, áudio, efeitos, UI, GameShop e MU Helper sem depender do cliente legado em tempo de execução.
- Jogador em falha de conexão: recebe estado visível de conexão, erro recuperável quando possível e retorno seguro para login/seleção sem travar nem corromper sessão local.
- Jogador em assets ausentes ou inválidos: recebe falha clara antes de entrar no mundo; o cliente não tenta converter assets em tempo de execução.
- Operador de QA: executa cenários comparáveis entre cliente legado e port Rust + Bevy, observa diferenças por tela, logs, testes e builds, e consegue bloquear regressões antes de release.
- Desenvolvedor: roda build e testes em Linux dev/CI, valida migrações por testes automatizados e usa assets previamente convertidos como entrada do runtime.
- Administrador/editor: abre a superfície de editor/admin equivalente, inspeciona e altera dados suportados do jogo conforme permissões, valida visualmente mudanças e recebe estados de erro/sem permissão/sem dados.

## Inventário de Produto

| Superfície | Rota/Página/Entrada | Slug/ID | Rótulo visível | Tipo de saída | Filtros/Controles | Datasets/Permissões | Estados vazio/carregando/erro/bloqueado | Diferenças por persona |
|---|---|---|---|---|---|---|---|---|
| Inicialização do cliente | Executável Windows x64 release | `client-launch` | Cliente MU | Janela do jogo | Resolução, tela cheia/janela, V-Sync/FPS quando exposto | Assets convertidos, config local | Loading inicial, erro de assets/config, erro de plataforma | Jogador usa release; dev/QA usa builds de validação |
| Login | Tela inicial | `login` | Login | Formulário/tela de autenticação | Servidor, conta, senha, lembrar usuário quando suportado | Config de servidor, credenciais do usuário, permissão de conexão | Loading de autenticação, credencial inválida, servidor indisponível, bloqueio de conta | Jogador autentica; QA testa sucesso/falha |
| Seleção/criação de personagem | Após login | `character-select` | Seleção de Personagem | Lista e ações de personagem | Selecionar, criar, excluir quando suportado | Conta, personagens, classes liberadas | Lista vazia, loading, erro de lista, ação negada | Jogador gerencia personagens; QA valida regras |
| Mundo jogável | Sessão em mapa | `world-play` | Mundo | Cena 3D/2D interativa | Movimento, câmera, interação, atalhos | Mapas, terrenos, objetos, monstros, personagens, servidor | Loading de mapa, erro de mapa, desconexão, morte/respawn | Jogador joga; QA compara gameplay |
| Combate e habilidades | Mundo jogável | `combat-skills` | Habilidades | Ações, efeitos, dano e feedback visual | Teclas/atalhos, alvo, skill selecionada | Skills, atributos, itens, estado de servidor | Skill indisponível, mana/recursos insuficientes, alvo inválido | Jogador executa combate; QA valida fórmulas e feedback |
| Inventário/equipamento/baú | Janelas de jogo | `inventory-vault-equipment` | Inventário / Equipamento / Baú | Grades de itens e detalhes | Mover, equipar, usar, dividir, vender quando suportado | Itens, inventário, baú, regras de item | Grade vazia, item inválido, operação negada, erro de sincronização | Jogador gerencia itens; QA valida serialização e regras |
| NPCs, comércio e diálogos | Interação no mundo | `npc-dialog-trade` | NPC / Loja | Diálogo, loja, ações | Comprar, vender, reparar, confirmar/cancelar | NPCs, lojas, moedas, inventário | NPC sem dados, saldo insuficiente, operação negada | Jogador interage; QA valida fluxos |
| Chat e comandos | HUD | `chat-commands` | Chat | Lista de mensagens e input | Canal, destinatário, comandos como FPS/V-Sync quando suportados | Mensagens, permissões de comando, config | Sem mensagens, envio falhou, comando negado | Jogador conversa; QA/admin valida comandos permitidos |
| HUD e barras de estado | Mundo jogável | `hud-status` | HUD | Painéis, barras e indicadores | Atalhos, minimapa quando suportado | Vida, mana, XP, buffs, alvo, party | Dados indisponíveis, estado desconectado | Jogador monitora sessão; QA compara layout/valores |
| Opções | Menu de opções | `options` | Opções | Formulários e toggles | Vídeo, áudio, controles, desempenho | Config local persistida | Config ausente, falha ao salvar, valores inválidos | Jogador configura; QA valida persistência |
| Áudio | Mundo e menus | `audio` | Áudio | Música, efeitos e controles | Volume música, volume efeitos, mute quando suportado | Assets de áudio convertidos, config local | Asset ausente, dispositivo indisponível, mute | Jogador ouve feedback; QA valida presença/sincronização |
| MU Helper | Janela/atalho em jogo | `mu-helper` | MU Helper | Painel de automação assistida | Ativar/desativar, regras de poção, skills, coleta quando suportado | Estado do personagem, inventário, regras do servidor | Sem recursos, regra inválida, bloqueio do servidor | Jogador configura helper; QA valida limites |
| GameShop | Janela/atalho em jogo | `game-shop` | GameShop | Catálogo e compra | Categoria, item, quantidade, confirmar/cancelar | Catálogo, moedas/créditos, permissões de compra | Catálogo vazio, saldo insuficiente, compra falhou, bloqueado | Jogador compra; operador/QA valida catálogo e transações |
| Editor/admin equivalente | Modo/admin autorizado | `editor-admin` | Editor / Admin | Ferramentas visuais e painéis administrativos | Inspecionar, editar, salvar, desfazer/cancelar quando suportado | Dados editáveis, permissões admin, assets/mapas | Sem permissão, sem dados, erro ao salvar, modo indisponível | Admin edita; dev/QA inspeciona; jogador não acessa |
| Pipeline de assets convertidos | Artefatos antes do runtime | `asset-conversion-output` | Assets Convertidos | Pacote/manifesto validável | Seleção de fonte/ambiente quando suportada fora do runtime | Assets legados, assets convertidos, manifesto | Asset faltando, conversão inválida, versão incompatível | Dev/CI produz e valida; jogador consome somente resultado |
| Logs e diagnósticos | Arquivos/saída de execução | `diagnostics` | Diagnóstico | Logs, códigos de erro, evidências de teste | Nível de log quando suportado | Eventos de cliente, rede, assets, render, áudio | Sem permissão de escrita, falha crítica registrada | QA/dev analisam; jogador recebe mensagens resumidas |

## Requisitos Não Funcionais

- Plataforma: entrega obrigatória para Windows x64 release; Linux deve suportar desenvolvimento e CI.
- Runtime: o cliente jogável deve usar Bevy em tempo de execução.
- Rede: comunicação de cliente deve ser nativa em Rust e cobrir login, sessão, mundo, inventário, combate, chat, GameShop e demais fluxos migrados.
- Assets: conversão de assets deve acontecer antes do runtime; o cliente em execução consome assets já convertidos e validáveis.
- Testes: toda migração funcional, de dados, rede, asset, UI, áudio, editor/admin ou gameplay deve ter teste automatizado ou evidência de validação definida; ausência de teste bloqueia conclusão da migração.
- Compatibilidade de produto: o port deve preservar o comportamento jogável atual, incluindo recursos Season 6 existentes no cliente, exceto diferenças documentadas e aprovadas.
- Performance: a experiência jogável deve ser fluida em Windows x64 release, sem travamentos perceptíveis em fluxos comuns de login, carregamento de mapa, combate, inventário, GameShop e MU Helper.
- Segurança: credenciais, sessão, permissões admin/editor e compras do GameShop não podem ser expostas em logs de texto claro nem permitir ações sem autorização do servidor.
- Acessibilidade e UX: telas devem apresentar estados de carregamento, erro e indisponibilidade compreensíveis; texto deve permanecer legível nas resoluções suportadas.
- Observabilidade: falhas de rede, assets, render, áudio, dados e permissões devem gerar sinal verificável por UI, logs ou testes.
- Governança de mudança: não realizar limpeza retroativa ampla do cliente legado fora do necessário para validar o port; mudanças devem ser focadas e rastreáveis.

## Critérios de Aceitação

AC-01. Um artefato Windows x64 release do cliente Rust + Bevy inicia em ambiente limpo com assets convertidos e mostra a tela inicial sem depender do cliente C++/C# legado em tempo de execução.
AC-02. O cliente executa em Linux dev/CI o conjunto de testes exigido para o port e produz resultado de sucesso/falha observável no pipeline ou comando de teste documentado posteriormente no plano.
AC-03. O runtime recusa assets ausentes, incompatíveis ou não convertidos com mensagem visível e log diagnóstico; nenhum fluxo jogável inicia conversão de asset durante a execução.
AC-04. Login, erro de login, desconexão e retorno seguro para tela apropriada são observáveis por UI e logs.
AC-05. Seleção, criação e exclusão de personagem quando suportadas pelo servidor funcionam com estados de lista vazia, loading, erro e ação negada.
AC-06. Entrada no mundo carrega mapa, personagem, câmera, HUD, movimento, objetos, NPCs e entidades remotas suficientes para uma sessão jogável.
AC-07. Combate, habilidades, dano, XP, buffs/debuffs e feedback visual/sonoro são validados por testes ou comparação documentada contra comportamento esperado.
AC-08. Inventário, equipamento e baú preservam regras de item, movimentação, uso, equipar/desequipar, serialização e erros de sincronização.
AC-09. NPCs, lojas e diálogos permitem compra, venda, reparo ou ações equivalentes hoje suportadas, com confirmação, cancelamento e falhas visíveis.
AC-10. Chat e comandos suportados exibem envio, recebimento, erro de envio e negação de comando quando aplicável.
AC-11. Opções de vídeo, áudio, controles e desempenho suportadas são editáveis, persistidas e restauradas em nova execução.
AC-12. Áudio de menus, mundo, combate, interface e eventos importantes toca com controles de volume/mute e falhas diagnosticáveis.
AC-13. MU Helper oferece a configuração e execução das regras suportadas, respeita limites do servidor e mostra estados ativo, inativo, recurso insuficiente e configuração inválida.
AC-14. GameShop lista catálogo, detalhes, saldo quando disponível, confirmação de compra, sucesso, saldo insuficiente, catálogo vazio e falha transacional sem duplicar compra.
AC-15. Editor/admin equivalente exige permissão, mostra estado bloqueado para usuário não autorizado, permite inspecionar e alterar dados suportados e evidencia sucesso/falha de salvamento.
AC-16. Cada superfície listada no Inventário de Produto tem pelo menos um teste automatizado, teste de integração, teste visual, log verificável ou evidência de validação definida no plano antes de ser considerada migrada.
AC-17. Diferenças intencionais em relação ao cliente atual são registradas com justificativa de produto e evidência de aprovação antes de release.
AC-18. Fluxos de erro críticos de rede, assets, áudio, render, permissão e dados produzem mensagem de usuário segura e log técnico sem expor credenciais ou segredos.
AC-19. A entrega final passa por validação `e2e-validator`; fases intermediárias que liberarem superfícies jogáveis, GameShop, rede, assets ou editor/admin também devem ser bloqueadas por validação equivalente.
AC-20. A documentação de uso final descreve como jogador, QA/desenvolvedor e admin/editor usam as superfícies migradas, sem depender de leitura de código.

## Escopo

Inclui:

- Cliente jogável completo em Rust + Bevy.
- Rede nativa Rust para todos os fluxos de cliente migrados.
- Assets convertidos antes do runtime e manifesto/validação observável.
- UI jogável completa, HUD, menus, janelas e estados de erro.
- Áudio, controles, opções, chat, comandos suportados e diagnósticos.
- MU Helper e GameShop funcionais.
- Editor/admin equivalente ao uso atual necessário para inspeção e edição suportada.
- Testes obrigatórios por migração e validação e2e final.
- Suporte de produto a Windows x64 release e Linux dev/CI.

Fora do escopo:

- Manter x86 como alvo obrigatório do port.
- Converter assets durante execução do cliente.
- Reescrever ou redesenhar servidores OpenMU ou protocolo de servidor fora do necessário para interoperabilidade do cliente.
- Criar novos modos de jogo, novas classes, novos mapas ou novas economias não existentes no produto atual.
- Expandir GameShop, MU Helper ou editor/admin além da equivalência funcional exigida.
- Limpezas retroativas amplas no cliente legado que não sejam necessárias para comparação, testes ou validação.

## Limites

- O único arquivo alterado por este trabalho de especificação é `.features/20260518-0000-rust-bevy-port/spec.md`.
- O plano futuro deve respeitar `AGENTS.md` e `docs/CODING_RULES.md`, especialmente separação entre renderização, lógica de jogo, UI, dados, rede e input.
- Implementações futuras não podem marcar migração como concluída sem teste ou evidência de validação.
- Decisões de arquitetura, crates, organização de arquivos, endpoints, schemas, ferramentas de conversão e ordem de implementação pertencem ao `plan.md`, não a este spec.
- Qualquer alteração de comportamento jogável, economia, permissões, segurança ou protocolo percebida pelo usuário deve ser aprovada como decisão de produto antes de execução.
- A entrega deve ser faseável; nenhuma fase com superfície jogável deve ser considerada pronta sem validação observável.

## Open Questions / Perguntas em Aberto

Nenhuma pergunta bloqueante para produto. Assumem-se como decisões já tomadas:

- O port é completo, não incremental como produto parcial final.
- Bevy é o runtime do cliente jogável.
- A rede do cliente é nativa em Rust.
- Assets são convertidos antes do runtime.
- Alvos obrigatórios são Windows x64 release e Linux dev/CI.
- UI, áudio, MU Helper, GameShop e editor/admin equivalente fazem parte do completo.
- Toda migração requer teste ou evidência de validação.

## Definição de Pronto

- Todos os critérios de aceitação estão satisfeitos por sinais observáveis.
- Todas as superfícies do Inventário de Produto foram migradas ou tiveram diferença aprovada e registrada.
- O artefato Windows x64 release é jogável do login ao mundo com UI, áudio, rede, MU Helper, GameShop e editor/admin equivalente conforme permissões.
- Linux dev/CI executa validação automatizada exigida para o port.
- Nenhuma conversão de asset ocorre em runtime.
- A validação final `e2e-validator` foi executada e aprovada.
- Documentação de uso para jogador, QA/desenvolvedor e admin/editor está atualizada.
