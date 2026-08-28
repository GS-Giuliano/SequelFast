# ADR 0006: Edição inline restrita a `Id` sem `JOIN`/subquery

## Status
Aceito (em uso).

## Contexto
O editor SQL permite editar diretamente uma célula do grid de resultados, gerando um `UPDATE` no banco. É preciso garantir que esse `UPDATE` afete exatamente a linha física que o usuário está vendo.

## Decisão
A edição inline só é habilitada quando a query tem um campo `Id` e **não** tem `JOIN` nem subquery (checagem via regex ad hoc sobre `FROM`/`JOIN`/`WHERE`, não um parser SQL real). Quando habilitada, o `UPDATE` gerado é sempre da forma `UPDATE tabela SET campo=valor WHERE id=<id>`.

## Consequências
- **Positivas**: evita a classe de bug mais perigosa de um editor de dados — atualizar a linha errada ou gerar um `UPDATE` semanticamente inválido quando o resultado vem de múltiplas tabelas (`JOIN`) ou não representa uma linha física 1:1 (subquery).
- **Negativas observadas**: a checagem de "existe campo `id`" é **case-sensitive** (`== "id"`) em `query2TableView`, mas **case-insensitive** (`toLower() == "id"`) em `handleTableCopyUpdate_triggered` — inconsistência entre o critério da edição inline e o do recurso "Copiar como UPDATE", sinal de que os dois recursos evoluíram em momentos diferentes sem alinhamento de critério. O parsing via regex (não um parser SQL real) também aceita falsos positivos/negativos em queries com comentários, aliases incomuns ou SQL mais complexo.

## Alternativas consideradas (inferidas)
- Exigir chave primária real declarada no schema (via `information_schema`) em vez de heurística textual sobre o nome da coluna `id`: mais robusto, mas exigiria uma consulta adicional ao servidor a cada execução de query — provavelmente descartado por custo/latência.
