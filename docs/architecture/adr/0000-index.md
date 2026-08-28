# ADRs (Architecture Decision Records)

Estes registros documentam decisões arquiteturais **identificadas por engenharia reversa do código-fonte** (o projeto não mantinha ADRs formais até este mapeamento). Cada ADR descreve a decisão como ela existe hoje no código, o racional inferido a partir de evidências concretas (nome de arquivos, commits, comportamento), e as consequências observadas — incluindo dívidas técnicas.

| ADR | Título | Status |
|---|---|---|
| [0001](0001-qt-widgets-mdi.md) | Qt Widgets com janela principal MDI | Aceito (em uso) |
| [0002](0002-qmake-buildsystem.md) | qmake como sistema de build | Aceito (em uso) |
| [0003](0003-sqlite-local-persistence.md) | SQLite embutido para persistência local | Aceito (em uso) |
| [0004](0004-ssh-tunnel-via-external-process.md) | Túnel SSH via processo externo (`ssh`/`sshpass`) | Aceito, com dívida de segurança |
| [0005](0005-custom-macro-preprocessor.md) | Sistema de macros como pré-processador textual | Aceito (em uso) |
| [0006](0006-inline-edit-id-no-join.md) | Edição inline restrita a `Id` sem `JOIN`/subquery | Aceito (em uso) |
| [0007](0007-safety-linter-chain.md) | Safety Linter como Chain of Responsibility (escopo limitado) | Aceito, escopo parcial |
| [0008](0008-qss-theming.md) | Temas via QSS + paleta de cores paralela | Aceito (em uso) |
| [0009](0009-shared-favorites-in-mysql.md) | Favoritos compartilhados armazenados no próprio MySQL do usuário | Aceito (em uso) |
| [0010](0010-plaintext-credential-storage.md) | Armazenamento de credenciais em texto puro | **Dívida técnica — recomenda-se revisão** |
