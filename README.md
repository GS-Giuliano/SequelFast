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

### Drivers do MySQL

<!-- ```bash
brew install mysql cmake ninja
brew --prefix mysql
# entre na pasta dos fontes do qt
cd ~/Qt/6.9.1/Src
./configure -debug-and-release -sql-mysql  -- -DMySQL_ROOT=$(brew --prefix mysql)/
cmake --build . --parallel
sudo cmake --install .
```
 -->

```bash
brew install mysql cmake ninja
mkdir build-sqldrivers
cd build-sqldrivers
~/Qt/6.9.1/macos/bin/qt-cmake -G Ninja \
~/Qt/6.9.1/Src/qtbase/src/plugins/sqldrivers \
-DCMAKE_INSTALL_PREFIX=~/Qt/6.9.1/macos \
-DFEATURE_sql_mysql=ON \
-DMySQL_ROOT="$(brew --prefix mysql)" \
-DMySQL_INCLUDE_DIR=$(brew --prefix mysql)/include/mysql \
-DMySQL_LIBRARY=$(brew --prefix mysql)/lib/libmysqlclient.dylib \
-DCMAKE_BUILD_TYPE=Debug

cmake --build . --parallel
cmake --install .
cp plugins/sqldrivers/libqsqlmysql.dylib ~/Qt/6.9.1/macos/plugins/sqldrivers/
```


```bash
brew install mariadb cmake ninja
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
