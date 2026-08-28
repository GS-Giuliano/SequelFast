# ADR 0002: qmake como sistema de build

## Status
Aceito (em uso).

## Contexto
O projeto precisa compilar em Linux, macOS (Intel e Silicon) e Windows, gerar pacotes de instalação distintos por plataforma, e embutir recursos (ícones, temas QSS, fontes, traduções) no binário final.

## Decisão
Usar **qmake** com um único `SequelFast.pro` como fonte de verdade de: fontes/headers/UI, módulos Qt (`core gui sql charts widgets`), recursos (`.qrc`), traduções (`TRANSLATIONS` + `CONFIG += lrelease embed_translations`), flags de compilador e regras específicas por plataforma (`win32{...}`, `mac{...}`). Scripts shell dedicados (`make_installer_linux.sh`, `make_installer_macos_intel.sh`, `make_installer_macos_silicon.sh`) fazem o empacotamento pós-build.

Note-se que os **drivers SQL do Qt (`QMYSQL`) são compilados separadamente via CMake** (fora do `.pro` do app), pois é assim que o próprio Qt distribui o build de plugins `sqldrivers` — não é uma inconsistência do projeto, é uma restrição do próprio Qt.

## Consequências
- **Positivas**: um único arquivo de projeto simples de manter para um código-base de porte pequeno/médio (~9 mil linhas); boa integração com Qt Creator; caminho de build documentado e testado em 3 SOs no README.
- **Negativas**: qmake está em modo de manutenção pelo próprio Qt (o caminho recomendado a longo prazo é CMake); scripts de instalação por plataforma são shell scripts imperativos, não uma definição declarativa única, o que aumenta o custo de manter os 3 pipelines de release sincronizados.

## Alternativas consideradas (inferidas)
- CMake: seria o padrão mais atual e uniformizaria o build do app com o build dos drivers SQL (que já usa CMake), mas exigiria reescrever toda a configuração de recursos/deploy atual.
