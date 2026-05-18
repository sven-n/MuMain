# Inventario inicial do port Rust

Status permitidos: `a portar`, `referencia`, `suporte de validacao`,
`fora do runtime`, `pendente de decisao`.

Nenhuma entrada desta fase pode ser marcada como concluida sem evidencia de
validacao associada.

| Entrada | Status inicial | Evidencia exigida | Observacao |
|---|---|---|---|
| `.github/` | `suporte de validacao` | CI Rust especifico ou decisao documentada | Workflows atuais ficam como suporte e referencia de validacao. |
| `.gemini/` | `fora do runtime` | Evidencia de uso ou decisao de descarte | Configuracao assistiva, sem impacto direto no runtime. |
| `ClientLibrary/` | `referencia` | Testes/provas de equivalencia de rede | Base C# para contratos cliente-servidor existentes. |
| `ConstantsReplacer/` | `referencia` | Decisao sobre substituicao ou descarte | Ferramenta auxiliar atual, sem migracao nesta fase. |
| `cmake/` | `suporte de validacao` | Decisao de coexistencia/remocao futura | Continua servindo ao build legado. |
| `docs/` | `referencia` | Docs Rust atualizados quando houver superficie | Documentacao atual orienta comportamento esperado. |
| `src/` | `referencia` | Inventario granular e validacao por area | Cliente C++ legado permanece como referencia funcional. |
| `tests/` | `suporte de validacao` | Equivalente Rust ou fixture comparativa | Testes existentes orientam equivalencia futura. |
| `.editorconfig` | `suporte de validacao` | Politica Rust equivalente ou reutilizacao | Pode orientar formatacao do port. |
| `.gitattributes` | `suporte de validacao` | Decisao de atributos para artefatos Rust | Regras de atributos continuam na raiz. |
| `.gitignore` | `suporte de validacao` | Regras para `target/` se necessario | Avaliar ignore especifico quando Cargo gerar artefatos. |
| `.gitmodules` | `referencia` | Decisao sobre submodulos/assets futuros | Submodulos atuais seguem como referencia. |
| `AGENTS.md` | `referencia` | Regras aplicadas ao port | Instrucoes obrigatorias para agentes e humanos. |
| `CLAUDE.md` | `referencia` | Confirmacao de apontamento para `AGENTS.md` | Mantem ponte para instrucoes principais. |
| `CMakeLists.txt` | `referencia` | Decisao de coexistencia com Cargo | Build raiz legado nao muda nesta fase. |
| `CMakePresets.json` | `referencia` | Decisao de equivalencia de presets | Presets atuais continuam para CMake. |
| `README.md` | `referencia` | README Rust alinhado ao status real | README raiz segue como entrada do projeto atual. |
| `TRANSLATION_SYSTEM_INTEGRATION.md` | `referencia` | Plano futuro de i18n Rust | Referencia para internacionalizacao futura. |
| `stylecop.json` | `fora do runtime` | Decisao de nao aplicabilidade ou equivalente | Regra .NET, sem uso direto no runtime Rust. |
| `toolchain-x64.cmake` | `referencia` | Decisao de toolchain Rust Windows x64 futura | Referencia para ambiente Windows x64. |
| `toolchain-x86.cmake` | `referencia` | Decisao sobre suporte x86 futuro | Referencia para suporte x86 futuro. |

