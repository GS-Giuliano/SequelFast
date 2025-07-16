# SequelFast
OpenSource MySQL and MariaDB Client focused on speed

First release version.

What is working now:

## Workspace

- Dark and light themes
- Tabs, tile or cascade child windows

## Connections

- Add, edit, clone and delete
- Connect over pipe, TCP/IP and SSH
- Select color (for list and for SQL Editor)
- Filter

## Users

- Add, edit and delete
- Auto FLUSH PRIVILEGES
- Easy way to check permissions

## Statistics

- Show charset, collation, encryption, size (mb) and tables count
- Show environment variables 

## Schemes

- Open, add (create), delete (drop)
- Statistics
- Filter
- Save filter favorite

## Tables

- Open, add (create), edit (table format), delete (drop)
- Filter
- Save filter favorite

## Table editor

- Add, edit and delete fields
- Log

## SQL editor

- Syntax highlight
- Format
- Save default query for open table
- Font size option (increase and decrease)
- Save actual font size (optionally)
- Timer: auto execute queries for many times at specific intervals 

## Data editor

- Edit directly on table (if query doesn't have JOINs and have Id field)
- Calendar for date and date time fields
- Large text editor for large varchar or text fields
- Masked number for numeric fields
- Copy only selected data (columns or rows) to clipboard as CSV, Insert, Update


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
sudo apt -y install mariadb-server cmake build-essential libxcb-cursor0 libxcb-cursor-dev
patchelf mariadb cmake ninja

mkdir build-sqldrivers
cd build-sqldrivers

~/Qt/6.9.1/gcc_64/bin/qt-cmake -G Ninja ~/Qt/6.9.1/Src/qtbase/src/plugins/sqldrivers -DCMAKE_INSTALL_PREFIX=~/Qt/6.9.1/gcc_64/ 
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



