# Documentação Técnica — SequelFast

Mapeamento completo do código-fonte do SequelFast (cliente desktop MySQL/MariaDB em Qt 6.9.1 + C++17), gerado a partir da leitura integral de todos os arquivos em `src/`. Cobre arquitetura, referência de API por classe, fluxogramas dos principais processos e ADRs (Architecture Decision Records) com o racional inferido de cada decisão relevante.

## Por onde começar

1. **[Visão Geral da Arquitetura](architecture/overview.md)** — mapa de componentes, camadas, stack tecnológica, fluxo ponta a ponta (conectar → executar query) e lista consolidada de riscos/dívidas técnicas.
2. **[ADRs](architecture/adr/0000-index.md)** — por que o projeto é como é: MDI, qmake, SQLite local, túnel SSH via processo externo, macros como pré-processador, edição inline restrita, safety linter, temas QSS, favoritos compartilhados, e o alerta de segurança sobre credenciais em texto puro.
3. **Referência de API por subsistema** (em [`api/`](api/)):

   | Subsistema | Classes cobertas | Documento |
   |---|---|---|
   | Shell da aplicação | `MainWindow`, `main.cpp`, `functions.cpp` | [core-shell.md](api/core-shell.md) |
   | Conexões e túnel SSH | `Connection`, `TunnelSqlManager` | [connections.md](api/connections.md) |
   | Editor SQL e macros | `Sql`, `SqlHighlighter`, `TextEditCompleter`, `MacroInputDialog`, `MacroFormatDialog` | [sql-editor.md](api/sql-editor.md) |
   | Estrutura de tabelas | `Structure`, `TwoCheckboxDelegate`, `TwoCheckboxListModel`, delegates regex | [schema-data.md](api/schema-data.md) |
   | Administração e operações | `Users`, `Statistics`, `Batch`, `Backup`, `Restore`, `SafetyLinterHandler` | [admin-ops.md](api/admin-ops.md) |

Cada documento de API contém: diagrama de classes (Mermaid), tabela de métodos/signals/slots públicos por classe, fluxogramas/sequence diagrams dos principais fluxos de uso, e observações de implementação (comportamentos não óbvios, limitações, código morto/incompleto).

## Achados que merecem atenção prioritária

Levantados durante o mapeamento (detalhados nos ADRs e nos documentos de API correspondentes):

- 🔴 **Credenciais (DB e SSH) armazenadas em texto puro** no SQLite local, e senha SSH exposta via argumento de processo — [ADR 0010](architecture/adr/0010-plaintext-credential-storage.md).
- 🟡 **`Batch` é apenas um esqueleto de UI**, sem nenhuma lógica de execução em lote implementada — [admin-ops.md](api/admin-ops.md).
- 🟡 **`SafetyLinterHandler` protege apenas contra `DELETE`/`UPDATE` sem `WHERE`**, e só está integrado ao editor SQL, não a `Restore`/`Batch` — [ADR 0007](architecture/adr/0007-safety-linter-chain.md).
- 🟡 **Dois caminhos de conexão MySQL divergentes** com timeouts e suporte a SSH diferentes (`Connection` vs. `functions.cpp::connectMySQL`) — [connections.md](api/connections.md).
- 🟡 **`TwoCheckboxListModel` é código morto** (nunca instanciado) — [schema-data.md](api/schema-data.md).

## Escopo e limitações deste mapeamento

- Cobre 100% dos arquivos `.h`/`.cpp` em `src/` (18 headers, 17 implementações, ~9.100 linhas).
- Os arquivos `.ui` (Qt Designer) foram consultados apenas para entender nomes de widgets referenciados no código, não documentados campo a campo.
- Scripts de build/empacotamento (`*.sh`, `SequelFast.pro`) foram usados como evidência para as ADRs de build/deploy, mas não têm documento de API próprio (não são "APIs" no sentido de classes/interfaces de código).
- Onde o comportamento do código não estava claro ou parecia incompleto/morto, isso foi sinalizado explicitamente nos documentos em vez de ser inferido — buscar por "não implementado", "código morto" ou "lacuna" nos arquivos de `api/`.
