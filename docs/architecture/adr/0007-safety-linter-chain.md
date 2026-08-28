# ADR 0007: Safety Linter como Chain of Responsibility (escopo limitado)

## Status
Aceito, escopo parcial — ver ressalvas.

## Contexto
Comandos `DELETE`/`UPDATE` sem cláusula `WHERE` são a causa mais comum de perda de dados acidental em clientes SQL. O projeto introduziu proteção para esse caso especificamente (git log: "undo, redo, safe queries, fixes").

## Decisão
`SafetyLinterHandler` implementa um padrão **Chain of Responsibility** (`QueryHandler::setNext`), interceptando o texto da query antes da execução em `Sql` e bloqueando/alertando quando detecta `DELETE`/`UPDATE` sem `WHERE` (via regex com mascaramento de strings/comentários, não um parser SQL completo).

## Consequências
- **Positivas**: protege contra o erro mais comum e mais destrutivo, com uma arquitetura (chain of responsibility) que permite adicionar novos handlers de validação no futuro sem alterar o `Sql`.
- **Negativas observadas**: hoje a cadeia tem **um único handler** — a extensibilidade do padrão não está sendo aproveitada. O linter está acoplado **apenas ao editor `Sql`**; `Batch` (que nem está implementado) e `Restore`/`Backup` não passam por nenhuma validação equivalente, então um script restaurado ou rodado em lote pode conter um `DELETE`/`UPDATE` sem `WHERE` sem qualquer aviso.

## Alternativas consideradas (inferidas)
- Confirmação genérica "tem certeza?" para qualquer comando destrutivo (sem análise de `WHERE`): mais simples, porém geraria fricção mesmo quando o `WHERE` já limita corretamente o impacto — a decisão atual é mais precisa, ao custo de mais complexidade de parsing.

## Recomendação
Se o objetivo é "proteção contra queries perigosas" de forma consistente, avaliar estender o `SafetyLinterHandler` (ou reaproveitar a mesma cadeia) para os caminhos de execução usados por `Restore` e, quando `Batch` for implementado, por ele também.
