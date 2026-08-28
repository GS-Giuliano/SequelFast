# ADR 0001: Qt Widgets com janela principal MDI

## Status
Aceito (em uso desde o início do projeto).

## Contexto
O SequelFast precisa oferecer múltiplas ferramentas simultâneas sobre uma ou mais conexões (editor SQL, estrutura de tabela, usuários, estatísticas, backup/restore), com aparência nativa e boa performance em Linux, macOS e Windows.

## Decisão
Usar **Qt Widgets** (não QML/Qt Quick) e organizar a aplicação como uma única `MainWindow` com uma área **MDI** (`QMdiArea`). Cada funcionalidade é implementada como uma `QMainWindow` independente (`Sql`, `Structure`, `Users`, `Statistics`, `Batch`, `Backup`, `Restore`) embrulhada em um `QMdiSubWindow`. O README expõe isso ao usuário como suporte a "tabs, tiled, or cascaded child windows".

## Consequências
- **Positivas**: reaproveitamento do modelo de janelas do Qt Widgets (menus, toolbars, `.ui` do Qt Designer); usuário pode abrir várias queries/tabelas ao mesmo tempo dentro de uma única janela do SO; visual nativo controlável via QSS.
- **Negativas / dívidas observadas**: o estado de "conexão/contexto atual" (`dbMysql`, `actual_host`, `actual_schema`, `actual_table`) é mantido em variáveis globais `extern` em `functions.cpp`, não por sub-janela — múltiplas sub-janelas MDI abertas simultaneamente competem pelo mesmo contexto global em vez de terem contexto isolado. Isso é um risco de bug se o usuário alternar entre sub-janelas de hosts diferentes rapidamente.

## Alternativas consideradas (inferidas)
- Uma janela top-level por funcionalidade (sem MDI): descartada — perderia a organização em abas/tiled/cascade que o README anuncia como diferencial.
- QML/Qt Quick: não há nenhum vestígio de QML no `.pro` (`QT += core gui sql charts` + `widgets`), então nunca foi cogitado ou foi descartado antes deste mapeamento.
