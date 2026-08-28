# ADR 0004: Túnel SSH via processo externo (`ssh`/`sshpass`)

## Status
Aceito, com dívida de segurança identificada.

## Contexto
O SequelFast precisa suportar conexões MySQL/MariaDB através de um túnel SSH para hosts remotos que não expõem a porta do banco diretamente.

## Decisão
`TunnelSqlManager` implementa o túnel **invocando os binários externos `ssh`/`sshpass` via `QProcess`** (encaminhamento de porta local, `-L 127.0.0.1:<portaLocal>:...`), em vez de linkar uma biblioteca SSH embutida (libssh2/libssh). A porta local é escolhida por um contador global incremental (`sshPort`), nunca reciclado. Após subir o túnel, o app espera um tempo fixo (5s, `waitWithProcessing`) antes de tentar abrir a conexão MySQL — não há polling ativo da porta.

## Consequências
- **Positivas**: reaproveita a instalação de `ssh` já presente no SO e toda a configuração de autenticação do usuário (agentes SSH, `~/.ssh/config`, chaves já confiáveis); implementação simples comparada a integrar uma lib SSH em C++.
- **Negativas (dívida de segurança)**: quando a autenticação usa senha, ela é passada como **argumento de linha de comando ao `sshpass`**, o que a expõe a qualquer processo capaz de listar `ps` no sistema local. Depender de binários externos também significa que a aplicação falha silenciosamente em ambientes sem `ssh`/`sshpass` instalados, sem checagem explícita de pré-requisito.
- Espera fixa de 5s em vez de polling ativo da porta é uma fonte potencial de falha intermitente em máquinas/redes mais lentas (o túnel pode não estar pronto ainda).
- O keepalive da sessão ("manter conexão viva") **não vive dentro de `TunnelSqlManager`** — é um `QTimer` de 15s em `MainWindow` que roda `SELECT 1`; o túnel confia apenas em `ServerAliveInterval`/`ServerAliveCountMax` do próprio `ssh`. Isso significa que a responsabilidade de "manter vivo" está espalhada entre duas camadas diferentes.

## Alternativas consideradas (inferidas)
- Biblioteca SSH embutida (libssh2): eliminaria a dependência de binário externo e a exposição de senha via argv (poderia usar variável de ambiente/pipe), mas aumentaria a complexidade de build multiplataforma — provavelmente o motivo de não ter sido escolhida.
