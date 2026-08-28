# ADR 0008: Temas via QSS + paleta de cores paralela

## Status
Aceito (em uso).

## Contexto
O SequelFast oferece temas claro e escuro completos, incluindo cores customizáveis por conexão na lista de conexões e no editor SQL.

## Decisão
Os temas são folhas de estilo Qt (`.qss`) completas em `themes/light/` e `themes/dark/`, empacotadas como recursos Qt (`.qrc`) e carregadas em runtime (`:themes/<tema>/<tema>style.qss`). Como QSS não cobre cores "semânticas" de itens de lista (ex.: cor de destaque de uma conexão específica), existe uma **paleta de cores em JSON separada** (`colorThemes`) mantida em paralelo ao QSS para resolver o nome lógico de cor de cada conexão contra o tema ativo.

## Consequências
- **Positivas**: troca de tema em tempo real sem reiniciar a aplicação; cor de conexão é um nome lógico (não RGB fixo), então mudar de tema não exige reconfigurar a cor de cada conexão salva.
- **Negativas**: duas fontes de estilo (QSS + paleta JSON) precisam ser mantidas sincronizadas manualmente — adicionar uma nova cor/elemento visual exige lembrar de atualizar ambos os lugares, sem um mecanismo que garanta consistência entre eles.

## Alternativas consideradas (inferidas)
- `QPalette`/`QStyle` customizado nativo do Qt: daria mais integração com o sistema operacional, mas ofereceria muito menos controle fino sobre a aparência de widgets individuais do que QSS — provavelmente por isso QSS foi escolhido para um app que valoriza uma identidade visual própria e consistente entre plataformas.
