# SequelFast
OpenSource MySQL and MariaDB Client focused on speed



## Technology

Qt 6.9.1
C++ 17
Iconsax
GNU GPL 3.0
By Lusaxweb
https://www.freeicons.org/icons/iconsax
https://iconsax.io/?ref=freeicons.org

### Qt

Baixe e execute o instalador no link: https://www.qt.io/download-open-source
Atenção para a seleção de instalação dos fontes (Sources).

## Instalação no Linux (Ubuntu / Zorin)

### Drivers do MySQL

```bash
sudo apt install libmysqlclient-dev cmake ninja

# entre na pasta dos fontes do qt
cd ~/Qt/6.9.1/Src/qtbase
./configure.sh -sql-mysql
cmake --build . --parallel
sudo cmake --install .
```

## Instalação no macOS

## Pré-requisitos

Certifique-se de ter instalado "Command Line Tools" do Xcode, e requisitos para o módulo MySQL:

```bash
xcode-select --install
brew install mariadb cmake ninja
```

## Ambiente

Configure corretamente o ambiente:

Adicione as linhas no arquivo ~/.zshrc

```bash
export QTDIR=~/Qt/6.9.1/macos
export PATH=$PATH:$QTDIR/bin
```

### Drivers do MySQL - macOS Intel

Para maior compatibilidade, instale os drivers do MariaDB aon invés do MySQL.

```bash
mkdir build-sqldrivers
cd build-sqldrivers

~/Qt/6.9.1/macos/bin/qt-cmake -G Ninja \
~/Qt/6.9.1/Src/qtbase/src/plugins/sqldrivers \
-DCMAKE_INSTALL_PREFIX=~/Qt/6.9.1/macos \
-DFEATURE_sql_mysql=ON \
-DMySQL_ROOT="$(brew --prefix mariadb)" \
-DMySQL_INCLUDE_DIR=$(brew --prefix mariadb)/include/mysql \
-DMySQL_LIBRARY=$(brew --prefix mariadb)/lib/libmysqlclient.dylib \
-DCMAKE_BUILD_TYPE=Debug

cmake --build . --parallel
cmake --install .
cp plugins/sqldrivers/libqsqlmysql.dylib ~/Qt/6.9.1/macos/plugins/sqldrivers/
```

### Drivers do MySQL - macOS Silicon

Para maior compatibilidade, instale os drivers do MariaDB aon invés do MySQL.

```bash
brew install mariadb cmake ninja
mkdir build-sqldrivers
cd build-sqldrivers

~/Qt/6.9.1/macos/bin/qt-cmake -G Ninja \
~/Qt/6.9.1/Src/qtbase/src/plugins/sqldrivers \
-DCMAKE_INSTALL_PREFIX=~/Qt/6.9.1/macos \
-DCMAKE_OSX_ARCHITECTURES=arm64 \
-DCMAKE_BUILD_TYPE=Debug \
-DCMAKE_CXX_STANDARD=17 \
-DCMAKE_CXX_EXTENSIONS=OFF \
-DFEATURE_sql_mysql=ON \
-DMySQL_ROOT="$(brew --prefix mariadb)" \
-DMySQL_INCLUDE_DIR="$(brew --prefix mariadb)/include/mysql" \
-DMySQL_LIBRARY="$(brew --prefix mariadb)/lib/libmysqlclient.dylib"

cmake --build . --parallel
cmake --install .
cp plugins/sqldrivers/libqsqlmysql.dylib ~/Qt/6.9.1/macos/plugins/sqldrivers/
```



## Instalação do sistema no Linux

Se você quiser que o ícone apareça no menu de aplicativos, Dock (como no KDE, GNOME, etc.):

Crie um arquivo .desktop, por exemplo:
```
[Desktop Entry]
Version=1.0
Name=SequelFast
Comment=Ferramenta SQL
Exec=/caminho/para/SequelFast
Icon=/caminho/absoluto/para/icons/icon_128.png
Terminal=false
Type=Application
Categories=Development;Database;
```

Salve como:

~/.local/share/applications/sequelfast.desktop (usuário)
ou /usr/share/applications/sequelfast.desktop (global)

Depois, rode:

```bash
chmod +x ~/.local/share/applications/sequelfast.desktop
update-desktop-database ~/.local/share/applications/
```

📦 Instalação recomendada dos ícones no sistema (opcional)

Você pode instalar ícones com múltiplas resoluções nas pastas padrão:

```bash
sudo cp icons/icon_128.png /usr/share/icons/hicolor/128x128/apps/sequelfast.png
sudo gtk-update-icon-cache /usr/share/icons/hicolor
```



