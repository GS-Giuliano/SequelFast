# SequelFast
OpenSource MySQL and MariaDB Client focused on speed

## Technology

* Qt 6.9.1
* C++ 17
* Iconsax GNU GPL 3.0

By Lusaxweb
https://www.freeicons.org/icons/iconsax
https://iconsax.io/?ref=freeicons.org

## Install (Deploy)

### macOS Intel

- Download SequelFast.dmg on "release"
- Open SequelFast.dmg and move SequelFast.app to folder /Applications


### Linux 

Unzip SequelFast.zip and run 

```bash
./install_linux.sh
```

## Developing

### Qt

Download and run Qt installer 

https://www.qt.io/download-open-source

Beware to select right software:

*Qt Design Studio*

- Qt Design Studio 4.7.2 (or newer)

*Extensions*

- Qt PDF
- Qt WebEngine

*Qt*

- Qt 6.9.1
  - Desktop (at least)
  - Sources
  - Additional Libraries
  - Build Tools
    - Qt Installer Framework 4.10
    - CMake 3.30.5
    - Ninja 1.12.1
    - OpenSSL 3.0.16 Toolkit

*Qt Creator*

- Qt Creator 17.0.0


## Requirements

### MySQL/MariaDB Drivers 

Use *mariadb* drivers for better compatibility.

#### Linux

```bash
sudo apt install patchelf mariadb cmake ninja
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

#### macOS

Check for Command Line Build Tools and install MariaDB drivers:

```bash
xcode-select --install
```

Environment setup:

Add to ~/.zshrc:

```bash
export QTDIR=~/Qt/6.9.1/macos
export PATH=$PATH:$QTDIR/bin
```

#### macOS Intel

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

#### macOS Silicon

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



