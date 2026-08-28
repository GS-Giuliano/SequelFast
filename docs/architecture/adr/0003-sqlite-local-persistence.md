# ADR 0003: SQLite embutido para persistência local

## Status
Aceito (em uso).

## Contexto
A aplicação precisa guardar conexões salvas, preferências do usuário (tema, fonte, layout) e log de queries executadas, de forma local à máquina do usuário, sem depender de um servidor externo apenas para isso.

## Decisão
Usar um banco **SQLite embutido** (`preferences.db`, driver `QSQLITE`) como armazenamento local, gerenciado por `functions.cpp` (tabela `conns` para conexões, entre outras para preferências/log). Isso é deliberadamente separado dos bancos MySQL/MariaDB que a ferramenta gerencia — o SequelFast nunca usa um dos bancos "alvo" do usuário para guardar sua própria configuração.

## Consequências
- **Positivas**: zero dependência externa para rodar a aplicação pela primeira vez; portátil (um arquivo); não exige que o usuário já tenha um MySQL acessível só para abrir o app.
- **Negativas observadas**: dados sensíveis (senhas de conexão e SSH) são gravados nessa mesma base **sem criptografia** — ver [ADR 0010](0010-plaintext-credential-storage.md). Além disso, parte do estado (`QJsonArray` de conexões em memória) é sincronizado manualmente com a tabela SQLite via `openPreferences()`, criando uma fonte de verdade duplicada que pode divergir se não for recarregada em todos os pontos corretos.

## Alternativas consideradas (inferidas)
- `QSettings` (INI/registro do SO): mais idiomático para preferências simples do Qt, mas não foi escolhido — provavelmente porque a lista de conexões e o log de queries se beneficiam de um schema relacional (filtros, favoritos compartilhados, etc.) mais do que `QSettings` ofereceria.
