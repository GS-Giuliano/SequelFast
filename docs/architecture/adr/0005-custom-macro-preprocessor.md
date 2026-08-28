# ADR 0005: Sistema de macros como pré-processador textual

## Status
Aceito (em uso).

## Contexto
O editor SQL precisa suportar queries parametrizadas que podem ser salvas como favoritas e reexecutadas pedindo valores ao usuário (`~campo`, `~campo@tipo`, `~campo@combo~tabela...`).

## Decisão
As macros são resolvidas por um **pré-processador textual** próprio: uma regex identifica os padrões `~campo[@tipo[~...]]` no texto bruto da query, `MacroInputDialog`/`MacroFormatDialog` coletam e formatam os valores, e a substituição final é feita via `QString::replace` **antes** de qualquer `QSqlQuery::exec()` — não são usados bind parameters nativos do Qt (`QSqlQuery::bindValue`).

## Consequências
- **Positivas**: a query salva como favorita é 100% autodescritiva em texto puro (tipo do campo, tabela/chave/ordem de uma combo) — algo que bind parameters posicionais/nomeados não expressam sozinhos sem uma estrutura de metadados separada.
- **Negativas (segurança)**: a única sanitização contra SQL injection é escapar `'` → `''` para strings; o tipo `number` não tem validação de que o valor é de fato numérico além do que o `QSpinBox` da UI já impõe. Não há whitelisting central de tipo — a segurança depende inteiramente do widget de entrada usado no diálogo, não de uma camada de validação central.

## Alternativas consideradas (inferidas)
- Bind parameters nativos do Qt: mais seguro contra injection e mais idiomático, mas exigiria um formato de armazenamento paralelo (JSON/metadado) para guardar tipo/combo de cada parâmetro junto da query salva, em vez de tudo estar embutido no próprio texto SQL.
